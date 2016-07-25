unit unitfrmMakeUpdate;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls, ADODB, DB, FileCtrl, unitFun;

type
  TfrmMakeUpdate = class(TForm)
    alabMainVer: TLabel;
    txtMainVer: TEdit;
    alabPackBase: TLabel;
    txtPackBase: TEdit;
    lvUpdatePack: TListView;
    alabSaveDir: TLabel;
    txtSaveDir: TEdit;
    btnSaveDir: TButton;
    alabUpdateName: TLabel;
    txtUpdateName: TEdit;
    btnRun: TButton;
    btnClose: TButton;
    chkTestFinished: TCheckBox;
    alabSubVer: TLabel;
    txtSubVer: TEdit;
    alabPackAdd: TLabel;
    txtPackAdd: TEdit;
    procedure btnCloseClick(Sender: TObject);
    procedure btnSaveDirClick(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure txtMainTextChange(Sender: TObject);
    procedure btnRunClick(Sender: TObject);
  private
    { Private declarations }
    procedure InitList();
  public
    { Public declarations }
    mySubVer: integer;
    myADOConn: TADOConnection;
    myADOCommand: TADOCommand;
  end;


implementation

{$R *.dfm}

procedure TfrmMakeUpdate.InitList();
var
  aRS: _RecordSet;
  aVerFrom: integer;
  aUpdateName: string;
  aListItem: TListItem;
  i: integer;
  IsRSFirst: boolean;
begin
  lvUpdatePack.Clear;
  if (txtUpdateName.Text = '') or (txtSaveDir.Text = '') then exit;

  myADOCommand.CommandText :=
        'Select Name,MainVer,SubVer,Path From tblPac Where MainVer=''' +txtMainVer.Text +
            ''' and SubVer > 0 and SubVer < ' + inttostr(mySubVer) + ' Order by SubVer';
  aRS := myADOCommand.Execute;
  //生成列表
  aVerFrom := 0;
  aUpdateName := txtUpdateName.Text + txtMainVer.Text + '_' + 
          inttostr(aVerFrom) + '-' + inttostr(mySubVer) + '.pak';
  aListItem := lvUpdatePack.Items.Add;
  aListItem.Caption := aUpdateName;
  aListItem.SubItems.Add(inttostr(aVerFrom));
  aListItem.SubItems.Add(inttostr(mySubVer));
  aListItem.SubItems.Add(txtSaveDir.Text + '\' + aUpdateName);
  aListItem.SubItems.Add(
          ExtractFilePath(ParamStr(0)) +'PackExp.exe' +
          ' "' + txtPackBase.Text + '"' +
          ' "' + 'none' + '"' +
          ' "' + txtPackAdd.Text + '"' +
          ' "' + txtSaveDir.Text + '\' + aUpdateName + '"');   //运行的文件
  aListItem.Checked := true;
  if aRS.RecordCount = 0 then  //如果以前没有升级包
    begin
    for i:=1 to mySubVer-1 do
      begin
      aUpdateName := txtUpdateName.Text + txtMainVer.Text + '_' +
              inttostr(i) + '-' + inttostr(mySubVer) + '.pak';
      aListItem := lvUpdatePack.Items.Add;
      aListItem.Caption := aUpdateName;
      aListItem.SubItems.Add(inttostr(i));
      aListItem.SubItems.Add(inttostr(mySubVer));
      aListItem.SubItems.Add(txtSaveDir.Text + '\' + aUpdateName);
      aListItem.SubItems.Add(
          ExtractFilePath(ParamStr(0)) +'PackExp.exe' +
          ' "' + txtPackBase.Text + '"' +
          ' "' + 'none' + '"' +
          ' "' + txtPackAdd.Text + '"' +
          ' "' + txtSaveDir.Text + '\' + aUpdateName + '"');   //运行的文件
      aListItem.Checked := true;
      end;
    exit;
    end;

  IsRSFirst := true;
  for i:=1 to mySubVer-1 do
  begin
    if (IsRSFirst) and (i < aRS.Fields['SubVer'].Value) then  //如果某个升级包以前没有升级包
      begin
      aUpdateName := txtUpdateName.Text + txtMainVer.Text + '_' +
              inttostr(i) + '-' + inttostr(mySubVer) + '.pak';
      aListItem := lvUpdatePack.Items.Add;
      aListItem.Caption := aUpdateName;
      aListItem.SubItems.Add(inttostr(i));
      aListItem.SubItems.Add(inttostr(mySubVer));
      aListItem.SubItems.Add(txtSaveDir.Text + '\' + aUpdateName);
      aListItem.SubItems.Add(
          ExtractFilePath(ParamStr(0)) +'PackExp.exe' +
          ' "' + txtPackBase.Text + '"' +
          ' "' + 'none' + '"' +
          ' "' + txtPackAdd.Text + '"' +
          ' "' + txtSaveDir.Text + '\' + aUpdateName + '"');   //运行的文件
      aListItem.Checked := true;
      end
    else
      begin
      if aRS.EOF then
          aRS.MoveLast
      else if i < aRS.Fields['SubVer'].Value then
          aRS.MovePrevious;

      aUpdateName := txtUpdateName.Text + txtMainVer.Text + '_' +
            inttostr(i) + '-' + inttostr(mySubVer) + '.pak';
      aListItem := lvUpdatePack.Items.Add;
      aListItem.Caption := aUpdateName;
      aListItem.SubItems.Add(inttostr(i));
      aListItem.SubItems.Add(inttostr(mySubVer));
      aListItem.SubItems.Add(txtSaveDir.Text + '\' + aUpdateName);
      aListItem.SubItems.Add(
          ExtractFilePath(ParamStr(0)) +'PackExp.exe' +
          ' "' + txtPackBase.Text + '"' +
          ' "' + aRS.Fields['Path'].Value + '"' +
          ' "' + txtPackAdd.Text + '"' +
          ' "' + txtSaveDir.Text + '\' + aUpdateName + '"');   //运行的文件
      aListItem.Checked := true;
      if aRS.RecordCount <> 1 then aRS.MoveNext;
      IsRSFirst := false;
      end;
  end
end;

procedure TfrmMakeUpdate.btnCloseClick(Sender: TObject);
begin
  close;
end;

procedure TfrmMakeUpdate.btnSaveDirClick(Sender: TObject);
var
  aPakSavePath: string;
begin
  if SelectDirectory('选择游戏目录','',aPakSavePath) then
      txtSaveDir.Text := aPakSavePath;
end;

procedure TfrmMakeUpdate.FormShow(Sender: TObject);
begin
  InitList();
  //tUpdateName.Text := 'Update' + '[' + txtMainVer.Text + ']';
end;

procedure TfrmMakeUpdate.txtMainTextChange(Sender: TObject);
begin
  InitList();
  btnRun.Enabled := (txtUpdateName.Text <> '') and (txtSaveDir.Text <> '');
end;


procedure TfrmMakeUpdate.btnRunClick(Sender: TObject);
var
  i: integer;
  aCmd: string;
  iListItem: TListItem;
begin
  if MessageDlg('主版本：' + txtMainVer.Text + ' 副版本：' + inttostr(mySubVer)
                 + #10#13 + '与该版本相关的更新包信息将会更新，请确认。',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if not DirectoryExists(txtSaveDir.Text) then
     ForceDirectories(txtSaveDir.Text);
  myADOCommand.CommandText :=
        'Delete From tblUpdate Where MainVer = ''' + txtMainVer.Text + ''' and VerTo = ' + inttostr(mySubVer);
  myADOCommand.Execute;

  for i:=0 to lvUpdatePack.Items.Count-1 do
    begin
    iListItem := lvUpdatePack.Items.Item[i];
    if not iListItem.Checked then continue;
    aCmd := iListItem.SubItems.Strings[3];
    if chkTestFinished.Checked then aCmd := aCmd + ' test';
    WinExecAndWait32(PChar(aCmd),1);

    myADOCommand.CommandText :=
        'INSERT INTO tblUpdate(Name,MainVer,VerFrom,VerTo,Path) Values(''' +
                            iListItem.Caption + ''',''' +
                            txtMainVer.Text+ ''',' +
                            iListItem.SubItems.Strings[0] + ',' +
                            inttostr(mySubVer) + ',''' +
                            iListItem.SubItems.Strings[2] + ''')';
    myADOCommand.Execute;
    end;

  self.ModalResult := 1;
end;



end.
