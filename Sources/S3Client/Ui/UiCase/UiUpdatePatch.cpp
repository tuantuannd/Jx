/*****************************************************************************************
//	界面--程序更新窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-12
*****************************************************************************************/
#include "KWin32.h"
#include "KIniFile.h"
#include "../Elem/WndMessage.h"
#include "../Elem/Wnds.h"
#include "../UiBase.h"
#include "../UiShell.h"
#include "UiInit.h"
#include "UiLoginBg.h"
#include "UiUpdatePatch.h"

#define	SCHEME_INI_UPDATAPATCH			"下载补钉.ini"
//define for PatchSehll Dll Name;
#define PATCHSHELL_LIB					"SUpdate.DLL"
//define for PatchSehll function Name;
#define PATCHSHELL_FN_UPDATE			"SwordUpdate"
#define PATCHSHELL_FN_GETRESULT			"SwordUpdateGetResult"
#define PATCHSHELL_FN_REGLAUNCHEAPPNAME	"RegisterAppLaunchedAfterUpdate"


KUiUpdataPatch* KUiUpdataPatch::m_pSelf = NULL;

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KUiUpdataPatch::KUiUpdataPatch()
{
	m_PatchShell = NULL;
	m_fnSwordUpdateGetResult = NULL;
	m_bQuit = false;
}

//--------------------------------------------------------------------------
//	功能：析构函数
//--------------------------------------------------------------------------
KUiUpdataPatch::~KUiUpdataPatch()
{
	m_pSelf = NULL;
	if (m_PatchShell)
		FreeLibrary(m_PatchShell);
}

//--------------------------------------------------------------------------
//	功能：打开窗口，返回唯一的一个类对象实例
//--------------------------------------------------------------------------
KUiUpdataPatch* KUiUpdataPatch::OpenWindow()
{
	if (m_pSelf == NULL)
	{
		m_pSelf = new KUiUpdataPatch;
		if (m_pSelf)
		{
			m_pSelf->Initialize();
			m_pSelf->Show();
		}
	}	
	return m_pSelf;
}

//--------------------------------------------------------------------------
//	功能：关闭窗口
//--------------------------------------------------------------------------
void KUiUpdataPatch::CloseWindow()
{
	if (m_pSelf)
	{
		m_pSelf->Destroy();
		m_pSelf = NULL;
	}
}

