unit unitfrmSubVer;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls,FileCtrl,unitFun,Math;

type
  TfrmSubVer = class(TForm)
    alabMainVer: TLabel;
    alabGameDir: TLabel;
    Label1: TLabel;
    txtMainVer: TEdit;
    txtGameDir: TEdit;
    btnGameDir: TButton;
    btnRun: TButton;
    btnClose: TButton;
    chkTestFinished: TCheckBox;
    txtPackSaveFile: TEdit;
    btnPackSaveFile: TButton;
    dlgSave: TSaveDialog;
    Label2: TLabel;
    txtPackBase: TEdit;
    txtSubVer: TEdit;
    alabSubVer: TLabel;
    procedure txtMainTextChange(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure btnGameDirClick(Sender: TObject);
    procedure btnPackSaveFileClick(Sender: TObject);
    procedure btnRunClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    mySubVer: integer;
  end;

implementation

{$R *.dfm}

procedure TfrmSubVer.txtMainTextChange(Sender: TObject);
begin
  btnRun.Enabled := (txtGameDir.Text <> '') and (txtMainVer.Text <> '') and (txtSubVer.Text <> '');
end;
procedure TfrmSubVer.btnCloseClick(Sender: TObject);
begin
  close;
end;

procedure TfrmSubVer.btnGameDirClick(Sender: TObject);
var
  aGamePath: string;
begin
  if SelectDirectory('选择游戏目录','',aGamePath) then
      txtGameDir.Text := aGamePath;
end;

procedure TfrmSubVer.btnPackSaveFileClick(Sender: TObject);
begin
  if not dlgSave.Execute then exit;
  txtPackSaveFile.Text := dlgSave.FileName;
  if ExtractFileExt(txtPackSaveFile.Text) <> '.pac' then
      txtPackSaveFile.Text := txtPackSaveFile.Text + '.pac';
end;

procedure TfrmSubVer.btnRunClick(Sender: TObject);
var
  aCmd: string;
  aSubVer: integer;
begin
  try
    aSubVer := StrToInt(txtSubVer.Text);
    if aSubVer < mySubVer then
      begin
      MessageDlg('请输入比以前大的版本号！'
              , mtError,[mbOk], 0);
      exit;
      end;
  except
    MessageDlg('副版本输入框需要填写数字！'
              , mtError,[mbOk], 0);
    exit;
  end;

  if FileExists(txtPackSaveFile.Text) then
    begin
    if MessageDlg(txtPackSaveFile.Text + #10#13 + '该文件已存在，是否覆盖？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
    end;

  aCmd := ExtractFilePath(ParamStr(0)) +'packUpdate.exe' +
          ' "' + txtPackBase.Text + '"' +
          ' "' + txtGameDir.Text + '"' +
          ' "' + txtPackSaveFile.Text + '"';
  if chkTestFinished.Checked then aCmd := aCmd + ' test';
  if MessageDlg('打包程序已经准备好，请确认。'
                 ,mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;

  WinExecAndWait32(PChar(aCmd),1);

  self.ModalResult := 1;
end;

end.
