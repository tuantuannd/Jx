/*****************************************************************************************
	Client Connection List Window
	Copyright : Kingsoft 2002
	Author	:   Wooy(Wu yue)
	CreateTime:	2002-12-24
*****************************************************************************************/
#include "KEngine.h"
#include "ClientConnectListWnd.h"
#include "SwordOnLineServer.h"
#include <commctrl.h>

KClientConnectListWnd* KClientConnectListWnd::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
LRESULT CALLBACK KClientConnectListWnd::WndProc(HWND hWnd, unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(uParam))
		{
		case ID_REFRESH:
			if (m_pSelf)
				m_pSelf->UpdateData();
			break;
		case ID_EXPORT:
			if (m_pSelf)
				m_pSelf->OnExport();
			break;
		case ID_EXIT:
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

KClientConnectListWnd::KClientConnectListWnd()
{
	m_hWnd		= NULL;
	m_hListWnd	= NULL;
	m_hModule	= NULL;
	m_pServer	= NULL;
}

bool KClientConnectListWnd::Create(HINSTANCE hModule, KSwordOnLineSever* pServer)
{
	if (m_pSelf)
	{
		BringWindowToTop(m_pSelf->m_hWnd);
		return true;
	}

	if (hModule == NULL)
		return false;
	m_pSelf = new KClientConnectListWnd;
	if (m_pSelf)
	{
		m_pSelf->m_hWnd = ::CreateDialog(hModule,
			MAKEINTRESOURCE(IDD), NULL, (DLGPROC)WndProc);
		if (m_pSelf->m_hWnd)
		{
			m_pSelf->m_hModule = hModule;
			m_pSelf->m_pServer = pServer;
			m_pSelf->OnInitWnd();
			m_pSelf->UpdateData();
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

void KClientConnectListWnd::OnInitWnd()
{
	m_hListWnd = GetDlgItem(m_hWnd, IDC_LIST);

	//设置文件列表框的属性与分栏信息
	ListView_SetExtendedListViewStyle(m_hListWnd,
		ListView_GetExtendedListViewStyle(m_hListWnd) |	LVS_EX_FULLROWSELECT);

	LVCOLUMN	lvc;
	char		szHeaderText[40];

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = szHeaderText;
	lvc.cchTextMax = 40;
	
	lvc.cx = 112;
	strcpy(szHeaderText, "Client Address");
	ListView_InsertColumn(m_hListWnd, 0, &lvc);

	lvc.mask |= LVCF_SUBITEM;
	lvc.cx = 200;
	lvc.iSubItem = 1;
	strcpy(szHeaderText, "Character Name");
	ListView_InsertColumn(m_hListWnd, 1, &lvc);

	::ShowWindow(m_pSelf->m_hWnd, SW_SHOW);
}

void KClientConnectListWnd::UpdateData()
{
	ListView_DeleteAllItems(m_hListWnd);
	SetDlgItemInt(m_hWnd, IDC_CLIENT_NUM, 0, false);
	SetDlgItemInt(m_hWnd, IDC_CHARACTER_NUM, 0, false);
	if (m_pServer && m_hListWnd)
	{
		int		nNumClient = 0;
		int		nNumCharacter = 0;
		int		nPos = 0;
		char	Buffer[32];
		LV_ITEM	lvi = { 0 };

		lvi.mask = LVIF_TEXT;
		ClientConnectInfo	Info;
		lvi.pszText			= Buffer;
		lvi.cchTextMax		= 16;
		while(m_pServer->GetClientConnectInfo(&Info, nPos))
		{
			strcpy(Buffer, Info.AddrInfo);
			lvi.iItem       = nNumClient;
			lvi.iItem = ListView_InsertItem(m_hListWnd, &lvi);
			if (Info.Character[0])
			{
				nNumCharacter++;
				ListView_SetItemText(m_hListWnd, lvi.iItem, 1, Info.Character);
			}
			nNumClient ++;			
		}
		SetDlgItemInt(m_hWnd, IDC_CLIENT_NUM, nNumClient, false);
		SetDlgItemInt(m_hWnd, IDC_CHARACTER_NUM, nNumCharacter, false);
	}
}

void KClientConnectListWnd::Close()
{
	if (m_pSelf)
	{
		m_pSelf->m_hListWnd = NULL;
		if (m_pSelf->m_hWnd)
		{
			::DestroyWindow(m_pSelf->m_hWnd);
			m_pSelf->m_hWnd = NULL;
		}
		m_pSelf->m_pServer = NULL;
		m_pSelf->m_hModule = NULL;
		delete (m_pSelf);
		m_pSelf = NULL;
	}
}

void KClientConnectListWnd::OnExport()
{
}