//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
int KUiUpdataPatch::Initialize()
{
	AddChild(&m_MsgList);
	AddChild(&m_MsgScroll);
	AddChild(&m_CancelBtn);
	AddChild(&m_StatusText);
	AddChild(&m_Progress);
	m_MsgList.SetScrollbar(&m_MsgScroll);

	char Scheme[256];
	g_UiBase.GetCurSchemePath(Scheme, 256);
	m_pSelf->LoadScheme(Scheme);
	Wnd_AddWindow(this, true);

	//to be delete
	{
		SetCloseParam("★★点确定按钮继续！★★", false, true);
		return true;
	}
	
	if (m_PatchShell == NULL)
		m_PatchShell =LoadLibrary(PATCHSHELL_LIB);
	if (m_PatchShell == NULL)
	{
		SetCloseParam("无法加载升级程序，点确定退出。", true, true);
		return false;
	}
	fnSwordUpdate	fnUpDate;
	fnUpDate=(fnSwordUpdate)GetProcAddress(m_PatchShell,PATCHSHELL_FN_UPDATE);
	m_fnSwordUpdateGetResult=(fnSwordUpdateGetResult)GetProcAddress(m_PatchShell,PATCHSHELL_FN_GETRESULT);
	if (fnUpDate == NULL || m_fnSwordUpdateGetResult==NULL)
	{
		SetCloseParam("升级程序版本内容不对，点确定退出。", true, true);
		return false;
	}
	if(fnUpDate(0,0) != SWORDUPDATE_R_LAUCHING)
	{
		SetCloseParam("可能升级程序已经运行，点确定退出。", true, true);
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------
//	功能：载入界面方案
//--------------------------------------------------------------------------
void KUiUpdataPatch::LoadScheme(const char* pScheme)
{
	char		Buff[128];
	KIniFile	Ini;
	sprintf(Buff, "%s\\%s", pScheme, SCHEME_INI_UPDATAPATCH);
	if (Ini.Load(Buff))
	{
		KWndShowAnimate::Init(&Ini, "Main");
		int	nFrame;
		Ini.GetInteger("Main", "LoginBgFrame", 0, &nFrame);
		KUiLoginBackGround::SetFrame(nFrame);
		m_MsgList  .Init(&Ini, "MsgList");
		m_MsgScroll.Init(&Ini, "MsgScroll");
		m_OkBtn	   .Init(&Ini, "OkBtn");
		m_CancelBtn   .Init(&Ini, "CancelBtn");
		m_StatusText  .Init(&Ini, "StatusText");
		m_Progress .Init(&Ini, "Progress");
		m_MsgList  .SetCapability(100);

		//to be delete
		{
			m_MsgList.AddOneMessage("------------------------", -1);
			m_MsgList.AddOneMessage("★★ 下载补丁被忽略 ★★", -1);
			m_MsgList.AddOneMessage("------------------------", -1);
			m_MsgList.AddOneMessage(" 剑侠情缘，永远的情缘。 ", -1);
			m_MsgList.AddOneMessage("------------------------", -1);
			m_MsgList.AddOneMessage("不断传来Money的剑侠网络!", -1);
			m_MsgList.AddOneMessage("------------------------", -1);
			m_MsgList.AddOneMessage("Killthegod万岁，万万岁！", -1);
			m_MsgList.AddOneMessage("------------------------", -1);
			m_MsgList.AddOneMessage("  许吴合作，其利断金！  ", -1);
			m_MsgList.AddOneMessage("------------------------", -1);
			m_MsgList.AddOneMessage("  Enjoy EnjoyableWooy ! \n  Enjoy your life ! ", -1);
			m_MsgList.AddOneMessage("------------------------", -1);
		}
	}
}

//--------------------------------------------------------------------------
//	功能：窗体绘制
//--------------------------------------------------------------------------
void KUiUpdataPatch::Breathe()
{
	if (m_fnSwordUpdateGetResult)
	{
		char	sInformation[256];
		int		nMsgLen = 256;
		int		nProgressPercent = 0;
		int	nRet = m_fnSwordUpdateGetResult(sInformation, &nMsgLen, &nProgressPercent, 0);
		//添加新的消息
		if (nMsgLen > 0)
			m_MsgList.AddOneMessage(sInformation, nMsgLen);
		//更新进展条
		m_Progress.SetPart(nProgressPercent, 100);
		switch(nRet)
		{
		case SWORDUPDATE_R_FINISHED:
		case SWORDUPDATE_R_NEEDLESS:	
			OnFinished(false);
			break;
		case SWORDUPDATE_R_FINISHED_UPDATE_SELF:
		case SWORDUPDATE_R_FINISHED_WAIT:
			OnFinished(true);
			break;
		case SWORDUPDATE_R_ERROR:
			SetCloseParam("升级程序执行过程中发生错误，点确定退出。", true, true);
			break;
		}
	}
}

//--------------------------------------------------------------------------
//	功能：窗口函数
//--------------------------------------------------------------------------
int KUiUpdataPatch::WndProc(unsigned int uMsg, unsigned int uParam, int nParam)
{
	switch(uMsg)
	{
	case WND_N_BUTTON_CLICK:
		if(uParam == (unsigned int)(KWndWindow*)&m_OkBtn ||
			uParam == (unsigned int)(KWndWindow*)&m_CancelBtn)
		{
			if (m_bQuit)
			{
				CloseWindow();
				UiPostQuitMsg();
			}
			else
			{
				CloseWindow();
				KUiInit::OpenWindow();
			}
		}
		break;
	case WND_N_SCORLLBAR_POS_CHANGED:
		if (uParam == (unsigned int)(KWndWindow*)&m_MsgScroll)
			m_MsgList.SetFirstShowLine(nParam);
		break;
	default:
		return KWndShowAnimate::WndProc(uMsg, uParam, nParam);
	}
	return 0;
}

//--------------------------------------------------------------------------
//	功能：升级结束
//--------------------------------------------------------------------------
void KUiUpdataPatch::OnFinished(bool bRestart)
{
	if (bRestart && m_PatchShell)
	{
		fnRegisterAppLaunchedAfterUpdate fnRegApp;
		fnRegApp = (fnRegisterAppLaunchedAfterUpdate)GetProcAddress(m_PatchShell,PATCHSHELL_FN_REGLAUNCHEAPPNAME);
		if (fnRegApp != NULL)
		{
			char sAppName[255];
			GetModuleFileName(NULL,sAppName,sizeof(sAppName));
			fnRegApp(sAppName,"");
			SetCloseParam("升级完毕，游戏需要重启，请点确定按钮。", true, true);
		}
		else
			SetCloseParam("升级完毕，点确定按钮，游戏将退出。请手工重启游戏。", true, true);
	}
	else
		SetCloseParam("升级完毕，点确定按钮继续游戏。", false, true);
}

void KUiUpdataPatch::SetCloseParam(const char* pInfo, bool bQuit, bool bOk)
{
	m_fnSwordUpdateGetResult = NULL;
	m_bQuit = bQuit;
	if (pInfo)
		m_StatusText.SetText(pInfo);
	if (bOk)
	{
		m_CancelBtn.LeaveAlone();
		m_OkBtn.LeaveAlone();
		AddChild(&m_OkBtn);
	}
	else
	{
		m_CancelBtn.LeaveAlone();
		m_OkBtn.LeaveAlone();
		AddChild(&m_CancelBtn);
	}
}