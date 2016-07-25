/*****************************************************************************************
	broadcast window
	Copyright : Kingsoft 2002
	Author	:   Wooy(Wu yue)
	CreateTime:	2002-12-25
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "resource.h"

class KSwordOnLineSever;

class KBroadCastWnd
{
public:
	static bool	Create(HINSTANCE hModule, KSwordOnLineSever* pServer);	//创建窗口界面
	static void	Close();
private:
	KBroadCastWnd();
	void	OnSend();
	static LRESULT CALLBACK WndProc(HWND hWnd, unsigned int uMsg, unsigned int uParam, int nParam);//响应窗口消息
private:
	static KBroadCastWnd*	m_pSelf;
	HWND	m_hWnd;
	KSwordOnLineSever*	m_pServer;
	enum	{	IDD = IDD_BROADCAST };				//窗口资源ID
};
