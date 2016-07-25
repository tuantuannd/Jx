unit unitfrmMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs,Registry, xmldom, XMLIntf, msxmldom, XMLDoc, StdCtrls, XPMan, FileCtrl,
  ExtCtrls, jpeg, Shellapi, IniFiles,
  unitThreadCopyFile, ComCtrls,enumStuff;

type
  TfrmMain = class(TForm)
    XMLDoc: TXMLDocument;
    XPManifest1: TXPManifest;
    gbGamePath: TGroupBox;
    txtGamePath: TEdit;
    btnGamePath: TButton;
    btnUpdate: TButton;
    btnExit: TButton;
    imgIcon: TImage;
    btnRunGame: TButton;
    imgTitle: TImage;
    memoStatus: TListBox;
    pbCopy: TProgressBar;
    procedure FormShow(Sender: TObject);
    procedure btnUpdateClick(Sender: TObject);
    procedure btnExitClick(Sender: TObject);
    procedure btnGamePathClick(Sender: TObject);
    procedure btnRunGameClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private
    { Private declarations }
    AppPath: string;

    ThreadCopyFile: TThreadCopyFile;
    function CheckVersion(const aGamePath: string): boolean;
    procedure InitApp();
  public
    { Public declarations }
  end;
  function IsProcessExists(exe: string) : boolean;
  
var
  frmMain: TfrmMain;

implementation

{$R *.dfm}
function IsProcessExists(exe: string) : boolean;
var pl  : TProcessList;
   i1  : integer;
begin
  pl := GetProcessList;
  for i1 := 0 to high(pl) do
    if CompareText(ExtractFileName(exe), ExtractFileName(pl[i1].name)) = 0 then
      begin
      result := true;
      exit;
      end;
  result := false;
end;

function TfrmMain.CheckVersion(const aGamePath: string): boolean;
//检查版本
var
  iniVer: TIniFile;
  aVerFile: string;
  aMajorVer: integer;
  aNewVer: integer;
  aVerFor: integer;
  aForComment: string;
begin
  aVerFile := aGamePath + '\version.cfg';
  if not FileExists(aVerFile) then
    begin
    memoStatus.Items.Add('·版本文件丢失，请重新安装游戏。');
    result := false;
    exit;
    end;
  iniVer := TIniFile.Create(aVerFile);
  aMajorVer := iniVer.ReadInteger('Version','MajorVersion',0);
  try
    aNewVer := frmMain.XMLDoc.ChildNodes.Nodes['Root'].ChildNodes.Nodes['MajorVersion'].NodeValue;
  except
    memoStatus.Items.Add('·配置文件有误，不能进行升级，请重新下载升级包。');
    result := false;
    exit;
  end;

  try
    aVerFor := frmMain.XMLDoc.ChildNodes.Nodes['Root'].ChildNodes.Nodes['VersionFor'].NodeValue;
  except
    aVerFor := -1;
  end;

  if (aVerFor > 0) and (aMajorVer < aNewVer) then
    begin
    if aVerFor <> aMajorVer then
      begin
      try
        aForComment := frmMain.XMLDoc.ChildNodes.Nodes['Root'].ChildNodes.Nodes['VersionForComment'].NodeValue;
      except
        aForComment := '';
      end;
      if aForComment = '' then
          MessageDlg('游戏版本与升级包不符，请先下载相应的升级包。', mtWarning, [mbOK], 0)
      else
          MessageDlg(aForComment, mtWarning, [mbOK], 0);
      close();
      end;
    end;

  if aMajorVer < aNewVer then
    begin
    result := true;
    exit;
    end
  else if aMajorVer = aNewVer then
    begin
    if MessageDlg('本地已经是最新版本，是否继续安装？',
                mtConfirmation, [mbYes, mbNo], 0) = mrYes then
      begin
      result := true;
      exit;
      end
    else
      begin
      close();
      end;
    end
  else
    begin
    MessageDlg('检测到升级包版本比游戏版本旧，无法进行升级。', mtWarning, [mbOK], 0);
    memoStatus.Items.Add('·检测到升级包版本比游戏版本旧，无法进行升级。');
    result := false;
    exit;
    end;
  result := false;
