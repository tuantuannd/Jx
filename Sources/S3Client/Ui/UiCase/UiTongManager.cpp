/********************************************************************
File        : UiTongManager.cpp
Creator     : Fyt(Fan Zhanpeng)
create data : 08-29-2003(mm-dd-yyyy)
Description : 管理帮会的界面
*********************************************************************/

#include "KWin32.h"
#include "KIniFile.h"
#include "CoreShell.h"
#include "KPlayerDef.h"

#include "../elem/wnds.h"
#include "../elem/wndmessage.h"

#include "../UiBase.h"
#include "../UiSoundSetting.h"

#include "../../../Engine/src/KFilePath.h"

#include "UiPopupPasswordQuery.h"
#include "UiTongAssignBox.h"
#include "UiTongManager.h"
#include "UiInformation.h"
#include "UiTongGetString.h"

KUiTongManager* KUiTongManager::ms_pSelf = NULL;
KTongInfo       KUiTongManager::m_TongData;
int             KUiTongManager::m_nElderDataIndex = 0;
int             KUiTongManager::m_nMemberDataIndex = 0;
int             KUiTongManager::m_nCaptainDataIndex = 0;

#define TONG_MANAGER_INI "帮会管理界面.ini"
#define TONG_DATA_TEMP_FILE "帮会临时文件.ini"

#define TONG_REQUEST_INTERVAL 100000

extern iCoreShell* g_pCoreShell;
extern KUiInformation g_UiInformation;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KUiTongManager::KUiTongManager()
{
	m_szPassword[0] = 0;
	m_nSelectIndex = -1;
	m_nCurrentCheckBox = -1;
	m_TongData.szName[0] = 0;
	m_szTargetPlayerName[0] = 0;
	m_uLastRequestTongDataTime = 0;
	m_uLastRequestElderListTime = 0;
	m_uLastRequestMemberListTime = 0;
	m_uLastRequestCaptainListTime = 0;
}

KUiTongManager::~KUiTongManager()
{

}


/*********************************************************************
* 功能：打开窗口
**********************************************************************/
KUiTongManager* KUiTongManager::OpenWindow()
{
	if(g_pCoreShell)
	{
    	if (ms_pSelf == NULL)
    	{
		    ms_pSelf = new KUiTongManager;
		    if (ms_pSelf)
    			ms_pSelf->Initialize();
    	}
    	if (ms_pSelf)
    	{
		    UiSoundPlay(UI_SI_WND_OPENCLOSE);
			ms_pSelf->BringToTop();
			ms_pSelf->Show();
	    }
	}
	return ms_pSelf;
}


/*********************************************************************
* 功能：打开窗口，同时安排界面布局
**********************************************************************/
KUiTongManager* KUiTongManager::OpenWindow(char* pszPlayerName)
{
	OpenWindow();
	if(ms_pSelf)
	{
		ms_pSelf->ArrangeComposition(pszPlayerName);
	}
	return ms_pSelf;
}


/*********************************************************************
* 功能：根据传入的玩家名字，安排界面布局
**********************************************************************/
void KUiTongManager::ArrangeComposition(char* pszPlayerName)
{
	if(g_pCoreShell)
	{
    	KUiPlayerItem Player;
		int nKind;

    	ClearTongData();
    	memset(&Player, 0, sizeof(KUiPlayerItem));
    	if(pszPlayerName && pszPlayerName[0])
    	{
			if(g_pCoreShell->FindSpecialNPC(pszPlayerName, &Player, nKind))
				m_nIndex = Player.nIndex;
	    }
	    else
	    {
    		KUiPlayerBaseInfo Me;
		    g_pCoreShell->GetGameData(GDI_PLAYER_BASE_INFO, (unsigned int)&Me, 0);
			if(g_pCoreShell->FindSpecialNPC(Me.Name, &Player, nKind))
				m_nIndex = Player.nIndex;
	    }
		if(m_nIndex >= 0)
		{
			strcpy(m_szPlayerName, Player.Name);
	        ms_pSelf->UpdateBtnTheme(0, TRUE);

	        g_pCoreShell->TongOperation(GTOI_REQUEST_PLAYER_TONG, (unsigned int)&Player, (int)TRUE);
		}
		else
		{
			CloseWindow();
		}
	}
}


/*********************************************************************
* 功能：如果窗口正被显示，则返回实例指针
**********************************************************************/
KUiTongManager* KUiTongManager::GetIfVisible()
{
	if (ms_pSelf && ms_pSelf->IsVisible())
		return ms_pSelf;
	return NULL;
}


/*********************************************************************
* 功能：关闭窗口，同时可以选则是否删除对象实例
**********************************************************************/
void KUiTongManager::CloseWindow(bool bDestory)
{
	if (ms_pSelf)
	{
		ms_pSelf->Hide();
		if (bDestory)
		{
			ms_pSelf->Destroy();
			ms_pSelf = NULL;
		}
	}
}


