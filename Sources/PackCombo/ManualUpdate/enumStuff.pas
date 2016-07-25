unit enumStuff;

//  Delphi 4,5 enumeration implementation of several win32 APIs

interface

uses windows;

type TACardinal   = array [0..maxInt shr 2-1] of cardinal;
     TPACardinal  = ^TACardinal;
     TDACardinal  = array of cardinal;

type TOperatingSystem = (osUnknown, osWin311, osWin95, osWin95osr2, osWin98, osWinNT3, osWinNT4, osWinNT4SP4, osWinNT5);
function GetOperatingSystem : TOperatingSystem;
// Tests which system is running...

type TExeType = (etUnknown, etDos, etWin16, etConsole, etWin32);
function GetExeType(exefile: string) : TExeType;
// Determines the type of the executable.

type TWindowList         = array of record
                                      pid       : cardinal;
                                      tid       : cardinal;
                                      window    : cardinal;
                                      parent    : cardinal;
                                      owner     : cardinal;
                                      visible   : boolean;
                                      enabled   : boolean;
                                      inTaskbar : boolean;
                                      rect      : TRect;
                                      title     : string;
                                      className : string;
                                    end;
     TThreadList         = array of record
                                      pid       : cardinal;
                                      tid       : cardinal;
                                      windows   : TWindowList;
                                    end;
     TProcessList        = array of record
                                      pid       : cardinal;
                                      name      : string;
                                      exeType   : TExeType;
                                      threads   : TThreadList;
                                    end;
     TDesktopList        = array of record
                                      name      : string;
                                      windows   : TWindowList;
                                    end;
     TWindowStationList  = array of record
                                      name      : string;
                                      desktops  : TDesktopList;
                                    end;
     TCachedPasswordList = array of record
                                      resource  : string;
                                      password  : string;
                                      resType   : byte;
                                    end;

function GetProcessList (threadsToo: boolean = false; windowsToo: boolean = false) : TProcessList;
// Lists the currently running processes.

function GetThreadList (pid: cardinal = 0; windowsToo: boolean = false) : TThreadList;
// Lists the currently running threads of the process "pid" or of all processes.

function GetWindowList (pid: cardinal = 0; tid: cardinal = 0; onlyThoseInTaskbar: boolean = false) : TWindowList;
// Lists the currently existing top level windows of the process "pid" or of all
// processes and of the thread "tid" or of all threads.

function GetChildWindowList (window: cardinal) : TWindowList;
// Lists the the child windows of "window".

function GetWindowStationList (desktopsToo: boolean = false; windowsToo: boolean = false) : TWindowStationList;
// Lists the currently existing window stations.  (works only under winNT)

function GetDesktopList (ws: cardinal = 0; windowsToo: boolean = false) : TDesktopList;
// Lists the currently existing desktops.  (works only under winNT)

function GetDesktopWindowList (dt: cardinal = 0) : TWindowList;
// Lists the currently existing windows of the current desktop.  (works only under winNT)

function GetCachedPasswords : TCachedPasswordList;
// Lists all cached passwords of the currently logged in user.  (works only under win95/98)

implementation

uses ShellAPI, sysUtils;

type TPThreadList        = ^TThreadList;
     TPProcessList       = ^TProcessList;

var OS      : TOperatingSystem;
    OSReady : boolean = false;
function GetOperatingSystem : TOperatingSystem;
var os1 : TOSVersionInfo;
begin
  if not OSReady then begin
    OSReady:=true;
    os1.dwOSVersionInfoSize:=sizeOf(os1); GetVersionEx(os1);
    case os1.dwPlatformID of
      VER_PLATFORM_WIN32s        : OS:=osWin311;
      VER_PLATFORM_WIN32_WINDOWS : if (os1.dwMajorVersion=4) and (os1.dwMinorVersion=0) then begin
                                     if os1.dwBuildNumber>1000 then OS:=osWin95osr2 else OS:=osWin95;
                                   end else if (os1.dwMajorVersion=4) and (os1.dwMinorVersion=10) then
                                     OS:=osWin98
                                   else OS:=osUnknown;
      VER_PLATFORM_WIN32_NT      : case os1.dwMajorVersion of
                                     0..3 : OS:=osWinNT3;
                                     4    : if string(os1.szCSDVersion)='Service Pack 4' then OS:=osWinNT4SP4
                                            else                                              OS:=osWinNT4;
                                     5    : OS:=osWinNT5;
                                   end;
      else                         OS:=osUnknown;
    end;
  end;
  result:=OS;
