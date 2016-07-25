/*****************************************************************************************
//	网络连接，汇集欲发送包与派送抵达包的代理中心
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-10-6
------------------------------------------------------------------------------------------
*****************************************************************************************/
#include "KWin32.h"
#include <objbase.h>
#include <initguid.h>
#include "KEngine.h"
#include "NetConnectAgent.h"
#include "NetMsgTargetObject.h"
#include "../Ui/Elem/Wnds.h"
#include "../Ui/UiCase/UiSysMsgCentre.h"
#include "../Ui/UiShell.h"
#include "../../Core/Src/CoreShell.h"
#include "../../../Headers/KProtocolDef.h"
#include "crtdbg.h"

#define	NETCONNECT_MODULE			"Rainbow.dll"

extern int			g_bDisconnect;


void __stdcall ClientCallBack(LPVOID lpParam, const unsigned long &ulnEventType);

extern iCoreShell*		g_pCoreShell;
KNetConnectAgent g_NetConnectAgent;
#ifdef _DEBUG
static int g_snBugLog;
#endif

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KNetConnectAgent::KNetConnectAgent()
{
	memset(&m_MsgTargetObjs, 0, sizeof(m_MsgTargetObjs));
	m_pClient = NULL;
	m_pGameSvrClient = NULL;
	m_hModule = NULL;
	m_bIsClientConnecting = false;
	m_bTobeDisconnect = false;
	m_uClientRequestTime = 0;

	m_pFactroyFun = NULL;
	m_pClientFactory = NULL;

}

//--------------------------------------------------------------------------
//	功能：析构函数
//--------------------------------------------------------------------------
KNetConnectAgent::~KNetConnectAgent()
{
	Exit();
}

//--------------------------------------------------------------------------
//	功能：初始化
//--------------------------------------------------------------------------
int KNetConnectAgent::Initialize()
{
	m_hModule = ::LoadLibrary(NETCONNECT_MODULE);

	if (m_hModule)
	{
		m_pFactroyFun = (pfnCreateClientInterface)GetProcAddress(m_hModule, "CreateInterface");
		if (m_pFactroyFun)
		{
			if (SUCCEEDED(m_pFactroyFun(IID_IClientFactory, reinterpret_cast< void ** >( &m_pClientFactory))))
			{
				m_pClientFactory->SetEnvironment(1024 * 512);
			}
		}
	}

	if (!m_pFactroyFun || !m_pClientFactory)
        return false;

	m_bIsClientConnecting = false;
	return true;
}