/*********************************************************************
* 功能：初始化
**********************************************************************/
void KUiTongManager::Initialize()
{
	AddChild(&m_MemberCount);
	AddChild(&m_MasterName);
	AddChild(&m_Alignment);
	AddChild(&m_TongName);
	AddChild(&m_Money);

	AddChild(&m_BtnDismiss);
	AddChild(&m_BtnRecruit);
	AddChild(&m_BtnAssign);
	AddChild(&m_BtnDemise);
	AddChild(&m_BtnLeave);

	AddChild(&m_BtnCaptainList);
	AddChild(&m_BtnMemberList);
	AddChild(&m_BtnElderList);
	AddChild(&m_BtnCancel);
	AddChild(&m_BtnApply);

	AddChild(&m_List);
	AddChild(&m_ListScroll);
	m_List.SetScrollbar(&m_ListScroll);

	AddChild(&m_BtnRefresh);
	AddChild(&m_BtnClose);

	char Scheme[256];
	g_UiBase.GetCurSchemePath(Scheme, 256);
	LoadScheme(Scheme);

	Wnd_AddWindow(this);
}


/*********************************************************************
* 功能：载入界面方案
**********************************************************************/
void KUiTongManager::LoadScheme(const char* pScheme)
{
	if(ms_pSelf)
	{
		char		Buff[128];
		KIniFile	Ini;
		sprintf(Buff, "%s\\%s", pScheme, TONG_MANAGER_INI);

		if(Ini.Load(Buff))
		{
			ms_pSelf->Init(&Ini, "Main");

			ms_pSelf->m_TongName.Init(&Ini, "TextTongName");
	        ms_pSelf->m_MasterName.Init(&Ini, "TextMasterName");
	        ms_pSelf->m_Alignment.Init(&Ini, "Faction");
	        ms_pSelf->m_MemberCount.Init(&Ini, "MemberCount");
	        ms_pSelf->m_Money.Init(&Ini, "Money");

	        ms_pSelf->m_BtnDismiss.Init(&Ini, "BtnDismiss");
	        ms_pSelf->m_BtnAssign.Init(&Ini, "BtnAssign");
	        ms_pSelf->m_BtnDemise.Init(&Ini, "BtnDemise");
	        ms_pSelf->m_BtnLeave.Init(&Ini, "BtnLeave");
	        ms_pSelf->m_BtnRecruit.Init(&Ini, "BtnRecruit");

	        ms_pSelf->m_BtnApply.Init(&Ini, "BtnApply");
	        ms_pSelf->m_BtnCancel.Init(&Ini, "BtnCancel");
			ms_pSelf->m_BtnElderList.Init(&Ini, "BtnDirectorList");
	        ms_pSelf->m_BtnCaptainList.Init(&Ini, "BtnManagerList");
	        ms_pSelf->m_BtnMemberList.Init(&Ini, "BtnMemberList");

			ms_pSelf->m_List.Init(&Ini, "List");
			ms_pSelf->m_ListScroll.Init(&Ini, "ListScrollbar");
			
			ms_pSelf->m_BtnRefresh.Init(&Ini, "BtnRefresh");
			ms_pSelf->m_BtnClose.Init(&Ini, "BtnClose");

			Ini.GetString("Main", "StringDismiss", "", ms_pSelf->m_szDismiss, sizeof(ms_pSelf->m_szDismiss));
			Ini.GetString("Main", "StringAssign", "", ms_pSelf->m_szAssign, sizeof(ms_pSelf->m_szAssign));
			Ini.GetString("Main", "StringDemise", "", ms_pSelf->m_szDemise, sizeof(ms_pSelf->m_szDemise));
			Ini.GetString("Main", "StringLeave", "", ms_pSelf->m_szLeave, sizeof(ms_pSelf->m_szLeave));

			Ini.GetString("Main", "StringJustice", "", ms_pSelf->m_szJustice, sizeof(ms_pSelf->m_szJustice));
			Ini.GetString("Main", "StringBalance", "", ms_pSelf->m_szBalance, sizeof(ms_pSelf->m_szBalance));
			Ini.GetString("Main", "StringEvil", "", ms_pSelf->m_szEvil, sizeof(ms_pSelf->m_szEvil));

			Ini.GetString("Main", "StringConfirm", "", ms_pSelf->m_szConfirm, sizeof(ms_pSelf->m_szConfirm));
			Ini.GetString("Main", "StringCancel", "", ms_pSelf->m_szCancel, sizeof(ms_pSelf->m_szCancel));
		}
	}
}