end;

const MAX_MODULE_NAME32 = 255;
type
  TProcessEntry32 = record
                      dwSize              : DWORD;
                      cntUsage            : DWORD;
                      th32ProcessID       : DWORD;	  // this process
                      th32DefaultHeapID   : DWORD;
                      th32ModuleID        : DWORD;	  // associated exe
                      cntThreads          : DWORD;
                      th32ParentProcessID : DWORD;        // this process's parent process
                      pcPriClassBase      : integer;      // Base priority of process's threads
                      dwFlags             : DWORD;
                      szExeFile           : array [0..MAX_PATH-1] of char;    // Path
                    end;
  TThreadEntry32  = record
                      dwSize              : DWORD;
                      cntUsage            : DWORD;
                      th32ThreadID        : DWORD;	  // this thread
                      th32OwnerProcessID  : DWORD;        // Process this thread is associated with
                      tpBasePri           : integer;
                      tpDeltaPri          : integer;
                      dwFlags             : DWORD;
                    end;
  TModuleEntry32  = record
                      dwSize              : DWORD;
                      th32ModuleID        : DWORD;        // This module
                      th32ProcessID       : DWORD;        // owning process
                      GlblcntUsage        : DWORD;        // Global usage count on the module
                      ProccntUsage        : DWORD;        // Module usage count in th32ProcessID's context
                      modBaseAddr         : pointer;      // Base address of module in th32ProcessID's context
                      modBaseSize         : DWORD;        // Size in bytes of module starting at modBaseAddr
                      hModule             : HMODULE;      // The hModule of this module in th32ProcessID's context
                      szModule            : array [0..MAX_MODULE_NAME32] of char;
                      szExePath           : array [0..MAX_PATH-1] of char;
                    end;
const TH32CS_SnapProcess = 2;
      TH32CS_SnapThread  = 4;
      TH32CS_SnapModule  = 8;
var   //PsApiHandle    : cardinal = 0;
      CreateToolhelp32Snapshot :
        function (dwFlags,th32ProcessID: cardinal) : cardinal; stdcall
        = nil;
      Process32First :
        function (hSnapshot: cardinal; var lppe: TProcessEntry32) : bool; stdcall
        = nil;
      Process32Next :
        function (hSnapshot: cardinal; var lppe: TProcessEntry32) : bool; stdcall
        = nil;
      Thread32First :
        function (hSnapshot: cardinal; var lpte: TThreadEntry32) : bool; stdcall
        = nil;
      Thread32Next :
        function (hSnapshot: cardinal; var lpte: TThreadEntry32) : bool; stdcall
        = nil;
      Module32First :
        function (hSnapshot: cardinal; var lpme: TModuleEntry32) : bool; stdcall
        = nil;
      Module32Next :
        function (hSnapshot: cardinal; var lpme: TModuleEntry32) : bool; stdcall
        = nil;
      EnumProcesses :
        function (idProcess: TPACardinal; cb: cardinal; var cbNeeded: cardinal) : bool; stdcall
        = nil;
      EnumProcessModules :
        function (hProcess: cardinal; var hModule: cardinal; cb: cardinal; var cbNeeded: cardinal) : bool; stdcall
        = nil;
      GetModuleFileNameEx :
        function (hProcess,hModule: cardinal; fileName: PChar; nSize: cardinal) : cardinal; stdcall
        = nil;

function TestToolhelpFunctions : boolean;
var c1 : cardinal;
begin
  c1:=GetModuleHandle('kernel32');
  @CreateToolhelp32Snapshot:=GetProcAddress(c1,'CreateToolhelp32Snapshot');
  @Process32First          :=GetProcAddress(c1,'Process32First'          );
  @Process32Next           :=GetProcAddress(c1,'Process32Next'           );
  @Thread32First           :=GetProcAddress(c1,'Thread32First'           );
  @Thread32Next            :=GetProcAddress(c1,'Thread32Next'            );
  @Module32First           :=GetProcAddress(c1,'Module32First'           );
  @Module32Next            :=GetProcAddress(c1,'Module32Next'            );
  result:=(@CreateToolhelp32Snapshot<>nil) and
          (@Process32First<>nil) and (@Process32Next<>nil) and
          (@Thread32First<>nil) and (@Thread32Next<>nil) and
          (@Module32First<>nil) and (@Module32Next<>nil);
