
#include "kengine.h"
#include "KWin32Wnd.h"
//#include "KCore.h"
//#include "KMp3Music.h"
//#include "KSubWorldSet.h"
#include "ime.h"

class KMyApp : public KWin32App
{
private:
	KDirectDraw			m_Draw;
	KDirectInput		m_Input;
	KDirectSound		m_Sound;

	
	KMouse				m_Mouse;
	KKeyboard			m_Keyboard;
	KCanvas				m_Canvas;

    KFont				m_Font;

	CIme				* pIme;

public:
	KMyApp();
	BOOL				GameInit();
	BOOL				GameLoop();
	BOOL				GameExit();
	LRESULT				MsgProc(HWND hWnd, 
						UINT uMsg, 
						WPARAM wParam, 
						LPARAM lParam
					);

private:

	void				GetUserInput();	
	void				Paint();
	void				ResetWindow();
};
