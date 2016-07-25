/*****************************************************************************************
//	界面--快速输入消息界面
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-12-11
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "../elem/wnds.h"
#include "../Elem/WndMessage.h"
#include "UiFastInputMsg.h"
#include "UiMsgCentrePad.h"
#include "../../TextCtrlCmd/TextCtrlCmd.h"

#include "../UiBase.h"
#include "../../../core/src/gamedatadef.h"
#include "../../../core/src/coreshell.h"
#include "../../../core/src/MsgGenreDef.h"
#include <crtdbg.h>

#include "../../../Represent/iRepresent/iRepresentShell.h"
#include "../../../Engine/src/Text.h"

extern iRepresentShell*	g_pRepresentShell;

extern iCoreShell*		g_pCoreShell;

#define	SCHEME_INI		"快速输入消息界面.ini"

#define		SWITCH_OPEN_CLOSE		1
#define		SEL_COLOR_MENU			1
#define		SEL_CHANNEL_MENU		2

KUiFastInputMsg* KUiFastInputMsg::m_pSelf = NULL;

KUiFastInputMsg::KUiFastInputMsg()
{
	m_pMenuData    = NULL;
	m_pChannelData = NULL;
	m_bChannelMenu =  false;
	m_bColorMenu   = false;
	m_bLocked = false;
}

//--------------------------------------------------------------------------
//	功能：如果窗口正被显示，则返回实例指针
//--------------------------------------------------------------------------
KUiFastInputMsg* KUiFastInputMsg::GetIfVisible()
{
	if (m_pSelf && m_pSelf->IsVisible())
		return m_pSelf;
	return NULL;
}

KUiFastInputMsg::~KUiFastInputMsg()
{
	Hide();
	if (m_bLocked == false)
		Wnd_UnRegisterHotKey(this, SWITCH_OPEN_CLOSE);
}

//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiFastInputMsg* KUiFastInputMsg::OpenWindow()
{
	if (m_pSelf)
		m_pSelf->Show();
	else
	{
		m_pSelf = new KUiFastInputMsg;
		if (m_pSelf)
			m_pSelf->Initialize();
	}
	return m_pSelf;
}

//--------------------------------------------------------------------------
//	功能：关闭窗口，同时可以选则是否删除对象实例
//--------------------------------------------------------------------------
void KUiFastInputMsg::CloseWindow(bool bDestroy)
{
	if (m_pSelf)
	{
		if (bDestroy == false)
			m_pSelf->Hide();
		else
		{
			KIniFile* pSetting = g_UiBase.GetPrivateSettingFile();
			if (pSetting)
			{
				char	Buffer[16];
				sprintf(Buffer, "%d,%d,%d", m_pSelf->m_CurColor.Color_b.r,
					m_pSelf->m_CurColor.Color_b.g, m_pSelf->m_CurColor.Color_b.b);
				pSetting->WriteString("UiConfig", "InputMsgColor", Buffer);
				g_UiBase.ClosePrivateSettingFile(true);
			}
			m_pSelf->Destroy();
			m_pSelf = NULL;
		}
	}
}

//显示窗口
void KUiFastInputMsg::Show()
{
	m_bChannelMenu =false;
	m_bColorMenu = false;
	Wnd_SetFocusWnd(&m_InputEdit);

	UpdateCurrentChannel();
	KWndImage::Show();
}

void KUiFastInputMsg::UpdateCurrentChannel()
{
	if (m_pSelf)
	{
		KUiChatChannel	Channel;
		memset(&Channel, 0, sizeof(KUiChatChannel));
		g_pCoreShell->GetGameData(GDI_CHAT_CURRENT_SEND_CHANNEL, 
			(unsigned int)&Channel, 0);
		m_pSelf->m_ChannelName.SetText(Channel.cTitle);
	}
}


//隐藏窗口
void KUiFastInputMsg::Hide()
{
	CancelMenu();
	KWndImage::Hide();
}

//初始化
void KUiFastInputMsg::Initialize()
{
	AddChild(&m_InputEdit);
	AddChild(&m_ChannelBtn);
	AddChild(&m_ColorBtn);
	AddChild(&m_SendBtn);
	AddChild(&m_ChannelName);

	char Buffer[256];
	g_UiBase.GetCurSchemePath(Buffer, 256);
	LoadScheme(Buffer);

	m_Style &= ~WND_S_VISIBLE;
	Wnd_AddWindow(this, WL_TOPMOST);

	KIniFile* pSetting = g_UiBase.GetPrivateSettingFile();
	if (pSetting)
	{
		pSetting->GetString("UiConfig", "InputMsgColor", "244,230,197", Buffer, sizeof(Buffer));
		m_CurColor.Color_dw = GetColor(Buffer);
		g_UiBase.ClosePrivateSettingFile(false);
	}
	else
	{
		m_CurColor.Color_b.r = 244;
		m_CurColor.Color_b.g = 230;
		m_CurColor.Color_b.b = 197;
	}
	m_CurColor.Color_b.a = 0;

	m_bLocked = false;
	Wnd_RegisterHotKey(this, SWITCH_OPEN_CLOSE, VK_RETURN, 0);
}

//载入界面方案
void KUiFastInputMsg::LoadScheme(const char* pScheme)
{
	if (m_pSelf)
	{
		char		Buff[128];
		KIniFile	Ini;
		sprintf(Buff, "%s\\%s", pScheme, SCHEME_INI);
		if (Ini.Load(Buff))
			m_pSelf->LoadScheme(&Ini);
	}
}

void KUiFastInputMsg::LoadScheme(class KIniFile* pIni)
{
	_ASSERT(pIni);
	Init(pIni, "Main");
	m_InputEdit	.Init(pIni, "InputEdit");
	m_ChannelBtn	.Init(pIni, "ChannelBtn");
	m_SendBtn	.Init(pIni, "SendBtn");
	m_ColorBtn	.Init(pIni, "ColorBtn");
	m_ChannelName.Init(pIni, "ChannelName");
	
	int	i;
	m_cNumColor = 0;
	char	Buffer[16], Value[4];
	for (i = 0; i < UI_INPUT_MSG_MAX_COLOR; i++)
	{		
		itoa(i, Value, 10);
		if (pIni->GetString("ColorList", Value, "", Buffer, sizeof(Buffer)))
		{
			m_ColorList[m_cNumColor] = GetColor(Buffer);
			m_ColorList[m_cNumColor] = (m_ColorList[m_cNumColor] & 0xFFFFFF);
			m_cNumColor ++;
		}
		else
			break;	
	}
}

//窗口函数
int KUiFastInputMsg::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WND_N_BUTTON_CLICK:
		if (uParam == (unsigned int)(KWndWindow*)&m_SendBtn)
			OnSend();			
		else if (uParam == (unsigned int)(KWndWindow*)&m_ChannelBtn)
		{
			if (m_bChannelMenu)
				CancelMenu();
			else
				PopupChannelMenu();
		}
		else if (uParam == (unsigned int)(KWndWindow*)&m_ColorBtn)
		{
			if (m_bColorMenu)
				CancelMenu();
			else
				PopupColorMenu();
		}
		break;
	case WND_N_EDIT_SPECIAL_KEY_DOWN:
		if (m_bLocked && nParam == VK_RETURN)
			OnSend();
		else if (nParam == VK_UP || nParam == VK_DOWN)
			InputRecentMsg(nParam == VK_UP);
	case WND_M_HOTKEY:
		if (uParam == SWITCH_OPEN_CLOSE)
		{
			if (IsVisible())
				OnSend();
			else
				Show();
		}
		break;
	case WND_N_EDIT_CHANGE:
		m_cPreMsgCounter = 0;
		break;
	case WND_M_MENUITEM_SELECTED:
		if (uParam == SEL_COLOR_MENU)
		{
			if (nParam >= 0 && nParam < m_cNumColor)
				m_CurColor.Color_dw = m_ColorList[nParam];
		}
		else if (uParam == SEL_CHANNEL_MENU && m_pChannelData && nParam >= 0)
		{
			g_pCoreShell->OperationRequest(GOI_SET_SEND_CHAT_CHANNEL,
				(unsigned int)&m_pChannelData[nParam], 0);
		}
		CancelMenu();
		break;
	case WM_SYSKEYDOWN:
		if (uParam == 'L')
		{
			SwitchLockMode();
			return 1;
		}
		break;
	default:
		return KWndImage::WndProc(uMsg, uParam, nParam);
	}
	return 0;
	
}

void KUiFastInputMsg::PaintWindow()
{
	KWndImage::PaintWindow();

	int	nWidth, nHeight;
	KRUShadow	Shadow;
	m_ColorBtn.GetSize(&nWidth, &nHeight);
	m_ColorBtn.GetAbsolutePos(&Shadow.oPosition.nX, &Shadow.oPosition.nY);
	Shadow.oEndPos.nX = Shadow.oPosition.nX + nWidth;
	Shadow.oEndPos.nY = Shadow.oPosition.nY + nHeight;
	Shadow.Color.Color_dw = m_CurColor.Color_dw;
	g_pRepresentShell->DrawPrimitives(1, &Shadow, RU_T_SHADOW, true);
}

void KUiFastInputMsg::PopupColorMenu()
{
	CancelMenu();
	m_pMenuData = (KPopupMenuData*)malloc(MENU_DATA_SIZE(UI_INPUT_MSG_MAX_COLOR));
	if (m_pMenuData == NULL)
		return;
	int i, j;
	m_pMenuData->bHaveHeadTailImg = false;
	m_ColorBtn.GetAbsolutePos(&i, &j);
	m_pMenuData->nX = i;
	m_pMenuData->nY = j;
	m_ColorBtn.GetSize(&i, &j);
	m_pMenuData->nItemWidth  = i;
	m_pMenuData->nItemHeight = j;
	m_pMenuData->bZoomSelectedItem = true;
	m_pMenuData->nFontSize = 12;
	m_pMenuData->nItemTitleIndent = 0;
	m_pMenuData->byItemTitleUpSpace = 0;
	m_pMenuData->nNumItem = UI_INPUT_MSG_MAX_COLOR;
	m_pMenuData->nSelectedItem = -1;
	m_pMenuData->uBorderLineColor = 0xff000000;
	m_pMenuData->uSelItemBgColor = 0;
	m_pMenuData->uSelTextColor = 0;
	m_pMenuData->uTextColor = 0;
	for (i = 0; i < UI_INPUT_MSG_MAX_COLOR; i++)
	{
		m_pMenuData->Items[i].szTitle[0] = 0;
		m_pMenuData->Items[i].uBgColor = m_ColorList[i];
	}
	KPopupMenu::Popup(m_pMenuData, (KWndWindow*)this, SEL_COLOR_MENU);
	m_bColorMenu = true;
}

void KUiFastInputMsg::PopupChannelMenu()
{
	CancelMenu();
	int nCount = g_pCoreShell->GetGameData(GDI_CHAT_SEND_CHANNEL_LIST, 0, 0);
	if (nCount <= 0)
		return;
	m_pChannelData = (KUiChatChannel*)malloc(sizeof(KUiChatChannel) * nCount);
	if (m_pChannelData == NULL)
		return;
	nCount = g_pCoreShell->GetGameData(GDI_CHAT_SEND_CHANNEL_LIST,
		(unsigned int)m_pChannelData, nCount);
	_ASSERT(nCount > 0);
	m_pMenuData = (KPopupMenuData*)malloc(MENU_DATA_SIZE(nCount));
	if (m_pMenuData == NULL)
	{
		free(m_pChannelData);
		m_pChannelData = NULL;
		return;
	}

	KPopupMenu::InitMenuData(m_pMenuData, nCount);
	int i, j;
	m_ChannelBtn.GetAbsolutePos(&i, &j);
	m_pMenuData->nX = i;
	m_pMenuData->nY = j;
	for (i = 0; i < nCount; i++)
		strcpy(m_pMenuData->Items[i].szTitle, m_pChannelData[i].cTitle);
	KPopupMenu::Popup(m_pMenuData, (KWndWindow*)this, SEL_CHANNEL_MENU);
	m_bChannelMenu = true;
}

void KUiFastInputMsg::CancelMenu()
{
	KPopupMenu::Cancel();
	if (m_pMenuData)
	{
		free(m_pMenuData);
		m_pMenuData = NULL;
	}
	m_bChannelMenu = false;
	m_bColorMenu = false;
	if (m_pChannelData)
	{
		free(m_pChannelData);
		m_pChannelData = NULL;
	}
}

void KUiFastInputMsg::OnSend()
{
	if (g_pCoreShell)
	{
		char	Buffer[512];
		KUiMsgParam	Param;
		Param.nMsgLength = m_InputEdit.GetText(Buffer, sizeof(Buffer));
		if (Param.nMsgLength)
		{
			memcpy(m_RecentMsg[m_cLatestMsgIndex], Buffer, Param.nMsgLength);
			m_RecentMsg[m_cLatestMsgIndex][Param.nMsgLength] = 0;
			m_cLatestMsgIndex = (m_cLatestMsgIndex + 1) % MAX_RECENT_MSG_COUNT;
			m_cPreMsgCounter = 0;
			if (TextMsgFilter(Buffer, Param.nMsgLength, Param.eGenre) == false)
			{
				strcpy(Param.szName, "自己");
				Param.cChatPrefixLen = 4;
				Param.cChatPrefix[0] = KTC_COLOR;
				Param.cChatPrefix[1] = m_CurColor.Color_b.r;
				Param.cChatPrefix[2] = m_CurColor.Color_b.g;
				Param.cChatPrefix[3] = m_CurColor.Color_b.b;
				if (Param.eGenre == MSG_G_CHAT)
				{
					Param.nMsgLength = TEncodeText(Buffer, Param.nMsgLength);
					KUiMsgCentrePad::MessageArrival((const char*)Buffer, &Param);
				}
				g_pCoreShell->OperationRequest(GOI_SEND_MSG, (unsigned int)Buffer, (int)&Param);
			}
		}
	}
	m_InputEdit.SetText("");
	if (m_bLocked == false)
		Hide();
}

void KUiFastInputMsg::SwitchLockMode()
{
	m_bLocked = !m_bLocked;
	if (m_bLocked)
		Wnd_UnRegisterHotKey(this, SWITCH_OPEN_CLOSE);
	else
		Wnd_RegisterHotKey(this, SWITCH_OPEN_CLOSE, VK_RETURN, 0);
}

void KUiFastInputMsg::InputRecentMsg(bool bPrior)
{
	int nCounter;
	if (bPrior)
		nCounter = m_cPreMsgCounter - 1;
	else
		nCounter = m_cPreMsgCounter + 1;
	if (nCounter < 0 && nCounter >= - MAX_RECENT_MSG_COUNT)
	{
		int nIndex = m_cLatestMsgIndex + nCounter;
		if (nIndex < 0)
			nIndex += 8;
		if (m_RecentMsg[nIndex][0])
		{
			m_InputEdit.SetText(m_RecentMsg[nIndex]);
			m_cPreMsgCounter = nCounter;
		}
	}
	else if (nCounter >= 0)
	{
		m_InputEdit.SetText("");
		m_cPreMsgCounter = 0;
	}
}

void KUiFastInputMsg::Clear()
{
	if (m_pSelf)
	{
		m_pSelf->m_cLatestMsgIndex = 0;
		m_pSelf->m_cPreMsgCounter = 0;
		for(int i = 0; i < MAX_RECENT_MSG_COUNT; i++)
			m_pSelf->m_RecentMsg[i][0] = 0;
	}
}