/*********************************************************************
* 功能：窗口函数
**********************************************************************/
int KUiTongManager::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WND_N_LIST_ITEM_SEL:
		if(uParam == (unsigned int)&m_List)
		{
			if(nParam == -1)
			{
				m_BtnDismiss.Enable(FALSE);
				m_BtnAssign.Enable(FALSE);
				m_BtnDemise.Enable(FALSE);
			}
			else
			{
				m_BtnDismiss.Enable(TRUE);
				m_BtnAssign.Enable(TRUE);
				m_BtnDemise.Enable(TRUE);
			}
			m_nSelectIndex = nParam;
		}
		break;

	case WND_N_BUTTON_CLICK:
		if(uParam == (unsigned int)&m_BtnDismiss)
		{
			PopupConfirmWindow(m_szDismiss, RESULT_T_DISMISS);
		}
		else if(uParam == (unsigned int)&m_BtnAssign)
		{
			KPilgarlicItem *pNode = GetSelectedPlayer();

			KUiTongAssignBox::OpenWindow();
			KUiTongAssignBox::LinkToMainWindow(this);
			KUiTongAssignBox::ArrangeData(m_nRelation, pNode->Name, pNode->szAgname, m_nCurrentCheckBox, m_nIndex);
		}
		else if(uParam == (unsigned int)&m_BtnDemise)
		{
			PopupConfirmWindow(m_szDemise, RESULT_T_DEMISE);
		}
		else if(uParam == (unsigned int)&m_BtnLeave)
		{
			PopupConfirmWindow(m_szLeave, RESULT_T_LEAVE);
		}
		else if(uParam == (unsigned int)&m_BtnRecruit)
		{
			g_pCoreShell->TongOperation(GTOI_TONG_RECRUIT,
				                        m_BtnRecruit.IsButtonChecked(), 0);
		}
		else if(uParam == (unsigned int)&m_BtnApply)
		{
			OnAction(TONG_ACTION_APPLY);
		}
		else if(uParam == (unsigned int)&m_BtnCancel || uParam == (unsigned int)&m_BtnClose)
		{
			CloseWindow();
		}
		else if(uParam == (unsigned int)&m_BtnRefresh)
		{
			OnRefresh();
		}
		else if(uParam == (unsigned int)&m_BtnElderList)
		{
			UpdateListCheckButton(enumTONG_FIGURE_DIRECTOR);
			if(m_ElderData.GetCount() == 0)
			    LoadData(enumTONG_FIGURE_DIRECTOR);
		}
		else if(uParam == (unsigned int)&m_BtnCaptainList)
		{
			UpdateListCheckButton(enumTONG_FIGURE_MANAGER);
			if(m_CaptainData.GetCount() == 0)
			    LoadData(enumTONG_FIGURE_MANAGER);
		}
		else if(uParam == (unsigned int)&m_BtnMemberList)
		{
			UpdateListCheckButton(enumTONG_FIGURE_MEMBER);
			if(m_MemberData.GetCount() == 0)
			    LoadData(enumTONG_FIGURE_MEMBER);
		}
		break;

	case WND_M_OTHER_WORK_RESULT:
		Show();
		if(nParam)
		{
			strcpy(m_szTargetPlayerName, (char *)nParam);
		}
		if(uParam == RESULT_T_DISMISS && nParam)
		{
			OnAction(TONG_ACTION_DISMISS);
		}
		else if(uParam == RESULT_T_ASSIGN && nParam)
		{
			OnAction(TONG_ACTION_ASSIGN);
		}
		else if(uParam == RESULT_T_PASSWORD_TO_DEMISE)
		{
			strcpy(m_szPassword, (const char *)nParam);
			PopupConfirmWindow(m_szDemise, RESULT_T_DEMISE);
		}
		else if(uParam == RESULT_T_DEMISE)
		{
			if(nParam)
			{
			    OnAction(TONG_ACTION_DEMISE);
			}
			else
			{
				memset(m_szPassword, 0, sizeof(m_szPassword));
			}
		}
		else if(uParam == RESULT_T_LEAVE && nParam)
		{
			OnAction(TONG_ACTION_LEAVE);
		}
		m_szTargetPlayerName[0] = 0;
		break;

	case WND_N_SCORLLBAR_POS_CHANGED:
		if(uParam == (unsigned int)&m_ListScroll)
		{
    		m_List.SetTopItemIndex(nParam);
		}
		break;

	default:
		return KWndImage::WndProc(uMsg, uParam, nParam);
	}
	return 1;
}


/*********************************************************************
* 功能：更新按钮配置方案
**********************************************************************/
void KUiTongManager::UpdateBtnTheme(int nType, BOOL IsDissable)
{
	m_BtnDismiss.Hide();
	m_BtnRecruit.Hide();
	m_BtnAssign.Hide();
	m_BtnDemise.Hide();
	m_BtnLeave.Hide();

	m_BtnCaptainList.Hide();
	m_BtnMemberList.Hide();
	m_BtnElderList.Hide();
	m_BtnCancel.Hide();
	m_BtnApply.Hide();

	m_BtnRefresh.Hide();

	if(IsDissable)
	{
		return;
	}

	switch(nType)
	{
	case enumTONG_FIGURE_MASTER:
		m_BtnDismiss.Show();
		m_BtnRecruit.Show();
		m_BtnAssign.Show();
		m_BtnDemise.Show();

		m_BtnCaptainList.Show();
	    m_BtnMemberList.Show();
	    m_BtnElderList.Show();
		break;

	case enumTONG_FIGURE_DIRECTOR:
		m_BtnDismiss.Show();
		m_BtnRecruit.Show();
		m_BtnAssign.Show();
		m_BtnLeave.Show();

		m_BtnCaptainList.Show();
	    m_BtnMemberList.Show();
	    m_BtnElderList.Show();
		break;

	case enumTONG_FIGURE_MANAGER:
		m_BtnRecruit.Show();
		m_BtnLeave.Show();

		m_BtnElderList.Show();
		break;

	case enumTONG_FIGURE_MEMBER:
		m_BtnLeave.Show();

		m_BtnElderList.Show();
		break;

	default:
		m_BtnElderList.Show();
		m_BtnApply.Show();
		m_BtnCancel.Show();
		break;
	}

	m_BtnRefresh.Show();

	if(g_pCoreShell && nType >= enumTONG_FIGURE_MANAGER && nType <= enumTONG_FIGURE_MASTER)
	{
		KUiPlayerItem Player;
		memset(&Player, 0, sizeof(KUiPlayerItem));
		strcpy(Player.Name, m_szPlayerName);
		m_BtnRecruit.CheckButton(g_pCoreShell->TongOperation(GTOI_TONG_IS_RECRUIT, (unsigned int)&Player, 0));
	}
}


