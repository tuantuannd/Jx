//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "unitfrmAttrChange.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAttrChange *frmAttrChange;
//---------------------------------------------------------------------------
__fastcall TfrmAttrChange::TfrmAttrChange(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmAttrChange::btnCancelClick(TObject *Sender)
{
  Close();  
}
//---------------------------------------------------------------------------

void __fastcall TfrmAttrChange::btnOKClick(TObject *Sender)
{
  this->ModalResult = 1;  
}
//---------------------------------------------------------------------------

