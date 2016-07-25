// -------------------------------------------------------------------------
//	文件名		：	UiEquip.cpp
//	Author		：	吕桂华, Wooy(Wu yue)
//	创建时间	：	2002-9-10 16:22:58
// -------------------------------------------------------------------------
#include "KWin32.h"
#include "KIniFile.h"
#include "../elem/wnds.h"
#include "../elem/wndmessage.h"
#include "uiequip.h"
#include "../../../core/src/coreshell.h"
#include "../../../Core/Src/GameDataDef.h"
#include "../../../Engine/Src/Text.h"
#include "../UiBase.h"
#include <crtdbg.h>
#include "../UiSoundSetting.h"

extern iCoreShell*		g_pCoreShell;

#define SCHEME_INI_EQUIP	"装备.ini"
#define EQUIP_INI_PAGE		"装备分页.ini"
#define HORSE_INI_PAGE		"装备马分页.ini"

// -------------------------------------------------------------------------
// ---> 建立控件与UIEP_*以及可接纳物品的类型的对应关系
static struct UE_CTRL_MAP
{
	int				nPosition;
	const char*		pIniSection;
}CtrlItemMap[_ITEM_COUNT] =
{
	{ UIEP_HEAD,		"Cap"		},	//装备-帽子
	{ UIEP_HAND,		"Weapon"	},	//装备-武器
	{ UIEP_NECK,		"Necklace"	},	//装备-项链
	{ UIEP_FINESSE,		"Bangle"	},	//装备-手镯
	{ UIEP_BODY,		"Cloth"		},	//装备-衣服
	{ UIEP_WAIST,		"Sash"		},	//装备-腰带
	{ UIEP_FINGER1,		"Ring1"		},	//装备-戒指
	{ UIEP_FINGER2,		"Ring2"		},	//装备-戒指
	{ UIEP_WAIST_DECOR,	"Pendant"	},	//装备-腰缀
	{ UIEP_FOOT,		"Shoes"		}	//装备-鞋子
};

KUiEquip* KUiEquip::m_pSelf = NULL;

void KUiHorsePage::Initialize()
{
	AddChild(&m_HorseBox);
	AddChild(&m_UpDownHorseBtn);
	AddChild(&m_HorseDesc);
}

//载入界面方案
// -------------------------------------------------------------------------
// 功能	: 载入界面方案
// -------------------------------------------------------------------------
void KUiHorsePage::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, HORSE_INI_PAGE);
	if (Ini.Load(Buff))
	{
		KWndPage::Init(&Ini, "Main");
		m_HorseBox.Init(&Ini, "Horse");
		m_UpDownHorseBtn.Init(&Ini, "UpDownHorse");
		m_HorseDesc.Init(&Ini, "HorseDesc");
	}
}

//--------------------------------------------------------------------------
//	功能：装备变化更新
//--------------------------------------------------------------------------
void KUiHorsePage::UpdateEquip(KUiObjAtRegion* pEquip, int bAdd)
{
	if (pEquip && pEquip->Region.v == UIEP_HORSE)
	{
		UiSoundPlay(UI_SI_PICKPUT_ITEM);
		if (bAdd && pEquip->Obj.uGenre != CGOG_NOTHING)
		{
			m_HorseBox.HoldObject(pEquip->Obj.uGenre, pEquip->Obj.uId,
				pEquip->Region.Width, pEquip->Region.Height);
			m_UpDownHorseBtn.Enable(true);
			char	Desc[1024];
			int		nLen = 0;
			g_pCoreShell->GetGameData(GDI_GAME_OBJ_DESC, (unsigned int)pEquip, (int)&Desc);
			if (Desc[0])
				nLen = TEncodeText(Desc, strlen(Desc));
			m_HorseDesc.SetText(Desc, nLen);
		}
		else
		{
			Clear();
		}
	}
}

//更新骑马状态
void KUiHorsePage::UpdateRideStatus(int bRiding)
{
	m_UpDownHorseBtn.CheckButton(bRiding);
}

void KUiHorsePage::Clear()
{
	m_HorseBox.HoldObject(CGOG_NOTHING, 0, 0, 0);
	m_UpDownHorseBtn.CheckButton(false);
	m_HorseDesc.SetText("", 0);
}