/*********************************************************************
* 功能：更新帮会讯息的显示
**********************************************************************/
void KUiTongManager::UpdateTongInfoView()
{
	char Buff[16];

	m_TongName.SetText(m_TongData.szName);
	m_MasterName.SetText(m_TongData.szMasterName);
	switch(m_TongData.nFaction)
	{
	case camp_justice:
		m_Alignment.SetText(m_szJustice);
		break;

	case camp_balance:
		m_Alignment.SetText(m_szBalance);
		break;

	case camp_evil:
		m_Alignment.SetText(m_szEvil);
		break;
	}
	m_MemberCount.SetText(itoa(m_TongData.nMemberCount + m_TongData.nManagerCount + m_TongData.nDirectorCount, Buff, 10));
	m_Money.SetText(itoa(m_TongData.nMoney, Buff, 10));
}


/*********************************************************************
* 功能：更新列表栏的显示
**********************************************************************/
void KUiTongManager::UpdateTongListView()
{
	KLinkStruct<KPilgarlicItem> *pData = GetCurrentSelectedData();

	if(pData)
	{
    	int nViewCount = m_List.GetCount();
    	int nDataCount = pData->GetCount();
		if(nDataCount > nViewCount)
		{
			KPilgarlicItem *pNode = pData->GetItem(nViewCount);
			if(pNode)
			{
				char Buff[64];
				for(int i = nViewCount;i < nDataCount;i++)
				{
					//	if(IsPlayerExist(pNode, pData))
					//	{
					//		continue;
					//	}
					sprintf(Buff, "%s %s", pNode->szAgname, pNode->Name);
					m_List.AddString(i, Buff);
					pNode = pData->NextItem();
				}
			}
		}
	}
}


/*********************************************************************
* 功能：返回当前所选择的列表类型所对应的数据集
**********************************************************************/
KLinkStruct<KPilgarlicItem>* KUiTongManager::GetCurrentSelectedData()
{
	switch(m_nCurrentCheckBox)
	{
	case enumTONG_FIGURE_DIRECTOR:
		return &m_ElderData;
		break;

	case enumTONG_FIGURE_MANAGER:
		return &m_CaptainData;
		break;

	case enumTONG_FIGURE_MEMBER:
		return &m_MemberData;
		break;

	default:
		return NULL;
		break;
	}
}


/*********************************************************************
* 功能：返回所选择的会员
**********************************************************************/
KPilgarlicItem* KUiTongManager::GetSelectedPlayer()
{
	KLinkStruct<KPilgarlicItem>* pData = NULL;
	KPilgarlicItem* pNode = NULL;

	pData = GetCurrentSelectedData();
	if(pData)
	{
    	pNode = pData->GetItem(m_nSelectIndex);
	}
	return pNode;
}


/*********************************************************************
* 功能：三个列表CheckBox的管理
**********************************************************************/
void KUiTongManager::UpdateListCheckButton(int nType)
{
   	m_BtnElderList.CheckButton(FALSE);
   	m_BtnMemberList.CheckButton(FALSE);
   	m_BtnCaptainList.CheckButton(FALSE);

   	switch(nType)
   	{
   	case enumTONG_FIGURE_DIRECTOR:
	    m_BtnElderList.CheckButton(TRUE);
	    break;

    case enumTONG_FIGURE_MANAGER:
   		m_BtnCaptainList.CheckButton(TRUE);
	    break;

    case enumTONG_FIGURE_MEMBER:
   		m_BtnMemberList.CheckButton(TRUE);
	    break;

	default:
		return;
    }
	if(nType != m_nCurrentCheckBox)
	{
		m_nCurrentCheckBox = nType;
		m_nSelectIndex = -1;
		m_List.ResetContent();
		UpdateTongListView();
	}
}


/*********************************************************************
* 功能：把帮会信息栏清空
**********************************************************************/
void KUiTongManager::ClearTongData()
{
	m_MemberCount.SetText("");
	m_MasterName.SetText("");
	m_Alignment.SetText("");
	m_TongName.SetText("");
	m_Money.SetText("");

	m_List.ResetContent();

	m_CaptainData.Clear();
	m_MemberData.Clear();
	m_ElderData.Clear();

	m_nElderDataIndex = 0;
	m_nMemberDataIndex = 0;
	m_nCaptainDataIndex = 0;
}


