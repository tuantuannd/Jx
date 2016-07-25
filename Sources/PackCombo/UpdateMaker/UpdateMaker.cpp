//---------------------------------------------------------------------------
//#include "vcl.h"
#include "stdafx.h"
#include "basepch0.h"
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("unitfrmMain.cpp", frmMain);
USEFORM("unitfrmAttrChange.cpp", frmAttrChange);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TfrmMain), &frmMain);
     Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}