// -------------------------------------------------------------------------
// 功能	: 窗口函数
// -------------------------------------------------------------------------
int	KUiHorsePage::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	int nRet = 0;
	if (uMsg == WND_N_ITEM_PICKDROP)
	{
		if (g_UiBase.IsOperationEnable(UIS_O_MOVE_ITEM))
				OnEquiptChanged((ITEM_PICKDROP_PLACE*)uParam, (ITEM_PICKDROP_PLACE*)nParam);
	}
	else if (uMsg == WND_N_BUTTON_CLICK && (KWndWindow*)uParam == (KWndWindow*)&m_UpDownHorseBtn)
	{
		g_pCoreShell->OperationRequest(GOI_PLAYER_ACTION, PA_RIDE, 0);
	}
	else
	{
		nRet = KWndPage::WndProc(uMsg, uParam, nParam);
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：响应界面操作引起装备的改变
//--------------------------------------------------------------------------
void KUiHorsePage::OnEquiptChanged(ITEM_PICKDROP_PLACE* pPickPos, ITEM_PICKDROP_PLACE* pDropPos)
{
	KUiObjAtContRegion	Drop, Pick;
	KUiDraggedObject	Obj;

	UISYS_STATUS	eStatus = g_UiBase.GetStatus();
	if (pPickPos)
	{
		_ASSERT(pPickPos->pWnd);
		m_HorseBox.GetObject(Obj);
		Pick.Obj.uGenre = Obj.uGenre;
		Pick.Obj.uId = Obj.uId;
		Pick.Region.Width = Obj.DataW;
		Pick.Region.Height = Obj.DataH;
		Pick.Region.h = 0;
		Pick.Region.v = UIEP_HORSE;
		Pick.eContainer = UOC_EQUIPTMENT;

		if (eStatus == UIS_S_TRADE_REPAIR)
		{
			g_pCoreShell->OperationRequest(GOI_TRADE_NPC_REPAIR,
				(unsigned int)(&Pick), 0);
			return;
		}
		else if (eStatus == UIS_S_TRADE_SALE)
		{
			g_pCoreShell->OperationRequest(GOI_TRADE_NPC_SELL,
				(unsigned int)(&Pick), 0);
			return;
		}
		else if (eStatus == UIS_S_TRADE_NPC || eStatus == UIS_S_TRADE_BUY)
		{			
			return;
		}
	}
	else if (pDropPos == NULL)
	{
		return;
	}

	if (pDropPos)
	{
		Wnd_GetDragObj(&Obj);
		Drop.Obj.uGenre = Obj.uGenre;
		Drop.Obj.uId = Obj.uId;
		Drop.Region.Width = Obj.DataW;
		Drop.Region.Height = Obj.DataH;
		Drop.Region.h = 0;
		Drop.Region.v = UIEP_HORSE;
		Drop.eContainer = UOC_EQUIPTMENT;
	}

	g_pCoreShell->OperationRequest(GOI_SWITCH_OBJECT,
		pPickPos ? (unsigned int)&Pick : 0,
		pDropPos ? (int)&Drop : 0);
}

// -------------------------------------------------------------------------
// 功能	: 初始化
// -------------------------------------------------------------------------
void KUiEquipPage::Initialize()
{
	for (int i = 0; i < _ITEM_COUNT; i ++)
	{
		m_EquipBox[i].SetObjectGenre(CGOG_ITEM);
		AddChild(&m_EquipBox[i]);
	}
}

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
void KUiEquipPage::Clear()
{
	for (int i = 0; i < _ITEM_COUNT; i ++)
		m_EquipBox[i].Celar();
}

//--------------------------------------------------------------------------
//	功能：装备变化更新
//--------------------------------------------------------------------------
void KUiEquipPage::UpdateEquip(KUiObjAtRegion* pEquip, int bAdd)
{
	if (pEquip)
	{
		for (int i = 0; i < _ITEM_COUNT; i++)
		{
			if (CtrlItemMap[i].nPosition == pEquip->Region.v)
			{
				if (bAdd)
					m_EquipBox[i].HoldObject(pEquip->Obj.uGenre, pEquip->Obj.uId,
						pEquip->Region.Width, pEquip->Region.Height);
				else
					m_EquipBox[i].HoldObject(CGOG_NOTHING, 0, 0, 0);
				break;
			}
		}
	}
}

// -------------------------------------------------------------------------
// 功能	: 载入界面方案
// -------------------------------------------------------------------------
void KUiEquipPage::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, EQUIP_INI_PAGE);
	if (Ini.Load(Buff))
	{
		if (g_pCoreShell->GetGameData(GDI_PLAYER_IS_MALE, 0, 0))
			KWndPage::Init(&Ini, "Male");
		else
			KWndPage::Init(&Ini, "Female");
		for (int i = 0; i < _ITEM_COUNT; i ++)
		{
			m_EquipBox[i].Init(&Ini, CtrlItemMap[i].pIniSection);
		}
	}
}

