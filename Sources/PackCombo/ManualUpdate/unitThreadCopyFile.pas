unit unitThreadCopyFile;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, xmldom, XMLIntf, msxmldom, XMLDoc,Controls,
  IniFiles;

type
  TPackComb = function(stdFilePath,addFilePath:PChar): HRESULT;stdcall;
  TThreadCopyFile = class(TThread)
  private
    { Private declarations }
    myMemoText: string;
    procedure UpdateMemo();
    procedure UpdateMemo2();
    procedure UpdateProgress();
    procedure ThreadTerminate(Sender: TObject);
  protected
    procedure Execute; override;
  end;

implementation
uses
  unitfrmMain;
{ Important: Methods and properties of objects in visual components can only be
  used in a method called using Synchronize, for example,

      Synchronize(UpdateCaption);

  and UpdateCaption could look like,

    procedure TThreadCopyFile.UpdateCaption;
    begin
      Form1.Caption := 'Updated in a thread';
    end; }

{ TThreadCopyFile }

procedure TThreadCopyFile.Execute;
var
  i: integer;
  aNodeList: IXMLNodeList;
  aFilePath,aFileName: string;
  AppPath: string;
  ErrNum: Cardinal;
  IsOK: boolean;

  LibHandle: THandle;
  PackComb: TPackComb;
  aCompPackSource,aCompPackDesc: string;

  iniVer: TIniFile;
  aNewVer: integer;
