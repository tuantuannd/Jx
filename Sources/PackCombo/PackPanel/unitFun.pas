unit unitFun;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls;

  function Sgn(num:integer) : integer;//取符号函数
  procedure SortListView(aListView:TListView; aColumn:TListColumn; SortOrder:boolean);//ListView排序
  function GetFileSize(const FileName: String) : integer; //得到文件大小
  Function WinExecAndWait32(FileName : String; Visibility : integer):dWord;
implementation

function Sgn(num:integer) : integer;
//取符号函数
begin
  if num=0 then
    result:=0
  else
    result:=num div abs(num);
end;

procedure SortListView(aListView:TListView; aColumn:TListColumn; SortOrder:boolean);
//ListView排序
  function CustomSortProc(Item1, Item2: TListItem; ParamSort: integer): integer; stdcall;
  //自定义排序,SortListView的子函数
  var
    aColumnIndex : integer;
  begin
    aColumnIndex := abs(ParamSort)-2;
    //////判断是不是都是选中或都没有选中，一般情况可以省略此处////////////
    if Item1.Checked <> Item2.Checked then
      begin//一个选中另一个没有选中，选中的总在前头
      if Item1.Checked then Result := -1 else Result := 1;
      exit;
      end;
    /////////////////////////////////////////////////////////////////////
    //都是选中或都没有选中，一般的排序方法
    if aColumnIndex < 0 then
      Result := Sgn(ParamSort) * CompareText(Item1.Caption,Item2.Caption)
    else
      Result := Sgn(ParamSort) * CompareText(Item1.SubItems.Strings[aColumnIndex],Item2.SubItems.Strings[aColumnIndex]);
  end;
begin
  if SortOrder then
    aListView.CustomSort(@CustomSortProc, (aColumn.Index+1))//（顺序）
  else
    aListView.CustomSort(@CustomSortProc, -(aColumn.Index+1));//（逆序）
end;

function GetFileSize(const FileName: string): LongInt;
var
  SearchRec: TSearchRec;
begin
  try
    if FindFirst(ExpandFileName(FileName), faAnyFile, SearchRec) = 0 then
      Result := SearchRec.Size
    else Result := -1;
  finally
    SysUtils.FindClose(SearchRec);
  end;
end;
{
function GetFileSize(const FileName: String) : integer;
//得到文件大小
var
  aFile: File;
begin
  try
    //AssignFile(aFile, FileName);
    //reset(aFile,1);
    //Result := FileSize(aFile);
    //CloseFile(aFile);
  except
    Result := 0;
  end;
end; }

Function WinExecAndWait32(FileName : String; Visibility : integer):dWord;
var
  zAppName:array[0..512] of char;
  zCurDir:array[0..255] of char;
  WorkDir:String;
  StartupInfo:TStartupInfo;
  ProcessInfo:TProcessInformation;
begin
  StrPCopy(zAppName,FileName);
  GetDir(0,WorkDir);
  StrPCopy(zCurDir,WorkDir);
  FillChar(StartupInfo,Sizeof(StartupInfo),#0);
  StartupInfo.cb := Sizeof(StartupInfo);

  StartupInfo.dwFlags := STARTF_USESHOWWINDOW;
  StartupInfo.wShowWindow := Visibility;
  if not CreateProcess(nil,
        zAppName, { pointer to command line string }
        nil, { pointer to process security attributes }
        nil, { pointer to thread security attributes }
        false, { handle inheritance flag }
        CREATE_NEW_CONSOLE or { creation flags }
        NORMAL_PRIORITY_CLASS,
        nil, { pointer to new environment block }
        nil, { pointer to current directory name }
        StartupInfo, { pointer to STARTUPINFO }
        ProcessInfo)
    then Result := 0 { pointer to PROCESS_INF }
  else
    begin
    WaitforSingleObject(ProcessInfo.hProcess,INFINITE);
    GetExitCodeProcess(ProcessInfo.hProcess,Result);
    end;
end;
end.
