unit unitfrmUpdateView;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmUpdateView = class(TForm)
    alabMainVer: TLabel;
    alabVerFrom: TLabel;
    alabPacFileName: TLabel;
    alabPacFilePath: TLabel;
    txtMainVer: TEdit;
    txtVerFrom: TEdit;
    txtPacFileName: TEdit;
    txtPacFilePath: TEdit;
    btnPacFilePath: TButton;
    btnClose: TButton;
    btnOK: TButton;
    dlgOpen: TOpenDialog;
    alabVerTo: TLabel;
    txtVerTo: TEdit;
    procedure txtMainTextChange(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure btnPacFilePathClick(Sender: TObject);
    procedure btnOKClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;



implementation

{$R *.dfm}

procedure TfrmUpdateView.btnCloseClick(Sender: TObject);
begin
  close;
end;

procedure TfrmUpdateView.btnPacFilePathClick(Sender: TObject);
begin
  if not dlgOpen.Execute then exit;
  txtPacFilePath.Text := dlgOpen.FileName;
  if ExtractFileExt(txtPacFilePath.Text) <> '.pak' then
      txtPacFilePath.Text := txtPacFilePath.Text + '.pak';
end;

procedure TfrmUpdateView.txtMainTextChange(Sender: TObject);
begin
  txtPacFileName.Text := ExtractFileName( txtPacFilePath.Text);
  btnOK.Enabled := (txtPacFileName.Text <> '') and (txtPacFileName.Text <> '');
end;

procedure TfrmUpdateView.btnOKClick(Sender: TObject);
begin
  self.ModalResult := 1;
end;

end.