/*********************************************************************
* 功能：弹出确认窗口
**********************************************************************/
void KUiTongManager::PopupConfirmWindow(const char* pszInfo, unsigned int uHandleID)
{
	//UIMessageBox(pszInfo, this, m_szConfirm, m_szCancel, uHandleID);
	KPilgarlicItem *pPlayer = GetSelectedPlayer();
	if(pPlayer)
	    KUiTongGetString::OpenWindow(pszInfo, pPlayer->Name, this, uHandleID, 1, 31);
	else
		KUiTongGetString::OpenWindow(pszInfo, m_szPlayerName, this, uHandleID, 1, 31);
	Hide();
}


/*********************************************************************
* 功能：接受新的数据的数据接口
**********************************************************************/
void KUiTongManager::NewDataArrive(KUiGameObjectWithName *ParamInfo, KTongMemberItem *pIncome)
{
	KLinkStruct<KPilgarlicItem> *pData;
	int *pNum, nCount;

	if(ParamInfo->nData == enumTONG_FIGURE_DIRECTOR)
	{
		if(ms_pSelf)
		{
		    pData = &ms_pSelf->m_ElderData;
		}
		pNum = &m_nElderDataIndex;
		nCount = m_TongData.nDirectorCount;
	}
	else if(ParamInfo->nData == enumTONG_FIGURE_MANAGER)
	{
		if(ms_pSelf)
		{
		    pData = &ms_pSelf->m_CaptainData;
		}
		pNum = &m_nCaptainDataIndex;
		nCount = m_TongData.nManagerCount;
	}
	else if(ParamInfo->nData == enumTONG_FIGURE_MEMBER)
	{
		if(ms_pSelf)
		{
		    pData = &ms_pSelf->m_MemberData;
		}
		pNum = &m_nMemberDataIndex;
		nCount = m_TongData.nMemberCount;
	}
	else
	{
		return;
	}
	if(*pNum != ParamInfo->nParam)
	{
		return;
	}

	if(ParamInfo->uParam)
	{
		*pNum += ParamInfo->uParam;
		if(!strcmp(m_TongData.szName, ParamInfo->szName) && ms_pSelf)
		{
			KPilgarlicItem Add;

			for(int i = 0;i < ParamInfo->uParam;i++)
			{
				if(!pIncome[i].Name[0])
				{
					continue;
				}
				memcpy(&Add, pIncome + i, sizeof(KTongMemberItem));
				Add.nHashID = StringToHash(Add.Name);
				pData->AddItem(Add);
			}
			ms_pSelf->UpdateTongListView();
		}
		SaveNewData(ParamInfo, pIncome);
		if(*pNum < nCount)
		{
			RequestData(ParamInfo->nData, *pNum);
		}
	}
}


/*********************************************************************
* 功能：查询特定位置的前面，某个名字是否已经存在
**********************************************************************/
int KUiTongManager::IsPlayerExist(KPilgarlicItem *MeToCmp, KLinkStruct<KPilgarlicItem> *pData)
{
	unsigned int nID;
	KPilgarlicItem *pNode;

	nID = MeToCmp->nHashID;
	pNode = pData->Begin();
	while(pNode || pNode != MeToCmp)
	{
		if(pNode->nHashID == nID)
		{
			if(!strcmp(MeToCmp->Name, pNode->Name))
			{
				return 1;
			}
		}
		pNode = pData->NextItem();
	}
	return 0;
}


/*********************************************************************
* 功能：接受帮会信息
**********************************************************************/
void KUiTongManager::TongInfoArrive(KUiPlayerRelationWithOther* Relation, KTongInfo *pTongInfo)
{
	if(ms_pSelf && !strcmp(Relation->Name, ms_pSelf->m_szPlayerName))
	{
		memcpy(&ms_pSelf->m_TongData, pTongInfo, sizeof(KTongInfo));
		ms_pSelf->m_nRelation = Relation->nRelation;

		ms_pSelf->UpdateTongInfoView();
		ms_pSelf->UpdateBtnTheme(Relation->nRelation);
		if(Relation->nRelation <= enumTONG_FIGURE_MASTER && Relation->nRelation >= enumTONG_FIGURE_MEMBER)
		{
			ms_pSelf->m_Relation = (TONG_MEMBER_FIGURE)Relation->nRelation;
    		ms_pSelf->UpdateListCheckButton(enumTONG_FIGURE_DIRECTOR);
		}
	}
}


/*********************************************************************
* 功能：把新到的列表信息存到临时文件
**********************************************************************/
void KUiTongManager::SaveNewData(KUiGameObjectWithName *ParamInfo, KTongMemberItem *pIncome)
{
	char szSection[32];
	KIniFile Ini;

	if(ParamInfo->nData == enumTONG_FIGURE_DIRECTOR)
	{
		sprintf(szSection, "%s_Director", ParamInfo->szName);
	}
	else if(ParamInfo->nData == enumTONG_FIGURE_MANAGER)
	{
		sprintf(szSection, "%s_Manager", ParamInfo->szName);
	}
	else if(ParamInfo->nData == enumTONG_FIGURE_MEMBER)
	{
		sprintf(szSection, "%s_Member", ParamInfo->szName);
	}
	if(Ini.Load(TONG_DATA_TEMP_FILE))
	{
		int nCount, nDataCount;
		char szKey[32];

		Ini.GetInteger(szSection, "Count", 0, &nCount);
		nDataCount = nCount + ParamInfo->nParam;
		for(int i = nCount;i < nDataCount;i++)
		{
			sprintf(szKey, "%d_Name", i);
			Ini.WriteString(szSection, szKey, pIncome[i - nCount].Name);

			sprintf(szKey, "%d_AgName", i);
			Ini.WriteString(szSection, szKey, pIncome[i - nCount].szAgname);

			sprintf(szKey, "%d_Hash", i);
			Ini.WriteInteger(szSection, szKey, StringToHash(pIncome[i - nCount].Name));
		}
		Ini.Save(TONG_DATA_TEMP_FILE);
	}
}