begin
  self.OnTerminate := ThreadTerminate;
  if self.Terminated then exit;
  AppPath := ExtractFilePath(ParamStr(0));
  IsOK := true;
  try
  frmMain.txtGamePath.Enabled := false;
  frmMain.btnGamePath.Enabled := false;
  frmMain.btnUpdate.Enabled := false;
  
  myMemoText := '·开始复制文件，可能需要几分钟时间，请稍等...';
  Synchronize(UpdateMemo);
  myMemoText := '------------------------------------------------------';
  Synchronize(UpdateMemo);
  aNodeList := frmMain.XMLDoc.ChildNodes.Nodes['Root'].ChildNodes.Nodes['FileList'].ChildNodes;

  frmMain.pbCopy.Max := aNodeList.Count-1;
  frmMain.pbCopy.Min := 0;
  for i:=0 to aNodeList.Count-1 do
    begin
    aFilePath := aNodeList.Nodes[i].ChildNodes.Nodes['FilePath'].NodeValue;
    aFileName := aNodeList.Nodes[i].ChildNodes.Nodes['FileName'].NodeValue;
    myMemoText := '·正在复制文件：' + aFileName;
    Synchronize(UpdateMemo);
    if not DirectoryExists(frmMain.txtGamePath.Text + '\' + aFilePath) then
      begin
      if not ForceDirectories(frmMain.txtGamePath.Text + '\' + aFilePath) then
        begin
        myMemoText := '·复制' +  aFilePath + '\' + aFileName + '时出错！（错误：创建子目录失败）';
        Synchronize(UpdateMemo);
        IsOK := false;
        continue;
        end;
      end;
    if not CopyFile(PChar(AppPath + aFileName),
                    PChar(frmMain.txtGamePath.Text + '\' + aFilePath + '\'+ aFileName), false) then
      begin
      ErrNum := GetLastError();
      case ErrNum of
        5:myMemoText := '·复制' +  aFilePath + '\' + aFileName + '时出错！（错误：文件为只读）';
        2:myMemoText := '·复制' +  aFilePath + '\' + aFileName + '时出错！（错误：复制失败）';
        32:myMemoText := '·复制' +  aFilePath + '\' + aFileName + '时出错！（错误：文件正在使用）';
        else myMemoText := '·复制' +  aFilePath + '\' + aFileName + '时出错！（错误号：' + inttostr(ErrNum) + '）';
      end;
      Synchronize(UpdateMemo);
      IsOK := false;
      end
    else
      begin
      myMemoText :=  ' 完成！';
      Synchronize(UpdateMemo2);
      end;
    Synchronize(UpdateProgress);
    end;

  frmMain.pbCopy.Visible := false;
  if IsOK then
    begin
    myMemoText := '·文件复制完成。';
    Synchronize(UpdateMemo);
    end
  else
    begin
    myMemoText := '·文件复制错误，请重试或重新下载升级包。';
    Synchronize(UpdateMemo);
    exit;
    end;
  except
    else
    myMemoText := '·文件复制中断！';
    Synchronize(UpdateMemo);
    exit;
  end;

  myMemoText := '------------------------------------------------------';
  Synchronize(UpdateMemo);
  try
    aCompPackSource := frmMain.XMLDoc.ChildNodes.Nodes['Root'].
                        ChildNodes.Nodes['CompPack'].ChildNodes.Nodes['Source'].NodeValue;
    aCompPackDesc := frmMain.XMLDoc.ChildNodes.Nodes['Root'].
                        ChildNodes.Nodes['CompPack'].ChildNodes.Nodes['Desc'].NodeValue;
  Except
    aCompPackSource := '';
    aCompPackDesc := '';
  end;
  if ((aCompPackSource<>'') and (aCompPackDesc<>'')) then
    begin
    myMemoText := '·正在处理资源文件，请稍等...';
    Synchronize(UpdateMemo);

    if not FileExists(frmMain.txtGamePath.Text + aCompPackSource) then
      begin
      myMemoText := '·asdasd...';
    Synchronize(UpdateMemo);
      MoveFile(PChar(frmMain.txtGamePath.Text + aCompPackDesc),
                PChar(frmMain.txtGamePath.Text + aCompPackSource));
      end
    else
      begin
      LibHandle := LoadLibrary('UpdateCompPack.dll');
      if LibHandle=0 then
        begin
        myMemoText := 'UpdateCompPack.dll不存在，请重新下载升级包。';
        Synchronize(UpdateMemo);
        exit;
        end
      else//if LibHandle=0 then
        begin
        @PackComb := GetProcAddress(LibHandle,'PackComb');
        if @PackComb = nil then
          begin
          myMemoText := 'UpdateCompPack.dll调用失败，请重试或重新下载升级包。';
          Synchronize(UpdateMemo);
          FreeLibrary(LibHandle);
          exit;
          end
        else//if @PackComb = nil then
          begin
          if PackComb(PChar(frmMain.txtGamePath.Text + aCompPackSource),
                  PChar(frmMain.txtGamePath.Text + aCompPackDesc)) = S_OK then
            begin
            myMemoText :=  ' 完成！';
            Synchronize(UpdateMemo2);
            end
          else//if PackComb(PChar(frmMain.txtGamePath.Text + aCompPackSource),
            begin
            myMemoText :=  ' 失败，请重试或重新下载升级包。';
            Synchronize(UpdateMemo2);
            FreeLibrary(LibHandle);
            exit;
            end;//if PackComb(PChar(frmMain.txtGamePath.Text + aCompPackSource),
          end;//if @PackComb = nil then
        FreeLibrary(LibHandle);
        end;//if LibHandle=0 then
      end;//if not FileExists(frmMain.txtGamePath.Text + aCompPackSource) then
    end;//if ((aCompPackSource<>'') and (aCompPackDesc<>'')) then
  try
    //写版本号
    aNewVer := frmMain.XMLDoc.ChildNodes.Nodes['Root'].ChildNodes.Nodes['MajorVersion'].NodeValue;
    iniVer := TIniFile.Create(frmMain.txtGamePath.Text + '\version.cfg');
    iniVer.WriteInteger('Version','MajorVersion',aNewVer);
    iniVer.WriteInteger('Version','Version',0);
  except
    myMemoText := '·版本写入有误，请重试。';
    Synchronize(UpdateMemo);
    exit;
  end;

  if IsOK then
    begin
    myMemoText := '·升级完成，可以进入游戏。';
    Synchronize(UpdateMemo);
    frmMain.btnRunGame.Enabled := true;
    end
end;

procedure TThreadCopyFile.ThreadTerminate(Sender: TObject);
begin
  frmMain.btnExit.Caption := '退出(&C)';
end;

procedure TThreadCopyFile.UpdateMemo();
  begin
    frmMain.memoStatus.Items.Add(myMemoText);
    frmMain.memoStatus.Selected[frmMain.memoStatus.Items.Count-1] := true;
  end;

procedure TThreadCopyFile.UpdateMemo2();
  begin
    frmMain.memoStatus.Items.Strings[frmMain.memoStatus.Items.Count-1] := frmMain.memoStatus.Items.Strings[frmMain.memoStatus.Items.Count-1] + myMemoText;
    frmMain.memoStatus.Selected[frmMain.memoStatus.Items.Count-1] := true;
  end;

procedure TThreadCopyFile.UpdateProgress();
  begin
    frmMain.pbCopy.Position := frmMain.pbCopy.Position + 1;
  end;
end.