end;

{function TestPsApi : boolean;
begin
  if PsApiHandle=0 then begin
    PsApiHandle:=LoadLibrary('psapi');
    result:=PsApiHandle<>0;
    if result then begin
      @EnumProcesses      :=GetProcAddress(PsApiHandle,'EnumProcesses'       );
      @EnumProcessModules :=GetProcAddress(PsApiHandle,'EnumProcessModules'  );
      @GetModuleFileNameEx:=GetProcAddress(PsApiHandle,'GetModuleFileNameExA');
      result:=(@EnumProcesses<>nil) and (@EnumProcessModules<>nil) and (@GetModuleFileNameEx<>nil);
    end;
  end else result:=true;
end;}

function GetExeType(exefile: string) : TExeType;
var c1  : cardinal;
    sfi : TSHFileInfo;
    s1  : string;
begin
  c1:=SHGetFileInfo(pchar(exefile),0,sfi,SizeOf(sfi),SHGFI_EXETYPE);
  s1:=chr(c1 and $ff)+chr((c1 and $ff00) shr 8);
  if       s1='MZ'                                                                             then result:=etDos
  else if  s1='NE'                                                                             then result:=etWin16
  else if (s1='PE') and (hiWord(c1)=0)                                                         then result:=etConsole
  else if (s1='PE') and (hiWord(c1)>0)                                                         then result:=etWin32
  else if CompareText(AnsiUpperCase(ExtractFileName(exefile)),AnsiUpperCase('winoa386.mod'))=0 then result:=etDos
  else                                                                                              result:=etUnknown;
end;

function NT4_EnumProcessesAndThreads(pl: TPProcessList; tl: TPThreadList; windowsToo: boolean) : boolean;
type TPerfDataBlock           = packed record
                                  signature              : array [0..3] of wchar;
                                  littleEndian           : cardinal;
                                  version                : cardinal;
                                  revision               : cardinal;
                                  totalByteLength        : cardinal;
                                  headerLength           : cardinal;
                                  numObjectTypes         : cardinal;
                                  defaultObject          : cardinal;
                                  systemTime             : TSystemTime;
                                  perfTime               : comp;
                                  perfFreq               : comp;
                                  perfTime100nSec        : comp;
                                  systemNameLength       : cardinal;
                                  systemnameOffset       : cardinal;
                                end;
     TPPerfDataBlock          = ^TPerfDataBlock;

     TPerfObjectType          = packed record
                                  totalByteLength        : cardinal;
                                  definitionLength       : cardinal;
                                  headerLength           : cardinal;
                                  objectNameTitleIndex   : cardinal;
                                  objectNameTitle        : PWideChar;
                                  objectHelpTitleIndex   : cardinal;
                                  objectHelpTitle        : PWideChar;
                                  detailLevel            : cardinal;
                                  numCounters            : cardinal;
                                  defaultCounter         : integer;
                                  numInstances           : integer;
                                  codePage               : cardinal;
                                  perfTime               : comp;
                                  perfFreq               : comp;
                                end;
     TPPerfObjectType         = ^TPerfObjectType;

     TPerfCounterDefinition   = packed record
                                  byteLength             : cardinal;
                                  counterNameTitleIndex  : cardinal;
                                  counterNameTitle       : PWideChar;
                                  counterHelpTitleIndex  : cardinal;
                                  counterHelpTitle       : PWideChar;
                                  defaultScale           : integer;
                                  defaultLevel           : cardinal;
                                  counterType            : cardinal;
                                  counterSize            : cardinal;
                                  counterOffset          : cardinal;
                                end;
     TPPerfCounterDefinition  = ^TPerfCounterDefinition;

     TPerfInstanceDefinition  = packed record
                                  byteLength             : cardinal;
                                  parentObjectTitleIndex : cardinal;
                                  parentObjectInstance   : cardinal;
                                  uniqueID               : integer;
                                  nameOffset             : cardinal;
                                  nameLength             : cardinal;
                                end;
     TPPerfInstanceDefinition = ^TPerfInstanceDefinition;
     TAPChar                  = array [0..maxInt div 4-1] of pchar;
     TPCardinal               = ^cardinal;
