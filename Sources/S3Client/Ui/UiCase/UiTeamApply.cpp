/*****************************************************************************************
//	界面--申请队伍界面
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-2-26
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "../elem/wnds.h"
#include "../Elem/WndMessage.h"
#include "../UiBase.h"
#include "UiTeamApply.h"
#include "../UiSoundSetting.h"
#include "../../../core/src/gamedatadef.h"
#include "../../../core/src/CoreShell.h"

extern iCoreShell*		g_pCoreShell;

#define	SCHEME_INI	"加入组队.ini"

KUiTeamApply* KUiTeamApply::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：如果窗口正被显示，则返回实例指针
//--------------------------------------------------------------------------
KUiTeamApply* KUiTeamApply::GetIfVisible()
{
	if (m_pSelf && m_pSelf->IsVisible())
		return m_pSelf;
	return NULL;
}

KUiTeamApply::KUiTeamApply()
{
	m_pDataList = NULL;
	m_nCount = 0;
}

KUiTeamApply::~KUiTeamApply()
{
	Clear();
}

//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiTeamApply* KUiTeamApply::OpenWindow()
{
	if (m_pSelf == NULL)
	{
		m_pSelf = new KUiTeamApply;
		if (m_pSelf)
			m_pSelf->Initialize();
	}
	if (m_pSelf)
	{
		UiSoundPlay(UI_SI_WND_OPENCLOSE);
		m_pSelf->m_RefuseBtn.CheckButton(
			g_pCoreShell->GetGameData(GDI_TEAM_REFUSE_INVITE_STATUS, 0, 0));
		g_pCoreShell->OperationRequest(GOI_TEAM_COLLECT_NEARBY_LIST, 0, 0);
		m_pSelf->BringToTop();
		m_pSelf->Show();
	}
	return m_pSelf;
}

//初始化
void KUiTeamApply::Initialize()
{
	AddChild(&m_RefuseBtn);
	AddChild(&m_ApplyBtn);
	AddChild(&m_NewBtn);
	AddChild(&m_CloseBtn);
	AddChild(&m_TeamList);
	AddChild(&m_ListScroll);
	AddChild(&m_RefreshBtn);
	m_TeamList.SetScrollbar(&m_ListScroll);

	Wnd_AddWindow(this);

	char Scheme[256];
	g_UiBase.GetCurSchemePath(Scheme, 256);
	LoadScheme(Scheme);

}

//--------------------------------------------------------------------------
//	功能：关闭窗口，同时可以选则是否删除对象实例
//--------------------------------------------------------------------------
void KUiTeamApply::CloseWindow()
{
	if (m_pSelf)
	{
		m_pSelf->Destroy();
		m_pSelf = NULL;
	}
}

void KUiTeamApply::LoadScheme(const char* pScheme)
{
	if (m_pSelf)
	{
		char		Buff[128];
		KIniFile	Ini;
		sprintf(Buff, "%s\\%s", pScheme, SCHEME_INI);
		if (Ini.Load(Buff))
		{
			m_pSelf->Init(&Ini, "Main");
			m_pSelf->m_RefuseBtn .Init(&Ini, "RefuseBtn");
			m_pSelf->m_ApplyBtn  .Init(&Ini, "ApplyBtn");
			m_pSelf->m_NewBtn    .Init(&Ini, "NewBtn");
			m_pSelf->m_CloseBtn  .Init(&Ini, "CloseBtn");
			m_pSelf->m_RefreshBtn.Init(&Ini, "RefreshBtn");
			m_pSelf->m_TeamList	.Init(&Ini, "TeamList");
			m_pSelf->m_ListScroll.Init(&Ini, "Scroll");
		}
	}
}

//自己新建立队伍
void KUiTeamApply::OnNewTeam()
{
	g_pCoreShell->OperationRequest(GOI_TEAM_NEW, 0, 0);
	CloseWindow();
}

//窗口函数
int KUiTeamApply::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uMsg)
	{
	case WND_N_BUTTON_CLICK:
		if (uParam == (unsigned int)(KWndWindow*)&m_NewBtn)
		{
			OnNewTeam();
		}
		else if (uParam == (unsigned int)(KWndWindow*)&m_RefuseBtn)
		{
			g_pCoreShell->OperationRequest(GOI_TRAM_REFUSE_INVITE, 0, nParam);
			m_ApplyBtn.Enable(!nParam);
		}
		else if (uParam == (unsigned int)(KWndWindow*)&m_ApplyBtn)
			OnApply();
		else if (uParam == (unsigned int)(KWndWindow*)&m_CloseBtn)
			CloseWindow();
		else if (uParam == (unsigned int)(KWndWindow*)&m_RefreshBtn)
		{
			g_pCoreShell->OperationRequest(GOI_TEAM_COLLECT_NEARBY_LIST, 0, 0);
		}
		break;
	case WND_N_SCORLLBAR_POS_CHANGED:
		if (uParam == (unsigned int)(KWndWindow*)&m_ListScroll)
			m_TeamList.SetTopItemIndex(nParam);
		break;
	default:
		nRet = KWndShowAnimate::WndProc(uMsg, uParam, nParam);
	}
	return nRet;
}

void KUiTeamApply::UpdateData(KUiTeamItem* pList, int nCount)
{
	if (m_pSelf == NULL)
		return;
	m_pSelf->Clear();
	if (pList && nCount)
	{		
		m_pSelf->m_pDataList = (KUiTeamItem*)malloc(sizeof(KUiTeamItem) * nCount);
		if (m_pSelf->m_pDataList)
		{
			memcpy(m_pSelf->m_pDataList, pList, sizeof(KUiTeamItem) * nCount);
			m_pSelf->m_nCount = nCount;
			m_pSelf->m_TeamList.SetContent((unsigned char*)m_pSelf->m_pDataList, nCount, sizeof(KUiPlayerItem),
				(char*)&(m_pSelf->m_pDataList->Leader.Name) - (char*)m_pSelf->m_pDataList);
			nCount = m_pSelf->m_TeamList.GetVisibleItemCount();
		}
	}
	int bRefused = m_pSelf->m_RefuseBtn.IsButtonChecked();
	m_pSelf->m_ApplyBtn.Enable(!bRefused);
}

void KUiTeamApply::OnApply()
{
	int nSel = m_TeamList.GetCurSel();
	if (nSel >= 0 && nSel < m_nCount)
	{
		g_pCoreShell->OperationRequest(GOI_TEAM_APPLY,
			(unsigned int)&m_pDataList[nSel], 0);
		CloseWindow();
	}
}

void KUiTeamApply::Clear()
{
	m_TeamList.SetContent(0, 0, 0, 0);
	if (m_pDataList)
	{
		free(m_pDataList);
		m_pDataList = NULL;
	}
	m_nCount = 0;
}
