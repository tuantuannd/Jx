/*****************************************************************************************
//	界面--管理窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-3
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "../Elem/WndMessage.h"
#include "../elem/wnds.h"
#include "uimanage.h"
#include "../UiBase.h"
#include "../../../core/src/gamedatadef.h"
#include "../../../core/src/coreshell.h"

extern iCoreShell*		g_pCoreShell;

#define MANAGE_INI_SHEET			"管理主窗口.ini"
#define MANAGE_INI_CLIQUE			"帮派管理.ini"
#define MANAGE_INI_CONFRATERNITY	"帮会管理.ini"

//KUiManage	g_UiManage;
KUiManage* KUiManage::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
int KUiCliqueManage::Init()
{
	return true;
}

//--------------------------------------------------------------------------
//	功能：载入窗口的界面方案
//--------------------------------------------------------------------------
void KUiCliqueManage::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, MANAGE_INI_CLIQUE);
	if (Ini.Load(Buff))
	{
		KWndPage::Init(&Ini, "Main");
	}
}

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
int	KUiCliqueManage::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	return KWndPage::WndProc(uMsg, uParam, nParam);
}

//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
int KUiConfraternityManage::Init()
{
	return true;
}

//--------------------------------------------------------------------------
//	功能：载入窗口的界面方案
//--------------------------------------------------------------------------
void KUiConfraternityManage::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, MANAGE_INI_CONFRATERNITY);
	if (Ini.Load(Buff))
	{
		KWndPage::Init(&Ini, "Main");
	}
}

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
int	KUiConfraternityManage::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)//窗口函数
{
	return KWndPage::WndProc(uMsg, uParam, nParam);
}

//--------------------------------------------------------------------------
//	功能：如果窗口正被显示，则返回实例指针
//--------------------------------------------------------------------------
KUiManage* KUiManage::GetIfVisible()
{
	if (m_pSelf && m_pSelf->IsVisible())
		return m_pSelf;
	return NULL;
}

//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiManage* KUiManage::OpenWindow()
{
	if (m_pSelf)
		m_pSelf->Show();
	else
	{
		m_pSelf = new KUiManage;
		if (m_pSelf)
			m_pSelf->Initialize();
	}
	return m_pSelf;
}

//--------------------------------------------------------------------------
//	功能：关闭窗口，同时可以选则是否删除对象实例
//--------------------------------------------------------------------------
void KUiManage::CloseWindow(bool bDestroy)
{
	if (m_pSelf)
	{
		if (bDestroy == false)
			m_pSelf->Hide();
		else
		{
			m_pSelf->Destroy();
			m_pSelf = NULL;
		}
	}
}
//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
void KUiManage::Initialize()
{
	AddChild(&m_Close);
	AddChild(&m_LeaderShipLevel);
	AddChild(&m_LeaderShipExperience);
	
	m_TeamPad.Init();
	AddPage(&m_TeamPad, &m_TeamPadBtn);
	
	m_CliquePad.Init();
	AddPage(&m_CliquePad, &m_CliquePadBtn);

	m_ConfraternityPad.Init();
	AddPage(&m_ConfraternityPad, &m_ConfraternityPadBtn);

	//帮会管理和门派管理界面暂不开放。
	m_CliquePadBtn.Enable(false);
	m_ConfraternityPadBtn.Enable(false);

	char Scheme[256];
	g_UiBase.GetCurSchemePath(Scheme, 256);
	LoadScheme(Scheme);

	UpdateLeaderData();

	Wnd_AddWindow(this);
}

//--------------------------------------------------------------------------
//	功能：载入窗口的界面方案
//--------------------------------------------------------------------------
void KUiManage::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, MANAGE_INI_SHEET);
	if (m_pSelf && Ini.Load(Buff))
	{
		m_pSelf->Init(&Ini, "Main");
		m_pSelf->m_Close			.Init(&Ini, "CloseBtn");

		m_pSelf->m_TeamPadBtn	.Init(&Ini, "TeamBtn");
		m_pSelf->m_CliquePadBtn	.Init(&Ini, "CliqueBtn");
		m_pSelf->m_ConfraternityPadBtn.Init(&Ini, "ConfraternityBtn");

		m_pSelf->m_LeaderShipLevel     .Init(&Ini, "LeadLevel");
		m_pSelf->m_LeaderShipExperience.Init(&Ini, "LeadExp");

		Ini.Clear();

		m_pSelf->m_TeamPad.LoadScheme(pScheme);
		m_pSelf->m_CliquePad.LoadScheme(pScheme);
		m_pSelf->m_ConfraternityPad.LoadScheme(pScheme);
	}
}

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
int KUiManage::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	if (uMsg == WND_N_BUTTON_CLICK &&
		uParam == (unsigned int)(KWndWindow*)&m_Close)
	{
		Hide();
		return 0;
	}
	else
		return KWndPageSet::WndProc(uMsg, uParam, nParam);
}

//--------------------------------------------------------------------------
//	功能：队伍内容变更
//--------------------------------------------------------------------------
void KUiManage::UpdateTeamData(int nID)
{
	m_TeamPad.UpdateDataTeam(nID);
}

//--------------------------------------------------------------------------
//	功能：队伍成员变更
//--------------------------------------------------------------------------
void KUiManage::UpdateTeamChange(const KUiPlayerItem* playerItem,int bAdd)
{
	m_TeamPad.UpdateIncrementUpdateDataTeam(playerItem,bAdd);
}

//--------------------------------------------------------------------------
//	功能：队伍加入列表的变更
//--------------------------------------------------------------------------
void KUiManage::UpdateToJoinChange(const KUiPlayerItem* playerItem,int bAdd)
{
	m_TeamPad.UpdateIncrementUpdateDataJoinList(playerItem,bAdd);
}

//--------------------------------------------------------------------------
//	功能：玩家数值变更
//--------------------------------------------------------------------------
void KUiManage::UpdateLeaderData()
{
	KUiPlayerLeaderShip	Leader = { 0 };	
	g_pCoreShell->OperationRequest(GDI_PLAYER_LEADERSHIP, (unsigned int)&Leader, 0);
	m_LeaderShipLevel.SetIntText(Leader.nLeaderShipLevel);

	m_LeaderShipExperience.SetPart(Leader.nLeaderShipExperience,
		Leader.nLeaderShipExperienceFull);
}