/*****************************************************************************************
	Client Connection List Window
	Copyright : Kingsoft 2002
	Author	:   Wooy(Wu yue)
	CreateTime:	2002-12-24
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once
#include "resource.h"

class KSwordOnLineSever;

class KSwordOnLineSever;

class KClientConnectListWnd
{
public:
	static bool	Create(HINSTANCE hModule, KSwordOnLineSever* pServer);	//创建窗口界面
	static void	Close();
	void		UpdateData();
private:
	KClientConnectListWnd();
	void	OnInitWnd();
	void	OnExport();
	static LRESULT CALLBACK WndProc(HWND hWnd, unsigned int uMsg, unsigned int uParam, int nParam);//响应窗口消息
private:
	static KClientConnectListWnd* m_pSelf;
	HWND		m_hWnd;
	HWND		m_hListWnd;
	HINSTANCE	m_hModule;
	KSwordOnLineSever*	m_pServer;
	enum	{	IDD = IDD_CONNECT_INFO };		//窗口资源ID
};
