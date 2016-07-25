unit unitfrmMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls, ExtCtrls, ADODB, DB, XPMan, math, FileCtrl,
  unitFun,unitfrmMainVer,unitfrmSubVer,unitfrmMakeUpdate,
  unitfrmPacView, unitfrmUpdateView,unitDataStruct, unitfrmAttrChange;

type
  TfrmMain = class(TForm)
    tabMain: TPageControl;
    tabsViewPac: TTabSheet;
    tabsViewUpdate: TTabSheet;
    gbSubPack: TGroupBox;
    lvSubPack: TListView;
    gbMainPack: TGroupBox;
    alabMainPackSize: TLabel;
    labMainPackSize: TLabel;
    txtMainPack: TEdit;
    alabMainVer: TLabel;
    cbMainVer: TComboBox;
    btnMainVerBuild: TButton;
    cbUpdateMainVer: TComboBox;
    alabUpateMainVer: TLabel;
    XPManifest1: TXPManifest;
    lvUpdatePack: TListView;
    alabMainPackName: TLabel;
    labMainPack: TLabel;
    alabMainPack: TLabel;
    btnSubVerBuild: TButton;
    btnMakeUpdate: TButton;
    btnMainVerDel: TButton;
    ADOConn: TADOConnection;
    ADOCommand: TADOCommand;
    btnSubVerDel: TButton;
    btnUpdateDel: TButton;
    btnMainVerMod: TButton;
    btnSubVerMod: TButton;
    btnUpdateMod: TButton;
    cbUpdateSubVer: TComboBox;
    alabUpateSubVer: TLabel;
    tabsViewMake: TTabSheet;
    alabMakeMainVer: TLabel;
    txtMakeMainVer: TEdit;
    alabMakeSubVer: TLabel;
    txtMakeSubVer: TEdit;
    alabWebDown: TLabel;
    txtMakeWebDown: TEdit;
    gbMakeSaveFile: TGroupBox;
    btnMakeOpenSaveFile: TButton;
    gbMakeSaveDir: TGroupBox;
    txtMakeSaveDir: TEdit;
    btnMakeSaveDir: TButton;
    btnMakeRun: TButton;
    btnUpdateCopyToMake: TButton;
    btnMakeClear: TButton;
    dlgMakeOpen: TOpenDialog;
    lvMakeSaveFile: TListView;
    btnMakeDelFile: TButton;
    btnMakeModFile: TButton;
    procedure FormCreate(Sender: TObject);
    procedure cbMainVerChange(Sender: TObject);
    procedure cbUpdateVerChange(Sender: TObject);
    procedure btnMainVerBuildClick(Sender: TObject);
    procedure btnMainVerDelClick(Sender: TObject);
    procedure btnSubVerBuildClick(Sender: TObject);
    procedure btnSubVerDelClick(Sender: TObject);
    procedure lvSubPackSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure btnMakeUpdateClick(Sender: TObject);
    procedure lvSubPackColumnClick(Sender: TObject; Column: TListColumn);
    procedure lvUpdatePackColumnClick(Sender: TObject;
      Column: TListColumn);
    procedure btnUpdateDelClick(Sender: TObject);
    procedure lvUpdatePackSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure btnMainVerModClick(Sender: TObject);
    procedure btnSubVerModClick(Sender: TObject);
    procedure lvSubPackDblClick(Sender: TObject);
    procedure btnUpdateModClick(Sender: TObject);
    procedure lvUpdatePackDblClick(Sender: TObject);
    procedure btnMakeClearClick(Sender: TObject);
    procedure txtMakeTextChange(Sender: TObject);
    procedure btnMakeSaveDirClick(Sender: TObject);
    procedure lvMakeSaveFileInsert(Sender: TObject; Item: TListItem);
    procedure lvMakeSaveFileDeletion(Sender: TObject; Item: TListItem);
    procedure btnMakeOpenSaveFileClick(Sender: TObject);
    procedure lvMakeSaveFileSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure btnMakeDelFileClick(Sender: TObject);
    procedure lvMakeSaveFileColumnClick(Sender: TObject;
      Column: TListColumn);
    procedure btnMakeRunClick(Sender: TObject);
    procedure btnMakeModFileClick(Sender: TObject);
    procedure lvMakeSaveFileDblClick(Sender: TObject);
    procedure btnUpdateCopyToMakeClick(Sender: TObject);
    procedure cbUpdateSubVerChange(Sender: TObject);
  private
    { Private declarations }
    procedure InitCombo();
    procedure LoadPacData(theMainVer: string);
    procedure LoadUpdateData(theMainVer: string; theSubVer: string);
    procedure AddMakeFileList(aList: TStringList);
  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation

{$R *.dfm}
/////////自定义函数////////////////////////////////////////////
procedure TfrmMain.AddMakeFileList(aList: TStringList);
var
  i,j,k: integer;
  aListItem: TListItem;
  aFileName: string;
