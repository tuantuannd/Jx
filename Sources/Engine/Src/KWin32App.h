//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KWin32App.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	KWin32App Class
//---------------------------------------------------------------------------
#ifndef KWin32App_H
#define KWin32App_H
//---------------------------------------------------------------------------
#define SWORD_ICON 101

//---------------------------------------------------------------------------
class ENGINE_API KWin32App
{
protected:
	char	m_szClass[32];
	char	m_szTitle[32];
	BOOL	m_bShowMouse;
	BOOL	m_bActive;
	BOOL	m_bMultiGame;
	virtual	BOOL	InitClass(HINSTANCE hInstance);
	virtual	BOOL	InitWindow(HINSTANCE hInstance);
	virtual	BOOL	GameInit();
	virtual BOOL	GameLoop();
	virtual BOOL	GameExit();
	virtual int		HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }
public:
	KWin32App();
	virtual BOOL	Init(HINSTANCE hInstance,char* AppName="Sword3");
	virtual void	Run();
	virtual	void	ShowMouse(BOOL bShow);
	virtual void	SetMultiGame(BOOL bMulti);
	virtual LRESULT	MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	void			SetMouseHoverTime(unsigned int uHoverTime);
private:
	void			GenerateMsgHoverMsg();
	unsigned int	m_uMouseHoverTimeSetting;
	unsigned int	m_uMouseHoverStartTime;
	int				m_nLastMousePos;
	unsigned int	m_uLastMouseStatus;
};
//---------------------------------------------------------------------------
#endif