// -------------------------------------------------------------------------
// 功能	: 窗口函数
// -------------------------------------------------------------------------
int	KUiEquipPage::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WND_N_ITEM_PICKDROP:
		if (g_UiBase.IsOperationEnable(UIS_O_MOVE_ITEM))
			OnEquiptChanged((ITEM_PICKDROP_PLACE*)uParam, (ITEM_PICKDROP_PLACE*)nParam);
		break;
	default:
		return KWndPage::WndProc(uMsg, uParam, nParam);
	}
	return 0;
}

//--------------------------------------------------------------------------
//	功能：响应界面操作引起装备的改变
//--------------------------------------------------------------------------
void KUiEquipPage::OnEquiptChanged(ITEM_PICKDROP_PLACE* pPickPos, ITEM_PICKDROP_PLACE* pDropPos)
{
	KUiObjAtContRegion	Drop, Pick;
	KUiDraggedObject	Obj;
	KWndWindow*			pWnd = NULL;

	UISYS_STATUS	eStatus = g_UiBase.GetStatus();
	if (pPickPos)
	{
		_ASSERT(pPickPos->pWnd);
		((KWndObjectBox*)(pPickPos->pWnd))->GetObject(Obj);
		Pick.Obj.uGenre = Obj.uGenre;
		Pick.Obj.uId = Obj.uId;
		Pick.Region.Width = Obj.DataW;
		Pick.Region.Height = Obj.DataH;
		Pick.Region.h = 0;
		Pick.eContainer = UOC_EQUIPTMENT;
		pWnd = pPickPos->pWnd;

		if (eStatus == UIS_S_TRADE_REPAIR)
		{
			g_pCoreShell->OperationRequest(GOI_TRADE_NPC_REPAIR,
				(unsigned int)(&Pick), 0);
			return;
		}
		else if (eStatus == UIS_S_TRADE_SALE)
		{
			g_pCoreShell->OperationRequest(GOI_TRADE_NPC_SELL,
				(unsigned int)(&Pick), 0);
			return;
		}
		else if (eStatus == UIS_S_TRADE_NPC)
		{
			
			return;
		}
		else if (eStatus == UIS_S_TRADE_BUY)
			return;
	}
	else if (pDropPos)
	{
		pWnd = pDropPos->pWnd;
	}
	else
		return;

	if (pDropPos)
	{
		Wnd_GetDragObj(&Obj);
		Drop.Obj.uGenre = Obj.uGenre;
		Drop.Obj.uId = Obj.uId;
		Drop.Region.Width = Obj.DataW;
		Drop.Region.Height = Obj.DataH;
		Drop.Region.h = 0;
		Drop.eContainer = UOC_EQUIPTMENT;
	}

	for (int i = 0; i < _ITEM_COUNT; i++)
	{
		if (pWnd == (KWndWindow*)&m_EquipBox[i])
		{
			Drop.Region.v = Pick.Region.v = CtrlItemMap[i].nPosition;
			break;
		}
	}
	_ASSERT(i < _ITEM_COUNT);

	g_pCoreShell->OperationRequest(GOI_SWITCH_OBJECT,
		pPickPos ? (unsigned int)&Pick : 0,
		pDropPos ? (int)&Drop : 0);
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ---> 装备窗口
//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiEquip* KUiEquip::OpenWindow()
{
	if (m_pSelf == NULL)
	{
		m_pSelf = new KUiEquip;
		if (m_pSelf)
			m_pSelf->Initialize();
	}
	if (m_pSelf)
	{
		UiSoundPlay(UI_SI_WND_OPENCLOSE);
		m_pSelf->m_EquipPage.Clear();
		m_pSelf->m_HorsePage.Clear();
		
		KUiObjAtRegion	Equips[_ITEM_COUNT + 1];
		int nCount = g_pCoreShell->GetGameData(GDI_EQUIPMENT, (unsigned int)&Equips, 0);
		for (int i = 0; i < nCount; i++)
		{
			if (Equips[i].Obj.uGenre != CGOG_NOTHING)
			{
				if (Equips[i].Region.v != UIEP_HORSE)
					m_pSelf->m_EquipPage.UpdateEquip(&Equips[i], true);
				else
					m_pSelf->m_HorsePage.UpdateEquip(&Equips[i], true);
			}
		}
		m_pSelf->Show();		
		m_pSelf->BringToTop();
	}

	return m_pSelf;
}

//--------------------------------------------------------------------------
//	功能：如果窗口正被显示，则返回实例指针
//--------------------------------------------------------------------------
KUiEquip* KUiEquip::GetIfVisible()
{
	if (m_pSelf && m_pSelf->IsVisible())
		return m_pSelf;
	return NULL;
}

//--------------------------------------------------------------------------
//	功能：关闭窗口，同时可以选则是否删除对象实例
//--------------------------------------------------------------------------
void KUiEquip::CloseWindow(bool bDestroy)
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

// -------------------------------------------------------------------------
// 功能	: 初始化
// -------------------------------------------------------------------------
void KUiEquip::Initialize()
{
	m_EquipPage.Initialize();
	AddPage(&m_EquipPage, &m_Equip1Btn);
	m_HorsePage.Initialize();
	AddPage(&m_HorsePage, &m_Equip2Btn);
	
	AddChild(&m_Close);
	char Scheme[256];
	g_UiBase.GetCurSchemePath(Scheme, 256);
	LoadScheme(Scheme);

	Wnd_AddWindow(this);
}

// -------------------------------------------------------------------------
// 功能	: 载入界面方案
// -------------------------------------------------------------------------
void KUiEquip::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;

	sprintf(Buff, "%s\\%s", pScheme, SCHEME_INI_EQUIP);
	if (m_pSelf && Ini.Load(Buff))
	{
		m_pSelf->Init(&Ini, "Main");	// 窗口背景数据
		m_pSelf->m_Close.Init(&Ini, "Close");
		m_pSelf->m_Equip1Btn.Init(&Ini, "Equip1Btn");
		m_pSelf->m_Equip2Btn.Init(&Ini, "Equip2Btn");
		m_pSelf->m_EquipPage.LoadScheme(pScheme);
		m_pSelf->m_HorsePage.LoadScheme(pScheme);
	}
}

// -------------------------------------------------------------------------
// 功能	: 窗口函数
// -------------------------------------------------------------------------
int	KUiEquip::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	if (uMsg == WND_N_BUTTON_CLICK && (KWndWindow*)uParam == (KWndWindow*)&m_Close)
	{
		Hide();	// 关闭装备框
		return 0;
	}
	return KWndPageSet::WndProc(uMsg, uParam, nParam);
}

// -------------------------------------------------------------------------
// 功能	: 装备变化更新
// -------------------------------------------------------------------------
void KUiEquip::UpdateEquip(KUiObjAtRegion* pEquipt, int bAdd)
{
	if (pEquipt)
	{
		if (pEquipt->Region.v != UIEP_HORSE)
			m_EquipPage.UpdateEquip(pEquipt, bAdd);
		else
			m_HorsePage.UpdateEquip(pEquipt, bAdd);
	}
}

//更新骑马状态
void KUiEquip::UpdateRideStatus(int bRiding)
{
	if (m_pSelf)
		m_pSelf->m_HorsePage.UpdateRideStatus(bRiding);
}
