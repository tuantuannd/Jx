/*****************************************************************************************
//	界面--聊天窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-27
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "../Elem/WndMessage.h"
#include "../Elem/Wnds.h"
#include "UiFriendInterview.h"
#include "UiSysMsgCentre.h"
#include "UiSelColor.h"
#include <crtdbg.h>
#include "../UiSoundSetting.h"
#include "../../../core/src/coreshell.h"
#include "../../../core/src/GameDataDef.h"
#include "../../../Engine/src/Text.h"
#include "../../../Represent/iRepresent/iRepresentShell.h"
#include "../../../Engine/src/Text.h"
#include "../UiBase.h"
#include "UiMsgCentrePad.h"
extern iCoreShell*		g_pCoreShell;
extern iRepresentShell*	g_pRepresentShell;

KUiFriendInterview* KUiFriendInterview::m_pFirstSelf = NULL;
#define	SCHEME_INI		"你一句我一句聊天.ini"
#define	SEL_COLOR_MENU	1

char			KUiFriendInterview::m_szSelfName[32] = "";
int				KUiFriendInterview::m_nSelfNameLen = 0;
unsigned int	KUiFriendInterview::m_uLastTextColor = 0x00000000;

//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiFriendInterview* KUiFriendInterview::OpenWindow(KUiPlayerItem* pFriend)
{
	KUiFriendInterview* pWnd = NULL;
	if (pFriend)
	{
		pWnd = FindWindow(pFriend, true);
		if (pWnd)
		{
			pWnd->BringToTop();
			pWnd->Show();
		}
	}
	return pWnd;
}

void KUiFriendInterview::CloneTheme(KUiFriendInterview* pCopy)
{
	_ASSERT(pCopy);
	Clone(pCopy);
	m_MsgList	.Clone(&(pCopy->m_MsgList));
	m_MsgScroll	.Clone(&(pCopy->m_MsgScroll));
	m_InputWnd  .Clone(&(pCopy->m_InputWnd));
	m_SendBtn	.Clone(&(pCopy->m_SendBtn));
	m_ColorBtn	.Clone(&(pCopy->m_ColorBtn));
	m_CloseBtn	.Clone(&(pCopy->m_CloseBtn));
	m_FaceBtn	.Clone(&(pCopy->m_FaceBtn));
	m_FriendName.Clone(&(pCopy->m_FriendName));
	pCopy->m_SelfTextColor.Color_dw = m_SelfTextColor.Color_dw;
	pCopy->m_InputWnd.SetColor(m_SelfTextColor.Color_dw);
}

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KUiFriendInterview::KUiFriendInterview()
{
	m_pNextSelf = NULL;
	m_SelfTextColor.Color_dw = 0x00000000;
	memset(&m_FriendData, 0, sizeof(KUiPlayerItem));
}

KUiFriendInterview::~KUiFriendInterview()
{
	if (m_pFirstSelf == NULL)
	{
		KIniFile* pSetting = g_UiBase.GetPrivateSettingFile();
		if (pSetting)
		{
			char	szColor[16];
			m_SelfTextColor.Color_dw = m_uLastTextColor;
			sprintf(szColor, "%d,%d,%d", m_SelfTextColor.Color_b.r,
				m_SelfTextColor.Color_b.g, m_SelfTextColor.Color_b.b);
			pSetting->WriteString("UiConfig", "ChatColor", szColor);
			g_UiBase.ClosePrivateSettingFile(true);
		}
	}
}

int	KUiFriendInterview::DoesHaveVisibleWnd()
{
	KUiFriendInterview* pWnd = m_pFirstSelf;
	while (pWnd)
	{
		if (pWnd->IsVisible())
			return true;
		pWnd = pWnd->m_pNextSelf;
	}
	return false;
}

//--------------------------------------------------------------------------
//	功能：关闭窗口，同时可以选则是否删除对象实例
//--------------------------------------------------------------------------
void KUiFriendInterview::CloseWindow()
{
	while (m_pFirstSelf)
	{
		KUiFriendInterview* pWnd = m_pFirstSelf;
		m_pFirstSelf = m_pFirstSelf->m_pNextSelf;
		pWnd->Destroy();
	}
}

void KUiFriendInterview::CloseSelf()
{
	if (this == m_pFirstSelf)
	{
		m_pFirstSelf = this->m_pNextSelf;
	}
	else
	{
		KUiFriendInterview* pWnd = m_pFirstSelf;
		while(pWnd)
		{
			if (pWnd->m_pNextSelf == this)
			{
				pWnd->m_pNextSelf = this->m_pNextSelf;
				break;
			}
			pWnd = pWnd->m_pNextSelf;
		}
	}
	Destroy();
}

KUiFriendInterview* KUiFriendInterview::FindWindow(KUiPlayerItem* pFriend, bool bCreate)
{
	KUiFriendInterview *pWnd = NULL, *pPreWnd = NULL;
	if (pFriend)
	{
		pWnd = m_pFirstSelf;
		while(pWnd)
		{
			if (strcmp(pWnd->m_FriendData.Name, pFriend->Name) == 0 &&
				pWnd->m_FriendData.uId == pFriend->uId)
			{
				break;
			}
			else if (pWnd->IsRecyclable())
			{
				UiSoundPlay(UI_SI_WND_OPENCLOSE);
				pWnd->SetFriendData(pFriend);
				break;
			}
			pPreWnd = pWnd;
			pWnd = pWnd->m_pNextSelf;
		}	
	}
	if (pWnd && pPreWnd)
	{
		pPreWnd->m_pNextSelf = pWnd->m_pNextSelf;
		pWnd->m_pNextSelf = m_pFirstSelf;
		m_pFirstSelf = pWnd;
	}

	if (pWnd == NULL)
	{
		pWnd = new KUiFriendInterview;
		if (pWnd)
		{
			pWnd->Initialize();
			if (m_pFirstSelf)
			{
				m_pFirstSelf->CloneTheme(pWnd);
				pWnd->m_pNextSelf = m_pFirstSelf;
				m_pFirstSelf = pWnd;
			}
			else
			{
				m_pFirstSelf = pWnd;
				LoadScheme(NULL);
			}
			UiSoundPlay(UI_SI_WND_OPENCLOSE);
			pWnd->SetFriendData(pFriend);
		}
	}

	return pWnd;
}

void KUiFriendInterview::SetFriendData(KUiPlayerItem* pFriend)
{
	if (pFriend)
	{
		m_FriendData = *pFriend;
		m_FriendName.SetText(pFriend->Name);
	}
}

//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
int KUiFriendInterview::Initialize()
{
	AddChild(&m_MsgList);
	AddChild(&m_MsgScroll);
	m_MsgList.SetScrollbar(&m_MsgScroll);

	AddChild(&m_InputWnd);
	
	AddChild(&m_SendBtn);
	AddChild(&m_ColorBtn);
	AddChild(&m_CloseBtn);
	AddChild(&m_FaceBtn);
	AddChild(&m_FriendName);

	m_Style &= ~WND_S_VISIBLE;
	Wnd_AddWindow(this);

	return true;
}

//--------------------------------------------------------------------------
//	功能：载入窗口的界面方案
//--------------------------------------------------------------------------
void KUiFriendInterview::LoadScheme(const char* pScheme)
{
	if (m_pFirstSelf == NULL)
		return;
	if (pScheme == NULL)
	{
		char Scheme[128];
		g_UiBase.GetCurSchemePath(Scheme, 256);
		pScheme = Scheme;
	}

	KUiPlayerBaseInfo	Info;
	memset(&Info, 0, sizeof(KUiPlayerBaseInfo));
	if (g_pCoreShell)
	{
		g_pCoreShell->GetGameData(GDI_PLAYER_BASE_INFO, (int)&Info, 0);
		SetSelfName(Info.Name);
	}

	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\"SCHEME_INI, pScheme);
	if (Ini.Load(Buff))
	{
		m_pFirstSelf->Init(&Ini, "Main");
		m_pFirstSelf->m_MsgList	.Init(&Ini, "MsgList");
		m_pFirstSelf->m_MsgScroll.Init(&Ini, "MsgScroll");
		m_pFirstSelf->m_InputWnd.Init(&Ini, "Input");
		m_pFirstSelf->m_SendBtn.Init(&Ini, "SendBtn");
		m_pFirstSelf->m_ColorBtn.Init(&Ini, "ColorBtn");
		m_pFirstSelf->m_CloseBtn.Init(&Ini, "CloseBtn");
		m_pFirstSelf->m_FaceBtn.Init(&Ini, "FaceBtn");
		m_pFirstSelf->m_FriendName.Init(&Ini, "FriendName");
		Ini.GetString("MsgList", "MsgColor", "0,0,0", Buff, sizeof(Buff));
		m_uLastTextColor = (::GetColor(Buff) & 0xFFFFFF);
	}

	KIniFile* pSetting = g_UiBase.GetPrivateSettingFile();
	if (pSetting)
	{
		if (pSetting->GetString("UiConfig", "ChatColor", "0,0,0", Buff, sizeof(Buff)))
			m_uLastTextColor = (::GetColor(Buff) & 0xFFFFFF);
		g_UiBase.ClosePrivateSettingFile(false);
	}
	m_pFirstSelf->m_SelfTextColor.Color_dw = m_uLastTextColor;
	m_pFirstSelf->m_InputWnd.SetColor(m_uLastTextColor);

	KUiFriendInterview* pWnd = m_pFirstSelf->m_pNextSelf;
	while(pWnd)
	{
		m_pFirstSelf->CloneTheme(pWnd);
		pWnd = pWnd->m_pNextSelf;
	}
}

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
int KUiFriendInterview::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uMsg)
	{
	case WND_N_BUTTON_CLICK:
		if (uParam == (unsigned int)(KWndWindow*)&m_SendBtn)
			OnSend();
		else if (uParam == (unsigned int)(KWndWindow*)&m_CloseBtn)
			CloseSelf();
		else if (uParam == (unsigned int)(KWndWindow*)&m_FaceBtn)
			PopupFaceMenu();
		else if (uParam == (unsigned int)(KWndWindow*)&m_ColorBtn)
		{
			int x, y, nWidth;
			m_ColorBtn.GetAbsolutePos(&x, &y);
			m_ColorBtn.GetSize(&nWidth, NULL);
			KUiSelColor::PopupMenu((KWndWindow*)this, SEL_COLOR_MENU, x, y, nWidth);
		}
		break;
	case WND_N_EDIT_SPECIAL_KEY_DOWN:	//文本编辑窗口获得回车按键操作
		if (nParam == VK_RETURN)
		{
			OnSend();
			nRet = 1;
		}
		break;
	case WND_N_SCORLLBAR_POS_CHANGED:
		if (uParam == (unsigned int)(KWndWindow*)&m_MsgScroll)
			m_MsgList.SetFirstShowLine(nParam);
		break;
	case WND_M_MENUITEM_SELECTED:
		if (uParam == (unsigned int)(KWndWindow*)this)
		{
			if (HIWORD(nParam) == SEL_COLOR_MENU && short(LOWORD(nParam)) >= 0)
			{
				m_SelfTextColor.Color_dw = KUiSelColor::GetColor(LOWORD(nParam));
				m_uLastTextColor = m_SelfTextColor.Color_dw;
				m_InputWnd.SetColor(m_SelfTextColor.Color_dw);
			}
		}
		break;
	default:
		nRet =  KWndImage::WndProc(uMsg, uParam, nParam);
	}
	return nRet;
}

void KUiFriendInterview::PopupFaceMenu()
{
}

int	KUiFriendInterview::IsRecyclable()
{
	return (IsVisible() == 0 && m_MsgList.GetMsgCount() == 0);
}

void KUiFriendInterview::PaintWindow()
{
	KWndImage::PaintWindow();

	int	nWidth, nHeight;
	KRUShadow	Shadow;
	m_ColorBtn.GetSize(&nWidth, &nHeight);
	m_ColorBtn.GetAbsolutePos(&Shadow.oPosition.nX, &Shadow.oPosition.nY);
	Shadow.oEndPos.nX = Shadow.oPosition.nX + nWidth;
	Shadow.oEndPos.nY = Shadow.oPosition.nY + nHeight;
	Shadow.Color.Color_dw = m_SelfTextColor.Color_dw;
	g_pRepresentShell->DrawPrimitives(1, &Shadow, RU_T_SHADOW, true);
}

//--------------------------------------------------------------------------
//	功能：响应点击发送按钮
//--------------------------------------------------------------------------
void KUiFriendInterview::OnSend()
{
	char	szBuffer[300];
	KUiChatMessage	Msg;
	Msg.uColor = m_SelfTextColor.Color_dw;
	Msg.nContentLen = m_InputWnd.GetText(&Msg.szContent[0], sizeof(Msg.szContent), true);

	if (Msg.nContentLen)
	{
		memcpy(szBuffer, m_szSelfName, m_nSelfNameLen);
		szBuffer[m_nSelfNameLen] = ':';
		szBuffer[m_nSelfNameLen + 1] = KTC_ENTER;
		szBuffer[m_nSelfNameLen + 2] = KTC_COLOR;
		szBuffer[m_nSelfNameLen + 3] = m_SelfTextColor.Color_b.r;
		szBuffer[m_nSelfNameLen + 4] = m_SelfTextColor.Color_b.g;
		szBuffer[m_nSelfNameLen + 5] = m_SelfTextColor.Color_b.b;

		int nLen = m_nSelfNameLen + 6;
		memcpy(&szBuffer[nLen], Msg.szContent, Msg.nContentLen);
		nLen += Msg.nContentLen;
		m_MsgList.AddOneMessage(szBuffer, nLen);
		m_InputWnd.ClearText();

		g_pCoreShell->OperationRequest(GOI_CHAT_WORDS_TO_FRIEND,
			(unsigned int)(&Msg), (int)(&m_FriendData));
	}
}

void KUiFriendInterview::SetSelfName(const char* pszSelfName)
{
	if (pszSelfName)
	{
		m_nSelfNameLen = strlen(pszSelfName);
		if (m_nSelfNameLen < 32)
		{
			strcpy(m_szSelfName, pszSelfName);
		}
		else
		{
			strcpy(m_szSelfName, "[self]");
			m_nSelfNameLen = strlen(m_szSelfName);
		}
	}
}
