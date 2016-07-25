//---------------------------------------------------------------------------

#ifndef unitfrmAttrChangeH
#define unitfrmAttrChangeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfrmAttrChange : public TForm
{
__published:	// IDE-managed Components
  TLabel *Label1;
  TEdit *txtFileName;
  TLabel *Label2;
  TEdit *txtFilePath;
  TLabel *Label3;
  TEdit *txtBelong;
  TLabel *Label4;
  TEdit *txtCheckVersionMethod;
  TLabel *Label5;
  TLabel *Label6;
  TEdit *txtUpdateFileFlag;
  TLabel *Label7;
  TLabel *Label8;
  TEdit *txtLocalPath;
  TLabel *Label9;
  TLabel *Label10;
  TLabel *Label11;
  TLabel *Label12;
  TEdit *txtUpdateFileMethod;
  TLabel *Label13;
  TLabel *Label14;
  TEdit *txtRemotePath;
  TLabel *Label15;
  TLabel *Label16;
  TEdit *txtUpdateRelative;
  TButton *btnOK;
  TButton *btnCancel;
  void __fastcall btnCancelClick(TObject *Sender);
  void __fastcall btnOKClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TfrmAttrChange(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAttrChange *frmAttrChange;
//---------------------------------------------------------------------------
#endif