end;

procedure TfrmMain.InitApp();
var
  myFileName: string;
  Reg: TRegistry;
  aGamePath: string;
  aAppTitle: string;
begin
  //检测游戏是否正在运行

  if (IsProcessExists('Game.exe') or IsProcessExists('JXOnline.exe')) then
    begin
    MessageDlg('检测到游戏正在运行，请先把游戏关闭再升级。', mtError, [mbOk], 0);
    close();
    end;

  AppPath := ExtractFilePath(ParamStr(0));
  myFileName := AppPath + 'Update.dat';
  if not FileExists(myFileName) then
    begin
    MessageDlg('缺少配置文件，请重新下载升级包！', mtError, [mbOk], 0);
    close;
    end;
  XMLDoc.LoadFromFile(myFileName);

  try
    aAppTitle := frmMain.XMLDoc.ChildNodes.Nodes['Root'].ChildNodes.Nodes['AppTitle'].NodeValue;
    self.Caption := aAppTitle;
  except
  end;

  Reg := TRegistry.Create;
  //读取路径
  reg.RootKey:= HKEY_LOCAL_MACHINE;
  reg.OpenKey('\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\JXOnline.exe\',false);
  aGamePath := reg.ReadString('Path');
  reg.CloseKey;

  if aGamePath = '' then
    begin
    if DirectoryExists('C:\Program Files\Kingsoft\JXOnline') then
        aGamePath := 'C:\Program Files\Kingsoft\JXOnline'
    else if DirectoryExists('D:\Program Files\Kingsoft\JXOnline') then
        aGamePath := 'D:\Program Files\Kingsoft\JXOnline'
    else if DirectoryExists('D:\Games\JXOnline') then
        aGamePath := 'D:\Games\JXOnline'
    else if DirectoryExists('E:\games\JXOnline') then
        aGamePath := 'E:\games\JXOnline';
    end;

  txtGamePath.Text := aGamePath;
  if (txtGamePath.Text <> '') and DirectoryExists(txtGamePath.Text) then
    memoStatus.Items.Add('·搜索到游戏路径，请确认是否正确，然后开始升级。')
  else
    begin
    memoStatus.Items.Add('·游戏路径有误，请选择正确的游戏路径。');
    end;
end;

procedure TfrmMain.FormShow(Sender: TObject);
begin
  InitApp();
  //btnUpdateClick(nil);
end;

procedure TfrmMain.btnUpdateClick(Sender: TObject);
begin
  if (txtGamePath.Text = '') or (not DirectoryExists(txtGamePath.Text)) then
    begin
    memoStatus.Items.Add('·游戏路径有误，请选择正确的游戏路径。');
    end
  else
    begin
    if not CheckVersion(txtGamePath.Text) then
      begin
      btnUpdate.Enabled := false;
      exit;
      end;
    pbCopy.Visible:= true;
    ThreadCopyFile := TThreadCopyFile.Create(true);
    ThreadCopyFile.FreeOnTerminate := true;
    ThreadCopyFile.Resume;
    end;
end;

procedure TfrmMain.btnExitClick(Sender: TObject);
begin
  close;
end;

procedure TfrmMain.btnGamePathClick(Sender: TObject);
var
  aGamePath: string;
begin
  if SelectDirectory('选择游戏路径','',aGamePath) then
      txtGamePath.Text := aGamePath;
end;

procedure TfrmMain.btnRunGameClick(Sender: TObject);
begin
  ShellExecute(handle,'open', PChar(txtGamePath.Text + '\JXOnline.exe'),
          '', PChar(txtGamePath.Text + '\'), SW_SHOWNORMAL); 
  close;
end;

procedure TfrmMain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if assigned(ThreadCopyFile) then ThreadCopyFile.Terminate;
end;
end.
