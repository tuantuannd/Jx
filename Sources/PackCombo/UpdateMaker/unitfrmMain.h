//---------------------------------------------------------------------------

#ifndef unitfrmMainH
#define unitfrmMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <FileCtrl.hpp>
//---------------------------------------------------------------------------
class TfrmMain : public TForm
{
__published:	// IDE-managed Components
        TLabel *alabMainVer;
        TEdit *txtMainVer;
        TLabel *alabSubVer;
        TEdit *txtSubVer;
  TLabel *alabWebDown;
  TEdit *txtWebDown;
        TGroupBox *gbSaveFile;
        TButton *btnOpenSaveFile;
        TListView *lvSaveFile;
        TGroupBox *gbSaveDir;
        TEdit *txtSaveDir;
        TButton *btnSaveDir;
        TButton *btnRun;
        TButton *btnClose;
  TOpenDialog *dlgOpen;
        void __fastcall btnCloseClick(TObject *Sender);
  void __fastcall btnOpenSaveFileClick(TObject *Sender);
  void __fastcall btnSaveDirClick(TObject *Sender);
  void __fastcall lvSaveFileDblClick(TObject *Sender);
  void __fastcall btnRunClick(TObject *Sender);
  void __fastcall txtSaveDirChange(TObject *Sender);
private:	// User declarations
  FileAttribute FileList[100];
  int FileListCount;
  void RunAutoBat(const string &DataPath, const string &ToolsPath, const string &InfoPath, const string &IndexPath);
  void RunProcess(char *CommandLine);
public:		// User declarations
        __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
