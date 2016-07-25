program ManualUpdate;

uses
  Forms,
  unitfrmMain in 'unitfrmMain.pas' {frmMain},
  unitThreadCopyFile in 'unitThreadCopyFile.pas',
  enumStuff in 'enumStuff.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.Title := '剑侠情缘Online 手动更新程序';
  Application.CreateForm(TfrmMain, frmMain);
  Application.Run;
end.