//--------------------------------------------------------------------------
//	功能：退出
//--------------------------------------------------------------------------
void KNetConnectAgent::Exit()
{
	memset(&m_MsgTargetObjs, 0, sizeof(m_MsgTargetObjs));
	
    DisconnectClient();
	DisconnectGameSvr();

    if (m_pClientFactory)
    {
        m_pClientFactory->Release();
        m_pClientFactory = NULL;
    }

    m_pFactroyFun = NULL;

	if (m_hModule)
	{
		::FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

//--------------------------------------------------------------------------
//	功能：建立连接
//--------------------------------------------------------------------------
int	KNetConnectAgent::ClientConnectByNumericIp(const unsigned char* pIpAddress, unsigned short nPort)
{	
	DisconnectClient();
	
    if (!pIpAddress || !nPort)
        return false;

    if (!m_pClientFactory)
        return false;

	m_pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >(&m_pClient));

    if (!m_pClient)
        return false;

	if (FAILED(m_pClient->Startup()))
		return false;

	m_pClient->RegisterMsgFilter((void*)false, ClientCallBack);

    m_bIsClientConnecting = false;

	char	Address[128];
	sprintf(Address, "%d.%d.%d.%d", pIpAddress[0], pIpAddress[1],
		pIpAddress[2], pIpAddress[3]);
	if (SUCCEEDED(m_pClient->ConnectTo(Address, nPort)))
	{
		g_DebugLog("[Gateway] connectted.");
		m_bIsClientConnecting = true;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
//	功能：关闭连接
//--------------------------------------------------------------------------
void KNetConnectAgent::DisconnectClient()
{
	if (m_bIsGameServConnecting == false)
		m_uClientRequestTime = 0;

	if (m_pClient && m_bIsClientConnecting)
	{
		g_DebugLog("[Gateway] connection closed.");
		m_bIsClientConnecting = false;
		m_pClient->Shutdown();
	}

	if (m_pClient)
	{
		m_pClient->Cleanup();
		m_pClient->Release();
		m_pClient = NULL;
	}

}

int KNetConnectAgent::ConnectToGameSvr(const unsigned char* pIpAddress, unsigned short uPort, GUID* pGuid)
{
	
	DisconnectGameSvr();
	if (pGuid == NULL)
        return false;

    if (!m_pClientFactory)
        return false;

    m_pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >(&m_pGameSvrClient));
    
    if (!m_pGameSvrClient)
        return false;

	if (FAILED(m_pGameSvrClient->Startup()))
		return false;

	m_pGameSvrClient->RegisterMsgFilter((void*)true, ClientCallBack);


	char	Address[128];
	sprintf(Address, "%d.%d.%d.%d", pIpAddress[0], pIpAddress[1],
		pIpAddress[2], pIpAddress[3]);
	
	if (FAILED(m_pGameSvrClient->ConnectTo(Address, uPort)))
		return false;

	m_bIsGameServConnecting = true;
	tagLogicLogin ll;
	ll.cProtocol = c2s_logiclogin;
	memcpy( &ll.guid, pGuid, sizeof(GUID));
	if (FAILED(m_pGameSvrClient->SendPackToServer(&ll, sizeof(tagLogicLogin))))
		return false;

	if (g_pCoreShell)
		g_pCoreShell->SetClient(m_pGameSvrClient);

	return true;
}

void KNetConnectAgent::DisconnectGameSvr()
{
	if (m_bIsClientConnecting == false)
		m_uClientRequestTime = 0;

	if (m_pGameSvrClient && m_bIsGameServConnecting)
	{
		if (g_pCoreShell)
			g_pCoreShell->SetClient(NULL);

		m_bIsGameServConnecting = false;
		m_pGameSvrClient->Shutdown();
	}    

	if (m_pGameSvrClient)
	{
		m_pGameSvrClient->Cleanup();
		m_pGameSvrClient->Release();
		m_pGameSvrClient = NULL;
	}

}

//--------------------------------------------------------------------------
//	功能：发送消息
//--------------------------------------------------------------------------
int KNetConnectAgent::SendMsg(const void *pBuffer, int nSize)
{
	if (m_pClient)
	{
		m_pClient->SendPackToServer((BYTE*)pBuffer, nSize);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
//	功能：持续性行为
//--------------------------------------------------------------------------
void KNetConnectAgent::Breathe()
{
	//----超时判断----
	if (m_uClientRequestTime &&
		GetTickCount() - m_uClientRequestTime >= m_uClientTimeoutLimit)
	{
		g_bDisconnect = true;
		m_bTobeDisconnect = true;
	}

	//----断线判断----
	if (m_bTobeDisconnect)
	{
		m_bTobeDisconnect = false;
//		m_bIsClientConnecting = false;//to be check
//		m_bIsGameServConnecting = false;
		DisconnectClient();
		DisconnectGameSvr();
		return;
	}

	unsigned int nSize;
	const char* pBuffer = NULL;

	//----处理来自网关的数据----
	if (m_bIsClientConnecting)
	{
		while (true)
		{
            if (!m_pClient)
                break;

            pBuffer = (const char*)m_pClient->GetPackFromServer(nSize);
            if (!(pBuffer && nSize))
                break;

			PROTOCOL_MSG_TYPE*	pMsg = (PROTOCOL_MSG_TYPE*)pBuffer;
			PROTOCOL_MSG_TYPE	Msg = *pMsg;			

			_ASSERT(Msg > s2c_multiserverbegin || Msg < s2c_end);
				
			if (m_MsgTargetObjs[Msg])
				(m_MsgTargetObjs[Msg])->AcceptNetMsg(pMsg);
		}
	}
	//----处理来自游戏服务器的数据----
	if (m_bIsGameServConnecting && m_pGameSvrClient)
	{
		while (true)
		{
			if (!m_pGameSvrClient)
                break;
            
            pBuffer = (const char*)m_pGameSvrClient->GetPackFromServer(nSize);

            if (!(pBuffer && nSize))
                break;

            
            PROTOCOL_MSG_TYPE*	pMsg = (PROTOCOL_MSG_TYPE*)pBuffer;
			while(pMsg < (PROTOCOL_MSG_TYPE*)(pBuffer + nSize))
			{
				PROTOCOL_MSG_TYPE	Msg = *pMsg;			


				// 跨服务器的协议
				if (Msg == s2c_notifyplayerexchange)
				{
					ProcessSwitchGameSvrMsg(pMsg);
					break;
				}
				else if (g_pCoreShell)
				{
					_ASSERT(Msg > s2c_clientbegin && Msg < s2c_end);
					_ASSERT(g_pCoreShell->GetProtocolSize(Msg) != 0);
					g_pCoreShell->NetMsgCallbackFunc(pMsg);
					if (g_pCoreShell->GetProtocolSize(Msg) > 0)
						pMsg = (PROTOCOL_MSG_TYPE*)(((char*)pMsg) + g_pCoreShell->GetProtocolSize(Msg));
					else
						pMsg = (PROTOCOL_MSG_TYPE*)(((char*)pMsg) + PROTOCOL_MSG_SIZE + (*(unsigned short*) (((char*)pMsg) + PROTOCOL_MSG_SIZE)));
				}

			}
		}
	}
}

//--------------------------------------------------------------------------
//	功能：注册抵达消息响应函数
//--------------------------------------------------------------------------
void KNetConnectAgent::RegisterMsgTargetObject(PROTOCOL_MSG_TYPE Msg, iKNetMsgTargetObject* pObject)
{
	if (Msg >= 0 && Msg < MAX_MSG_COUNT)
		m_MsgTargetObjs[Msg] = pObject;
}

void KNetConnectAgent::TobeDisconnect()
{
	m_bTobeDisconnect = true;
}

void KNetConnectAgent::UpdateClientRequestTime(bool bCancel, unsigned int uTimeLimit)
{
	if (m_bIsClientConnecting || m_bIsGameServConnecting)
	{
		if (bCancel == false)
		{
			m_uClientRequestTime = GetTickCount();
			if (m_uClientRequestTime == 0)
				m_uClientRequestTime = 1;
			m_uClientTimeoutLimit = uTimeLimit;
		}
		else
			m_uClientRequestTime = 0;
	}
}

int	KNetConnectAgent::IsConnecting(int bGameServ)
{
	if (bGameServ)
		return m_bIsGameServConnecting;
	else
		return m_bIsClientConnecting;
}

void __stdcall ClientCallBack(LPVOID lpParam, const unsigned long &ulnEventType)
{
	switch(ulnEventType)
	{
	case enumServerConnectCreate:
		break;
	case enumServerConnectClose:
		{
			int	bGameServ = (int)lpParam;
			if (g_NetConnectAgent.IsConnecting(bGameServ))
			{
				g_bDisconnect = true;
				g_NetConnectAgent.TobeDisconnect();
			}
		}
		break;
	}
}

//处理游戏世界服务器的网络消息
bool KNetConnectAgent::ProcessSwitchGameSvrMsg(void* pMsgData)
{
	tagNotifyPlayerExchange* pInfo = (tagNotifyPlayerExchange*)pMsgData;
	_ASSERT(pInfo && pInfo->cProtocol == s2c_notifyplayerexchange);
	
	DWORD	dwIp = pInfo->nIPAddr;
	int	nPort = pInfo->nPort;
	GUID guid;
	guid = pInfo->guid;

	if (dwIp && nPort)
	{
		// 开始与GameSvr进行连接
		g_pCoreShell->OperationRequest(GOI_EXIT_GAME, 0, 0);
		if (ConnectToGameSvr((const unsigned char*)&dwIp, nPort, &guid))
		{
			Wnd_GameSpaceHandleInput(false);
			UpdateClientRequestTime(false);
			UiOnGameServerConnected();
			return true;
		}
	}
	else
	{
		// 跨服务器失败
//		MessageBox(NULL, "", "", MB_OK);

#define	MSG_EXCHANGE_FAIL	"前方路途不太通畅，还是过会再来吧。"
		KSystemMessage	Msg;
		Msg.byConfirmType = SMCT_NONE;
		Msg.byParamSize = 0;
		Msg.byPriority = 0;
		Msg.eType = SMT_SYSTEM;
		
		strcpy(Msg.szMessage, MSG_EXCHANGE_FAIL);
		KUiSysMsgCentre::AMessageArrival(&Msg, 0);
		g_bDisconnect = false;
		return false;
		
	}
	g_bDisconnect = true;
	g_NetConnectAgent.TobeDisconnect();
	return false;	
}