/*********************************************************************
* 功能：把指定类型的列表数据整个存储起来
**********************************************************************/
void KUiTongManager::SaveWholeData(int nType)
{
	if(ms_pSelf && ms_pSelf->m_TongData.szName[0])
	{
    	KLinkStruct<KPilgarlicItem> *pData = NULL;
		char szSection[32];
		switch(nType)
		{
		case enumTONG_FIGURE_DIRECTOR:
			pData = &ms_pSelf->m_ElderData;
			sprintf(szSection, "%s_Director", ms_pSelf->m_TongData.szName);
			break;

		case enumTONG_FIGURE_MANAGER:
			pData = &ms_pSelf->m_CaptainData;
			sprintf(szSection, "%s_Manager", ms_pSelf->m_TongData.szName);
			break;

		case enumTONG_FIGURE_MEMBER:
			pData = &ms_pSelf->m_MemberData;
			sprintf(szSection, "%s_Member", ms_pSelf->m_TongData.szName);
			break;
		}
		if(pData)
		{
			KPilgarlicItem *pNode = pData->Reset();
			int nCount = pData->GetCount();
			char szKey[32];
			KIniFile Ini;
		/*----------------------------------*/
			Ini.Load(TONG_DATA_TEMP_FILE);
			Ini.EraseSection(szSection);
			for(int i = 0;i < nCount;i++)
			{
				sprintf(szKey, "%d_Name", i);
			    Ini.WriteString(szSection, szKey, pNode->Name);

			    sprintf(szKey, "%d_AgName", i);
			    Ini.WriteString(szSection, szKey, pNode->szAgname);

			    sprintf(szKey, "%d_Hash", i);
			    Ini.WriteInteger(szSection, szKey, pNode->nHashID);

				pNode = pData->NextItem();
			}
		}
	}
}


/*********************************************************************
* 功能：载入资料
**********************************************************************/
void KUiTongManager::LoadData(int nType)
{
	if(m_TongData.szName[0])
	{
    	char szSection[32];

	    switch(nType)
	    {
	    case enumTONG_FIGURE_DIRECTOR:
    		sprintf(szSection, "%s_Director", m_TongData.szName);
		    if(!(m_nElderDataIndex = LoadDataHandler(&m_ElderData, szSection)) &&
				m_nElderDataIndex < m_TongData.nDirectorCount)
			{
				m_nElderDataIndex = 0;
    			RequestData(enumTONG_FIGURE_DIRECTOR, m_nElderDataIndex);
			}
		    break;

	    case enumTONG_FIGURE_MANAGER:
    		sprintf(szSection, "%s_Manager", m_TongData.szName);
		    if(!(m_nCaptainDataIndex = LoadDataHandler(&m_CaptainData, szSection)))
			{
				m_nCaptainDataIndex = 0;
    			RequestData(enumTONG_FIGURE_MANAGER, m_nCaptainDataIndex);
			}
		    break;

	    case enumTONG_FIGURE_MEMBER:
    		sprintf(szSection, "%s_Member", m_TongData.szName);
		    if(!(m_nMemberDataIndex = LoadDataHandler(&m_MemberData, szSection)))
			{
				m_nMemberDataIndex = 0;
    			RequestData(enumTONG_FIGURE_MEMBER, m_nMemberDataIndex);
			}
		    break;

		default:
			break;
	    }
	}
}


/*********************************************************************
* 功能：载入资料的处理函数
**********************************************************************/
int KUiTongManager::LoadDataHandler(KLinkStruct<KPilgarlicItem> *pData,
									const char* szSection)
{
	char szFileName[256];
	KIniFile Ini;

	sprintf(szFileName, "%s\\%s", g_UiBase.GetUserTempDataFolder(), TONG_DATA_TEMP_FILE);
	if(Ini.Load(szFileName) && m_TongData.szName[0] && Ini.IsSectionExist(szSection))
	{
		int nCount, i;
		char szKey[32];
		KPilgarlicItem AddItem;

		pData->Clear();
		Ini.GetInteger(szSection, "Count", 0, &nCount);

		for(i = 0;i < nCount;i++)
		{
			sprintf(szKey, "%d_Name", i);
			Ini.GetString(szSection, szKey, "", AddItem.Name, sizeof(AddItem.Name));

			sprintf(szKey, "%d_AgName", i);
			Ini.GetString(szSection, szKey, "", AddItem.szAgname, sizeof(AddItem.szAgname));

			sprintf(szKey, "%d_Hash", i);
			Ini.GetInteger(szSection, szKey, 0, &AddItem.nHashID);

			pData->AddItem(AddItem);
		}
		UpdateTongListView();
		return nCount;
	}
	return 0;
}


