/* 
 * File:     UiContainer.cpp
 * Desc:     用户面板容器类
 * Author:   flying
 * Creation: 2003/7/4
 */
//-----------------------------------------------------------------------------
#pragma warning(disable:4243)

#include "KWin32.h"
#include "KIniFile.h"
#include "../elem/wnds.h"
#include "../Elem/WndMessage.h"
#include <crtdbg.h>
#include "UiContainer.h"
#include "UiPlayerBar.h"
#include "UiStatus.h"
#include "UiSkillTree.h"
#include "UiEscDlg.h"
#include "UiSelColor.h"
#include "UiMsgCentrePad.h"
#include "UiMiniMap.h"
#include "../ShortcutKey.h"
#include "../UiBase.h"
#include "../UiSoundSetting.h"
#include "../../../core/src/gamedatadef.h"
#include "../../../core/src/MsgGenreDef.h"
#include "../../../core/src/CoreObjGenreDef.h"
#include "../../../core/src/coreshell.h"
#include "../../TextCtrlCmd/TextCtrlCmd.h"
#include "../../../Represent/iRepresent/iRepresentShell.h"
#include "../../../Engine/src/Text.h"

// flying add these include files.
#include "UiChatCentre.h"
#include "UiItem.h"
#include "UiSkills.h"
#include "UiTeamManage.h"
//#include "KCore.h"
//#include "../../../Core/Src/GameDataDef.h"

extern iCoreShell* g_pCoreShell;

KUiContainer::KUiContainer()
{
	pCurrentWnd = NULL;
	nCurrentSwitch = 0;
}

KUiContainer::~KUiContainer()
{
}

int KUiContainer::DoSwitchUi(int nSwitch)
{
	int nResult = false;

	if (nSwitch >= SWITCH_COUNT || nSwitch < 0)
		goto Exit0;
	if (pCurrentWnd != NULL && nCurrentSwitch != SWITCH_TO_MESSAGE)
	{
		if (pCurrentWnd->IsVisible())
		{
			//pCurrentWnd->CloseWindow(false);
			pCurrentWnd->Hide();
			goto Exit2;
		}
	}
	nCurrentSwitch = nSwitch;
	switch (nSwitch)
	{
	case SWITCH_TO_MESSAGE:
		pCurrentWnd = (KWndWindow *)KUiMsgCentrePad::OpenWindow();
		break;
	case SWITCH_TO_FRIEND:
		pCurrentWnd = (KWndWindow *)KUiChatCentre::OpenWindow(true);
		break;
	case SWITCH_TO_STATUS:
		pCurrentWnd = (KWndWindow *)KUiStatus::OpenWindow();
		break;
	case SWITCH_TO_ITEM:
		pCurrentWnd = (KWndWindow *)KUiItem::OpenWindow();
		break;
	case SWITCH_TO_SKILL:
		pCurrentWnd = (KWndWindow *)KUiSkills::OpenWindow();
		break;
	case SWITCH_TO_TEAM:
		pCurrentWnd = (KWndWindow *)KUiTeamManage::OpenWindow();
		break;
	default:
		goto Exit0;
	}
	goto Exit1;
Exit2:
	nCurrentSwitch = -1;
	pCurrentWnd = NULL;
Exit1:
	nResult = true;
Exit0:
	return nResult;
}

int KUiContainer::Initialize(KWndWindow* InitWnd)
{
	if (InitWnd == NULL)
		return false;
	pCurrentWnd = InitWnd;
	nCurrentSwitch = 0;
	return 1;
}

//int KUiContainer::RegisterWnds(KWndWindow* pWnd, int nMessage)
//{
//	int nResult = false;
//	if (nMessage >= SWITCH_COUNT || nMessage < 0)
//		goto Exit0;
//	if (pWnd == NULL)
//		goto Exit0;
//	pWnds[nMessage] = pWnd;
//	nResult = true;
//Exit0:
//	return nResult;
//}

