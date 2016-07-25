// MainTest.cpp : Defines the entry point for the application.
//
#include "MainTest.h"


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	KMyApp	MyApp;

	if (MyApp.Init(hInstance))
		MyApp.Run();
	return 0;
}

KMyApp::KMyApp()
{
	pIme = new CIme;
}

BOOL KMyApp::GameInit()
{
	m_Draw.Mode(FALSE, 640, 480);
	if (!g_InitEngine())
		return FALSE;
	m_Font.Load("\\font\\lb20.fnt");
	m_Font.SetColor(0xFF,0x99,00);
	m_Font.SetLine(640,20);
	m_Canvas.Init(640, 480);
	pIme->EnableIme(); 
	return TRUE;
}

BOOL KMyApp::GameExit()
{
   return TRUE;
}

BOOL KMyApp::GameLoop()
{
	GetUserInput();
	Paint();
	return TRUE;	
}

void KMyApp::GetUserInput()
{
	m_Keyboard.UpdateState();
	m_Mouse.UpdateState();}

void KMyApp::Paint()
{

	m_Canvas.FillCanvas(0xFFFF);
    if (pIme->GetImeName())
	{
		m_Font.DrawText( 5,10,pIme->GetImeName());
		m_Font.DrawText(150,10, pIme->IfImeSharp()? "全角" : "半角");
		m_Font.DrawText(200,10,pIme->IfImeSymbol()? "中文标点" : "英文标点");
		char *szCompStr, *szCompReadStr, *szCandList;
		int ImeCurPos;
		pIme->GetImeInput(&szCompStr, &szCompReadStr,&ImeCurPos, &szCandList );
		m_Font.DrawText(0,50,szCompStr);
		m_Font.DrawText(0,100,szCompReadStr);
 		m_Font.DrawText(0,150,szCandList);
 		char szText[1024]  ;
		char* pszText;
		int MaxCount=100 ;
		int TextCursor ;

		pszText=szText;
		pIme->GetText(pszText,&MaxCount);
		TextCursor=pIme->GetCursorPos(); 
		m_Font.DrawText(0,200,szText); 
		m_Canvas.DrawLine(ImeCurPos*10,50,ImeCurPos*10,70,0x0000); 
		m_Canvas.DrawLine(TextCursor*10,200,TextCursor*10,220,0x0000); 
	}
 	m_Canvas.UpdateScreen();
}
LRESULT	KMyApp::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

 switch( uMsg ){
  case WM_KEYDOWN:
	switch( wParam ){
		case VK_F1:
			if( pIme )pIme->NextIme();
			break;
		case VK_F2:
			if( pIme )pIme->SharpIme(GetActiveWindow());
			break;
		case VK_F3:
			if( pIme )pIme->SymbolIme(GetActiveWindow());
			break;
		case VK_F4:
		case VK_F5:
			if( pIme )pIme->EnableIme();
			break;
		case VK_F6:
			if( pIme )pIme->DisableIme();
			break;
		case VK_ESCAPE:
			PostMessage( hWnd, WM_CLOSE, 0, 0 );
		break;
		}
   default:
	  if (!pIme->WndMsg(hWnd,uMsg,wParam,lParam))
		  return  KWin32App::MsgProc( hWnd, uMsg, wParam, lParam );
	  break;
	}

return 0;

}


