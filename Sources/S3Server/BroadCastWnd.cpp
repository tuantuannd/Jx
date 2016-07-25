/*****************************************************************************************
	Broadcast message Window
	Copyright : Kingsoft 2002
	Author	:   Wooy(Wu yue)
	CreateTime:	2002-12-25
*****************************************************************************************/
#include "KEngine.h"
#include "BroadCastWnd.h"
#include "SwordOnLineServer.h"
#include <commctrl.h>
#include "../Core/src/CoreServerShell.h"
#include "../Engine/src/Text.h"

KBroadCastWnd* KBroadCastWnd::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
LRESULT CALLBACK KBroadCastWnd::WndProc(HWND hWnd, unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(uParam))
		{
		case IDOK:
			if (m_pSelf)
				m_pSelf->OnSend();
			break;
		case IDCANCEL:
			if (m_pSelf)
				Close();
			else
				::DestroyWindow(hWnd);
			break;
		}
		break;
	}
	return 0;
}

KBroadCastWnd::KBroadCastWnd()
{
	m_hWnd		= NULL;
	m_pServer	= NULL;
}

bool KBroadCastWnd::Create(HINSTANCE hModule, KSwordOnLineSever* pServer)
{
	if (m_pSelf)
	{
		BringWindowToTop(m_pSelf->m_hWnd);
		return true;
	}

	if (hModule == NULL)
		return false;
	m_pSelf = new KBroadCastWnd;
	if (m_pSelf)
	{		
		m_pSelf->m_hWnd = ::CreateDialog(hModule,
			MAKEINTRESOURCE(IDD), NULL, (DLGPROC)WndProc);
		if (m_pSelf->m_hWnd)
		{
			m_pSelf->m_pServer = pServer;
			::ShowWindow(m_pSelf->m_hWnd, SW_SHOW);
			return true;
		}
		else
		{
			delete (m_pSelf);
			m_pSelf = NULL;
		}
	}
	return false;
}

void KBroadCastWnd::Close()
{
	if (m_pSelf)
	{
		if (m_pSelf->m_hWnd)
		{
			::DestroyWindow(m_pSelf->m_hWnd);
			m_pSelf->m_hWnd = NULL;
		}
		m_pSelf->m_pServer = NULL;
		delete (m_pSelf);
		m_pSelf = NULL;
	}
}

void KBroadCastWnd::OnSend()
{
	if (m_pServer && m_hWnd)
	{
		char	Message[300] = "";
		int		nLen;
		nLen = ::GetDlgItemText(m_hWnd, IDC_MSG_EDIT, Message, sizeof(Message));
		::SetDlgItemText(m_hWnd, IDC_INFO, "");
		if (nLen > 0)
		{
			if (nLen > 256)
				::SetDlgItemText(m_hWnd, IDC_INFO, "Msg exceed max len.");
			else if (m_pServer->OperationRequest(SSOI_BROADCASTING, (unsigned int)Message, nLen))
			{
				::SetDlgItemText(m_hWnd, IDC_INFO, "Message has been delivered.");
				::SetDlgItemText(m_hWnd, IDC_MSG_EDIT, "");
			}
			else
			{
				::SetDlgItemText(m_hWnd, IDC_INFO, "Message deliver FAILED!");
			}
		}
	}
}