var  i1,i2,i3,i4              : integer;
     b1,b2,b3,b4              : boolean;
     bt,bp                    : boolean;
     c1                       : cardinal;
     pCard                    : TPCardinal;
     perfDataBlock            : TPPerfDataBlock;
     perfObjectType           : TPPerfObjectType;
     perfCounterDef           : TPPerfCounterDefinition;
     perfInstanceDef          : TPPerfInstanceDefinition;
begin
  result:=false;
  bt:=tl=nil; if not bt then tl^:=nil; bp:=pl=nil; if not bp then pl^:=nil;
  if bt and bp then exit;
  perfDataBlock:=nil;
  try
    i1:=$10000;
    repeat
      ReallocMem(perfDataBlock,i1); i2:=i1;
      i4:=RegQueryValueEx(HKEY_PERFORMANCE_DATA,'230 232',nil,@i3,pointer(perfDataBlock),@i2);
      if i4=ERROR_MORE_DATA then i1:=i1*2;
    until (i4<>ERROR_MORE_DATA);
    if i4<>ERROR_SUCCESS then exit;
    perfObjectType:=pointer(cardinal(perfDataBlock)+perfDataBlock^.headerLength);
    for i1:=0 to integer(perfDataBlock^.numObjectTypes)-1 do begin
      b1:=             (pl<>nil) and (perfObjectType^.objectNameTitleIndex=230);   // 230 -> "Process"
      b2:=(not b1) and (tl<>nil) and (perfObjectType^.objectNameTitleIndex=232);   // 232 -> "Thread"
      if b1 or b2 then begin
        perfCounterDef:=pointer(cardinal(perfObjectType)+perfObjectType^.headerLength);
        for i2:=0 to perfObjectType^.numCounters-1 do begin
          b3:=              perfCounterDef^.counterNameTitleIndex=784;    // 784 -> "ID Process"
          b4:=(not b3) and (perfCounterDef^.counterNameTitleIndex=804);   // 804 -> "ID Thread"
          if b3 or b4 then begin
            perfInstanceDef:=pointer(cardinal(perfObjectType)+perfObjectType^.definitionLength);
            if b1 then SetLength(pl^,perfObjectType^.numInstances-1)
            else       SetLength(tl^,perfObjectType^.numInstances-1);
            for i3:=0 to perfObjectType^.numInstances-2 do begin
              c1:=TPCardinal(cardinal(perfInstanceDef)+perfInstanceDef^.byteLength+perfCounterDef^.counterOffset)^;
              if b1 then begin
                pl^[i3].pid:=c1;
                if c1<>0 then begin
                  pl^[i3].name:=wideString(PWideChar(cardinal(perfInstanceDef)+perfInstanceDef.nameOffset));
                  if pl^[i3].name<>'System' then pl^[i3].name:=pl^[i3].name+'.exe';
                end else pl^[i3].name:='[System Process]';
              end else if b3 then tl^[i3].pid:=c1 else tl^[i3].tid:=c1;
              pCard:=pointer(cardinal(perfInstanceDef)+perfInstanceDef^.byteLength);
              perfInstanceDef:=pointer(cardinal(pCard)+pCard^);
            end;
          end;
          inc(perfCounterDef);
        end;
        bt:=bt or b2; bp:=bp or b1; if bt and bp then break;
      end;
      perfObjectType:=pointer(cardinal(perfObjectType)+perfObjectType^.totalByteLength);
    end;
    result:=((pl<>nil) and (pl^<>nil)) or ((tl<>nil) and (tl^<>nil));
    if (tl<>nil) and windowsToo then
      if windowsToo then
        for i1:=0 to high(tl^) do
          if (tl^[i1].pid<>0) then
            tl^[i1].windows:=GetWindowList(tl^[i1].pid,tl^[i1].tid);
  finally FreeMem(perfDataBlock) end;