begin
  for i:=0 to aList.Count-1 do
    begin
    //如果有该项目就跳过
    for j:=0 to lvMakeSaveFile.Items.Count-1 do
        if AnsiLowerCase(lvMakeSaveFile.Items.Item[j].SubItems.Strings[0]) =
            AnsiLowerCase(aList.Strings[i]) then
                continue;

    aFileName := AnsiLowerCase(ExtractFileName(aList.Strings[i]));
    aListItem := lvMakeSaveFile.Items.Add;
    aListItem.Caption := aFileName;  //FileName
    aListItem.SubItems.Add(aList.Strings[i]);  //FilePath
    aListItem.SubItems.Add('-1');           //Belong
    aListItem.SubItems.Add('5');            //CheckVersionMethod
    aListItem.SubItems.Add('1');            //UpdateFileFlag
    aListItem.SubItems.Add('%PROGRAMDIR%'); //LocalPath
    aListItem.SubItems.Add('copy');         //UpdateFileMethod
    aListItem.SubItems.Add('');             //RemotePath
    aListItem.SubItems.Add('0');              //UpdateRelative

    if ((aFileName = 'autoupdateres.dll') or
          (aFileName = 'engine.dll') or
          (aFileName = 'represent2.dll') or
          (aFileName = 'represent3.dll') or
          (aFileName = 'updatedll.dll') or
          (aFileName = 'jxonline.exe')) then
      begin
      aListItem.SubItems.Strings[7] := '1';
      end
    else if (aFileName = 'chatsent.flt') then
      begin
      aListItem.SubItems.Strings[4] := '%PROGRAMDIR\Settings\'
      end
    else if ((aFileName[1] = 'u') and
              (aFileName[2] = 'p') and
              (aFileName[3] = 'd') and
              (aFileName[4] = 'a') and
              (aFileName[5] = 't') and
              (aFileName[6] = 'e')) then
      begin
      aListItem.SubItems.Strings[4] := '%PROGRAMDIR\data\';
      aListItem.SubItems.Strings[5] := 'Package(Update.pak)';
      aListItem.SubItems.Strings[1] := '';
      for k := 1 to length(aFileName) - 1 do
        begin
        if aFileName[k-1] = '_' then break;
        end;
      if k = length(aFileName) then k := 7;
      while (aFileName[k] <> '-') do
        begin
        aListItem.SubItems.Strings[1] := aListItem.SubItems.Strings[1] + aFileName[k];
        inc(k);
        end;
      end;
    end;
end;

procedure TfrmMain.InitCombo();
var
  aRS: _RecordSet;
  aMainVer: string;
begin
  cbMainVer.Clear;
  cbUpdateMainVer.Clear;
  cbUpdateSubVer.Clear;
  lvSubPack.Clear;
  lvUpdatePack.Clear;
  labMainPack.Caption := '';
  labMainPackSize.Caption := '';
  txtMainPack.Text := '';
  ADOCommand.CommandText := 'Select MainVer From tblMainVer';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount = 0 then exit;
  cbUpdateMainVer.AddItem('所有版本',nil);
  while not aRS.EOF do
    begin
    aMainVer := aRS.Fields['MainVer'].Value;
    cbMainVer.AddItem(aMainVer,nil);
    cbUpdateMainVer.AddItem(aMainVer,nil);
    aRS.MoveNext;
    end;

  cbMainVer.ItemIndex := cbMainVer.Items.Count-1;
  cbUpdateMainVer.ItemIndex := cbUpdateMainVer.Items.Count-1;
  cbMainVerChange(nil);
  cbUpdateVerChange(nil);
end;

procedure TfrmMain.LoadPacData(theMainVer: string);
var
  aMainVer: string;
  aSubVer: integer;
  aFileName: string;
  aFilePath: string;
  aRS: _RecordSet;
  aListItem: TListItem;