/*********************************************************************
* 功能：向Core发出数据请求
**********************************************************************/
void KUiTongManager::RequestData(int nType, int nIndex)
{
	if(g_pCoreShell)
	{
    	KUiGameObjectWithName ParamInfo;
    	strcpy(ParamInfo.szName, m_TongData.szName);
    	ParamInfo.nData = nType;
    	ParamInfo.nParam = nIndex;
    	g_pCoreShell->TongOperation(GTOI_REQUEST_TONG_DATA, (unsigned int)&ParamInfo, 0);
	}
}


/*********************************************************************
* 功能：删除临时文件
**********************************************************************/
int KUiTongManager::RemoveTempFile()
{
	char szPath[256], szFile[128];

	sprintf(szFile, "%s\\%s", g_UiBase.GetUserTempDataFolder(), TONG_DATA_TEMP_FILE);
	g_GetFullPath(szPath, szFile);
	return remove(szPath);
}


/*********************************************************************
* 功能：各种帮会操作的处理函数
**********************************************************************/
void KUiTongManager::OnAction(int nType)
{
	if(g_pCoreShell)
	{
		KPilgarlicItem *pNode = GetSelectedPlayer();
		KTongOperationParam Param;
		KTongMemberItem Player;

		memset(&Player, 0, sizeof(KTongMemberItem));
		Param.eOper = (TONG_ACTION_TYPE)nType;
		if(nType == TONG_ACTION_LEAVE)
		{
			Param.nData[0] = m_nRelation;
			Param.nData[1] = -1;
		}
		else
		{
			Param.nData[0] = m_nCurrentCheckBox;
			Param.nData[1] = m_nIndex;
		}
		strcpy(Param.szPassword, m_szPassword);

		if(m_szTargetPlayerName[0])
		{
			strcpy(Player.Name, m_szTargetPlayerName);
		}
	    else if(pNode)
	    {
		    strcpy(Player.Name, pNode->Name);
	    }
		else
		{
			strcpy(Player.Name, m_szPlayerName);
		}
		g_pCoreShell->TongOperation(GTOI_TONG_ACTION, (unsigned int)&Param, (int)&Player);
		memset(m_szPassword, 0, sizeof(m_szPassword));
	}
}


/*********************************************************************
* 功能：响应刷新按钮的操作函数
**********************************************************************/
void KUiTongManager::OnRefresh()
{
	if(m_TongData.szName[0])
	{
		int nType;
		KIniFile Ini;
		char szSection[32];

		if(m_nCurrentCheckBox == enumTONG_FIGURE_DIRECTOR)
		{
			sprintf(szSection, "%s_Director", m_TongData.szName);
			m_ElderData.Clear();
			m_nElderDataIndex = 0;
		}
		else if(m_nCurrentCheckBox == enumTONG_FIGURE_MANAGER)
		{
			sprintf(szSection, "%s_Manager", m_TongData.szName);
			m_CaptainData.Clear();
			m_nCaptainDataIndex = 0;
		}
		else if(m_nCurrentCheckBox == enumTONG_FIGURE_MEMBER)
		{
			sprintf(szSection, "%s_Member", m_TongData.szName);
			m_MemberData.Clear();
			m_nMemberDataIndex = 0;
		}
		else
		{
			return;
		}
		nType = m_nCurrentCheckBox;

	    if(Ini.Load(TONG_DATA_TEMP_FILE))
	    {
    		Ini.EraseSection(szSection);
			Ini.Save(TONG_DATA_TEMP_FILE);
	    }

		m_List.ResetContent();
		LoadData(nType);
	}
}


/*********************************************************************
* 功能：对结果的响应入口
**********************************************************************/
void KUiTongManager::ResponseResult(KUiGameObjectWithName *pResult, int nbIsSucceed)
{
	if(ms_pSelf)
	{
    	switch(pResult->nData)
    	{
    	case TONG_ACTION_DISMISS:
			ms_pSelf->ResponseDismiss(pResult, nbIsSucceed);
		    break;

	    case TONG_ACTION_DEMISE:
			ms_pSelf->ResponseDemise(pResult, nbIsSucceed);
    		break;

    	case TONG_ACTION_LEAVE:
			ms_pSelf->ResponseLeave(pResult, nbIsSucceed);
		    break;

		case TONG_ACTION_ASSIGN:
			ms_pSelf->ResponseAssign(pResult, nbIsSucceed);
		    break;

		case TONG_ACTION_APPLY:
			ms_pSelf->ResponseApply(pResult, nbIsSucceed);
			break;
	    }
	}
	else
	{
		RemoveTempFile();
	}
}


/*********************************************************************
* 功能：响应离帮后的操作
**********************************************************************/
void KUiTongManager::ResponseLeave(KUiGameObjectWithName *pResult, int nbIsSucceed)
{
	if(nbIsSucceed)
	{
    	RemoveTempFile();
    	CloseWindow();
	}
}