end;

function GetProcessList(threadsToo: boolean = false; windowsToo: boolean = false) : TProcessList;
var c1          : cardinal;
    i1,i2,i3,i4 : integer;
    tl          : TThreadList;
    pe          : TProcessEntry32;
begin
  result:=nil;
  if GetOperatingSystem in [osWin95,osWin95osr2,osWin98,osWinNT5] then begin
    if not TestToolhelpFunctions then begin
      MessageBox(0,'Toolhelp functions not available.','Error...',0);
      exit;
    end;
    c1:=CreateToolHelp32Snapshot(TH32CS_SnapProcess,0);
    try
      i1:=0;
      pe.dwSize:=sizeOf(pe);
      if Process32First(c1,pe) then
        repeat
          SetLength(result,i1+1);
          result[i1].pid:=pe.th32ProcessID; result[i1].name:=pe.szExeFile;
          result[i1].exeType:=GetExeType(result[i1].name);
          inc(i1);
        until not Process32Next(c1,pe);
    finally CloseHandle(c1) end;
    if threadsToo then tl:=GetThreadList(0,windowsToo);
  end else if GetOperatingSystem in [osWinNT3,osWinNT4,osWinNT4SP4] then
    if (     threadsToo  and (not NT4_EnumProcessesAndThreads(@result,@tl,windowsToo))) or  
       ((not threadsToo) and (not NT4_EnumProcessesAndThreads(@result,nil,false     ))) then
      MessageBox(0,'Error reading Performace Data.','Error...',0);
{    if not TestPsApi then begin
      MessageBox(0,'"PsApi.dll" not found.','Error...',0);
      exit;
    end;
    SetLength(s1,MAX_PATH+1);
    SetLength(s1,GetModuleFileName(psApiHandle,pchar(s1),MAX_PATH));
    c1:=100; SetLength(ac,c1);
    if EnumProcesses(pointer(ac),4*c1,c2) then begin
      while 4*c1=c2 do begin
        inc(c1,100); SetLength(ac,c1); EnumProcesses(pointer(ac),4*c1,c2);
      end;
      SetLength(result,c2 div 4);
    end;
    for i1:=0 to high(result) do begin
      result[i1].pid:=ac[i1];
      c1:=OpenProcess(PROCESS_QUERY_INFORMATION or PROCESS_VM_READ,false,ac[i1]);
      if c1<>0 then
        try
          if EnumProcessModules(c1,c2,4,c3) then begin
            SetLength(result[i1].name,MAX_PATH+1);
            if GetModuleFileNameEx(c1,c2,PChar(result[i1].name),length(result[i1].name))<>0 then begin
              result[i1].name:=string(PChar(result[i1].name));
              result[i1].exeType:=GetExeType(result[i1].name);
            end else begin result[i1].name:=''; result[i1].exeType:=etUnknown end;
          end;
        finally CloseHandle(c1) end;
    end;
  end; }
  i4:=high(tl);
  if i4>0 then
    for i1:=0 to i4 do
      for i2:=high(result) downto 0 do
        if tl[i1].pid=result[i2].pid then begin
          i3:=length(result[i2].threads); setLength(result[i2].threads,i3+1); result[i2].threads[i3]:=tl[i1];
        end;
end;

function GetThreadList(pid: cardinal = 0; windowsToo: boolean = false) : TThreadList;
var c1 : cardinal;
    i1 : integer;
    te : TThreadEntry32;
begin
  result:=nil;
  if GetOperatingSystem in [osWin95,osWin95osr2,osWin98,osWinNT5] then begin
    if not TestToolhelpFunctions then begin
      MessageBox(0,'Toolhelp functions not available.','Error...',0);
      exit;
    end;
    c1:=CreateToolHelp32Snapshot(TH32CS_SnapThread,0);
    try
      i1:=0;
      te.dwSize:=sizeOf(te);
      if Thread32First(c1,te) then
        repeat
          if (pid=0) or (pid=te.th32OwnerProcessID) then begin
            SetLength(result,i1+1);
            result[i1].tid:=te.th32ThreadID; result[i1].pid:=te.th32OwnerProcessID;
            inc(i1);
          end;
        until not Thread32Next(c1,te);
    finally CloseHandle(c1) end;
    if windowsToo then
      for i1:=0 to high(result) do
        if (result[i1].pid<>0) then
          result[i1].windows:=GetWindowList(result[i1].pid,result[i1].tid);
  end else if GetOperatingSystem in [osWinNT3,osWinNT4,osWinNT4SP4] then
    if not NT4_EnumProcessesAndThreads(nil,@result,windowsToo) then
      MessageBox(0,'Error reading Performace Data.','Error...',0);
