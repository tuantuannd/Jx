/*****************************************************************************************
	Sword online Server local control window
	Copyright : Kingsoft 2002
	Author	:   Wooy(Wu yue)
	CreateTime:	2002-12-19
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "resource.h"

class KSwordOnLineSever;

class KServerLocalControlWnd
{
public:
	static bool	Create(HINSTANCE hModule, KSwordOnLineSever* pServer);	//创建窗口界面
	static void UpdateData();
private:
	KServerLocalControlWnd();
	void	OnLaunch();
	void	OnExit();
	void	SwitchLaunchable(bool bEnable);
	static LRESULT CALLBACK WndProc(HWND hWnd, unsigned int uMsg, unsigned int uParam, int nParam);//响应窗口消息
private:
	static KServerLocalControlWnd*	m_pSelf;
	HWND	m_hWnd;
	HINSTANCE m_hModule;
	KSwordOnLineSever*	m_pServer;
	enum	{	IDD = IDD_LOCAL_CONTROL_WND };				//窗口资源ID
};