/*********************************************************************
* 功能：响应踢人后的操作
**********************************************************************/
void KUiTongManager::ResponseDismiss(KUiGameObjectWithName *pResult, int nbIsSucceed)
{
	if(nbIsSucceed)
	{
		if(strcmp(pResult->szName, m_szPlayerName) == 0)
		{
			RemoveTempFile();
			CloseWindow();
		}
		else
		{
			KLinkStruct<KPilgarlicItem> *pForDelete;
			pForDelete = FindPlayer(pResult->szName);
			if(pForDelete)
			{
				pForDelete->RemoveItem();
				SaveWholeData(pResult->nParam);
				m_List.ResetContent();
				UpdateTongListView();
			}
		}
	}
}


/*********************************************************************
* 功能：响应传位后的操作
**********************************************************************/
void KUiTongManager::ResponseDemise(KUiGameObjectWithName *pResult, int nbIsSucceed)
{
	if(nbIsSucceed)
	{
		RemoveTempFile();
		ArrangeComposition(NULL);
	}
}


/*********************************************************************
* 功能：响应传位后的操作
**********************************************************************/
void KUiTongManager::ResponseAssign(KUiGameObjectWithName *pResult, int nbIsSucceed)
{
	if(nbIsSucceed)
	{
		if(!strcmp(pResult->szName, m_szPlayerName))
		{
			RemoveTempFile();
		    ArrangeComposition(NULL);
		}
		else
		{
			KLinkStruct<KPilgarlicItem> *pPlayer = FindPlayer(pResult->szName);
			if(pPlayer)
			{
				KPilgarlicItem Add, *pNode = pPlayer->Current();
				memcpy(&Add, pNode, sizeof(KPilgarlicItem));
				pPlayer->RemoveItem();
				switch(pResult->nParam)
				{
				case enumTONG_FIGURE_DIRECTOR:
					m_ElderData.AddItem(Add);
					break;

				case enumTONG_FIGURE_MANAGER:
					m_CaptainData.AddItem(Add);
					break;

				case enumTONG_FIGURE_MEMBER:
					m_MemberData.AddItem(Add);
					break;

				default:
					return;
				}
				SaveWholeData(pResult->nParam);
				SaveWholeData(pResult->uParam);
				m_List.ResetContent();
				UpdateTongListView();
			}
			//////////
		}
		/////////
	}
	////////
}


/*********************************************************************
* 功能：响应申请后的操作
**********************************************************************/
void KUiTongManager::ResponseApply(KUiGameObjectWithName *pResult, int nbIsSucceed)
{
	if(nbIsSucceed)
	{
		if(!strcmp(m_szPlayerName, pResult->szName))
		{
			RemoveTempFile();
			ArrangeComposition(NULL);
		}
		else
		{
			KPilgarlicItem Add;
			strcpy(Add.Name, pResult->szName);
			strcpy(Add.szAgname, pResult->szString);
			Add.nHashID = StringToHash(Add.Name);
			m_MemberData.AddItem(Add);
			SaveWholeData(enumTONG_FIGURE_MEMBER);
			m_List.ResetContent();
			UpdateTongListView();
		}
	}
}


/*********************************************************************
* 功能：查询特定名字的玩家的数据位置
**********************************************************************/
KLinkStruct<KPilgarlicItem>* KUiTongManager::FindPlayer(char *pszName)
{
	KLinkStruct<KPilgarlicItem> *pData = NULL;
	if(pszName && pszName[0])
	{
    	pData = HandleFind(&m_MemberData, pszName);
    	if(!pData)
    	{
		    pData = HandleFind(&m_CaptainData, pszName);
		    if(!pData)
		    {
    			pData = HandleFind(&m_ElderData, pszName);
		    }
	    }
	}
	return pData;
}


/*********************************************************************
* 功能：查找玩家处理函数
**********************************************************************/
KLinkStruct<KPilgarlicItem>* KUiTongManager::HandleFind(KLinkStruct<KPilgarlicItem> *pData, char *pszName)
{
	KLinkStruct<KPilgarlicItem> *pReturn = NULL;
	KPilgarlicItem *pNode = pData->Reset();
	int nID = StringToHash(pszName);
	int nCount = pData->GetCount();

	for(int i = 0;i < nCount;i++)
	{
		if(nID == pNode->nHashID)
		{
			if(strcmp(pszName, pNode->Name) == 0)
			{
				pReturn = pData;
				break;
			}
		}
		pNode = pData->NextItem();
	}
	return pReturn;
}


unsigned long StringToHash(const char *pString, BOOL bIsCaseSensitive)
{
	if(pString && pString[0])
	{
		unsigned long id = 0;
		const char *ptr;
		int index = 0;

		if(bIsCaseSensitive)
		{
			ptr = pString;

			while(*ptr)
			{
    			id = (id + (++index) * (*ptr)) % 0x8000000b * 0xffffffef;
		        ptr++;
	        }
		}
		else
		{
			char Buff[256];
			strcpy(Buff, pString);
			strlwr(Buff);
			ptr = Buff;

        	while(*ptr)
		    {
    			id = (id + (++index) * (*ptr)) % 0x8000000b * 0xffffffef;
		        ptr++;
	        }
		}
		return (id ^ 0x12345678);
	}

	return 0;
}