end;

var ew_pid, ew_tid        : cardinal;
    ew_onlyThoseInTaskbar : boolean;
function EnumWindowsProc(hwnd: cardinal; lParam: integer) : LongBool; stdcall;
var pwl       : ^TWindowList;
    i1        : integer;
    cpid,ctid : cardinal;
    cpar,cown : cardinal;
    bvis,btsk : boolean;
begin
  result:=true;
  ctid:=GetWindowThreadProcessID(hwnd,@cpid);
  if ((ew_pid=0) or (ew_pid=cpid)) and ((ew_tid=0) or (ew_tid=ctid)) then begin
    bvis:=IsWindowVisible(hwnd);
    cown:=GetWindow(hwnd,GW_OWNER); cpar:=GetParent(hwnd);
    btsk:=(cown=0) and (cpar=0) and bvis and (GetWindowLong(hwnd,GWL_EXSTYLE) and WS_EX_TOOLWINDOW=0);
    if (not ew_onlyThoseInTaskbar) or btsk then begin
      pwl:=pointer(lParam);
      i1:=length(pwl^);
      SetLength(pwl^,i1+1);
      with pwl^[i1] do begin
        window:=hwnd;
        parent:=cpar; owner:=cown;
        visible:=bvis; enabled:=IsWindowEnabled(hwnd);
        inTaskbar:=btsk;
        GetWindowRect(hwnd,rect);
        SetLength(title,MAX_PATH);
        SetLength(title,GetWindowText(hwnd,pchar(title),MAX_PATH));
        SetLength(className,MAX_PATH);
        SetLength(className,GetClassName(hwnd,pchar(className),MAX_PATH));
        pid:=cpid; tid:=ctid;
      end;
    end;
  end;
end;

function GetWindowList(pid: cardinal = 0; tid: cardinal = 0; onlyThoseInTaskbar: boolean = false) : TWindowList;
begin
  result:=nil;
  ew_pid:=pid; ew_tid:=tid; ew_onlyThoseInTaskbar:=onlyThoseInTaskbar;
  if ew_tid=0 then EnumWindows      (       @EnumWindowsProc,integer(@result))
  else             EnumThreadWindows(ew_tid,@EnumWindowsProc,integer(@result));
end;

function GetChildWindowList(window: cardinal) : TWindowList;
begin
  result:=nil;
  ew_pid:=0; ew_tid:=0; ew_onlyThoseInTaskbar:=false;
  EnumChildWindows(window,@EnumWindowsProc,integer(@result));
end;

function EnumWindowStationsProc(windowStationName: pchar; lParam: integer) : LongBool; stdcall;
var i1   : integer;


    pwsl : ^TWindowStationList;
begin
  result:=true;
  pwsl:=pointer(lParam);
  i1:=length(pwsl^);
  SetLength(pwsl^,i1+1);
  pwsl^[i1].name:=windowStationName;
end;

function GetWindowStationList(desktopsToo: boolean = false; windowsToo: boolean = false) : TWindowStationList;
var c1 : cardinal;
    i1 : integer;
begin
  result:=nil;
  EnumWindowStations(@EnumWindowStationsProc,integer(@result));
  if desktopsToo then
    for i1:=0 to high(result) do begin
      c1:=OpenWindowStation(pchar(result[i1].name),false,WINSTA_ENUMDESKTOPS);
      if c1>0 then
        try
          result[i1].desktops:=GetDesktopList(c1,windowsToo);
        finally CloseWindowStation(c1) end;
    end;
end;

function EnumDesktopsProc(desktopName: pchar; lParam: integer) : LongBool; stdcall;
var i1  : integer;
    pdl : ^TDesktopList;
