unit unitfrmPacView;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmPacView = class(TForm)
    alabMainVer: TLabel;
    txtMainVer: TEdit;
    alabSubVer: TLabel;
    txtSubVer: TEdit;
    alabPacFileName: TLabel;
    txtPacFileName: TEdit;
    alabPacFilePath: TLabel;
    txtPacFilePath: TEdit;
    dlgOpen: TOpenDialog;
    btnPacFilePath: TButton;
    btnClose: TButton;
    btnOK: TButton;
    procedure txtMainTextChange(Sender: TObject);
    procedure btnOKClick(Sender: TObject);
    procedure btnPacFilePathClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

implementation

{$R *.dfm}

procedure TfrmPacView.txtMainTextChange(Sender: TObject);
begin
  txtPacFileName.Text := ExtractFileName( txtPacFilePath.Text);
  btnOK.Enabled := (txtPacFileName.Text <> '') and (txtPacFileName.Text <> '');

end;
procedure TfrmPacView.btnOKClick(Sender: TObject);
begin
  self.ModalResult := 1;
end;

procedure TfrmPacView.btnPacFilePathClick(Sender: TObject);
begin
  if not dlgOpen.Execute then exit;
  txtPacFilePath.Text := dlgOpen.FileName;
  if ExtractFileExt(txtPacFilePath.Text) <> '.pac' then
      txtPacFilePath.Text := txtPacFilePath.Text + '.pac';
end;

procedure TfrmPacView.btnCloseClick(Sender: TObject);
begin
  close;
end;

end.
