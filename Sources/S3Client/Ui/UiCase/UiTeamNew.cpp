/*****************************************************************************************
//	界面--新建队伍界面
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-2-26
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "../elem/wnds.h"
#include "../Elem/WndMessage.h"
#include "../UiBase.h"
#include "UiTeamNew.h"
#include <crtdbg.h>
#include "../../../core/src/coreshell.h"
extern iCoreShell*		g_pCoreShell;

#define	TEAM_NAME_LEN_MAX	20
#define	TEAM_NAME_LEN_MIN	4
#define	SCHEME_INI	"新建队伍.ini"

KUiTeamNew* KUiTeamNew::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：如果窗口正被显示，则返回实例指针
//--------------------------------------------------------------------------
KUiTeamNew* KUiTeamNew::GetIfVisible()
{
	if (m_pSelf && m_pSelf->IsVisible())
		return m_pSelf;
	return NULL;
}

//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiTeamNew* KUiTeamNew::OpenWindow()
{
	if (m_pSelf == NULL)
	{
		m_pSelf = new KUiTeamNew;
		if (m_pSelf)
			m_pSelf->Initialize();
	}
	if (m_pSelf)
	{
		m_pSelf->BringToTop();
		Wnd_SetExclusive(m_pSelf);
		Wnd_SetFocusWnd(&m_pSelf->m_TeamName);
		m_pSelf->Show();
	}
	return m_pSelf;
}

//初始化
void KUiTeamNew::Initialize()
{
	AddChild(&m_TeamName);
	AddChild(&m_OkBtn);
	AddChild(&m_CancelBtn);

	Wnd_AddWindow(this);

	char Scheme[256];
	g_UiBase.GetCurSchemePath(Scheme, 256);
	LoadScheme(Scheme);

}

//--------------------------------------------------------------------------
//	功能：关闭窗口，同时可以选则是否删除对象实例
//--------------------------------------------------------------------------
void KUiTeamNew::CloseWindow()
{
	if (m_pSelf)
	{
		Wnd_ReleaseExclusive(m_pSelf);
		m_pSelf->Destroy();
		m_pSelf = NULL;
	}
}

void KUiTeamNew::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, SCHEME_INI);
	if (Ini.Load(Buff))
	{
		Init(&Ini, "Main");
		m_TeamName	.Init(&Ini, "TeamName");
		m_OkBtn		.Init(&Ini, "OkBtn");
		m_CancelBtn	.Init(&Ini, "CancelBtn");
	}
}

//窗口函数
int KUiTeamNew::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uMsg)
	{
	case WND_N_BUTTON_CLICK:
		if (uParam == (unsigned int)(KWndWindow*)&m_OkBtn)
			OnOk();
		else if (uParam == (unsigned int)(KWndWindow*)&m_CancelBtn)
			CloseWindow();
		break;
	case WND_N_EDIT_SPECIAL_KEY_DOWN:
		if (nParam == VK_RETURN)
		{
			if (uParam == (unsigned int)(KWndWindow*)&m_TeamName)
				OnOk();
		}
		else if (nParam == VK_ESCAPE)
		{
			CloseWindow();
			nRet = 1;
		}
		break;
	default:
		nRet = KWndShowAnimate::WndProc(uMsg, uParam, nParam);
	}
	return nRet;
}

void KUiTeamNew::OnOk()
{
	char	Name[32];
	if (!CheckInput(Name))
		return;
	g_pCoreShell->OperationRequest(GOI_TEAM_NEW, (unsigned int)Name, 0);
	CloseWindow();
}

bool KUiTeamNew::CheckInput(char* pName)
{
	_ASSERT(pName);
	int nLen = m_TeamName.GetText(pName, 32);
	if (nLen >= TEAM_NAME_LEN_MIN && nLen <= TEAM_NAME_LEN_MAX)
	{
		return true;
	}
	//	UIMessageBox("★毛主席★说：密码和账号不能太短也不能太长，要不革命会失败！", this);
	return false;
}