begin
  result:=true;
  pdl:=pointer(lParam);
  i1:=length(pdl^);
  SetLength(pdl^,i1+1);
  pdl^[i1].name:=desktopName;
end;

function GetDesktopList(ws: cardinal = 0; windowsToo: boolean = false) : TDesktopList;
var c1 : cardinal;
    i1 : integer;
begin
  result:=nil;
  if ws=0 then ws:=GetProcessWindowStation;
  EnumDesktops(ws,@EnumDesktopsProc,integer(@result));
  if windowsToo then 
    for i1:=0 to high(result) do begin
      c1:=OpenDesktop(pchar(result[i1].name),0,false,DESKTOP_READOBJECTS);
      if c1>0 then
        try
          result[i1].windows:=GetDesktopWindowList(c1);
        finally CloseDesktop(c1) end;
    end;
end;

function GetDesktopWindowList(dt: cardinal = 0) : TWindowList;
begin
  result:=nil;
  if dt=0 then dt:=GetThreadDesktop(GetCurrentThreadID);
  ew_pid:=0; ew_tid:=0; ew_onlyThoseInTaskbar:=false;
  EnumDesktopWindows(dt,@EnumWindowsProc,integer(@result));
end;

{Button The class for a button.
ComboBox The class for a combo box.
Edit The class for an edit control.
ListBox The class for a list box.
MDIClient The class for an MDI client window.
ScrollBar The class for a scroll bar.
Static The class for a static control.


The following table describes the system classes that are available only for use by the system. They are listed here for completeness sake.

Class Description
ComboLBox The class for the list box contained in a combo box.
DDEMLEvent Windows NT: The class for DDEML events.
Message Windows NT 5.0 and later: The class for a message-only window.
#32768 The class for a menu.
#32769 The class for the desktop window.
#32770 The class for a dialog box.
#32771 The class for the task switch window.
#32772 Windows NT: The class for icon titles.  }

type TPasswordCacheEntry  = packed record
                              entry       : word;   // size of this entry, in bytes
                              resourceLen : word;   // size of resource name, in bytes
                              passwordLen : word;   // size of password, in bytes
                              entryIndex  : byte;   // entry index
                              entryType   : byte;   // type of entry
                              resource    : array [0..$FFFFFFF] of char;
                                                    // start of resource name
                                                    // password immediately follows resource name
                            end;
     TPPasswordCacheEntry = ^TPasswordCacheEntry;

function EnumPasswordCallbackProc(pce: TPPasswordCacheEntry; lParam: cardinal) : LongBool; stdcall;
var i1   : integer;
    ppcl : ^TCachedPasswordList;
begin
  result:=true;
  ppcl:=pointer(lParam);
  i1:=length(ppcl^);
  SetLength(ppcl^,i1+1);
  SetLength(ppcl^[i1].resource,pce^.resourceLen);
  Move(pce^.resource[0],pointer(ppcl^[i1].resource)^,pce^.resourceLen);
  ppcl^[i1].resource:=pchar(ppcl^[i1].resource);
  SetLength(ppcl^[i1].password,pce^.passwordLen);
  Move(pce^.resource[pce^.resourceLen],pointer(ppcl^[i1].password)^,pce^.passwordLen);
  ppcl^[i1].password:=pchar(ppcl^[i1].password);
  ppcl^[i1].resType:=pce^.entryType;
end;

var WNetEnumCachedPasswords : function (ps: pchar; pw: word; pb: byte; proc: pointer; lParam: cardinal) : word; stdcall
                              = nil;
    mpr                     : cardinal = 0;

function GetCachedPasswords : TCachedPasswordList;
begin
  result:=nil;
  if mpr=0 then begin
    mpr:=LoadLibrary('mpr');
    if mpr=0 then exit;
  end;
  if @WNetEnumCachedPasswords=nil then begin
    WNetEnumCachedPasswords:=GetProcAddress(mpr,'WNetEnumCachedPasswords');
    if @WNetEnumCachedPasswords=nil then exit;
  end;
  WNetEnumCachedPasswords(nil,0,$FF,@EnumPasswordCallbackProc,cardinal(@result));
end;

initialization
finalization
  if mpr        <>0 then FreeLibrary(mpr        );
//  if psApiHandle<>0 then FreeLibrary(psApiHandle);
end.
