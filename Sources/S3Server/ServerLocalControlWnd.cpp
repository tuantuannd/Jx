/*****************************************************************************************
	Sword online Server local control window
	Copyright : Kingsoft 2002
	Author	:   Wooy(Wu yue)
	CreateTime:	2002-12-19
*****************************************************************************************/
#include "KEngine.h"
#include "ServerLocalControlWnd.h"
#include "SwordOnLineServer.h"
#include "ClientConnectListWnd.h"
#include "BroadCastWnd.h"
#include "../Core/src/CoreServerShell.h"


KServerLocalControlWnd*	KServerLocalControlWnd::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
LRESULT CALLBACK KServerLocalControlWnd::WndProc(HWND hWnd, unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(uParam))
		{
		case ID_LAUNCH:
			if (m_pSelf)
				m_pSelf->OnLaunch();
			break;
		case ID_SHUTDOWN:
			if (m_pSelf && m_pSelf->m_pServer)
			{
				m_pSelf->m_pServer->ShutDown();
				UpdateData();
			}
			break;
		case ID_CLIENT_LIST:
			if (m_pSelf)
				KClientConnectListWnd::Create(
						m_pSelf->m_hModule, m_pSelf->m_pServer);
			break;
		case ID_SETTING:
			if (m_pSelf && m_pSelf->m_pServer)
				m_pSelf->m_pServer->OperationRequest(SSOI_RELOAD_WELCOME_MSG, 0, 0);
			break;
		case ID_BROADCAST:
			if (m_pSelf && m_pSelf->m_pServer)
				KBroadCastWnd::Create(
					m_pSelf->m_hModule, m_pSelf->m_pServer);
			break;
		case ID_EXIT:
			KClientConnectListWnd::Close();
			if (m_pSelf)
				m_pSelf->OnExit();
			else
				::DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
		break;
	}
	return 0;
}

void KServerLocalControlWnd::OnLaunch()
{
	if (m_pSelf->m_pServer)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, ID_LAUNCH), false);
		SetDlgItemText(m_hWnd, IDC_INFO, "Launching......");
		if (m_pServer->Launch())
		{
			::EnableWindow(::GetDlgItem(m_hWnd, ID_LAUNCH),		false);
			::EnableWindow(::GetDlgItem(m_hWnd, ID_SHUTDOWN),	true);
			::EnableWindow(::GetDlgItem(m_hWnd, ID_BROADCAST),	true);
			::EnableWindow(::GetDlgItem(m_hWnd, ID_CLIENT_LIST),	true);
		}
		UpdateData();
	}
	else
		SetDlgItemText(m_hWnd, IDC_INFO, "Haven't conected to the server!");
}

void KServerLocalControlWnd::SwitchLaunchable(bool bEnable)
{
	HWND	hWnd = ::GetDlgItem(m_hWnd, ID_LAUNCH);
	if (hWnd)
		::EnableWindow(hWnd, bEnable);
	hWnd = ::GetDlgItem(m_hWnd, ID_SHUTDOWN);
	if (hWnd)
		::EnableWindow(hWnd, !bEnable);
}

KServerLocalControlWnd::KServerLocalControlWnd()
{
	m_hWnd		= NULL;
	m_hModule = NULL;
	m_pServer	= NULL;
}

bool KServerLocalControlWnd::Create(HINSTANCE hModule, KSwordOnLineSever* pServer)
{
	if (m_pSelf)
		return true;
	if (hModule == NULL)
		return false;
	m_pSelf = new KServerLocalControlWnd;
	if (m_pSelf)
	{
		m_pSelf->m_hWnd = ::CreateDialog(hModule,
			MAKEINTRESOURCE(IDD), NULL, (DLGPROC)WndProc);
		if (m_pSelf->m_hWnd)
		{
			m_pSelf->m_hModule = hModule;
			m_pSelf->m_pServer = pServer;
			::ShowWindow(m_pSelf->m_hWnd, SW_SHOW);
		}
		else
		{
			delete (m_pSelf);
			m_pSelf = NULL;
		}
		return true;
	}
	return false;
}

void KServerLocalControlWnd::UpdateData()
{
	char	Status[128], Info[128];
	if (m_pSelf && m_pSelf->m_pServer)
	{
		m_pSelf->m_pServer->GetStatus(Status, Info);
		::SetDlgItemText(m_pSelf->m_hWnd, IDC_STATUS, Status);
		::SetDlgItemText(m_pSelf->m_hWnd, IDC_INFO, Info);
	}
}

void KServerLocalControlWnd::OnExit()
{
	if (m_pSelf)
	{
		if (m_hWnd)
		{
			::DestroyWindow(m_hWnd);
			m_hWnd = NULL;
		}
		m_pServer = NULL;
		m_hModule = NULL;
		delete (m_pSelf);
		m_pSelf = NULL;
	}
}