begin
  lvSubPack.Clear;
  ADOCommand.CommandText :=
        'Select Name,MainVer,SubVer,Path From tblPac Where MainVer=''' + theMainVer +'''';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount = 0 then exit;
  while not aRS.EOF do
    begin
    aMainVer := aRS.Fields['MainVer'].Value;
    aFileName:= aRS.Fields['Name'].Value;
    aFilePath:= aRS.Fields['Path'].Value;
    aSubVer := aRS.Fields['SubVer'].Value;
    if aSubVer = 0 then
      begin
      labMainPack.Caption := aFileName;
      if FileExists(aFilePath) then
        begin
        labMainPackSize.Caption := inttostr(GetFileSize(aFilePath));
        txtMainPack.Text := aFilePath;
        end
      else
        begin
        labMainPackSize.Caption := '-1';
        txtMainPack.Text := '错误！该路径找不到相应文件：' + aFilePath;
        end;
      end
    else//if aSubVer = 0
      begin
      aListItem := lvSubPack.Items.Add;
      aListItem.Caption := aFileName;
      aListItem.SubItems.Add(aMainVer);
      aListItem.SubItems.Add(inttostr(aSubVer));
      if FileExists(aFilePath) then
        begin
        aListItem.SubItems.Add(inttostr(GetFileSize(aFilePath)));
        aListItem.SubItems.Add(aFilePath);
        end
      else
        begin
        aListItem.SubItems.Add('-1');
        aListItem.SubItems.Add('错误！该路径找不到相应文件：' + aFilePath);
        end;
      end;//if aSubVer = 0
    aRS.MoveNext;
    end;//while not aRS.EOF do
end;


procedure TfrmMain.LoadUpdateData(theMainVer: string; theSubVer: string);
var
  aMainVer: string;
  aVerFrom, aVerTo: integer;
  aFileName: string;
  aFilePath: string;
  aRS: _RecordSet;
  aListItem: TListItem;
begin
  lvUpdatePack.Clear;
  if (theMainVer = '') and (theSubVer = '') then
    begin
    ADOCommand.CommandText :=
          'Select Name,MainVer,VerFrom,VerTo,Path From tblUpdate'
    end
  else if (theMainVer <> '') and (theSubVer = '') then
    begin
    ADOCommand.CommandText :=
          'Select Name,MainVer,VerFrom,VerTo,Path From tblUpdate Where MainVer=''' + theMainVer +'''';
    end
  else if (theMainVer = '') and (theSubVer <> '') then
    begin
    ADOCommand.CommandText :=
          'Select Name,MainVer,VerFrom,VerTo,Path From tblUpdate Where VerTo=' + theSubVer;
    end
  else if (theMainVer <> '') and (theSubVer <> '') then
    begin
    ADOCommand.CommandText :=
          'Select Name,MainVer,VerFrom,VerTo,Path From tblUpdate Where MainVer=''' + theMainVer +''' and VerTo=' + theSubVer;
    end
  else
    exit;

  aRS := ADOCommand.Execute;
  if aRS.RecordCount = 0 then exit;
  while not aRS.EOF do
    begin
    aMainVer := aRS.Fields['MainVer'].Value;
    aFileName:= aRS.Fields['Name'].Value;
    aFilePath:= aRS.Fields['Path'].Value;
    aVerFrom := aRS.Fields['VerFrom'].Value;
    aVerTo := aRS.Fields['VerTo'].Value;

    aListItem := lvUpdatePack.Items.Add;
    aListItem.Caption := aFileName;
    aListItem.SubItems.Add(aMainVer);
    aListItem.SubItems.Add(inttostr(aVerFrom));
    aListItem.SubItems.Add(inttostr(aVerTo));
    if FileExists(aFilePath) then
      begin
      aListItem.SubItems.Add(inttostr(GetFileSize(aFilePath)));
      aListItem.SubItems.Add(aFilePath);
      end
    else
      begin
      aListItem.SubItems.Add('-1');
      aListItem.SubItems.Add('错误！该路径找不到相应文件：' + aFilePath);
      end;
    aRS.MoveNext;
    end;//while not aRS.EOF do
end;
///////////程序函数//////////////////////////////////////////////////
procedure TfrmMain.FormCreate(Sender: TObject);
begin
  InitCombo();
end;

procedure TfrmMain.cbMainVerChange(Sender: TObject);
begin
  LoadPacData(cbMainVer.Text);
end;

procedure TfrmMain.cbUpdateVerChange(Sender: TObject);
var
  aRS: _RecordSet;
  aVerTo: string;
begin
  if cbUpdateMainVer.Text = '所有版本' then
    ADOCommand.CommandText :=
            'Select VerTo From tblUpdate Where VerTo=-1'
  else
    ADOCommand.CommandText :=
            'Select VerTo From tblUpdate Where MainVer=''' + cbUpdateMainVer.Text +''' Group By VerTo Order By VerTo';
  aRS := ADOCommand.Execute;
  cbUpdateSubVer.Clear;
  cbUpdateSubVer.AddItem('所有版本',nil);
  if aRS.RecordCount <> 0 then
      while not aRS.EOF do
        begin
        aVerTo := aRS.Fields['VerTo'].Value;
        cbUpdateSubVer.AddItem(aVerTo,nil);
        aRS.MoveNext;
        end;
  cbUpdateSubVer.ItemIndex := cbUpdateSubVer.Items.Count-1;
  if (cbUpdateMainVer.Text = '所有版本') and (cbUpdateSubVer.Text = '所有版本') then
      LoadUpdateData('','')
  else if (cbUpdateMainVer.Text = '所有版本') and (cbUpdateSubVer.Text <> '所有版本') then
      LoadUpdateData('',cbUpdateSubVer.Text)
  else if (cbUpdateMainVer.Text <> '所有版本') and (cbUpdateSubVer.Text = '所有版本') then
      LoadUpdateData(cbUpdateMainVer.Text,'')
  else if (cbUpdateMainVer.Text <> '所有版本') and (cbUpdateSubVer.Text <> '所有版本') then
      LoadUpdateData(cbUpdateMainVer.Text,cbUpdateSubVer.Text)
  else
      exit;
end;

procedure TfrmMain.cbUpdateSubVerChange(Sender: TObject);
begin
  if (cbUpdateMainVer.Text = '所有版本') and (cbUpdateSubVer.Text = '所有版本') then
      LoadUpdateData('','')
  else if (cbUpdateMainVer.Text = '所有版本') and (cbUpdateSubVer.Text <> '所有版本') then
      LoadUpdateData('',cbUpdateSubVer.Text)
  else if (cbUpdateMainVer.Text <> '所有版本') and (cbUpdateSubVer.Text = '所有版本') then
      LoadUpdateData(cbUpdateMainVer.Text,'')
  else if (cbUpdateMainVer.Text <> '所有版本') and (cbUpdateSubVer.Text <> '所有版本') then
      LoadUpdateData(cbUpdateMainVer.Text,cbUpdateSubVer.Text)
  else
      exit;
end;

procedure TfrmMain.btnMainVerBuildClick(Sender: TObject);
var
  frmMainVer: TfrmMainVer;
  aMainVer: string;
  aRS: _RecordSet;
begin
  frmMainVer := TfrmMainVer.Create(self);
  frmMainVer.ShowModal;
  if frmMainVer.ModalResult <> 1 then exit;
  aMainVer := frmMainVer.txtMainVer.Text;
  ADOCommand.CommandText :=
        'Select MainVer From tblMainVer Where MainVer=''' + aMainVer +'''';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount <> 0 then
    begin
    MessageDlg('版本号已存在，设置将不会改变！'
              , mtError,[mbOk], 0);
    exit;
    end;
        
  //添加版本号
  ADOCommand.CommandText :=
        'INSERT INTO tblMainVer(MainVer) Values(''' + aMainVer + ''')';
  ADOCommand.Execute;

  //添加文件内容
  ADOCommand.CommandText :=
        'INSERT INTO tblPac(Name,MainVer,SubVer,Path) Values(''' +
                            ExtractFileName(frmMainVer.txtPackSaveFile.Text) + ''',''' +
                            aMainVer+ ''',' +
                            '0' + ',''' +
                            frmMainVer.txtPackSaveFile.Text+ ''')';
  ADOCommand.Execute;
  InitCombo();
end;

procedure TfrmMain.btnMainVerDelClick(Sender: TObject);
var
  aMainVer: string;
begin
  aMainVer := cbMainVer.Text;
  if aMainVer = '' then exit;
  if MessageDlg('版本号：' + aMainVer + #10#13 + '是否确定删除该主版本，所有副版本也会同时删除？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if MessageDlg('版本号：' + aMainVer + #10#13 + '主版本是很重要的，真的要删除么？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if MessageDlg('版本号：' + aMainVer + #10#13 + '最后的机会，想清楚再确定？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;

  //删除主版本
  ADOCommand.CommandText :=
        'Delete From tblMainVer Where MainVer = ''' + aMainVer + '''';
  ADOCommand.Execute;

  //删除主版本的全部副版本文件
  ADOCommand.CommandText :=
        'Delete From tblPac Where MainVer = ''' + aMainVer + '''';
  ADOCommand.Execute;
  //删除主版本的全部副版本文件
  InitCombo();
end;

procedure TfrmMain.btnSubVerBuildClick(Sender: TObject);
var
  frmSubVer: TfrmSubVer;
  aMainVer: string;
  aRS: _RecordSet;
begin
  aMainVer := cbMainVer.Text;
  if aMainVer = '' then exit;
  
  ADOCommand.CommandText :=
        'Select Path From tblPac Where MainVer=''' + aMainVer +''' and SubVer = 0';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount <> 1 then
    begin
    MessageDlg('读取版本错误！'
              , mtError,[mbOk], 0);
    exit;
    end;
  frmSubVer := TfrmSubVer.Create(self);
  frmSubVer.txtMainVer.Text := aMainVer;
  frmSubVer.txtPackBase.Text := aRS.Fields['Path'].Value;

  ADOCommand.CommandText :=
        'Select Top 1 SubVer From tblPac Where MainVer=''' + aMainVer +''' Order By SubVer DESC';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount = 0 then
    begin
    frmSubVer.txtSubVer.Text := '1';
    frmSubVer.mySubVer := 1;
    end
  else
    begin
    frmSubVer.txtSubVer.Text := inttostr(aRS.Fields['SubVer'].Value + 1);
    frmSubVer.mySubVer := aRS.Fields['SubVer'].Value + 1;
    end;

  frmSubVer.ShowModal;
  if frmSubVer.ModalResult <> 1 then exit;

  ADOCommand.CommandText :=
        'INSERT INTO tblPac(Name,MainVer,SubVer,Path) Values(''' +
                            ExtractFileName(frmSubVer.txtPackSaveFile.Text) + ''',''' +
                            aMainVer+ ''',' +
                            frmSubVer.txtSubVer.Text + ',''' +
                            frmSubVer.txtPackSaveFile.Text+ ''')';
  ADOCommand.Execute;
  InitCombo();
end;

procedure TfrmMain.btnSubVerDelClick(Sender: TObject);
var
  aMainVer,aSubVer: string;
begin
  aMainVer := cbMainVer.Text;
  aSubVer := lvSubPack.Selected.SubItems.Strings[1];
  if MessageDlg('主版本号：' + aMainVer + ' 副版本号：' + aSubVer +
          #10#13 + '是否确定删除该副版本？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if MessageDlg('主版本号：' + aMainVer + ' 副版本号：' + aSubVer +
          #10#13 + '副版本是很重要的，真的要删除么？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if MessageDlg('主版本号：' + aMainVer + ' 副版本号：' + aSubVer +
          #10#13 + '最后的机会，想清楚再确定？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;


  //删除主版本的全部副版本文件
  ADOCommand.CommandText :=
        'Delete From tblPac Where MainVer = ''' + aMainVer + ''' and SubVer = ' + aSubVer;
  ADOCommand.Execute;
  //删除主版本的全部副版本文件
  InitCombo();
end;

procedure TfrmMain.lvSubPackSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
  btnSubVerDel.Enabled := Selected;
  btnMakeUpdate.Enabled := Selected;
  btnSubVerMod.Enabled := Selected;
end;

procedure TfrmMain.btnMakeUpdateClick(Sender: TObject);
var
  frmMakeUpdate: TfrmMakeUpdate;
  aMainVer: string;
  aRS: _RecordSet;
begin
  aMainVer := cbMainVer.Text;
  if aMainVer = '' then exit;
  
  ADOCommand.CommandText :=
        'Select Path From tblPac Where MainVer=''' + aMainVer +''' and SubVer = 0';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount <> 1 then
    begin
    MessageDlg('读取版本错误！'
              , mtError,[mbOk], 0);
    exit;
    end;

  frmMakeUpdate := TfrmMakeUpdate.Create(self);
  with frmMakeUpdate do
    begin
    txtMainVer.Text := aMainVer;
    txtPackBase.Text := aRS.Fields['Path'].Value;
    txtSubVer.Text := lvSubPack.Selected.SubItems.Strings[1];
    txtPackAdd.Text := lvSubPack.Selected.SubItems.Strings[3];
    mySubVer := strtoint(lvSubPack.Selected.SubItems.Strings[1]);
    myADOConn := ADOConn;
    myADOCommand := ADOCommand;

    end;
  frmMakeUpdate.ShowModal;
  if frmMakeUpdate.ModalResult <> 1 then exit;
  tabMain.ActivePageIndex := 1;
  InitCombo();
end;

procedure TfrmMain.lvSubPackColumnClick(Sender: TObject;
  Column: TListColumn);
{$WRITEABLECONST ON}
const
  SortOrder:boolean = true;
begin
  SortListView(TListView(Sender),Column,SortOrder);
  SortOrder := (not SortOrder);
end;

procedure TfrmMain.lvUpdatePackColumnClick(Sender: TObject;
  Column: TListColumn);
{$WRITEABLECONST ON}
const
  SortOrder:boolean = true;
begin
  SortListView(TListView(Sender),Column,SortOrder);
  SortOrder := (not SortOrder);
end;

procedure TfrmMain.btnUpdateDelClick(Sender: TObject);
var
  aUpdate: string;
begin
  aUpdate := lvUpdatePack.Selected.Caption;
  if MessageDlg('升级包：' + aUpdate +
          #10#13 + '是否确定删除升级包？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if MessageDlg('升级包：' + aUpdate +
          #10#13 + '升级包是很重要的，真的要删除么？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
  if MessageDlg('升级包：' + aUpdate +
          #10#13 + '最后的机会，想清楚再确定？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;


  //删除主版本的全部副版本文件
  ADOCommand.CommandText :=
        'Delete From tblUpdate Where Name = ''' + aUpdate + '''';
  ADOCommand.Execute;
  InitCombo();
end;

procedure TfrmMain.lvUpdatePackSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
  btnUpdateDel.Enabled := Selected;
  btnUpdateMod.Enabled := Selected;
end;

procedure TfrmMain.btnMainVerModClick(Sender: TObject);
var
  frmPacView: TfrmPacView;
  aRS: _RecordSet;
begin
  ADOCommand.CommandText :=
        'Select Name,MainVer,SubVer,Path From tblPac Where Name = ''' + labMainPack.Caption + '''';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount <> 1 then
    begin
    MessageDlg('读取信息有错！'
              , mtError,[mbOk], 0);
    exit;
    end;
  frmPacView := TfrmPacView.Create(self);
  with frmPacView do
    begin
    txtMainVer.Text := cbMainVer.Text;
    txtSubVer.Text := '0';
    txtPacFileName.Text := aRS.Fields['Name'].Value;
    txtPacFilePath.Text := aRS.Fields['Path'].Value;
    end;
  frmPacView.ShowModal;
  if frmPacView.ModalResult <> 1 then exit;
  
  //修改文件信息
  ADOCommand.CommandText :=
        'Update tblPac Set Name = ''' + frmPacView.txtPacFileName.Text +
                        ''', MainVer = ''' + frmPacView.txtMainVer.Text +
                        ''', SubVer = ' + frmPacView.txtSubVer.Text +
                        ', Path = ''' + frmPacView.txtPacFilePath.Text +
                        ''' Where Name = ''' + labMainPack.Caption + '''';
  ADOCommand.Execute;
  InitCombo();
end;

procedure TfrmMain.btnSubVerModClick(Sender: TObject);
var
  frmPacView: TfrmPacView;
  aRS: _RecordSet;
begin
  ADOCommand.CommandText :=
        'Select Name,MainVer,SubVer,Path From tblPac Where Name = ''' + lvSubPack.Selected.Caption + '''';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount <> 1 then
    begin
    MessageDlg('读取信息有错！'
              , mtError,[mbOk], 0);
    exit;
    end;
  frmPacView := TfrmPacView.Create(self);
  with frmPacView do
    begin
    txtMainVer.Text := aRS.Fields['MainVer'].Value;
    txtSubVer.Text := aRS.Fields['SubVer'].Value;
    txtPacFileName.Text := aRS.Fields['Name'].Value;
    txtPacFilePath.Text := aRS.Fields['Path'].Value;
    end;
  frmPacView.ShowModal;
  if frmPacView.ModalResult <> 1 then exit;
  
  //修改文件信息
  ADOCommand.CommandText :=
        'Update tblPac Set Name = ''' + frmPacView.txtPacFileName.Text +
                        ''', MainVer = ''' + frmPacView.txtMainVer.Text +
                        ''', SubVer = ' + frmPacView.txtSubVer.Text +
                        ', Path = ''' + frmPacView.txtPacFilePath.Text +
                        ''' Where Name = ''' + lvSubPack.Selected.Caption + '''';
  ADOCommand.Execute;
  InitCombo();
end;

procedure TfrmMain.lvSubPackDblClick(Sender: TObject);
begin
  if lvSubPack.Selected =nil then exit;
  btnSubVerModClick(nil);
end;

procedure TfrmMain.btnUpdateModClick(Sender: TObject);
var
  frmUpdateView: TfrmUpdateView;
  aRS: _RecordSet;
begin
  ADOCommand.CommandText :=
        'Select Name,MainVer,VerFrom,VerTo,Path From tblUpdate Where Name = ''' + lvUpdatePack.Selected.Caption + '''';
  aRS := ADOCommand.Execute;
  if aRS.RecordCount <> 1 then
    begin
    MessageDlg('读取信息有错！'
              , mtError,[mbOk], 0);
    exit;
    end;
  frmUpdateView := TfrmUpdateView.Create(self);
  with frmUpdateView do
    begin
    txtMainVer.Text := aRS.Fields['MainVer'].Value;
    txtVerFrom.Text := aRS.Fields['VerFrom'].Value;
    txtVerTo.Text := aRS.Fields['VerTo'].Value;
    txtPacFileName.Text := aRS.Fields['Name'].Value;
    txtPacFilePath.Text := aRS.Fields['Path'].Value;
    end;
  frmUpdateView.ShowModal;
  if frmUpdateView.ModalResult <> 1 then exit;
  
  //修改文件信息
  ADOCommand.CommandText :=
        'Update tblUpdate Set Name = ''' + frmUpdateView.txtPacFileName.Text +
                        ''', MainVer = ''' + frmUpdateView.txtMainVer.Text +
                        ''', VerFrom = ' + frmUpdateView.txtVerFrom.Text +
                        ', VerTo = ' + frmUpdateView.txtVerTo.Text +
                        ', Path = ''' + frmUpdateView.txtPacFilePath.Text +
                        ''' Where Name = ''' + lvUpdatePack.Selected.Caption + '''';
  ADOCommand.Execute;
  InitCombo();

end;

procedure TfrmMain.lvUpdatePackDblClick(Sender: TObject);
begin
  if lvUpdatePack.Selected =nil then exit;
  btnUpdateModClick(nil);
end;

procedure TfrmMain.btnMakeClearClick(Sender: TObject);
//清除升级文件生成器的内容
begin
  txtMakeMainVer.Text := '';
  txtMakeSubVer.Text := '';
  txtMakeSaveDir.Text := '';
  lvMakeSaveFile.Clear;
end;

procedure TfrmMain.txtMakeTextChange(Sender: TObject);
begin
  btnMakeRun.Enabled := (txtMakeMainVer.Text <> '') and
                        (txtMakeSubVer.Text <> '') and
                        (txtMakeSaveDir.Text <> '') and
                        (lvMakeSaveFile.Items.Count > 0);
end;

procedure TfrmMain.btnMakeSaveDirClick(Sender: TObject);
var
  aGamePath: string;
begin
  if SelectDirectory('选择游戏目录','',aGamePath) then
      txtMakeSaveDir.Text := aGamePath;
end;

procedure TfrmMain.lvMakeSaveFileInsert(Sender: TObject; Item: TListItem);
begin
  btnMakeClearClick(nil);    
end;

procedure TfrmMain.lvMakeSaveFileDeletion(Sender: TObject;
  Item: TListItem);
begin
  btnMakeClearClick(nil);
end;

procedure TfrmMain.btnMakeOpenSaveFileClick(Sender: TObject);
var
  aFileList: TStringList;
begin
  if not dlgMakeOpen.Execute then exit;

  aFileList := TStringList.Create;
  aFileList.AddStrings(dlgMakeOpen.Files);
  AddMakeFileList(aFileList);
  aFileList.Destroy;
end;

procedure TfrmMain.lvMakeSaveFileSelectItem(Sender: TObject;
  Item: TListItem; Selected: Boolean);
begin
  btnMakeDelFile.Enabled := Selected;
  btnMakeModFile.Enabled := Selected;
end;

procedure TfrmMain.btnMakeDelFileClick(Sender: TObject);
begin
  if lvMakeSaveFile.Selected = nil then exit;

  lvMakeSaveFile.Selected.Delete;
end;

procedure TfrmMain.lvMakeSaveFileColumnClick(Sender: TObject;
  Column: TListColumn);
{$WRITEABLECONST ON}
const
  SortOrder:boolean = true;
begin
  SortListView(TListView(Sender),Column,SortOrder);
  SortOrder := (not SortOrder);
end;

procedure TfrmMain.btnMakeRunClick(Sender: TObject);
var
  aAppPath: string;
  aFilePath: string;
  aSaveStrList: TStringList;
  i,j: integer;
  aListItem: TListItem;
  aCmd: string;
begin
  if not DirectoryExists(txtMakeSaveDir.Text) then
      if not ForceDirectories(txtMakeSaveDir.Text) then
        begin
        MessageDlg('创建 ' + txtMakeSaveDir.Text + ' 时出错！',mtError, [mbOK], 0,);
        exit;
        end;
  for i:=0 to lvMakeSaveFile.Items.Count-1 do
    begin
    aFilePath := txtMakeSaveDir.Text + '\' + lvMakeSaveFile.Items.Item[i].Caption;
    if not CopyFile(PChar(lvMakeSaveFile.Items.Item[i].SubItems.Strings[0]),PChar(aFilePath),false) then
        MessageDlg('复制文件 ' + lvMakeSaveFile.Items.Item[i].SubItems.Strings[0] + #13#10 +
                    '到 ' + txtMakeSaveDir.Text + ' 时出错！',mtError, [mbOK], 0,);
    end;

  aAppPath := ExtractFilePath(ParamStr(0));
  aSaveStrList := TStringList.Create;

  //写version.cfg
  aFilePath := txtMakeSaveDir.Text + '\version.cfg';
  aSaveStrList.Add('[Version]');
  aSaveStrList.Add('Version = ' + txtMakeSubVer.Text);
  aSaveStrList.Add('MajorVersion = ' + txtMakeMainVer.Text);
  aSaveStrList.SaveToFile(aFilePath);
  aSaveStrList.Clear;

  //写commonindex.ini
  aFilePath := aAppPath + '\commonindex.ini';
  aSaveStrList.Add('[Version]');
  aSaveStrList.Add('Version = ' + txtMakeSubVer.Text);
  aSaveStrList.Add('MajorVersion = ' + txtMakeMainVer.Text);
  aSaveStrList.Add('WebDown = ' + txtMakeWebDown.Text);
  aSaveStrList.SaveToFile(aFilePath);
  aSaveStrList.Clear;

  //写FileComment.ini
  aFilePath := aAppPath + '\FileComment.ini';
  for i:=0 to lvMakeSaveFile.Items.Count-1 do
    begin
    aListItem := lvMakeSaveFile.Items.Item[i];
    aSaveStrList.Add('[' + aListItem.Caption + ']');
    for j:=1 to aListItem.SubItems.Count-1 do
      begin
      if aListItem.SubItems.Strings[j] <> '' then
          aSaveStrList.Add(lvMakeSaveFile.Columns.Items[j+1].Caption + ' = ' +
                        aListItem.SubItems.Strings[j]);
      end;
    aSaveStrList.Add('');
    end;
  aSaveStrList.Add('[Version.cfg]');
  aSaveStrList.Add('Belong = -1');
  aSaveStrList.Add('CheckVersionMethod = 5');
  aSaveStrList.Add('LocalPath = %PROGRAMDIR%');
  aSaveStrList.Add('UpdateFileFlag = 1');
  aSaveStrList.Add('UpdateFileMethod = Last;Copy');
  aSaveStrList.Add('UpdateRelative = 0');
  aSaveStrList.SaveToFile(aFilePath);
  aSaveStrList.Clear;

  //运行IndexGen.exe
  aCmd := aAppPath + '\' + 'IndexGen ' +
              txtMakeSaveDir.Text + ' ' +
              aAppPath + '\index.txt ' + ' 1  . ' +
              aAppPath + '\CommonIndex.ini ' +
              aAppPath + '\FileComment.ini ' +
              aAppPath + '\info.txt';
  WinExecAndWait32(PChar(aCmd),1);

  //运行CompressDll.exe
  aCmd := aAppPath + '\' + 'CompressDll ' +
            '-l ' + aAppPath + '\index.txt ' +
            txtMakeSaveDir.Text + '\index.dat';
  WinExecAndWait32(PChar(aCmd),1);
  
  MessageDlg('完成！'+ #13#10 + '请在 ' + txtMakeSaveDir.Text +
        '目录查看相应文件和index.dat文件。',mtInformation, [mbOK], 0,);
end;

procedure TfrmMain.btnMakeModFileClick(Sender: TObject);
var
  frmAttrChange: TfrmAttrChange;
  aListItem: TListItem;
begin
  frmAttrChange := TfrmAttrChange.Create(Self);
  aListItem := lvMakeSaveFile.Selected;
  frmAttrChange.txtFileName.Text := aListItem.Caption;
  frmAttrChange.txtFilePath.Text := aListItem.SubItems.Strings[0];
  frmAttrChange.txtBelong.Text := aListItem.SubItems.Strings[1];
  frmAttrChange.txtCheckVersionMethod.Text := aListItem.SubItems.Strings[2];
  frmAttrChange.txtUpdateFileFlag.Text := aListItem.SubItems.Strings[3];
  frmAttrChange.txtLocalPath.Text := aListItem.SubItems.Strings[4];
  frmAttrChange.txtUpdateFileMethod.Text := aListItem.SubItems.Strings[5];
  frmAttrChange.txtRemotePath.Text := aListItem.SubItems.Strings[6];
  frmAttrChange.txtUpdateRelative.Text := aListItem.SubItems.Strings[7];

  if frmAttrChange.ShowModal() <> 1 then exit;

  aListItem.SubItems.Strings[1] := frmAttrChange.txtBelong.Text;
  aListItem.SubItems.Strings[2] := frmAttrChange.txtCheckVersionMethod.Text;
  aListItem.SubItems.Strings[3] := frmAttrChange.txtUpdateFileFlag.Text;
  aListItem.SubItems.Strings[4] := frmAttrChange.txtLocalPath.Text;
  aListItem.SubItems.Strings[5] := frmAttrChange.txtUpdateFileMethod.Text;
  aListItem.SubItems.Strings[6] := frmAttrChange.txtRemotePath.Text;
  aListItem.SubItems.Strings[7] := frmAttrChange.txtUpdateRelative.Text;
end;

procedure TfrmMain.lvMakeSaveFileDblClick(Sender: TObject);
begin
  if lvMakeSaveFile.Selected =nil then exit;
  btnMakeModFileClick(nil);
end;

procedure TfrmMain.btnUpdateCopyToMakeClick(Sender: TObject);
var
  i: integer;
  aFileList: TStringList;
begin
  if (cbUpdateMainVer.Text = '所有版本') or (cbUpdateSubVer.Text = '所有版本') then exit;
  btnMakeClearClick(nil);
  txtMakeMainVer.Text := cbUpdateMainVer.Text;
  txtMakeSubVer.Text := cbUpdateSubVer.Text;

  aFileList := TStringList.Create;
  for i:=0 to lvUpdatePack.Items.Count-1 do
      aFileList.Add(lvUpdatePack.Items.Item[i].SubItems.Strings[4]);
  AddMakeFileList(aFileList);
  aFileList.Destroy;
  tabMain.TabIndex := 2;
end;



end.
