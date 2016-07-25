program PackPanel;

uses
  Forms,
  unitfrmMain in 'unitfrmMain.pas' {frmMain},
  unitfrmMainVer in 'unitfrmMainVer.pas' {frmMainVer},
  unitfrmSubVer in 'unitfrmSubVer.pas' {frmSubVer},
  unitfrmMakeUpdate in 'unitfrmMakeUpdate.pas' {frmMakeUpdate},
  unitFun in 'unitFun.pas',
  unitfrmPacView in 'unitfrmPacView.pas' {frmPacView},
  unitfrmUpdateView in 'unitfrmUpdateView.pas' {frmUpdateView},
  unitDataStruct in 'unitDataStruct.pas',
  unitfrmAttrChange in 'unitfrmAttrChange.pas' {frmAttrChange};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TfrmMain, frmMain);
  Application.Run;
end.
