unit unitfrmMainVer;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls,FileCtrl,unitFun;

type
  TfrmMainVer = class(TForm)
    alabMainVer: TLabel;
    txtMainVer: TEdit;
    alabGameDir: TLabel;
    txtGameDir: TEdit;
    btnGameDir: TButton;
    btnRun: TButton;
    btnClose: TButton;
    chkTestFinished: TCheckBox;
    Label1: TLabel;
    txtPackSaveFile: TEdit;
    btnPackSaveFile: TButton;
    dlgSave: TSaveDialog;
    procedure txtGameDirChange(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure btnGameDirClick(Sender: TObject);
    procedure txtMainVerChange(Sender: TObject);
    procedure btnRunClick(Sender: TObject);
    procedure btnPackSaveFileClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;


implementation

{$R *.dfm}

procedure TfrmMainVer.txtMainVerChange(Sender: TObject);
begin
  btnRun.Enabled := (txtGameDir.Text <> '') and (txtMainVer.Text <> '');
end;

procedure TfrmMainVer.txtGameDirChange(Sender: TObject);
begin
  btnRun.Enabled := (txtGameDir.Text <> '') and (txtMainVer.Text <> '');  
end;

procedure TfrmMainVer.btnCloseClick(Sender: TObject);
begin
  close;
end;

procedure TfrmMainVer.btnGameDirClick(Sender: TObject);
var
  aGamePath: string;
begin
  if SelectDirectory('选择游戏目录','',aGamePath) then
      txtGameDir.Text := aGamePath;
end;

procedure TfrmMainVer.btnRunClick(Sender: TObject);
var
  aCmd: string;
begin
  if FileExists(txtPackSaveFile.Text) then
    begin
    if MessageDlg(txtPackSaveFile.Text + #10#13 + '该文件已存在，是否覆盖？',
                    mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;
    end;

  aCmd := ExtractFilePath(ParamStr(0)) +'packnew.exe' +
          ' "' + txtGameDir.Text + '"' +
          ' "' + txtPackSaveFile.Text + '"';
  if chkTestFinished.Checked then aCmd := aCmd + ' test';
  if MessageDlg('打包程序已经准备好，请确认。'
                 ,mtConfirmation, [mbYes, mbNo], 0) = mrNo then exit;

  WinExecAndWait32(PChar(aCmd),1);

  self.ModalResult := 1;
end;

procedure TfrmMainVer.btnPackSaveFileClick(Sender: TObject);
begin
  if not dlgSave.Execute then exit;
  txtPackSaveFile.Text := dlgSave.FileName;
  if ExtractFileExt(txtPackSaveFile.Text) <> '.pac' then
      txtPackSaveFile.Text := txtPackSaveFile.Text + '.pac';
end;

end.
