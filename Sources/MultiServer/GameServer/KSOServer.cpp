#include "StdAfx.h"
#ifdef _STANDALONE
#ifndef __linux
#include <winsock2.h>
#include <malloc.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#endif
#include "KWin32.h"
#include "package.h"
#endif

#include "CRC32.h"
#ifdef _STANDALONE
#include "KCore.h"
#include "KPlayerSet.h"
#include "CRC32.c"
#endif

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#endif

#define _SERVER

#ifdef _STANDALONE
//	#ifdef WIN32
//		#define FAILED(x) ((x) != S_OK)
//	#else
	#define FAILED(x) ((x) <= 0)
//	#endif
#endif

#ifndef _STANDALONE
#include <crtdbg.h>
#include <objbase.h>
#include <initguid.h>
#include "Library.h"
#include "inoutmac.h"
#endif

#include <iostream>
//#include <strstream>

#include "S3DBInterface.h"
#include "KProtocolDef.h"
#include "KProtocol.h"
#include "KRelayProtocol.h"
#include "KTongProtocol.h"

#include "KSOServer.h"

#ifndef _STANDALONE
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::CCriticalSection;

CLibrary g_theHeavenLibrary( "heaven.dll" );
CLibrary g_theRainbowLibrary( "rainbow.dll" );

OnlineGameLib::Win32::CUsesWinsock	 g_SocketInit;
CCriticalSection g_csFlow;

#else

ZMutex g_mutexFlow;

#endif

CPackager			m_theRecv;
CPackager			m_theSend;

#define				GAME_FPS		18
using namespace std;

//const int KSwordOnLineSever::m_snMaxPlayerCount = 500;
//const int KSwordOnLineSever::m_snPrecision = 10;
const int KSwordOnLineSever::m_snMaxBuffer = 10;
const int KSwordOnLineSever::m_snBufferSize = 1024 * 16;

KSwordOnLineSever g_SOServer;

enum PLAYER_GAME_STATUS
{
	enumPlayerBegin = 0,
	enumPlayerSyncEnd,
	enumPlayerPlaying,
	enumPlayerExchangingServer,
};

enum PLAYER_EXCHANGE_STATUS
{
	enumExchangeBegin = 0,
	enumExchangeSearchingWay,
	enumExchangeWaitForGameSvrRespone,
	enumExchangeCleaning,
};


int	g_nTongPCSize[defTONG_PROTOCOL_CLIENT_NUM] = 
{
	-1,										// enumS2C_TONG_CREATE_SUCCESS
	sizeof(STONG_CREATE_FAIL_SYNC),			// enumS2C_TONG_CREATE_FAIL
	sizeof(STONG_ADD_MEMBER_SUCCESS_SYNC),	// enumS2C_TONG_ADD_MEMBER_SUCCESS
	sizeof(STONG_ADD_MEMBER_FAIL_SYNC),		// enumS2C_TONG_ADD_MEMBER_FAIL
	-1,										// enumS2C_TONG_HEAD_INFO
	-1,										// enumS2C_TONG_MANAGER_INFO
	-1,										// enumS2C_TONG_MEMBER_INFO
	sizeof(STONG_BE_INSTATED_SYNC),			// enumS2C_TONG_BE_INSTATED
	sizeof(STONG_INSTATE_SYNC),				// enumS2C_TONG_INSTATE
	sizeof(STONG_KICK_SYNC),				// enumS2C_TONG_KICK
	sizeof(STONG_BE_KICKED_SYNC),			// enumS2C_TONG_BE_KICKED
	sizeof(STONG_LEAVE_SYNC),				// enumS2C_TONG_LEAVE
	sizeof(STONG_CHECK_GET_MASTER_POWER_SYNC),	// enumS2C_TONG_CHECK_CHANGE_MASTER_POWER
	sizeof(STONG_CHANGE_MASTER_FAIL_SYNC),	// enumS2C_TONG_CHANGE_MASTER_FAIL
	sizeof(STONG_CHANGE_AS_SYNC),			// enumS2C_TONG_CHANGE_AS
	sizeof(STONG_CHANGE_MASTER_SYNC),		// enumS2C_TONG_CHANGE_MASTER
	sizeof(STONG_LOGIN_DATA_SYNC),			// enumS2C_TONG_LOGIN_DATA
};



#ifndef _STANDALONE
typedef HRESULT ( __stdcall * pfnCreateClientInterface )( 
			REFIID riid, 
			void **ppv 
		);

typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
			REFIID	riid,
			void	**ppv
		);
#endif

#ifndef _STANDALONE
void __stdcall ServerEventNotify(
#else
void ServerEventNotify(								 
#endif
			LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
#ifndef _STANDALONE
	CCriticalSection::Owner locker( g_csFlow );
#else
	g_mutexFlow.lock();
#endif
	switch( ulnEventType )
	{
	case enumClientConnectCreate:
		g_SOServer.SetNetStatus(ulnID, enumNetConnected);
		break;
	case enumClientConnectClose:
		g_SOServer.SetNetStatus(ulnID, enumNetUnconnect);
		break;
	}
#ifdef _STANDALONE
	g_mutexFlow.unlock();
#endif
}

#ifndef _STANDALONE
void __stdcall GatewayClientEventNotify(
#else
void GatewayClientEventNotify(
#endif
			LPVOID lpParam,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		break;
	case enumServerConnectClose:
		printf("GateWay lost\n");
		g_SOServer.SetRunningStatus(FALSE);
		break;
	}
}

#ifndef _STANDALONE
void __stdcall ChatClientEventNotify(
#else
void ChatClientEventNotify(
#endif
			LPVOID	lpParam,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		break;
	case enumServerConnectClose:
		printf("Chat disconnect\n");
		break;
	}
}

#ifndef _STANDALONE
void __stdcall TongClientEventNotify(
#else
void TongClientEventNotify(
#endif
			LPVOID	lpParam,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:	// 连接建立时候的处理
		break;
	case enumServerConnectClose:	// 连接段开时候的处理
		printf("Tong disconnect\n");
		break;
	}
}

#ifndef _STANDALONE
void __stdcall DatabaseClientEventNotify(
#else
void DatabaseClientEventNotify(
#endif
			LPVOID lpParam,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		break;
	case enumServerConnectClose:
		printf("DataBase lost\n");
		g_SOServer.SetRunningStatus(FALSE);
		break;
	}
}

#ifndef _STANDALONE
void __stdcall TransferClientEventNotify(
#else
void TransferClientEventNotify(
#endif
			LPVOID lpParam,
			const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		break;
	case enumServerConnectClose:
///		g_SOServer.SetRunningStatus(FALSE);
		break;
	}
}


KSwordOnLineSever::KSwordOnLineSever()
{
	m_nGameLoop = 0;
	m_bIsRunning = TRUE;
	m_nServerPort = 6666;
	m_nGatewayPort = 5632;
	m_nDatabasePort = 5001;
	m_nTransferPort = 5003;
	m_nChatPort	= 5004;
	m_nTongPort	= 5005;
	ZeroMemory(m_szGatewayIP, sizeof(m_szGatewayIP));
	ZeroMemory(m_szDatabaseIP, sizeof(m_szDatabaseIP));
	ZeroMemory(m_szTransferIP, sizeof(m_szTransferIP));
	ZeroMemory(m_szChatIP, sizeof(m_szChatIP));
	ZeroMemory(m_szTongIP, sizeof(m_szTongIP));
	m_pServer = NULL;
	m_pGatewayClient = NULL;
	m_pDatabaseClient = NULL;
	m_pTransferClient = NULL;
	m_pChatClient = NULL;
	m_pTongClient = NULL;
	m_pCoreServerShell = NULL;
	m_pGameStatus = NULL;
}

KSwordOnLineSever::~KSwordOnLineSever()
{
}

BOOL KSwordOnLineSever::Init()
{
	/*
	 * Initialize class member
	 */
	m_bIsRunning = TRUE;
	g_SetRootPath(NULL);

	g_SetFilePath("\\");
	KIniFile iniFile;

	iniFile.Load("ServerCfg.ini");
	iniFile.GetInteger("GameServer", "Port", 6666, &m_nServerPort);
	extern int g_nPort;
	if (g_nPort)
		m_nServerPort = g_nPort;
	iniFile.GetString("Gateway", "Ip", "222.189.237.112", m_szGatewayIP, sizeof(m_szGatewayIP));
	iniFile.GetInteger("Gateway", "Port", 5632, &m_nGatewayPort);
	iniFile.GetString("Database", "Ip", "222.189.237.112", m_szDatabaseIP, sizeof(m_szDatabaseIP));
	iniFile.GetInteger("Database", "Port", 5001, &m_nDatabasePort);
	iniFile.GetString("Transfer", "Ip", "222.189.237.112", m_szTransferIP, sizeof(m_szTransferIP));
	iniFile.GetInteger("Transfer", "Port", 5003, &m_nTransferPort);
	iniFile.GetString("Chat", "Ip", "222.189.237.112", m_szChatIP, sizeof(m_szChatIP));
	iniFile.GetInteger("Chat", "Port", 5004, &m_nChatPort);
	iniFile.GetString("Tong", "Ip", "222.189.237.112", m_szTongIP, sizeof(m_szTongIP));
	iniFile.GetInteger("Tong", "Port", 5005, &m_nTongPort);
#ifdef WIN32
	iniFile.GetInteger("Overload", "MaxPlayer", 450, &m_nMaxPlayerCount);
	iniFile.GetInteger("Overload", "Precision", 20, &m_nPrecision);
#else
	iniFile.GetInteger("Overload", "MaxPlayer", 1000, &m_nMaxPlayerCount);
	iniFile.GetInteger("Overload", "Precision", 200, &m_nPrecision);
#endif

	m_nMaxPlayer = m_nMaxPlayerCount + m_nPrecision;
	
	if (m_nMaxPlayer <= 0)
	{
		cout << "Maximal player number <= 0!" << endl;
		return FALSE;
	}
	
	if (!m_pGameStatus)
	{
		m_pGameStatus = new GameStatus[m_nMaxPlayer];
	}

//	strcpy(m_szGameSvrIP, OnlineGameLib::Win32::net_ntoa(dwIp));
	
	/*
	 * Open this server to player
	 */
#ifndef _STANDALONE
	pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )( g_theHeavenLibrary.GetProcAddress( "CreateInterface" ) );

	IServerFactory *pServerFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( m_nMaxPlayer, m_nPrecision, m_snMaxBuffer, m_snBufferSize  );
		
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &m_pServer ) );
		
		pServerFactory->Release();
	}
	else
	{
		return FALSE;
	}
#else
	m_pServer = new IServer(m_nMaxPlayer, 4, 200 * 1024);
#endif

	if (!m_pServer)
	{
		cout << "Initialization failed! Don't find a correct heaven.dll" << endl;
		return FALSE;
	}

	m_pServer->Startup();

	m_pServer->RegisterMsgFilter( reinterpret_cast< void * >( m_pServer ), ServerEventNotify );

	if ( FAILED( m_pServer->OpenService( INADDR_ANY, m_nServerPort ) ) )
	{
		return FALSE;
	}
	
	if (!GetLocalIpAddress(&m_dwIntranetIp, &m_dwInternetIp))
	{
		cout << "Can't get server ip" << endl;
		return FALSE;
	}
	cout << m_dwIntranetIp << " : " << m_dwInternetIp << endl;

#ifndef _STANDALONE
	IServer *pCloneServer = NULL;
	m_pServer->QueryInterface( IID_IIOCPServer, ( void ** )&pCloneServer );
#else 
	IServer *pCloneServer = m_pServer;
#endif
	/*
	 * Init GameWorld
	 */
	if (m_pCoreServerShell == NULL)
		m_pCoreServerShell = ::CoreGetServerShell();

	if (m_pCoreServerShell == NULL)
	{
		cout << "Failed to Create CoreShell." << endl;
		return FALSE;
	}

	m_pCoreServerShell->OperationRequest(SSOI_LAUNCH, (unsigned int)pCloneServer, 0);

	/*
	 * Connect to database
	 */
#ifndef _STANDALONE
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 128 );

		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pDatabaseClient ) );

		pClientFactory->Release();
	}
	else
	{
		return FALSE;
	}
#else
//	net_buffer = new ZBuffer(20240000, 400);
	m_pDatabaseClient = new IClient(6000*1024, 6000*1024);
#endif
	if ( !m_pDatabaseClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		return FALSE;
	}
	if (FAILED(m_pDatabaseClient->Startup()))
	{
		cout << "Can't not startup database client service!" << endl;
		return FALSE;
	}
	cout << "Database IP : " << m_szDatabaseIP << " - Port : " << m_nDatabasePort << endl;

	m_pDatabaseClient->RegisterMsgFilter( reinterpret_cast< void * >( m_pDatabaseClient ), DatabaseClientEventNotify );

	if ( FAILED( m_pDatabaseClient->ConnectTo( m_szDatabaseIP, m_nDatabasePort ) ) )
	{
		cout << "Connect to database is failed!" << endl;
		return FALSE;
	}

	cout << "Connect database successful!" << endl;
	
	/*
	 * Connect to transfer
	 */
#ifndef _STANDALONE
	pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 512 );

		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pTransferClient ) );

		pClientFactory->Release();
	}
	else
	{
		return FALSE;
	}
#else
	m_pTransferClient = new IClient(6000*1024, 6000*1024);
#endif
	if ( !m_pTransferClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		return FALSE;
	}
		
	if (FAILED(m_pTransferClient->Startup()))
	{
		cout << "Can't not startup transfer client service!" << endl;
		return FALSE;
	}
	
	cout << "Transfer IP : " << m_szTransferIP << " - Port : " << m_nTransferPort << endl;
	
	m_pTransferClient->RegisterMsgFilter( reinterpret_cast< void * >( m_pTransferClient ), TransferClientEventNotify );
	
	if ( FAILED( m_pTransferClient->ConnectTo( m_szTransferIP, m_nTransferPort ) ) )
	{
		cout << "Connect to transfer is failed!" << endl;
		if (m_pTransferClient)
		{
			m_pTransferClient->Shutdown();
			m_pTransferClient->Cleanup();
			m_pTransferClient->Release();
			m_pTransferClient = NULL;
		}
	}
	else
	{		
		cout << "Connect transfer successful!" << endl;	
	}

	/*
	 * Connect to chat
	 */
#ifndef _STANDALONE
	pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 1024 * 4 );

		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pChatClient ) );

		pClientFactory->Release();
	}
	else
	{
		return FALSE;
	}
#else
	m_pChatClient = new IClient(6000*1024, 6000*1024);
#endif

	if ( !m_pChatClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		return FALSE;
	}
		
	if (FAILED(m_pChatClient->Startup()))
	{
		cout << "Can't not startup chat client service!" << endl;
		return FALSE;
	}
	
	cout << "Chat IP : " << m_szChatIP << " - Port : " << m_nChatPort << endl;
	
	m_pChatClient->RegisterMsgFilter( reinterpret_cast< void * >( m_pChatClient ), ChatClientEventNotify );
	
	if ( FAILED( m_pChatClient->ConnectTo( m_szChatIP, m_nChatPort ) ) )
	{
		cout << "Connect to chat is failed!" << endl;
		if (m_pChatClient)
		{
			m_pChatClient->Shutdown();
			m_pChatClient->Cleanup();
			m_pChatClient->Release();
			m_pChatClient = NULL;
		}
	}
	else
	{		
		cout << "Connect chat successful!" << endl;	
	}

//------------------------ Connect to tong ----------------------------
#ifndef _STANDALONE
	pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );
	pClientFactory = NULL;
	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 1024 * 4 );
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pTongClient ) );
		pClientFactory->Release();
	}
	else
	{
		return FALSE;
	}
#else
	m_pTongClient = new IClient(6000*1024, 6000*1024);
#endif

	if ( !m_pTongClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;
		return FALSE;
	}
		
	if (FAILED(m_pTongClient->Startup()))
	{
		cout << "Can't not startup tong client service!" << endl;
		return FALSE;
	}
	
	cout << "Tong IP : " << m_szTongIP << " - Port : " << m_nTongPort << endl;
	
	m_pTongClient->RegisterMsgFilter( reinterpret_cast< void * >( m_pTongClient ), TongClientEventNotify );
	
	if ( FAILED( m_pTongClient->ConnectTo( m_szTongIP, m_nTongPort ) ) )
	{
		cout << "Connect to tong is failed!" << endl;
		if (m_pTongClient)
		{
			m_pTongClient->Shutdown();
			m_pTongClient->Cleanup();
			m_pTongClient->Release();
			m_pTongClient = NULL;
		}
	}
	else
	{		
		cout << "Connect tong successful!" << endl;	
	}
//------------------------ Connect to tong end ----------------------------

	/*
	 * Connect to gateway
	 */
#ifndef _STANDALONE
	pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 1024 );

		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pGatewayClient ) );

		pClientFactory->Release();
	}
	else
	{
		return FALSE;
	}
#else
	m_pGatewayClient = new IClient(6000*1024, 6000*1024);
#endif
	if ( !m_pGatewayClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		return FALSE;
	}

	if ( FAILED(m_pGatewayClient->Startup()))
	{
		cout << "Can't not startup gateway client service!" << endl;
		return FALSE;
	}

	cout << "Gateway IP : " << m_szGatewayIP << " - Port : " << m_nGatewayPort << endl;

	m_pGatewayClient->RegisterMsgFilter( reinterpret_cast< void * >( m_pGatewayClient ), GatewayClientEventNotify );

	if ( FAILED( m_pGatewayClient->ConnectTo( m_szGatewayIP, m_nGatewayPort ) ) )
	{
		cout << "Connect to gateway is failed!" << endl;
		return FALSE;
	}

	cout << "Connect gateway successful!" << endl;

	/*
	 * Game timer start
	 */
	m_Timer.Start();
	m_nGameLoop = 0;
	return TRUE;
}

void KSwordOnLineSever::Release()
{
#ifndef _STANDALONE
	CCriticalSection::Owner locker( g_csFlow );
#else
	g_mutexFlow.lock();
#endif
	if (m_pServer)
	{
		// Close service for no one can login again
		m_pServer->CloseService();
		// Regist callback function to null
		m_pServer->RegisterMsgFilter( reinterpret_cast< void * >( m_pServer ), NULL );
		// Shut down all client and save their data.
		const char *pChar = NULL;

		ExitAllPlayer();
	}

	if (m_pCoreServerShell)
	{
		m_pCoreServerShell->Release();
		m_pCoreServerShell = NULL;
	}

	if (m_pGameStatus)
	{
		delete [] m_pGameStatus;
		m_pGameStatus = NULL;
	}

	if (m_pServer)
	{
		m_pServer->Cleanup();
		m_pServer->Release();
#ifndef _STANDALONE
		m_pServer = NULL;
#else
		delete m_pServer;			m_pServer = NULL;
#endif
	}

	if (m_pGatewayClient)
	{
		m_pGatewayClient->Shutdown();
		m_pGatewayClient->Cleanup();
		m_pGatewayClient->Release();
		m_pGatewayClient = NULL;
	}

	if (m_pDatabaseClient)
	{
		m_pDatabaseClient->Shutdown();
		m_pDatabaseClient->Cleanup();
		m_pDatabaseClient->Release();
		m_pDatabaseClient = NULL;
	}

	if (m_pTransferClient)
	{
		m_pTransferClient->Shutdown();
		m_pTransferClient->Cleanup();
		m_pTransferClient->Release();
		m_pTransferClient = NULL;
	}

	if (m_pChatClient)
	{
		m_pChatClient->Shutdown();
		m_pChatClient->Cleanup();
		m_pChatClient->Release();
		m_pChatClient = NULL;
	}

	if (m_pTongClient)
	{
		m_pTongClient->Shutdown();
		m_pTongClient->Cleanup();
		m_pTongClient->Release();
		m_pTongClient = NULL;
	}

	IP2CONNECTUNIT::iterator it;
	for (it = m_mapIp2TransferUnit.begin(); it != m_mapIp2TransferUnit.end(); it++)
	{
		KTransferUnit* pUnit = (*it).second;
		if (pUnit)
		{
			delete pUnit;
			pUnit = NULL;
		}
	}
	m_mapIp2TransferUnit.clear();
#ifdef _STANDALONE
//	delete net_buffer;
	delete m_pTransferClient;	m_pTransferClient = NULL;
	delete m_pDatabaseClient;	m_pDatabaseClient = NULL;
	delete m_pGatewayClient;	m_pGatewayClient = NULL;
#endif
#ifdef _STANDALONE
	g_mutexFlow.unlock();
#endif
}

BOOL KSwordOnLineSever::Breathe()
{
	if (m_pCoreServerShell && m_bIsRunning)
	{
		MessageLoop();
		//printf("breathe..%u\n", m_Timer.GetElapse());
		if (m_nGameLoop * 1000 <= m_Timer.GetElapse() * GAME_FPS)
		{
			MainLoop();
#ifdef WIN32
			Sleep(0);
#else
			usleep(0);
#endif

#ifdef _STANDALONE
			if (m_nGameLoop % (GAME_FPS * 60) == 0)	//一分钟一次
			{
extern ZPerf g_sendPerf;
extern ZPerf g_recvPerf;
				DWORD dwElapse = m_Timer.GetElapse();
				printf("GameLoop= %06d, Time= %02d:%02d:%02d, Svr(S: %dK R: %dK), Clt(S: %dK R: %dK), Online= %d\n",
									m_nGameLoop,
									dwElapse / (1000 * 3600),
									(dwElapse / (1000 * 60)) % 60,
									(dwElapse / 1000) % 60,
									m_pServer->sendPerf.total_size / 1024,
									m_pServer->recvPerf.total_size / 1024,
									g_sendPerf.total_size / 1024,
									g_recvPerf.total_size / 1024,
									PlayerSet.GetOnlinePlayerCount());
			}
#endif
		}
		else
		{
#ifndef __linux			//SwitchToThread();
			Sleep(1);
#else
			usleep(1000);
#endif
#ifdef _STANDALONE
			void DoEveryThing(IServer* pIServer);
			DoEveryThing(m_pServer);
#endif
		}
		return TRUE;
	}
	printf("main thread exit\n");
	return FALSE;
}

void KSwordOnLineSever::MessageLoop()
{
	int i;
	const char*		pChar = NULL;
	unsigned int	uSize = 0;

#ifndef _STANDALONE
	CCriticalSection::Owner locker( g_csFlow );
#else
	g_mutexFlow.lock();
#endif

	while(m_pGatewayClient)
	{
		pChar = (const char*)m_pGatewayClient->GetPackFromServer(uSize);
		if (!pChar || 0 == uSize)
			break;
		GatewayMessageProcess(pChar, uSize);
	}

	while(m_pDatabaseClient)
	{
		pChar = (const char*)m_pDatabaseClient->GetPackFromServer(uSize);
		if (!pChar || 0 == uSize)
			break;
		DatabaseMessageProcess(pChar, uSize);
	}

	m_pCoreServerShell->SendNetMsgToTransfer(m_pTransferClient);

	while(m_pTransferClient)
	{
		pChar = (const char*)m_pTransferClient->GetPackFromServer(uSize);
		if (!pChar || 0 == uSize)
			break;
		TransferMessageProcess(pChar, uSize);
	}

	m_pCoreServerShell->SendNetMsgToChat(m_pChatClient);
	while(m_pChatClient)
	{
		pChar = (const char*)m_pChatClient->GetPackFromServer(uSize);
		if (!pChar || 0 == uSize)
			break;
		ChatMessageProcess(pChar, uSize);
	}

	m_pCoreServerShell->SendNetMsgToTong(m_pTongClient);
	while(m_pTongClient)
	{
		pChar = (const char*)m_pTongClient->GetPackFromServer(uSize);
		if (!pChar || 0 == uSize)
			break;
		TongMessageProcess(pChar, uSize);
	}

	pChar = NULL;
	uSize = 0;
	for (i = 0; i < m_nMaxPlayer; i++)
	{
		if (enumNetUnconnect == GetNetStatus(i))
			continue;

		while(m_pServer)
		{
			pChar = (const char*)m_pServer->GetPackFromClient(i, uSize);
			if (!pChar || 0 == uSize)
				break;
		
			//printf("player msg..\n");
			PlayerMessageProcess(i, pChar, uSize);
		}
	}
#ifdef _STANDALONE
	g_mutexFlow.unlock();
#endif
}

void KSwordOnLineSever::ChatMessageProcess(const char *pChar, size_t nSize)
{
	_ASSERT( pChar && nSize);
#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pChar );
#else
	BYTE cProtocol = *(BYTE *)pChar;
#endif	

	switch (cProtocol)
	{
	case chat_relegate:
		{
			CHAT_RELEGATE* pCR = (CHAT_RELEGATE*)pChar;

			DWORD	theID = 0;
			switch (pCR->TargetCls)
			{
			case tgtcls_team:
			case tgtcls_fac:
				theID = pCR->TargetID;
				break;
			case tgtcls_scrn:
				theID = m_pGameStatus[pCR->TargetID].nPlayerIndex;
				break;
			default: 
				return;
			}
			m_pCoreServerShell->GroupChat(
				m_pChatClient,
				pCR->nFromIP,
				pCR->nFromRelayID,
				pCR->channelid,
				pCR->TargetCls,
				theID,
				pCR + 1,
				pCR->routeDateLength);
		}
		break;
	case chat_groupman:
		ChatGroupMan(pChar, nSize);
		break;
	case chat_specman:
		ChatSpecMan(pChar, nSize);
		break;
	case chat_everyone:
		{{
		CHAT_EVERYONE* pCeo = (CHAT_EVERYONE*)pChar;
		m_pCoreServerShell->OperationRequest(SSOI_BROADCASTING, (unsigned int)(pCeo + 1), pCeo->wChatLength);
		}}
		break;
	default:
		break;
	}
	
}

void KSwordOnLineSever::TongMessageProcess(const char *pChar, size_t nSize)
{
	if (!pChar)
		return;

	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pChar;

	if (pHeader->ProtocolFamily == pf_extend)
	{
		if (pHeader->ProtocolID == extend_s2c_passtosomeone)
		{
			EXTEND_PASSTOSOMEONE* pEps = (EXTEND_PASSTOSOMEONE*)pChar;

			if (CheckPlayerID(pEps->lnID ,pEps->nameid))
			{
				_ASSERT(sizeof(tagExtendProtoHeader) <= sizeof(EXTEND_PASSTOSOMEONE));
				unsigned long lnID = pEps->lnID;
				size_t pckgsize = sizeof(tagExtendProtoHeader) + pEps->datasize;
				tagExtendProtoHeader* pExHdr = (tagExtendProtoHeader*)(pEps + 1) - 1;
				pExHdr->ProtocolType = s2c_extendfriend;
				pExHdr->wLength = pckgsize - 1;

				m_pServer->PackDataToClient(lnID, pExHdr, pckgsize);
			}
		}
		else if (pHeader->ProtocolID == extend_s2c_passtobevy)
		{
			EXTEND_PASSTOBEVY* pEpb = (EXTEND_PASSTOBEVY*)pChar;

			if (pEpb->playercount > 0)
			{
				_ASSERT(sizeof(tagExtendProtoHeader) <= sizeof(EXTEND_PASSTOBEVY));
				void* pExPckg = pEpb + 1;
				WORD playercount = pEpb->playercount;
				tagPlusSrcInfo* pPlayers = (tagPlusSrcInfo*)((BYTE*)pExPckg + pEpb->datasize);
				size_t pckgsize = sizeof(tagExtendProtoHeader) + pEpb->datasize;
				tagExtendProtoHeader* pExHdr = (tagExtendProtoHeader*)pExPckg - 1;
				pExHdr->ProtocolType = s2c_extendfriend;
				pExHdr->wLength = pckgsize - 1;

				for (WORD i = 0; i < playercount; i++)
				{
					if (CheckPlayerID(pPlayers[i].lnID, pPlayers[i].nameid))
						m_pServer->PackDataToClient(pPlayers[i].lnID, pExHdr, pckgsize);
				}
			}
		}
	}
	else if (pHeader->ProtocolFamily == pf_tong)
	{
		// 协议长度检测
		if (nSize < sizeof(EXTEND_HEADER))
			return;
		if (pHeader->ProtocolID >= enumS2C_TONG_NUM)
			return;
		if (g_nTongPCSize[pHeader->ProtocolID] < 0)
		{
			if (nSize <= sizeof(EXTEND_HEADER) + 2)
				return;
			WORD	wLength = *((WORD*)((BYTE*)pChar + sizeof(EXTEND_HEADER)));
			if (wLength != nSize)
				return;
		}
		else if (g_nTongPCSize[pHeader->ProtocolID] != nSize)
		{
			return;
		}

		switch (pHeader->ProtocolID)
		{
		case enumS2C_TONG_CREATE_SUCCESS:
			{
				STONG_CREATE_SUCCESS_SYNC	*pSync = (STONG_CREATE_SUCCESS_SYNC*)pChar;
				STONG_SERVER_TO_CORE_CREATE_SUCCESS sSuccess;

				sSuccess.m_nCamp = pSync->m_btCamp;
				sSuccess.m_dwPlayerNameID = pSync->m_dwPlayerNameID;
				sSuccess.m_nPlayerIdx = pSync->m_dwParam;
				memcpy(sSuccess.m_szTongName, pSync->m_szTongName, pSync->m_btTongNameLength);
				sSuccess.m_szTongName[pSync->m_btTongNameLength] = 0;

				if (m_pCoreServerShell->OperationRequest(SSOI_TONG_CREATE, (unsigned int)&sSuccess, 0))
				{
				}
				else
				{
				}
			}
			break;
		case enumS2C_TONG_CREATE_FAIL:
			{
				STONG_CREATE_FAIL_SYNC	*pFail = (STONG_CREATE_FAIL_SYNC*)pChar;
				int		nNetID;
				TONG_CREATE_FAIL_SYNC	sFail;

				sFail.ProtocolType = s2c_extendtong;
				sFail.m_btMsgId = enumTONG_SYNC_ID_CREATE_FAIL;
				sFail.m_btFailId = pFail->m_btFailID;
				sFail.m_wLength = sizeof(sFail) - 1;
				nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, pFail->m_dwParam, 0);
				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sFail, sFail.m_wLength + 1);
			}
			break;
		case enumS2C_TONG_ADD_MEMBER_SUCCESS:
			{
				STONG_ADD_MEMBER_SUCCESS_SYNC	*pSync = (STONG_ADD_MEMBER_SUCCESS_SYNC*)pChar;
				STONG_SERVER_TO_CORE_ADD_SUCCESS	sAdd;

				sAdd.m_nCamp = pSync->m_btCamp;
				sAdd.m_dwPlayerNameID = pSync->m_dwPlayerNameID;
				sAdd.m_nPlayerIdx = pSync->m_dwParam;
				memcpy(sAdd.m_szMasterName, pSync->m_szMasterName, sizeof(sAdd.m_szMasterName));
				memcpy(sAdd.m_szTitleName, pSync->m_szTitleName, sizeof(sAdd.m_szTitleName));
				memcpy(sAdd.m_szTongName, pSync->m_szTongName, sizeof(sAdd.m_szTongName));

				if (m_pCoreServerShell->OperationRequest(SSOI_TONG_ADD, (unsigned int)&sAdd, 0))
				{
				}
				else
				{
				}
			}
			break;
		case enumS2C_TONG_ADD_MEMBER_FAIL:
			{
				STONG_ADD_MEMBER_FAIL_SYNC	*pSync = (STONG_ADD_MEMBER_FAIL_SYNC*)pChar;
			}
			break;

		case enumS2C_TONG_HEAD_INFO:
			{
				STONG_HEAD_INFO_SYNC	*pInfo = (STONG_HEAD_INFO_SYNC*)pChar;
				if ((int)pInfo->m_dwParam <= 0 || (int)pInfo->m_dwParam >= MAX_PLAYER)
					break;

				int nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, pInfo->m_dwParam, 0);
				if (nNetID < 0)
					break;

				TONG_HEAD_INFO_SYNC	sInfo;
				sInfo.ProtocolType		= s2c_extendtong;
				sInfo.m_btMsgId			= enumTONG_SYNC_ID_HEAD_INFO;
				sInfo.m_dwNpcID			= pInfo->m_dwNpcID;
				sInfo.m_dwMoney			= pInfo->m_dwMoney;
				sInfo.m_nCredit			= pInfo->m_nCredit;
				sInfo.m_btCamp			= pInfo->m_btCamp;
				sInfo.m_btLevel			= pInfo->m_btLevel;
				sInfo.m_btDirectorNum	= pInfo->m_btDirectorNum;
				sInfo.m_btManagerNum	= pInfo->m_btManagerNum;
				sInfo.m_dwMemberNum		= pInfo->m_dwMemberNum;
				memcpy(sInfo.m_szTongName, pInfo->m_szTongName, sizeof(pInfo->m_szTongName));
				memset(sInfo.m_sMember, 0, sizeof(sInfo.m_sMember));
				memcpy(sInfo.m_sMember, pInfo->m_sMember, sizeof(TONG_ONE_LEADER_INFO) * (1 + sInfo.m_btDirectorNum));
				sInfo.m_wLength = sizeof(sInfo) - 1 - sizeof(sInfo.m_sMember) + sizeof(TONG_ONE_LEADER_INFO) * (1 + sInfo.m_btDirectorNum);
				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sInfo, sInfo.m_wLength + 1);
			}
			break;

		case enumS2C_TONG_MANAGER_INFO:
			{
				STONG_MANAGER_INFO_SYNC	*pInfo = (STONG_MANAGER_INFO_SYNC*)pChar;
				if ((int)pInfo->m_dwParam <= 0 || (int)pInfo->m_dwParam >= MAX_PLAYER)
					break;

				int nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, pInfo->m_dwParam, 0);
				if (nNetID < 0)
					break;

				TONG_MANAGER_INFO_SYNC	sInfo;
				sInfo.ProtocolType		= s2c_extendtong;
				sInfo.m_btMsgId			= enumTONG_SYNC_ID_MANAGER_INFO;
				sInfo.m_dwMoney			= pInfo->m_dwMoney;
				sInfo.m_nCredit			= pInfo->m_nCredit;
				sInfo.m_btCamp			= pInfo->m_btCamp;
				sInfo.m_btLevel			= pInfo->m_btLevel;
				sInfo.m_btDirectorNum	= pInfo->m_btDirectorNum;
				sInfo.m_btManagerNum	= pInfo->m_btManagerNum;
				sInfo.m_dwMemberNum		= pInfo->m_dwMemberNum;
				sInfo.m_btStateNo		= pInfo->m_btStartNo;
				sInfo.m_btCurNum		= pInfo->m_btCurNum;
				memcpy(sInfo.m_szTongName, pInfo->m_szTongName, sizeof(pInfo->m_szTongName));
				memset(sInfo.m_sMember, 0, sizeof(sInfo.m_sMember));
				memcpy(sInfo.m_sMember, pInfo->m_sMember, sizeof(TONG_ONE_LEADER_INFO) * sInfo.m_btCurNum);
				sInfo.m_wLength = sizeof(sInfo) - 1 - sizeof(TONG_ONE_LEADER_INFO) * (defTONG_ONE_PAGE_MAX_NUM - sInfo.m_btCurNum);
				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sInfo, sInfo.m_wLength + 1);
			}
			break;

		case enumS2C_TONG_MEMBER_INFO:
			{
				STONG_MEMBER_INFO_SYNC	*pInfo = (STONG_MEMBER_INFO_SYNC*)pChar;
				if ((int)pInfo->m_dwParam <= 0 || (int)pInfo->m_dwParam >= MAX_PLAYER)
					break;

				int nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, pInfo->m_dwParam, 0);
				if (nNetID < 0)
					break;

				TONG_MEMBER_INFO_SYNC	sInfo;
				sInfo.ProtocolType		= s2c_extendtong;
				sInfo.m_btMsgId			= enumTONG_SYNC_ID_MEMBER_INFO;
				sInfo.m_dwMoney			= pInfo->m_dwMoney;
				sInfo.m_nCredit			= pInfo->m_nCredit;
				sInfo.m_btCamp			= pInfo->m_btCamp;
				sInfo.m_btLevel			= pInfo->m_btLevel;
				sInfo.m_btDirectorNum	= pInfo->m_btDirectorNum;
				sInfo.m_btManagerNum	= pInfo->m_btManagerNum;
				sInfo.m_dwMemberNum		= pInfo->m_dwMemberNum;
				sInfo.m_btStateNo		= pInfo->m_btStartNo;
				sInfo.m_btCurNum		= pInfo->m_btCurNum;
				memcpy(sInfo.m_szTitle, pInfo->m_szTitle, sizeof(pInfo->m_szTitle));
				memcpy(sInfo.m_szTongName, pInfo->m_szTongName, sizeof(pInfo->m_szTongName));
				memset(sInfo.m_sMember, 0, sizeof(sInfo.m_sMember));
				memcpy(sInfo.m_sMember, pInfo->m_sMember, sizeof(TONG_ONE_MEMBER_INFO) * sInfo.m_btCurNum);
				sInfo.m_wLength = sizeof(sInfo) - 1 - sizeof(TONG_ONE_MEMBER_INFO) * (defTONG_ONE_PAGE_MAX_NUM - sInfo.m_btCurNum);
				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sInfo, sInfo.m_wLength + 1);
			}
			break;

		case enumS2C_TONG_BE_INSTATED:
			{
				STONG_BE_INSTATED_SYNC	*pSync = (STONG_BE_INSTATED_SYNC*)pChar;
				if (m_pGameStatus[pSync->m_dwParam].nPlayerIndex <= 0)
					break;
				STONG_SERVER_TO_CORE_BE_INSTATED	sInstated;
				sInstated.m_nPlayerIdx = m_pGameStatus[pSync->m_dwParam].nPlayerIndex;
				sInstated.m_btFigure = pSync->m_btFigure;
				sInstated.m_btPos = pSync->m_btPos;
				memcpy(sInstated.m_szName, pSync->m_szName, sizeof(pSync->m_szName));
				memcpy(sInstated.m_szTitle, pSync->m_szTitle, sizeof(pSync->m_szTitle));
				m_pCoreServerShell->GetGameData(SGDI_TONG_BE_INSTATED, (unsigned int)&sInstated, 0);
			}
			break;

		case enumS2C_TONG_INSTATE:
			{
				STONG_INSTATE_SYNC	*pSync = (STONG_INSTATE_SYNC*)pChar;
				TONG_INSTATE_SYNC	sInstate;

				if ((int)pSync->m_dwParam <= 0 || (int)pSync->m_dwParam >= MAX_PLAYER)
					break;

				int nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, pSync->m_dwParam, 0);
				if (nNetID < 0)
					break;

				sInstate.ProtocolType		= s2c_extendtong;
				sInstate.m_btMsgId			= enumTONG_SYNC_ID_INSTATE;
				sInstate.m_wLength			= sizeof(sInstate) - 1;
				sInstate.m_dwTongNameID		= pSync->m_dwTongNameID;
				sInstate.m_btSuccessFlag	= pSync->m_btSuccessFlag;
				sInstate.m_btOldFigure		= pSync->m_btOldFigure;
				sInstate.m_btOldPos			= pSync->m_btOldPos;
				sInstate.m_btNewFigure		= pSync->m_btNewFigure;
				sInstate.m_btNewPos			= pSync->m_btNewPos;
				memcpy(sInstate.m_szTitle, pSync->m_szTitle, sizeof(pSync->m_szTitle));
				memcpy(sInstate.m_szName, pSync->m_szName, sizeof(pSync->m_szName));

				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sInstate, sInstate.m_wLength + 1);
			}
			break;

		case enumS2C_TONG_KICK:
			{
				STONG_KICK_SYNC	*pSync = (STONG_KICK_SYNC*)pChar;
				TONG_KICK_SYNC	sKick;

				if ((int)pSync->m_dwParam <= 0 || (int)pSync->m_dwParam >= MAX_PLAYER)
					break;

				int nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, pSync->m_dwParam, 0);
				if (nNetID < 0)
					break;

				sKick.ProtocolType		= s2c_extendtong;
				sKick.m_btMsgId			= enumTONG_SYNC_ID_KICK;
				sKick.m_wLength			= sizeof(sKick) - 1;
				sKick.m_dwTongNameID	= pSync->m_dwTongNameID;
				sKick.m_btSuccessFlag	= pSync->m_btSuccessFlag;
				sKick.m_btFigure		= pSync->m_btFigure;
				sKick.m_btPos			= pSync->m_btPos;
				memcpy(sKick.m_szName, pSync->m_szName, sizeof(pSync->m_szName));

				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sKick, sKick.m_wLength + 1);
			}
			break;

		case enumS2C_TONG_BE_KICKED:
			{
				STONG_BE_KICKED_SYNC	*pSync = (STONG_BE_KICKED_SYNC*)pChar;
				if (m_pGameStatus[pSync->m_dwParam].nPlayerIndex <= 0)
					break;
				STONG_SERVER_TO_CORE_BE_KICKED	sKicked;
				sKicked.m_nPlayerIdx = m_pGameStatus[pSync->m_dwParam].nPlayerIndex;
				sKicked.m_btFigure = pSync->m_btFigure;
				sKicked.m_btPos = pSync->m_btPos;
				memcpy(sKicked.m_szName, pSync->m_szName, sizeof(pSync->m_szName));
				m_pCoreServerShell->GetGameData(SGDI_TONG_BE_KICKED, (unsigned int)&sKicked, 0);
			}
			break;

		case enumS2C_TONG_LEAVE:
			{
				STONG_LEAVE_SYNC	*pSync = (STONG_LEAVE_SYNC*)pChar;
				STONG_SERVER_TO_CORE_LEAVE	sLeave;
				sLeave.m_nPlayerIdx = pSync->m_dwParam;
				sLeave.m_bSuccessFlag = pSync->m_btSuccessFlag;
				memcpy(sLeave.m_szName, pSync->m_szName, sizeof(pSync->m_szName));
				m_pCoreServerShell->GetGameData(SGDI_TONG_LEAVE, (unsigned int)&sLeave, 0);
			}
			break;

		case enumS2C_TONG_CHECK_CHANGE_MASTER_POWER:
			{
				STONG_CHECK_GET_MASTER_POWER_SYNC	*pSync = (STONG_CHECK_GET_MASTER_POWER_SYNC*)pChar;
				if (m_pGameStatus[pSync->m_dwParam].nPlayerIndex <= 0)
					break;
				STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER	sCheck;
				sCheck.m_dwTongNameID	= pSync->m_dwTongNameID;
				sCheck.m_btFigure		= pSync->m_btFigure;
				sCheck.m_btPos			= pSync->m_btPos;
				sCheck.m_nPlayerIdx		= m_pGameStatus[pSync->m_dwParam].nPlayerIndex;
				memcpy(sCheck.m_szName, pSync->m_szName, sizeof(pSync->m_szName));

				int nRet = 0;
				if (m_pCoreServerShell)
					nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_GET_MASTER_POWER, (unsigned int)&sCheck, 0);

				STONG_ACCEPT_MASTER_COMMAND	sAccept;

				sAccept.ProtocolFamily	= pf_tong;
				sAccept.ProtocolID		= enumC2S_TONG_ACCEPT_MASTER;
				sAccept.m_dwParam		= m_pGameStatus[pSync->m_dwParam].nPlayerIndex;
				sAccept.m_dwTongNameID	= pSync->m_dwTongNameID;
				sAccept.m_btFigure		= pSync->m_btFigure;
				sAccept.m_btPos			= pSync->m_btPos;
				if (nRet)
					sAccept.m_btAcceptFalg = 1;
				else
					sAccept.m_btAcceptFalg = 0;
				memcpy(sAccept.m_szName, pSync->m_szName, sizeof(pSync->m_szName));
				
				if (m_pTongClient)
					m_pTongClient->SendPackToServer((const void*)&sAccept, sizeof(sAccept));
			}
			break;

		case enumS2C_TONG_CHANGE_MASTER_FAIL:
			{
				STONG_CHANGE_MASTER_FAIL_SYNC	*pFail = (STONG_CHANGE_MASTER_FAIL_SYNC*)pChar;
				int nPlayer = pFail->m_dwParam;
				switch (pFail->m_btFailID)
				{
				case 0:
					break;
				case 1:
					nPlayer = m_pGameStatus[pFail->m_dwParam].nPlayerIndex;
					break;
				default:
					break;
				}

				if (nPlayer <= 0 || nPlayer >= MAX_PLAYER)
					break;
				int nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, nPlayer, 0);
				if (nNetID < 0)
					break;

				TONG_CHANGE_MASTER_FAIL_SYNC	sFail;
				sFail.ProtocolType		= s2c_extendtong;
				sFail.m_btMsgId			= enumTONG_SYNC_ID_CHANGE_MASTER_FAIL;
				sFail.m_wLength			= sizeof(sFail) - 1;
				sFail.m_dwTongNameID	= pFail->m_dwTongNameID;
				sFail.m_btFailID		= pFail->m_btFailID;
				memcpy(sFail.m_szName, pFail->m_szName, sizeof(pFail->m_szName));
				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sFail, sFail.m_wLength + 1);
			}
			break;

		case enumS2C_TONG_CHANGE_AS:
			{
				STONG_CHANGE_AS_SYNC	*pAs = (STONG_CHANGE_AS_SYNC*)pChar;
				if (m_pGameStatus[pAs->m_dwParam].nPlayerIndex <= 0)
					break;
				STONG_SERVER_TO_CORE_CHANGE_AS	sChange;
				sChange.m_nPlayerIdx = m_pGameStatus[pAs->m_dwParam].nPlayerIndex;
				sChange.m_dwTongNameID = pAs->m_dwTongNameID;
				sChange.m_btFigure = pAs->m_btFigure;
				sChange.m_btPos = pAs->m_btPos;
				// 自己的新头衔
				memcpy(sChange.m_szTitle, pAs->m_szTitle, sizeof(pAs->m_szTitle));
				// 新帮主的名字
				memcpy(sChange.m_szName, pAs->m_szName, sizeof(pAs->m_szName));
				m_pCoreServerShell->GetGameData(SGDI_TONG_CHANGE_AS, (unsigned int)&sChange, 0);
			}
			break;

		case enumS2C_TONG_CHANGE_MASTER:
			{
				STONG_CHANGE_MASTER_SYNC	*pMaster = (STONG_CHANGE_MASTER_SYNC*)pChar;
				STONG_SERVER_TO_CORE_CHANGE_MASTER	sChange;
				sChange.m_dwTongNameID = pMaster->m_dwTongNameID;
				memcpy(sChange.m_szName, pMaster->m_szName, sizeof(pMaster->m_szName));
				m_pCoreServerShell->GetGameData(SGDI_TONG_CHANGE_MASTER, (unsigned int)&sChange, 0);
			}
			break;

		case enumS2C_TONG_LOGIN_DATA:
			{
				STONG_LOGIN_DATA_SYNC	*pLogin = (STONG_LOGIN_DATA_SYNC*)pChar;
				STONG_SERVER_TO_CORE_LOGIN	sLogin;
				sLogin.m_dwParam	= pLogin->m_dwParam;
				sLogin.m_nFlag		= pLogin->m_btFlag;
				sLogin.m_nCamp		= pLogin->m_btCamp;
				sLogin.m_nFigure	= pLogin->m_btFigure;
				sLogin.m_nPos		= pLogin->m_btPos;
				memcpy(sLogin.m_szTongName, pLogin->m_szTongName, sizeof(pLogin->m_szTongName));
				memcpy(sLogin.m_szTitle, pLogin->m_szTitle, sizeof(pLogin->m_szTitle));
				memcpy(sLogin.m_szMaster, pLogin->m_szMaster, sizeof(pLogin->m_szMaster));
				memcpy(sLogin.m_szName, pLogin->m_szName, sizeof(pLogin->m_szName));
				m_pCoreServerShell->GetGameData(SGDI_TONG_LOGIN, (unsigned int)&sLogin, 0);
			}
			break;

		default:
			break;
		}
	}
	else if (pHeader->ProtocolFamily == pf_friend)
	{
	}
}

void KSwordOnLineSever::ChatGroupMan(const void *pData, size_t dataLength)
{
	_ASSERT(pData && dataLength);

	CHAT_GROUPMAN*	pCgc = (CHAT_GROUPMAN *)pData;
	_ASSERT(pCgc->wSize + 1 == dataLength);

	_ASSERT(sizeof(tagExtendProtoHeader) <= sizeof(CHAT_GROUPMAN));
	void* pExPckg = pCgc + 1;
	BYTE hasIdentify = pCgc->byHasIdentify;
	WORD playercount = pCgc->wPlayerCount;
	void* pPlayersData = (tagPlusSrcInfo*)((BYTE*)pExPckg + pCgc->wChatLength);
	size_t pckgsize = sizeof(tagExtendProtoHeader) + pCgc->wChatLength;
	tagExtendProtoHeader* pExHdr = (tagExtendProtoHeader*)pExPckg - 1;
	pExHdr->ProtocolType = s2c_extendchat;
	pExHdr->wLength = pckgsize - 1;

	if (hasIdentify)
	{
		tagPlusSrcInfo* pPlayers = (tagPlusSrcInfo*)pPlayersData;
		for (int i = 0; i < playercount; i++)
		{
			//TODO: 要加检验NameID和lnID的一致性
			if (CheckPlayerID(pPlayers[i].lnID, pPlayers[i].nameid))
				m_pServer->PackDataToClient(pPlayers[i].lnID, pExHdr, pckgsize);
		}
	}
	else
	{
		WORD* pPlayers = (WORD*)pPlayersData;
		for (int i = 0; i < playercount; i++)
		{
			//TODO: 不需要加检验NameID和lnID的一致性
			//if (pPlayers[i] >= 0)
				m_pServer->PackDataToClient((unsigned long)pPlayers[i], pExHdr, pckgsize);
		}
	}
}

void KSwordOnLineSever::ChatSpecMan(const void *pData, size_t dataLength)
{
	_ASSERT(pData && dataLength);

	CHAT_SPECMAN*	pCsm = (CHAT_SPECMAN *)pData;
	_ASSERT(pCsm->wSize + 1 == dataLength);

	if (!CheckPlayerID(pCsm->lnID, pCsm->nameid))
		return;

	_ASSERT(sizeof(tagExtendProtoHeader) <= sizeof(CHAT_SPECMAN));
	void* pExPckg = pCsm + 1;

	unsigned long lnID = pCsm->lnID;
	size_t pckgsize = sizeof(tagExtendProtoHeader) + pCsm->wChatLength;

	tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(pCsm + 1) - 1;
	pExHeader->ProtocolType = s2c_extendchat;
	pExHeader->wLength = pckgsize - 1;

	m_pServer->PackDataToClient(lnID, pExHeader, pckgsize);
}


void KSwordOnLineSever::DatabaseMessageProcess(const char* pData, size_t dataLength)
{
	_ASSERT( pData && dataLength );

#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pData );
#else
	BYTE cProtocol = *(BYTE *)pData;
#endif	

	// 大包处理（用于排名的数据）
	if ( cProtocol < s2c_micropackbegin )
	{
		DatabaseLargePackProcess(pData, dataLength);
		return;
	}

	switch ( cProtocol )
	{
	case s2c_roleserver_saverole_result:
		//m_bSaveFlag = TRUE;
		{
			TProcessData*	pPD = (TProcessData *)pData;
			int nIndex = pPD->ulIdentity;
			m_pCoreServerShell->SetSaveStatus(nIndex, SAVE_IDLE);
			printf("  Save Player Data finished(%d)!\n", nIndex);
		}
		break;
	default:
		printf("Protocol:(%d) -- database error", cProtocol);
		break;
	}
}

void KSwordOnLineSever::DatabaseLargePackProcess(const char* pData, size_t dataLength)
{
	_ASSERT( pData && dataLength );

#ifndef _STANDALONE
	CBuffer *pBuffer = m_theRecv.PackUp( pData, dataLength );
#else
	char *pBuffer = m_theRecv.PackUp( pData, dataLength );
#endif

	if ( pBuffer )
	{
#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pBuffer->GetBuffer() );
#else
	BYTE cProtocol = *(BYTE *)pBuffer;
#endif	
//		BYTE cProtocol = CPackager::Peek( pBuffer->GetBuffer() );

		switch ( cProtocol )
		{
		case s2c_gamestatistic:
			{
#ifndef _STANDALONE
				TProcessData*	pRD	= (TProcessData *)pBuffer->GetBuffer();
#else
				TProcessData*	pRD	= (TProcessData *)pBuffer;
#endif
				_ASSERT( pRD->nDataLen == sizeof(TGAME_STAT_DATA) );

				if (m_pCoreServerShell)
				{
					m_pCoreServerShell->SetLadder((void *)pRD->pDataBuffer, pRD->nDataLen);
				}
			}
			break;

		default:
			break;			
		}

#ifndef _STANDALONE
		try
		{
			if( pBuffer != NULL )
			{
				pBuffer->Release();
				pBuffer = NULL; 
			}
		}
		catch(...) 
		{
			//TRACE("SAFE_RELEASE error\n");
		}
#endif
	}
}

void KSwordOnLineSever::TransferMessageProcess(const char* pChar, size_t nSize)
{
//#ifndef _STANDALONE
	_ASSERT(pChar && nSize && nSize < 1024);

	EXTEND_HEADER	*pEH = (EXTEND_HEADER *)pChar;

	if (nSize < sizeof(EXTEND_HEADER))
		return;

	// 是否寻路包？
	if (pEH->ProtocolID == relay_c2c_askwaydata && pEH->ProtocolFamily == pf_relay)
	{
		TransferAskWayMessageProcess(pChar, nSize);
		return;
	}

	RELAY_DATA	*pRD = (RELAY_DATA *)pEH;
	if (nSize <= sizeof(RELAY_DATA) || nSize != pRD->routeDateLength + sizeof(RELAY_DATA))
		return;

	// 是否寻路失败包
	if (pEH->ProtocolID == relay_s2c_loseway && pEH->ProtocolFamily == pf_relay)
	{
		TransferLoseWayMessageProcess(pChar + sizeof(RELAY_DATA), nSize - sizeof(RELAY_DATA));
		return;
	}

	if (pEH->ProtocolID != relay_c2c_data && pEH->ProtocolFamily != pf_relay)
		return;

	KTransferUnit* pUnit = NULL;
	IP2CONNECTUNIT::iterator it = m_mapIp2TransferUnit.find(pRD->nFromRelayID);

	if (it != m_mapIp2TransferUnit.end())
	{
		pUnit = (*it).second;
		_ASSERT(pUnit);
	}
	else
	{
		pUnit = new KTransferUnit(pRD->nFromIP, pRD->nFromRelayID);
		m_mapIp2TransferUnit.insert(IP2CONNECTUNIT::value_type(pRD->nFromRelayID, pUnit));
	}

#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek(pChar, sizeof(RELAY_DATA));
#else
	BYTE cProtocol = *(BYTE *)(pChar + sizeof(RELAY_DATA));
#endif	

	if ( cProtocol < c2c_micropackbegin )
	{
		TransferLargePackProcess(pChar + sizeof(RELAY_DATA), pRD->routeDateLength, pUnit);
	}
	else if ( cProtocol > c2c_micropackbegin )
	{
		TransferSmallPackProcess(pChar + sizeof(RELAY_DATA), pRD->routeDateLength, pUnit);
	}
	else
	{
		_ASSERT( FALSE && "Error!" );
	}
//#endif
}

void KSwordOnLineSever::TransferLargePackProcess(const void *pData, size_t dataLength, KTransferUnit *pUnit)
{
	_ASSERT(pData && dataLength);
	if (!pData || !dataLength)
		return;
	
#ifndef _STANDALONE
	CBuffer *pBuffer = pUnit->m_RecvData.PackUp( pData, dataLength );
#else
	char *pBuffer = pUnit->m_RecvData.PackUp( pData, dataLength );
#endif
	char	szChar[1024];

	if ( pBuffer )
	{
#ifndef _STANDALONE
		BYTE cProtocol = CPackager::Peek( pBuffer->GetBuffer() );
#else
		BYTE cProtocol = *(BYTE *)pBuffer;
#endif		
		
		switch ( cProtocol )
		{
		case c2c_transferroleinfo:
			{
#ifndef _STANDALONE
				tagGuidableInfo *pGI = (tagGuidableInfo *)pBuffer->GetBuffer();
#else
				tagGuidableInfo *pGI = (tagGuidableInfo *)pBuffer;
#endif

				GUID guid;
				memcpy( &guid, &( pGI->guid ), sizeof( GUID ) );
				
				if ( pGI->datalength > 0 )
				{
					const TRoleData *pRoleData = ( const TRoleData * )( pGI->szData );
					DWORD	dwCRC = 0;
					dwCRC = CRC32(dwCRC, pRoleData, pRoleData->dwDataLen - 4);
					DWORD	dwCheck = *(DWORD *)(pGI->szData + pRoleData->dwDataLen - 4);
					BOOL	bCRC = (dwCheck == dwCRC);

					int nIdx = 0;
					if (bCRC)
					{
						nIdx = m_pCoreServerShell->AddCharacter(pGI->nExtPoint, pGI->nChangePoint, (void *)pRoleData, &guid);
					}
					else
					{
						FILE *fLog = fopen("crc_transfer_error", "a+b");
						if(fLog) 
						{
							char buffer[255];
							sprintf(buffer, "----\r\n%s\r\b%s\r\n", pRoleData->BaseInfo.szName, pRoleData->BaseInfo.caccname);
							fwrite(buffer, 1, strlen(buffer), fLog);
							fwrite(pRoleData, 1, pRoleData->dwDataLen, fLog);
							fclose(fLog);
						}
					}

					RELAY_DATA*	pRD = (RELAY_DATA *)szChar;
					pRD->ProtocolFamily = pf_relay;
					pRD->ProtocolID = relay_c2c_data;
					pRD->nFromIP = 0;
					pRD->nFromRelayID = 0;
					pRD->nToIP = pUnit->GetIp();
					pRD->nToRelayID = pUnit->GetRelayID();
					pRD->routeDateLength = sizeof(tagPermitPlayerExchange);

					tagPermitPlayerExchange *pPPEO = (tagPermitPlayerExchange *)(szChar + sizeof(RELAY_DATA));

					pPPEO->cProtocol = c2c_permitplayerexchangein;
					memcpy(&pPPEO->guid, &guid, sizeof(GUID));

					pPPEO->dwIp = m_dwInternetIp;//OnlineGameLib::Win32::net_aton(m_szGameSvrIP);
					pPPEO->wPort = m_nServerPort;

					if (nIdx)
					{
						pPPEO->bPermit = true;
						tagRegisterAccount	ra;
						ra.cProtocol = c2s_registeraccount;
						strcpy((char *)ra.szAccountName, pRoleData->BaseInfo.caccname);
						m_pGatewayClient->SendPackToServer(&ra, sizeof(tagRegisterAccount));
					}
					else
					{
						pPPEO->bPermit = false;
					}
					m_pTransferClient->SendPackToServer(pRD, sizeof(RELAY_DATA) + sizeof(tagPermitPlayerExchange));

				}
				else
				{
//					cout << "Don't find any valid data!" << endl;
				}
			}
			break;
		default:
			break;			
		}

#ifndef _STANDALONE
		try
		{
			if( pBuffer != NULL )
			{
				pBuffer->Release();
				pBuffer = NULL; 
			}
		}
		catch(...) 
		{
			//TRACE("SAFE_RELEASE error\n");
		}
#endif
	}	
}

void KSwordOnLineSever::TransferSmallPackProcess(const void *pData, size_t dataLength, KTransferUnit* pUnit)
{
//#ifndef _STANDALONE
	_ASSERT(pUnit && pData && dataLength);
	if (!pUnit || !pData || !dataLength)
		return;

#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pData );
#else
	BYTE cProtocol = *(BYTE *)pData;
#endif	
	
	switch ( cProtocol )
	{
	case s2s_broadcast:
		{
			BYTE* pS = (BYTE *)pData;
			m_pCoreServerShell->ProcessBroadcastMessage((const char *)(pS + 1), dataLength - 1);
		}
		break;
	case s2s_execute:
		{
			BYTE* pS = (BYTE *)pData;
			m_pCoreServerShell->ProcessExecuteMessage((const char *)(pS + 1), dataLength - 1);
		}
		break;
	case c2c_permitplayerexchangein:
		{
			tagPermitPlayerExchange* pPermit = (tagPermitPlayerExchange *)pData;
			int nIndex, lnID;
			m_pCoreServerShell->GetPlayerIndexByGuid(&pPermit->guid, &nIndex, &lnID);
			if (nIndex == 0 || lnID == -1)
				break;

			if (m_pGameStatus[lnID].nExchangeStatus != enumExchangeWaitForGameSvrRespone)
				break;

			tagNotifyPlayerExchange	npe;
			if (pPermit->bPermit)
			{
//				SavePlayerData(nIndex);
				m_pCoreServerShell->RemovePlayerForExchange(nIndex);
				npe.cProtocol = s2c_notifyplayerexchange;
				memcpy(&npe.guid, &pPermit->guid, sizeof(GUID));
				npe.nIPAddr = pPermit->dwIp;
				npe.nPort = pPermit->wPort;
				m_pServer->SendData(lnID, &npe, sizeof(tagNotifyPlayerExchange));

				tagLeaveGame	sLeaveGame;
				sLeaveGame.cProtocol = c2s_leavegame;
				sLeaveGame.cCmdType = HOLDACC_LEAVEGAME;
				char	szName[32];
				m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ACCOUNT, (unsigned int)szName, nIndex);
				sLeaveGame.nExtPoint = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_EXTPOINTCHANGED, nIndex, 0);

				strcpy((char *)sLeaveGame.szAccountName, (char *)szName);
				m_pGatewayClient->SendPackToServer(&sLeaveGame, sizeof(tagLeaveGame));

				tagLeaveGame2 lg2;
				
				lg2.ProtocolFamily = pf_normal;
				lg2.ProtocolID = c2s_leavegame;
				lg2.cCmdType = HOLDACC_LEAVEGAME;
				strcpy((char *)lg2.szAccountName, (char *)szName);

				if (m_pTransferClient)
					m_pTransferClient->SendPackToServer(&lg2, sizeof(tagLeaveGame2));
//				cout << sLeaveGame.szAccountName << " leave game!" << endl;
				m_pGameStatus[lnID].nExchangeStatus = enumExchangeCleaning;
				m_pGameStatus[lnID].nPlayerIndex = 0;
			}
			else
			{
				npe.cProtocol = s2c_notifyplayerexchange;
				memset(&npe.guid, 0, sizeof(GUID));
				// if false, ip to 0
				npe.nIPAddr = 0;
				npe.nPort = 0;
				m_pServer->SendData(lnID, &npe, sizeof(tagNotifyPlayerExchange));
				
				// TODO: Recover player to old game
				m_pCoreServerShell->RecoverPlayerExchange(nIndex);

				tagRoleEnterGame reg;
				reg.ProtocolType = c2s_roleserver_lock;
				reg.bLock = true;
				m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)reg.Name, nIndex);
				if (m_pDatabaseClient)
					m_pDatabaseClient->SendPackToServer((const void *)&reg, sizeof(tagRoleEnterGame));

				m_pGameStatus[lnID].nGameStatus = enumPlayerPlaying;
				m_pGameStatus[lnID].nExchangeStatus = enumExchangeBegin;
			}
		}
		break;
	case c2c_notifyexchange:
		{
			tagSearchWay *pSW = (tagSearchWay *)pData;

			if (pSW->lnID < 0 || pSW->lnID >= m_nMaxPlayer)
			{
				printf("break 1\n");
				break;
			}

			if (m_pGameStatus[pSW->lnID].nExchangeStatus != enumExchangeSearchingWay)
			{
				printf("break 2\n");
				break;
			}
			
			int nIndex = m_pGameStatus[pSW->lnID].nPlayerIndex;
			
			if (nIndex <= 0 || nIndex > m_nMaxPlayer || nIndex != pSW->nIndex)
			{
				printf("break 3\n");
				break;
			}

			if (pSW->dwPlayerID != m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, nIndex, 0))
			{
				printf("break 4\n");
				break;
			}

			char	szData[1024];

			RELAY_DATA *pRD = (RELAY_DATA *)szData;
			
			pRD->ProtocolFamily = pf_relay;
			pRD->ProtocolID = relay_c2c_data;
			pRD->nToIP = pUnit->GetIp();
			pRD->nToRelayID = pUnit->GetRelayID();
			pRD->nFromIP = 0;
			pRD->nFromRelayID = 0;

			tagGuidableInfo gi;

			TRoleData* pData = (TRoleData *)m_pCoreServerShell->PreparePlayerForExchange(nIndex);

			if (!pData)
			{
				printf("break 5\n");
				break;
			}

			gi.cProtocol = c2c_transferroleinfo;
			m_pCoreServerShell->GetGuid(nIndex, &gi.guid);
			
			gi.nExtPoint = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_EXTPOINT, nIndex, 0);
			gi.nChangePoint = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_EXTPOINTCHANGED, nIndex, 0);

			pData->dwDataLen += 4;
			gi.datalength = pData->dwDataLen;
			DWORD	dwCRC = 0;
			dwCRC = CRC32(dwCRC, pData, pData->dwDataLen - 4);

			m_theSend.AddData(c2c_transferroleinfo, (const char*)&gi, sizeof(tagGuidableInfo));
			m_theSend.AddData(c2c_transferroleinfo, (const char*)pData, gi.datalength - 4);
			m_theSend.AddData(c2c_transferroleinfo, (const char*)&dwCRC, 4);
			
#ifndef _STANDALONE
			CBuffer* pBuffer = m_theSend.GetHeadPack(c2c_transferroleinfo, 512);
#else
			size_t actLen = 0;
			char* pBuffer = m_theSend.GetHeadPack(c2c_transferroleinfo, actLen, 512);
#endif
			

			while(pBuffer)
			{

#ifndef _STANDALONE
				pRD->routeDateLength = pBuffer->GetUsed();
#else
				pRD->routeDateLength = actLen;
#endif
				_ASSERT(pRD->routeDateLength + sizeof(RELAY_DATA) < 1024);

#ifndef _STANDALONE
				memcpy(szData + sizeof(RELAY_DATA), pBuffer->GetBuffer(), pBuffer->GetUsed());
#else
				memcpy(szData + sizeof(RELAY_DATA), pBuffer, actLen);
#endif

				m_pTransferClient->SendPackToServer(szData, pRD->routeDateLength + sizeof(RELAY_DATA));
#ifndef _STANDALONE
				try
				{
					if (pBuffer)
					{
						pBuffer->Release();
						pBuffer = NULL;
					}
				}
				catch(...) 
				{
					//TRACE("SAFE_RELEASE error\n");
				}
#endif

#ifndef _STANDALONE
				pBuffer = m_theSend.GetNextPack(c2c_transferroleinfo);
#else
				pBuffer = m_theSend.GetNextPack(c2c_transferroleinfo, actLen);
#endif
			}
			m_theSend.DelData(c2c_transferroleinfo);
#ifndef _STANDALONE
			try
			{
				if (pBuffer)
				{
					pBuffer->Release();
					pBuffer = NULL;
				}
			}
			catch(...) 
			{
				//TRACE("SAFE_RELEASE error\n");
			}
#endif
			m_pGameStatus[pSW->lnID].nExchangeStatus = enumExchangeWaitForGameSvrRespone;
		}
		break;
	}
//#endif
}

void KSwordOnLineSever::PlayerMessageProcess(const unsigned long lnID, const char* pData, size_t dataLength)
{
//#ifndef _STANDALONE
	_ASSERT(pData && dataLength);

#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pData );
#else
	BYTE cProtocol = *(BYTE *)pData;
#endif	

	int nGameStatus = m_pGameStatus[lnID].nGameStatus;
	int nIndex = m_pGameStatus[lnID].nPlayerIndex;
	
	BYTE protocoltype = *(BYTE*)pData;
	if (protocoltype >= _c2s_begin_relay && protocoltype <= _c2s_end_relay)
	{
		IClient* pClient = NULL;
		switch (protocoltype)
		{
		case c2s_extend: pClient = m_pTransferClient; break;
		case c2s_extendchat: pClient = m_pChatClient; break;
		case c2s_extendfriend: pClient = m_pTongClient; break;
		default:
			return;
		};

		if (pClient && nIndex && dataLength > sizeof(tagExtendProtoHeader))
		{		
			size_t exsize = dataLength - sizeof(tagExtendProtoHeader);
			const void* pExPckg = pData + sizeof(tagExtendProtoHeader);

			if (protocoltype == c2s_extendchat)
			{
				CHAT_CHANNELCHAT_CMD* pEh = (CHAT_CHANNELCHAT_CMD*)pExPckg;
				if (pEh->ProtocolType == chat_channelchat && pEh->channelid != 0)	//非GM频道要过滤和付钱
				{
					if (!m_pCoreServerShell->PayForSpeech(nIndex, pEh->cost))
						return;
				}
			}

			size_t pckgsize = exsize + sizeof(tagPlusSrcInfo);
#ifdef WIN32
			void* pPckg2 = _alloca(pckgsize);
#else
			void* pPckg2 = (new char[pckgsize]);
#endif
			memcpy(pPckg2, pExPckg, exsize);
			tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pPckg2 + exsize);
			pSrcInfo->nameid = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, nIndex, 0);
			pSrcInfo->lnID = lnID;

			pClient->SendPackToServer(pPckg2, pckgsize);
#ifndef WIN32
			delete (char*)pPckg2;
#endif
		}
		return;
	}
	if (!m_pCoreServerShell->CheckProtocolSize(pData, dataLength))
		return;

	//printf("player msg arrived...%d\n", nGameStatus); //[wxb 2003-7-28]
	switch(nGameStatus)
	{
	case enumPlayerPlaying:
		if (nIndex)
		{
			if (*(BYTE*)pData == c2s_ping)
			{
				ProcessPingReply(lnID, pData, dataLength);
			}
			else if (*(BYTE*)pData == c2s_extendtong)
			{
				ProcessPlayerTongMsg(nIndex, pData, dataLength);
			}
			else
			{
				m_pCoreServerShell->ProcessClientMessage(nIndex, pData, dataLength);
			}
		}
		break;
	case enumPlayerExchangingServer:
		break;
	case enumPlayerSyncEnd:
		if (ProcessSyncReplyProtocol(lnID, pData, dataLength))
		{
			m_pCoreServerShell->AddPlayerToWorld(nIndex);
			m_pGameStatus[lnID].nGameStatus = enumPlayerPlaying;
			PingClient(lnID);

			tagEnterGame eg;
			eg.cProtocol = c2s_entergame;

			m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ACCOUNT, (unsigned int)eg.szAccountName, nIndex);
			
			if (m_pGatewayClient)
				m_pGatewayClient->SendPackToServer( ( const void * )&eg, sizeof( tagEnterGame ) );

			tagEnterGame2 eg2;
			eg2.ProtocolFamily = pf_normal;
			eg2.ProtocolID = c2s_entergame;
			strcpy(( char * )eg2.szAccountName, (char *)eg.szAccountName);
			eg2.dwNameID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, nIndex, 0);
			m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)eg2.szCharacterName, nIndex);
			eg2.lnID = lnID;
			if (m_pTransferClient)
				m_pTransferClient->SendPackToServer( (const void *)&eg2, sizeof(tagEnterGame2) );

			//锁定角色数据库
			tagRoleEnterGame reg;
			reg.ProtocolType = c2s_roleserver_lock;
			reg.bLock = true;
			strcpy((char *)reg.Name, (char *)eg2.szCharacterName);
			if (m_pDatabaseClient)
				m_pDatabaseClient->SendPackToServer((const void *)&reg, sizeof(tagRoleEnterGame));

			// 获取帮会信息
			int		nPlayerIdx = m_pGameStatus[lnID].nPlayerIndex;
			if (nPlayerIdx > 0 && nPlayerIdx < MAX_PLAYER)
			{
				DWORD	dwTongNameID = m_pCoreServerShell->GetGameData(SGDI_TONG_GET_TONG_NAMEID, 0, nPlayerIdx);
				char	szName[32];
				szName[0] = 0;
				m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)szName, nPlayerIdx);
				if (dwTongNameID > 0 && szName[0])
				{
					STONG_GET_LOGIN_DATA_COMMAND	sLogin;
					sLogin.ProtocolFamily	= pf_tong;
					sLogin.ProtocolID		= enumC2S_TONG_GET_LOGIN_DATA;
					sLogin.m_dwParam		= nPlayerIdx;
					sLogin.m_dwTongNameID	= dwTongNameID;
					strcpy(sLogin.m_szName, szName);
					if (m_pTongClient)
						m_pTongClient->SendPackToServer((const void*)&sLogin, sizeof(sLogin));
				}
				m_pCoreServerShell->GetGameData(SGDI_TONG_SEND_SELF_INFO, 0, nPlayerIdx);
			}
		}
		break;
	case enumPlayerBegin:
		{
			int nIndex = ProcessLoginProtocol(lnID, pData, dataLength);
			//printf("process login %d...\n", nIndex);
			if (nIndex)
			{
				if (SendGameDataToClient(lnID, nIndex))
				{
					//printf("process login %d ok...\n", nIndex);
					m_pGameStatus[lnID].nGameStatus = enumPlayerSyncEnd;
					m_pGameStatus[lnID].nPlayerIndex = nIndex;
				}

				else
				{
					//printf("process login %d fail...\n", nIndex);
					m_pGameStatus[lnID].nGameStatus = enumPlayerBegin;
					m_pGameStatus[lnID].nPlayerIndex = nIndex;
#ifndef _STANDALONE
					m_pServer->ShutdownClient(lnID);
#endif
				}
			}
		}
		break;
	}
	//printf("player msg over..\n");
//#endif
}

void KSwordOnLineSever::GatewayMessageProcess(const char* pData, size_t dataLength)
{
	_ASSERT(pData && dataLength);

#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pData );
#else
	BYTE cProtocol = *(unsigned char *)pData;
#endif
	
	if ( cProtocol < s2c_micropackbegin )
	{
		GatewayLargePackProcess(pData, dataLength);
	}
	else if ( cProtocol > s2c_micropackbegin )
	{
		GatewaySmallPackProcess(pData, dataLength);
	}
	else
	{
		_ASSERT( FALSE && "Error!" );
	}		
}

void KSwordOnLineSever::GatewayLargePackProcess(const void *pData, size_t dataLength)
{
#ifndef _STANDALONE
	_ASSERT( pData && dataLength );

	CBuffer *pBuffer = m_theRecv.PackUp( pData, dataLength );
#else
	char *pBuffer = m_theRecv.PackUp( pData, dataLength );
#endif
	if ( pBuffer )
	{
#ifndef _STANDALONE
		BYTE cProtocol = CPackager::Peek( pBuffer->GetBuffer() );
#else
		BYTE cProtocol = *(BYTE *)pBuffer;
#endif		
		switch ( cProtocol )
		{
		case s2c_syncgamesvr_roleinfo_cipher:
			{
#ifndef _STANDALONE
				tagGuidableInfo *pGI = ( tagGuidableInfo * )pBuffer->GetBuffer();
#else
				tagGuidableInfo *pGI = ( tagGuidableInfo * )pBuffer;
#endif
				GUID guid;
				memcpy( &guid, &( pGI->guid ), sizeof( GUID ) );
				
				if ( pGI->datalength > 0 )
				{
					const TRoleData *pRoleData = ( const TRoleData * )( pGI->szData );

					BOOL	bCRCCheck = TRUE;
					if (pRoleData->dwFriendOffset == pRoleData->dwDataLen - 4 && pRoleData->nFriendCount == 0)
					{
						// 说明是加过校验的，所以好友的偏移比长度少了4个字节
						DWORD	dwCRC = 0;
						dwCRC = CRC32(dwCRC, pRoleData, pRoleData->dwDataLen - 4);
						DWORD	dwCheck = *(DWORD *)(pGI->szData + pRoleData->dwDataLen - 4);
						if (dwCheck != dwCRC)
						{
							FILE *fLog = fopen("crc_db_error", "a+b");
							if(fLog) 
							{
								char buffer[255];
								sprintf(buffer, "----\r\n%s\r\b%s\r\n", pRoleData->BaseInfo.szName, pRoleData->BaseInfo.caccname);
								fwrite(buffer, 1, strlen(buffer), fLog);
								fwrite(pRoleData, 1, pRoleData->dwDataLen, fLog);
								fclose(fLog);
							}
							bCRCCheck = FALSE;
						}
					}					
//					cout << "AccName : " << pRoleData->BaseInfo.caccname;
//					cout << " - Name : " << pRoleData->BaseInfo.szName << endl;
					tagPermitPlayerLogin ppl;
					ppl.cProtocol = c2s_permitplayerlogin;
					memcpy( &( ppl.guid ), &guid, sizeof( GUID ) );
					strcpy( ( char * )( ppl.szRoleName ), ( const char * )( pRoleData->BaseInfo.szName ) );
					ppl.bPermit = false;
					
					KCoreConnectInfo	Info;
					int nCurrentPlayerCount = m_pCoreServerShell->GetConnectInfo(&Info);
					
					if (Info.nNumPlayer < m_nMaxPlayer && bCRCCheck )	// 去掉注释CRC错的用户就无法进入游戏了
					{
						int nIdx = m_pCoreServerShell->AddCharacter(pGI->nExtPoint, pGI->nChangePoint, (void *)pRoleData, &guid);
						
						if (nIdx > 0)
						{
							ppl.bPermit = true;
						}
						
					}
					m_pGatewayClient->SendPackToServer( ( const void * )&ppl, sizeof( tagPermitPlayerLogin ) );
				}
			}
			break;

		default:
			break;			
		}

#ifndef _STANDALONE
		try
		{
			if( pBuffer != NULL )
			{
				pBuffer->Release();
				pBuffer = NULL; 
			}
		}
		catch(...) 
		{
			//TRACE("SAFE_RELEASE error\n");
		}
#endif
	}
}

void KSwordOnLineSever::GatewaySmallPackProcess(const void *pData, size_t dataLength)
{
	_ASSERT( pData && dataLength );

#ifndef _STANDALONE
	BYTE cProtocol = CPackager::Peek( pData );
#else 
	BYTE cProtocol = *(BYTE *)pData;
#endif
/*	printf("-----REAL\ndatalen = %u, data = ", dataLength);
	for (int i = 0; i < 32; i++)
		printf("[%02X] ", ((BYTE *)pData)[i]);
	printf("\n");*/
	switch ( cProtocol )
	{
	case s2c_querymapinfo:
		{
			tagUpdateMapID *pUMI;
			char	sMapID[80];
			
			int nMapCount = m_pCoreServerShell->GetGameData(SGDI_LOADEDMAP_ID, (unsigned int)&sMapID, sizeof(sMapID));
			pUMI = ( tagUpdateMapID * )new BYTE[ nMapCount * sizeof(char) + sizeof( tagUpdateMapID ) ];
			memset( pUMI, 0, nMapCount * sizeof(char) + sizeof( tagUpdateMapID ) );
			
			pUMI->cProtocol = c2s_updatemapinfo;
			pUMI->cMapCount = nMapCount;
			memcpy( pUMI->szMapID, sMapID, nMapCount * sizeof(char));
			
/*			printf("-----Send \ndatalen = %u, data = ", nMapCount * sizeof(char) + sizeof( tagUpdateMapID ));
			for (int i = 0; i < 32; i++)
				printf("[%02X] ", ((BYTE *)pUMI)[i]);
			printf("\n");*/
			m_pGatewayClient->SendPackToServer( ( const void * )pUMI, nMapCount * sizeof(char) + sizeof( tagUpdateMapID ) );
			
			if(pUMI)
			{
				delete pUMI;
				pUMI = NULL;
			}
		}
		break;
		
	case s2c_querygameserverinfo:
		{
			tagGameSvrInfo ni;
			
			ni.cProtocol = c2s_updategameserverinfo;
			
			ni.nIPAddr_Internet = m_dwInternetIp;
			ni.nIPAddr_Intraner = m_dwIntranetIp;
			ni.nPort = m_nServerPort;
			ni.wCapability = m_nMaxPlayerCount;
			
			m_pGatewayClient->SendPackToServer( ( const void * )&ni, sizeof( tagGameSvrInfo ) );
			
		}
		break;
	case s2c_gateway_broadcast:
		GatewayBoardCastProcess((const char*)pData, dataLength);
		break;
	}
}

void KSwordOnLineSever::GatewayBoardCastProcess(const char* pData, size_t dataLength)
{
//#ifndef _STANDALONE
	if (dataLength != sizeof(tagGatewayBroadCast))
		return;
	
	tagGatewayBroadCast	*pGatewayBroadCast;
	pGatewayBroadCast = (tagGatewayBroadCast *)pData;
	//printf("BroadCast type = %d, %08x, %08x, %d\n",pGatewayBroadCast->uCmdType, (unsigned int)pGatewayBroadCast->szData, pGatewayBroadCast->szData, strlen(pGatewayBroadCast->szData));
	switch(pGatewayBroadCast->uCmdType)
	{
	case AP_WARNING_ALL_PLAYER_QUIT:
		{
			m_pCoreServerShell->OperationRequest(SSOI_BROADCASTING, (unsigned int)pGatewayBroadCast->szData, strlen(pGatewayBroadCast->szData));
			// TODO: Save Player and ExitGame when time out
			printf("ALL_PLAYER_QUIT\n");
			SetRunningStatus(FALSE);
//			ExitAllPlayer();
		}
		break;
	case AP_NOTIFY_GAMESERVER_SAFECLOSE:
		{
			m_pCoreServerShell->OperationRequest(SSOI_BROADCASTING, (unsigned int)pGatewayBroadCast->szData, strlen(pGatewayBroadCast->szData));
			printf("Safe close server\n");
			SetRunningStatus(FALSE);
		}
		break;
	case AP_NOTIFY_ALL_PLAYER:
		{
			m_pCoreServerShell->OperationRequest(SSOI_BROADCASTING, (unsigned int)pGatewayBroadCast->szData, strlen(pGatewayBroadCast->szData));
		}
		break;
	default:
		break;
	}
//#endif
}

void KSwordOnLineSever::MainLoop()
{
#ifndef _STANDALONE
	CCriticalSection::Owner locker( g_csFlow );
#else
	g_mutexFlow.lock();
#endif


	SavePlayerData();
	PlayerLogoutGateway();
	PlayerExchangeServer();
//	m_pServer->PreparePackSink();	空函数，没有调用的必要
	m_pCoreServerShell->Breathe();

// 定期向数据库查询排名情况
#define	defMAX_STAT_QUERY_TIME		18000 * 20		// 5小时查询

	if ( 0 == ( m_nGameLoop % defMAX_STAT_QUERY_TIME ) )
	{
		if (m_pDatabaseClient)
		{
			TProcessData	ProcessData;
			
			ProcessData.nProtoId = c2s_gamestatistic;
			ProcessData.nDataLen = 1;
			ProcessData.ulIdentity = -1;
			ProcessData.pDataBuffer[0] = 0;
			m_pDatabaseClient->SendPackToServer(&ProcessData, sizeof(TProcessData));
		}
	}

// 检查玩家是否很长时间没有发PING值了（1min）
	int lnID = m_nGameLoop % m_nMaxPlayer;
	int nIndex = m_pGameStatus[lnID].nPlayerIndex;
	if (nIndex > 0 && nIndex <= m_nMaxPlayer && m_pGameStatus[lnID].nGameStatus == enumPlayerPlaying)
	{
#define	defMAX_PING_TIMEOUT		60 * 20		// 60sec
#define	defMAX_PING_INTERVAL	5 * 20

		if (m_pGameStatus[lnID].nReplyPingTime != 0)
		{
			if (m_nGameLoop - m_pGameStatus[lnID].nSendPingTime > defMAX_PING_INTERVAL)
			{
				//printf("Send ping to client....\n");
				PingClient(lnID);
			}
		}
		else if (m_nGameLoop - m_pGameStatus[lnID].nSendPingTime > defMAX_PING_TIMEOUT)
		{
			printf("no response from client(%d, %d), kill it...\n", m_nGameLoop, m_pGameStatus[lnID].nSendPingTime);
			m_pServer->ShutdownClient(lnID);
		}
	}

	if (m_nGameLoop & 0x01)
	{
		m_pServer->SendPackToClient(-1);
	}
	m_nGameLoop++;
#ifdef _STANDALONE
	g_mutexFlow.unlock();
#endif
}

void KSwordOnLineSever::PingClient(const unsigned long lnID)
{
	_ASSERT(lnID < m_nMaxPlayer);
	_ASSERT(m_pGameStatus[lnID].nPlayerIndex > 0 && m_pGameStatus[lnID].nPlayerIndex <= m_nMaxPlayer);
	
	//printf("PingClient(%d) called\n", lnID);
	PING_COMMAND	pc;
	pc.m_dwTime = m_nGameLoop;
	pc.ProtocolType = s2c_ping;
	m_pServer->PackDataToClient(lnID, &pc, sizeof(PING_COMMAND));
	m_pGameStatus[lnID].nReplyPingTime = 0;
	m_pGameStatus[lnID].nSendPingTime = m_nGameLoop;
}

void KSwordOnLineSever::ProcessPingReply(const unsigned long lnID, const char* pData, size_t dataLength)
{
	_ASSERT(lnID < m_nMaxPlayer);
	_ASSERT(m_pGameStatus[lnID].nPlayerIndex > 0 && m_pGameStatus[lnID].nPlayerIndex <= m_nMaxPlayer);

	//printf("receive ping from client...\n");
	if (dataLength != sizeof(PING_CLIENTREPLY_COMMAND))
	{
		printf("ping cmd size not correct, may be Non-offical Client...\n");
		m_pServer->ShutdownClient(lnID);
		return;
	}
	
	PING_CLIENTREPLY_COMMAND*	pPC = (PING_CLIENTREPLY_COMMAND *)pData;
	if (pPC->m_dwReplyServerTime != m_pGameStatus[lnID].nSendPingTime)
	{
		printf("wrong time in ping cmd content, kill it...\n");
		m_pServer->ShutdownClient(lnID);
		return;
	}
	m_pGameStatus[lnID].nReplyPingTime = m_nGameLoop;
	// reply client ping to client to announce ping interval;
	PING_COMMAND	pc;
	pc.ProtocolType = s2c_replyclientping;
	pc.m_dwTime = pPC->m_dwClientTime;
	m_pServer->PackDataToClient(lnID, &pc, sizeof(PING_COMMAND));
}

// 保持连接的玩家自动存盘
void KSwordOnLineSever::SavePlayerData()
{
	if (!m_pDatabaseClient)
		return;

	int i = 0;
	
	TProcessData	sProcessData;

	sProcessData.nProtoId = c2s_roleserver_saveroleinfo;
	sProcessData.ulIdentity = -1;

	// 遍历网络连接
	for (i = 0; i < m_nMaxPlayer; i++)
	{
		if (GetNetStatus(i) == enumNetUnconnect)
			continue;

		int nIndex = m_pGameStatus[i].nPlayerIndex;

		if (nIndex <= 0 ||nIndex > m_nMaxPlayer)
			continue;

		//TRoleData* pData = (TRoleData *)m_pCoreServerShell->SavePlayerData(nIndex); 
		if (m_pCoreServerShell->GetSaveStatus(nIndex) == SAVE_REQUEST)
		{
			SavePlayerData(nIndex, false);
		}
	}
}

BOOL KSwordOnLineSever::SavePlayerData(int nIndex, bool bUnLock)
{
	if (!m_pDatabaseClient)
		return FALSE;

	if (nIndex <= 0 ||nIndex > m_nMaxPlayer)
		return FALSE;

	TProcessData	sProcessData;

	sProcessData.nProtoId = c2s_roleserver_saveroleinfo;
	sProcessData.ulIdentity = nIndex;
	sProcessData.bLeave = bUnLock;
	
	TRoleData* pData = (TRoleData *)m_pCoreServerShell->SavePlayerDataAtOnce(nIndex);
	if (pData && pData->dwDataLen)
	{
		sProcessData.nDataLen = pData->dwDataLen + sizeof(TProcessData) - 1;
		pData->dwDataLen += 4;
		// CRC
		DWORD	dwCRC = 0;
		dwCRC = CRC32(dwCRC, pData, pData->dwDataLen - 4);
		// CRC END

		m_theSend.AddData(c2s_roleserver_saveroleinfo, (const char*)&sProcessData, sizeof(TProcessData) - 1);
		m_theSend.AddData(c2s_roleserver_saveroleinfo, (const char*)pData, pData->dwDataLen - 4);
		m_theSend.AddData(c2s_roleserver_saveroleinfo, (const char*)&dwCRC, 4);
		
#ifndef _STANDALONE
		CBuffer* pBuffer = m_theSend.GetHeadPack(c2s_roleserver_saveroleinfo);
#else
		size_t actLen = 0;
		char* pBuffer = m_theSend.GetHeadPack(c2s_roleserver_saveroleinfo, actLen);
#endif
		
		while(pBuffer)
		{
#ifndef _STANDALONE
			m_pDatabaseClient->SendPackToServer(pBuffer->GetBuffer(), pBuffer->GetUsed());
#else
			m_pDatabaseClient->SendPackToServer(pBuffer, actLen);
#endif
			//				cout << "Save small package:" << pBuffer->GetUsed() << endl;
#ifndef _STANDALONE
			try
			{
				if (pBuffer)
				{
					pBuffer->Release();
					pBuffer = NULL;
				}
			}
			catch(...) 
			{
				//TRACE("SAFE_RELEASE error\n");
			}
#endif

#ifndef _STANDALONE
			pBuffer = m_theSend.GetNextPack(c2s_roleserver_saveroleinfo);
#else
			pBuffer = m_theSend.GetNextPack(c2s_roleserver_saveroleinfo, actLen);
#endif
		}
		m_theSend.DelData(c2s_roleserver_saveroleinfo);
#ifndef _STANDALONE
		try
		{
			if (pBuffer)
			{
				pBuffer->Release();
				pBuffer = NULL;
			}
		}
		catch(...) 
		{
			//TRACE("SAFE_RELEASE error\n");
		}
#endif
		//printf("Saving %s(%d)'s data, size:(%d)...\n", pData->BaseInfo.szName, nIndex, sProcessData.nDataLen);
		m_pCoreServerShell->SetSaveStatus(nIndex, SAVE_DOING);
		//printf("Save Player %d data ok.....\n", nIndex);
		return TRUE;
	}
	//printf("Save Player %d data failed.....\n", nIndex);
	return FALSE;
}

void KSwordOnLineSever::PlayerExchangeServer()
{
	if (!m_pGatewayClient || !m_pCoreServerShell)
		return;

	int i;

	char	szChar[1024];
	// 遍历网络连接
	for (i = 0; i < m_nMaxPlayer; i++)
	{
		int nIndex = m_pGameStatus[i].nPlayerIndex;
		if (nIndex <= 0 || nIndex > m_nMaxPlayer)
			continue;
		if (m_pCoreServerShell->IsPlayerExchangingServer(nIndex))
		{
			if (!m_pTransferClient)
			{
				tagNotifyPlayerExchange	npe;
				npe.cProtocol = s2c_notifyplayerexchange;
				memset(&npe.guid, 0, sizeof(GUID));
				// if false, ip to 0
				npe.nIPAddr = 0;
				npe.nPort = 0;
				m_pServer->SendData(i, &npe, sizeof(tagNotifyPlayerExchange));
				
				m_pCoreServerShell->RecoverPlayerExchange(nIndex);

				tagRoleEnterGame reg;
				reg.ProtocolType = c2s_roleserver_lock;
				reg.bLock = true;
				m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)reg.Name, nIndex);
				if (m_pDatabaseClient)
					m_pDatabaseClient->SendPackToServer((const void *)&reg, sizeof(tagRoleEnterGame));

				m_pGameStatus[i].nGameStatus = enumPlayerPlaying;
				m_pGameStatus[i].nExchangeStatus = enumExchangeBegin;
				continue;
			}
			m_pGameStatus[i].nGameStatus = enumPlayerExchangingServer;
			switch (m_pGameStatus[i].nExchangeStatus)
			{
			case enumExchangeBegin:
				{
					if (!SavePlayerData(nIndex, true))
						continue;
					RELAY_ASKWAY_DATA	*pRAD = (RELAY_ASKWAY_DATA *)szChar;

					pRAD->ProtocolFamily = pf_relay;
					pRAD->ProtocolID = relay_c2c_askwaydata;

					pRAD->nFromIP = 0;		// 0.0.0.0
					pRAD->nFromRelayID = 0;
					pRAD->seekRelayCount = 0;
					// 利用地图ID找SERVER
					pRAD->seekMethod = rm_map_id;
					pRAD->wMethodDataLength = 4;
					pRAD->routeDateLength = sizeof(tagSearchWay);

					*(DWORD *)(szChar + sizeof(RELAY_ASKWAY_DATA)) = m_pCoreServerShell->GetExchangeMap(nIndex);

					tagSearchWay *pSW = (tagSearchWay *)(szChar + sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength);
//					pSW->ProtocolFamily = pf_gameworld;
//					pSW->ProtocolID = c2c_notifyexchange;
					pSW->cProtocol = c2c_notifyexchange;
					pSW->lnID = i;
					pSW->nIndex = m_pGameStatus[i].nPlayerIndex;
					pSW->dwPlayerID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, pSW->nIndex, 0);
					// ask another game svr ip
					m_pTransferClient->SendPackToServer(pRAD, sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength + pRAD->routeDateLength);
					m_pGameStatus[i].nExchangeStatus = enumExchangeSearchingWay;
				}
				break;
			case enumExchangeSearchingWay:
				break;
			case enumExchangeWaitForGameSvrRespone:
				{
				}
				break;
			case enumExchangeCleaning:
				{
				}
				break;
			}
		}
	}
}

void KSwordOnLineSever::PlayerLogoutGateway()
{
	if (!m_pGatewayClient || !m_pCoreServerShell)
		return;

	tagLeaveGame	sLeaveGame;	
	sLeaveGame.cProtocol = c2s_leavegame;
	sLeaveGame.cCmdType = NORMAL_LEAVEGAME;

	tagLeaveGame2 lg2;
	
	lg2.ProtocolFamily = pf_normal;
	lg2.ProtocolID = c2s_leavegame;
	lg2.cCmdType = NORMAL_LEAVEGAME;

	// 遍历玩家（不能遍历连接，因为Timeout部分的玩家没有连接）
	for (int nIndex = 1; nIndex < MAX_PLAYER; nIndex++)
	{
		// Clear player which connect to gateway but not connect to gamesvr
		if (m_pCoreServerShell->IsPlayerLoginTimeOut(nIndex))
		{
			char	szName[32];
			m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ACCOUNT, (unsigned int)szName, nIndex);
			sLeaveGame.nExtPoint = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_EXTPOINTCHANGED, nIndex, 0);
			strcpy((char *)sLeaveGame.szAccountName, (char *)szName);
			m_pGatewayClient->SendPackToServer(&sLeaveGame, sizeof(tagLeaveGame));

			strcpy((char *)lg2.szAccountName, (char *)szName);
			
			if (m_pTransferClient)
				m_pTransferClient->SendPackToServer(&lg2, sizeof(tagLeaveGame2));
//			cout << sLeaveGame.szAccountName << " leave game!" << endl;
			m_pCoreServerShell->RemovePlayerLoginTimeOut(nIndex);
		}
		else if (m_pCoreServerShell->IsCharacterQuiting(nIndex))
		{
			SavePlayerData(nIndex, true);			
//			if (!SavePlayerData(nIndex))
//				continue;

			char	szName[32];
			m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ACCOUNT, (unsigned int)szName, nIndex);

			sLeaveGame.nExtPoint = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_EXTPOINTCHANGED, nIndex, 0);
			
			strcpy((char *)sLeaveGame.szAccountName, (char *)szName);
			m_pGatewayClient->SendPackToServer(&sLeaveGame, sizeof(tagLeaveGame));

			strcpy((char *)lg2.szAccountName, (char *)szName);
			
			if (m_pTransferClient)
				m_pTransferClient->SendPackToServer(&lg2, sizeof(tagLeaveGame2));
//			cout << sLeaveGame.szAccountName << " leave game!" << endl;
/*
			m_pGameStatus[lnID].nExchangeStatus = enumExchangeBegin;
			m_pGameStatus[lnID].nGameStatus = enumPlayerBegin;
			m_pGameStatus[lnID].nPlayerIndex = 0;
*/
			m_pCoreServerShell->RemoveQuitingPlayer(nIndex);
		}

	}
}

int KSwordOnLineSever::GetNetStatus(const unsigned long lnID)
{
	if (lnID >= m_nMaxPlayer)
		return enumNetUnconnect;

	return m_pGameStatus[lnID].nNetStatus;
}

void KSwordOnLineSever::SetNetStatus(const unsigned long lnID, NetStatus nStatus)
{
	if (lnID >= m_nMaxPlayer)
		return;

	if (nStatus == enumNetUnconnect)
	{
		if (m_pGameStatus[lnID].nGameStatus == enumPlayerPlaying
			|| (m_pGameStatus[lnID].nGameStatus == enumPlayerExchangingServer 
			&& m_pGameStatus[lnID].nExchangeStatus != enumExchangeCleaning))	// 跨服务器时自己处理
		{
			int nIndex = m_pGameStatus[lnID].nPlayerIndex;
			m_pCoreServerShell->ClientDisconnect(nIndex);
		}
		else
		{
			int nIndex = m_pGameStatus[lnID].nPlayerIndex;
			m_pCoreServerShell->PreparePlayerForLoginFailed(nIndex);
		}
		m_pGameStatus[lnID].nGameStatus = enumPlayerBegin;
		m_pGameStatus[lnID].nPlayerIndex = 0;
		m_pGameStatus[lnID].nExchangeStatus = enumExchangeBegin;
		m_pGameStatus[lnID].nReplyPingTime = 0;
		m_pGameStatus[lnID].nSendPingTime = 0;
	}
	if (nStatus == enumNetConnected)
	{
		m_pGameStatus[lnID].nGameStatus = enumPlayerBegin;
		m_pGameStatus[lnID].nPlayerIndex = 0;
		m_pGameStatus[lnID].nExchangeStatus = enumExchangeBegin;
		m_pGameStatus[lnID].nReplyPingTime = 0;
		m_pGameStatus[lnID].nSendPingTime = 0;
	}
	m_pGameStatus[lnID].nNetStatus = nStatus;		
}

int KSwordOnLineSever::ProcessLoginProtocol(const unsigned long lnID, const char* pData, size_t dataLength)
{
	const char* pBuffer = pData;

	if (*pBuffer != c2s_logiclogin)
	{
		return 0;
	}
	else
	{
		tagLogicLogin *pLL = ( tagLogicLogin * )pData;
//		cout << "A client try to login..." << endl;
		
		int nIdx = m_pCoreServerShell->AttachPlayer(lnID, &pLL->guid);
		if (nIdx)
		{
//			cout << "Found player " << nIdx << " is logging in system!" << endl;
			return nIdx;
		}
		else
		{
			// 非法的玩家，该怎么处理怎么处理
			return 0;
		}
	}
	return 0;	
}

BOOL KSwordOnLineSever::SendGameDataToClient(const unsigned long lnID, const int nPlayerIndex)
{
	BOOL			bRet = FALSE;
//#ifndef _STANDALONE
	_ASSERT(m_pServer);

	int				nStep = 0;
	unsigned int	nParam = 0;
	int				bSyncEnd = 0;

	m_pServer->PreparePackSink();
	while(true)
	{
		bSyncEnd = (nStep == STEP_SYNC_END);
		bRet = m_pCoreServerShell->PlayerDbLoading(nPlayerIndex, bSyncEnd, nStep, nParam);
		if (!bRet)
		{
			printf("PlayerDbLoading failed.\n");	//[wxb 2003-7-28]
			break;
		}
		if (bSyncEnd)
		{			
			m_pGameStatus[lnID].nGameStatus = enumPlayerSyncEnd;
		
			BYTE	SyncEnd;
			SyncEnd = s2c_syncend;
#ifndef _STANDALONE
			if (FAILED(m_pServer->PackDataToClient(lnID, &SyncEnd, sizeof(BYTE))))
#else
			if (!SUCCEEDED(m_pServer->PackDataToClient(lnID, &SyncEnd, sizeof(BYTE))))
#endif
			{
				printf("Packing failed. %d, %d, %d\n", lnID);	//[wxb 2003-7-28]
				bRet = FALSE;
				break;
			}
			g_DebugLog("[TRACE]SyncEnd:%d", lnID);
			break;
		}
	}

#ifdef WIN32
	if (FAILED(m_pServer->SendPackToClient(lnID)))
	{
		bRet = FALSE;
	}
#else
	if (m_pServer->SendPackToClient(lnID) <= 0)
		bRet = FALSE;
#endif
	return bRet;
}

BOOL KSwordOnLineSever::ProcessSyncReplyProtocol(const unsigned long lnID, const char* pData, size_t dataLength)
{
	const char*	pBuffer = pData;

	if (*pBuffer != c2s_syncend)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
	return FALSE;	
}

void KSwordOnLineSever::ExitAllPlayer()
{
#ifndef __linux			//SwitchToThread();
			Sleep(4000);
#else
			usleep(4000000);
#endif

	int lnID;
	const char*	pData;
	unsigned int	uSize;
	tagLeaveGame	sLeaveGame;	

	sLeaveGame.cProtocol = c2s_leavegame;
	sLeaveGame.cCmdType = NORMAL_LEAVEGAME;

	for (lnID = 0; lnID < m_nMaxPlayer; lnID++)
	{
		int nIndex = m_pGameStatus[lnID].nPlayerIndex;
		if (nIndex == 0)
			continue;
		BOOL bSave = SavePlayerData(nIndex, true);
		int nCount = 0;
		while(bSave)
		{
			nCount++;
			pData = (const char *)m_pDatabaseClient->GetPackFromServer(uSize);
			if (pData && uSize)
			{
				DatabaseMessageProcess(pData, uSize);
				if (m_pCoreServerShell->GetSaveStatus(nIndex) == SAVE_IDLE)
					break;
			}
			if (nCount > 1000)
				break;
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
		
		m_pCoreServerShell->ClientDisconnect(nIndex);
		if (m_pCoreServerShell->IsCharacterQuiting(nIndex))
		{
			char	szName[32];
	
			m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ACCOUNT, (unsigned int)szName, nIndex);
			sLeaveGame.nExtPoint = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_EXTPOINTCHANGED, nIndex, 0);

			strcpy((char *)sLeaveGame.szAccountName, (char *)szName);
			m_pGatewayClient->SendPackToServer(&sLeaveGame, sizeof(tagLeaveGame));
			if (m_pTransferClient)
				m_pTransferClient->SendPackToServer(&sLeaveGame, sizeof(tagLeaveGame));
			m_pCoreServerShell->RemoveQuitingPlayer(nIndex);
		}
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
	}
}

void KSwordOnLineSever::SetRunningStatus(BOOL bStatus)
{
	m_bIsRunning = bStatus;
}

BOOL KSwordOnLineSever::ConformAskWay(const void* pData, int nSize, DWORD *lpnNetID)
{
	BOOL	bRet = FALSE;
	RELAY_ASKWAY_DATA*	pRAD = (RELAY_ASKWAY_DATA *)pData;

	// TODO: 检查是否寻路正确
	int nMethod = pRAD->seekMethod;
	switch(nMethod)
	{
	case rm_role_id:
	case rm_account_id:
		{
			DWORD	dwNameID;
			DWORD	lnID;
			LPDWORD	lpdwPTR = (DWORD *)(((char *)pData) + sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength - sizeof(DWORD)*2);
			dwNameID = *lpdwPTR;
			lnID = *(lpdwPTR + 1);
			if (lnID >= m_nMaxPlayer || m_pGameStatus[lnID].nPlayerIndex <= 0 || m_pGameStatus[lnID].nPlayerIndex > m_nMaxPlayer)
			{
				bRet = FALSE;
				break;
			}
			DWORD	dwNameIDbyIndex = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, m_pGameStatus[lnID].nPlayerIndex, 0);
			if (dwNameIDbyIndex != dwNameID)
			{
				bRet = FALSE;
				break;
			}
			*lpnNetID = lnID;
			bRet = TRUE;
		}
		break;
	case rm_map_id:
		{
			DWORD	dwMapID;

			dwMapID = *(DWORD *)(((char *)pData) + sizeof(RELAY_ASKWAY_DATA));

			if (0)	// TODO:check mapid here
			{
				bRet = FALSE;
				break;
			}
			bRet = TRUE;
		}
		break;
	}
	return bRet;
}

BOOL KSwordOnLineSever::GetLocalIpAddress(DWORD *pIntranetAddr, DWORD *pInternetAddr)
{
#ifndef _STANDALONE
	return gGetMacAndIPAddress(NULL, pIntranetAddr, NULL, pInternetAddr);
#else
	char	szIp[16];
	DWORD	dwAddr1, dwAddr2;
	dwAddr1 = dwAddr2 = 0;

	char strHost[MAX_PATH];
	strHost[0] = 0;
	if(SOCKET_ERROR != gethostname(strHost, sizeof(strHost))) {
		struct hostent* hp;
		hp = gethostbyname( strHost );
#ifndef WIN32
		if (!hp ||
			hp && hp->h_addr_list[0] && *(unsigned long *)hp->h_addr_list[0] == 0x0100007f)
		{
			int sock;
			struct ifreq ifr;
			char* ip = NULL;
			int err = -1;

			sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
			if (sock >= 0) {
				strcpy(ifr.ifr_name, "eth0");
				ifr.ifr_addr.sa_family = AF_INET;

				err = ioctl(sock, SIOCGIFADDR, &ifr);
				if (err == 0) {
					dwAddr1 = (((struct sockaddr_in*) &(ifr.ifr_addr))->sin_addr).s_addr;
					ip =  inet_ntoa(((struct sockaddr_in*) &(ifr.ifr_addr))->sin_addr);
					printf("IP-addr1: %s\n", ip);
				}

				strcpy(ifr.ifr_name, "eth1");
				ifr.ifr_addr.sa_family = AF_INET;

				err = ioctl(sock, SIOCGIFADDR, &ifr);
				if (err == 0) {
					dwAddr2 = (((struct sockaddr_in*) &(ifr.ifr_addr))->sin_addr).s_addr;
					ip =  inet_ntoa(((struct sockaddr_in*) &(ifr.ifr_addr))->sin_addr);
					printf("IP-addr2: %s\n", ip);
				}
				else
					dwAddr2 = dwAddr1;
				closesocket(sock);
			}
		}
		else
#endif
		{
		if(hp && hp->h_addr_list[0]) {
			dwAddr1 = *(unsigned long *)hp->h_addr_list[0];
			if (!dwAddr1)
				dwAddr1 = *(unsigned long *)hp->h_addr_list[1];
/*			*((char *)&dwAddr1) = hp->h_addr_list[0][3];
			*((char *)&dwAddr1 + 1) = hp->h_addr_list[0][2];
			*((char *)&dwAddr1 + 2) = hp->h_addr_list[0][1];
			*((char *)&dwAddr1 + 3) = hp->h_addr_list[0][0];*/
//			dwAddr1 = hp->h_addr_list[0];
		}
		if(hp && hp->h_addr_list[1]) {
			dwAddr2 = *(unsigned long *)hp->h_addr_list[1];
			if (!dwAddr2)
				dwAddr2 = *(unsigned long *)hp->h_addr_list[0];
//			dwAddr2 = hp->h_addr_list[1];
/*			*((char *)&dwAddr2) = hp->h_addr_list[1][3];
			*((char *)&dwAddr2 + 1) = hp->h_addr_list[1][2];
			*((char *)&dwAddr2 + 2) = hp->h_addr_list[1][1];
			*((char *)&dwAddr2 + 3) = hp->h_addr_list[1][0];*/
		}
		else
			dwAddr2 = dwAddr1;
		}
	}

	if ((dwAddr1 & 0x0000FFFF) == 0x0000a8c0)	// intranet
	{
		*pIntranetAddr = dwAddr1;
		*pInternetAddr = dwAddr2;
	}
	else
	{
		*pIntranetAddr = dwAddr2;
		*pInternetAddr = dwAddr1;
	}
	return TRUE;
#endif
}

void KSwordOnLineSever::TransferAskWayMessageProcess(const char *pData, size_t dataLength)
{
	char	szChar[1024];

	EXTEND_HEADER	*pEH = (EXTEND_HEADER *)pData;
	RELAY_ASKWAY_DATA	*pRAD = (RELAY_ASKWAY_DATA *)pData;
	
	RELAY_DATA	*pRD = (RELAY_DATA *)szChar;
	pRD->ProtocolFamily = pEH->ProtocolFamily;
	pRD->nToIP = pRAD->nFromIP;
	pRD->nToRelayID = pRAD->nFromRelayID;
	pRD->nFromIP = 0;
	pRD->nFromRelayID = 0;
	pRD->routeDateLength = pRAD->routeDateLength;
	
	DWORD	lnID;
	// TODO: check routeDataLength
	if (!ConformAskWay(pRAD, dataLength, &lnID))
	{
		pRD->ProtocolID = relay_s2c_loseway;
		memcpy(szChar + sizeof(RELAY_DATA), pData, dataLength);
		m_pTransferClient->SendPackToServer(pRD, sizeof(RELAY_DATA) + dataLength);
	}
	else
	{
		pRD->ProtocolID = relay_c2c_data;
		switch(pRAD->seekMethod)
		{
		case rm_map_id:
			memcpy(szChar + sizeof(RELAY_DATA), pData + sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength, pRD->routeDateLength);
			m_pTransferClient->SendPackToServer(pRD, sizeof(RELAY_DATA) + pRD->routeDateLength);
			break;
		case rm_role_id:
		case rm_account_id:
			{
				if (pf_normal != *(pData + sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength))
				{
					m_pCoreServerShell->ProcessNewClientMessage(
						m_pTransferClient,
						pRAD->nFromIP,
						pRAD->nFromRelayID,
						m_pGameStatus[lnID].nPlayerIndex, 
						(pData + sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength), 
						pRAD->routeDateLength);
				}
				else
				{
					m_pCoreServerShell->ProcessClientMessage(
						m_pGameStatus[lnID].nPlayerIndex, 
						(pData + sizeof(RELAY_ASKWAY_DATA) + pRAD->wMethodDataLength + 1), 
						pRAD->routeDateLength - 1);
				}
			}
			break;
		default:
			break;
		}
	}
}

void KSwordOnLineSever::TransferLoseWayMessageProcess(const char *pData, size_t dataLength)
{
	if (!m_pCoreServerShell || !m_pServer)
		return;

	EXTEND_HEADER* pInHeader = (EXTEND_HEADER*)pData;

	if (pInHeader->ProtocolFamily == pf_relay)
	{
		tagSearchWay *pSW = NULL;

		switch (pInHeader->ProtocolID)
		{
		case relay_c2c_data:
			pSW = (tagSearchWay *)(pData + sizeof(RELAY_DATA));
			break;
		case relay_c2c_askwaydata:
			pSW = (tagSearchWay *)(pData + sizeof(RELAY_ASKWAY_DATA) + ((RELAY_ASKWAY_DATA*)pData)->wMethodDataLength);
			break;
		default:
			return;
		}

		int lnID, nIndex;

		lnID = pSW->lnID;
		if (lnID < 0 || lnID >= m_nMaxPlayer)
			return;
		if (m_pGameStatus[lnID].nExchangeStatus != enumExchangeSearchingWay)
			return;

		if (m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, pSW->nIndex, 0) != pSW->dwPlayerID)
			return;

		nIndex = m_pGameStatus[lnID].nPlayerIndex;

		tagNotifyPlayerExchange	npe;
		npe.cProtocol = s2c_notifyplayerexchange;
		memset(&npe.guid, 0, sizeof(GUID));
		// if false, ip to 0
		npe.nIPAddr = 0;
		npe.nPort = 0;
		m_pServer->SendData(lnID, &npe, sizeof(tagNotifyPlayerExchange));

		m_pCoreServerShell->RecoverPlayerExchange(nIndex);

		tagRoleEnterGame reg;
		reg.ProtocolType = c2s_roleserver_lock;
		reg.bLock = true;
		m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)reg.Name, nIndex);
		if (m_pDatabaseClient)
			m_pDatabaseClient->SendPackToServer((const void *)&reg, sizeof(tagRoleEnterGame));

		m_pGameStatus[lnID].nExchangeStatus = enumExchangeBegin;
		m_pGameStatus[lnID].nGameStatus = enumPlayerPlaying;
	}
}


void KSwordOnLineSever::ProcessPlayerTongMsg(const unsigned long nPlayerIdx, const char* pData, size_t dataLength)
{
	if (nPlayerIdx <= 0 || nPlayerIdx >= MAX_PLAYER)
		return;
	if (!pData)
		return;
	if (dataLength < sizeof(STONG_PROTOCOL_HEAD))
		return;
	int	nPLength = ((STONG_PROTOCOL_HEAD*)pData)->m_wLength;
	if (nPLength + 1 > dataLength)
		return;

	switch (((STONG_PROTOCOL_HEAD*)pData)->m_btMsgId)
	{
	// 申请创建帮会
	case enumTONG_COMMAND_ID_APPLY_CREATE:
		{
			TONG_APPLY_CREATE_COMMAND	*pApply = (TONG_APPLY_CREATE_COMMAND*)pData;
			STONG_SERVER_TO_CORE_APPLY_CREATE	sApply;

			sApply.m_nCamp = pApply->m_btCamp;
			sApply.m_nPlayerIdx = nPlayerIdx;
			memcpy(sApply.m_szTongName, pApply->m_szName, sizeof(pApply->m_szName));

			// 创建帮会条件检测
			int nRet = 0xff;
			if (m_pCoreServerShell)
				nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_APPLY_CREATE, (unsigned int)&sApply, 0);
			// 创建帮会条件成立
			if (nRet == 0)
			{
				char	szPlayerName[32];
				STONG_CREATE_COMMAND	sCreate;

				m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)szPlayerName, nPlayerIdx);
				sCreate.ProtocolFamily = pf_tong;
				sCreate.ProtocolID = enumC2S_TONG_CREATE;
				sCreate.m_btCamp = pApply->m_btCamp;
				sCreate.m_dwParam = nPlayerIdx;
				sCreate.m_dwPlayerNameID = g_FileName2Id(szPlayerName);
				sCreate.m_btPlayerNameLength = strlen(szPlayerName);
				sCreate.m_btTongNameLength = strlen(sApply.m_szTongName);
				memcpy(sCreate.m_szBuffer, sApply.m_szTongName, sCreate.m_btTongNameLength);
				memcpy(&sCreate.m_szBuffer[sCreate.m_btTongNameLength], szPlayerName, sCreate.m_btPlayerNameLength);
				sCreate.m_wLength = sizeof(STONG_CREATE_COMMAND) - sizeof(sCreate.m_szBuffer) + sCreate.m_btTongNameLength + sCreate.m_btPlayerNameLength;
				if (m_pTongClient)
					m_pTongClient->SendPackToServer((const void*)&sCreate, sCreate.m_wLength);
				break;
			}
			// 创建帮会条件不成立
			else
			{
				int		nNetID;
				TONG_CREATE_FAIL_SYNC	sFail;
				sFail.ProtocolType = s2c_extendtong;
				sFail.m_btMsgId = enumTONG_SYNC_ID_CREATE_FAIL;
				sFail.m_btFailId = nRet;
				sFail.m_wLength = sizeof(sFail) - 1;
				nNetID = m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NETID, nPlayerIdx, 0);
				if (m_pServer)
					m_pServer->PackDataToClient(nNetID, &sFail, sFail.m_wLength + 1);

				break;
			}
		}
		break;
	case enumTONG_COMMAND_ID_APPLY_ADD:
		{
			TONG_APPLY_ADD_COMMAND	*pApply = (TONG_APPLY_ADD_COMMAND*)pData;
			if (pApply->m_wLength != sizeof(TONG_APPLY_ADD_COMMAND) - 1)
				break;
			STONG_SERVER_TO_CORE_APPLY_ADD	sAdd;
			sAdd.m_nPlayerIdx = nPlayerIdx;
			sAdd.m_dwNpcID = pApply->m_dwNpcID;
			if (m_pCoreServerShell)
				m_pCoreServerShell->GetGameData(SGDI_TONG_APPLY_ADD, (unsigned int)&sAdd, 0);
		}
		break;
	case enumTONG_COMMAND_ID_ACCEPT_ADD:
		{
			TONG_ACCEPT_MEMBER_COMMAND	*pAccept = (TONG_ACCEPT_MEMBER_COMMAND*)pData;
			if (pAccept->m_wLength != sizeof(TONG_ACCEPT_MEMBER_COMMAND) - 1)
				break;
			if (pAccept->m_btFlag == 0)
			{
				STONG_SERVER_TO_CORE_REFUSE_ADD	sRefuse;
				sRefuse.m_nSelfIdx = nPlayerIdx;
				sRefuse.m_nTargetIdx = pAccept->m_nPlayerIdx;
				sRefuse.m_dwNameID = pAccept->m_dwNameID;
				m_pCoreServerShell->OperationRequest(SSOI_TONG_REFUSE_ADD, (unsigned int)&sRefuse, 0);
				break;
			}
			else
			{
				char	szTongName[32];
				STONG_SERVER_TO_CORE_CHECK_ADD_CONDITION	sAdd;
				sAdd.m_nSelfIdx = nPlayerIdx;
				sAdd.m_nTargetIdx = pAccept->m_nPlayerIdx;
				sAdd.m_dwNameID = pAccept->m_dwNameID;

				int		nRet = FALSE;
				if (m_pCoreServerShell)
					nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_CHECK_ADD_CONDITION, (unsigned int)szTongName, (unsigned int)&sAdd);
				// 向 relay 申请添加帮众
				if (nRet)
				{
					char	szPlayerName[32];
					STONG_ADD_MEMBER_COMMAND	sTong;

					szPlayerName[0] = 0;
					m_pCoreServerShell->GetGameData(SGDI_CHARACTER_NAME, (unsigned int)szPlayerName, sAdd.m_nTargetIdx);

					sTong.ProtocolFamily = pf_tong;
					sTong.ProtocolID = enumC2S_TONG_ADD_MEMBER;
					sTong.m_dwParam = sAdd.m_nTargetIdx;
					sTong.m_dwPlayerNameID = sAdd.m_dwNameID;
					sTong.m_btPlayerNameLength = strlen(szPlayerName);
					sTong.m_btTongNameLength = strlen(szTongName);
					memcpy(sTong.m_szBuffer, szTongName, sTong.m_btTongNameLength);
					memcpy(&sTong.m_szBuffer[sTong.m_btTongNameLength], szPlayerName, sTong.m_btPlayerNameLength);
					sTong.m_wLength = sizeof(STONG_ADD_MEMBER_COMMAND) - sizeof(sTong.m_szBuffer) + sTong.m_btTongNameLength + sTong.m_btPlayerNameLength;
					if (m_pTongClient)
						m_pTongClient->SendPackToServer((const void*)&sTong, sTong.m_wLength);

				}
				else
				{
					break;
				}
			}
		}
		break;
	case enumTONG_COMMAND_ID_APPLY_INFO:
		{
			TONG_APPLY_INFO_COMMAND	*pInfo = (TONG_APPLY_INFO_COMMAND*)pData;
			if (pInfo->m_wLength < sizeof(TONG_APPLY_INFO_COMMAND) - 1 - sizeof(pInfo->m_szBuf))
				break;
			STONG_SERVER_TO_CORE_GET_INFO	sGet;
			switch (pInfo->m_btInfoID)
			{
			case enumTONG_APPLY_INFO_ID_SELF:
				sGet.m_nSelfIdx = nPlayerIdx;
				sGet.m_nInfoID = pInfo->m_btInfoID;
				sGet.m_nParam1 = pInfo->m_nParam1;
				sGet.m_nParam2 = pInfo->m_nParam2;
				sGet.m_nParam3 = pInfo->m_nParam3;
				sGet.m_szName[0] = 0;
				if (m_pCoreServerShell)
					m_pCoreServerShell->GetGameData(SGDI_TONG_GET_INFO, (unsigned int)&sGet, 0);
				break;
			case enumTONG_APPLY_INFO_ID_MASTER:
				break;
			case enumTONG_APPLY_INFO_ID_DIRECTOR:
				break;
			case enumTONG_APPLY_INFO_ID_MANAGER:
				{
					int nRet = 0;
					sGet.m_nSelfIdx = nPlayerIdx;
					sGet.m_nInfoID = pInfo->m_btInfoID;
					sGet.m_nParam1 = pInfo->m_nParam1;
					sGet.m_nParam2 = pInfo->m_nParam2;
					sGet.m_nParam3 = pInfo->m_nParam3;
					sGet.m_szName[0] = 0;
					if (m_pCoreServerShell)
						nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_GET_INFO, (unsigned int)&sGet, 0);
					if (nRet == 0)
						break;

					STONG_GET_MANAGER_INFO_COMMAND	sGet;
					sGet.ProtocolFamily = pf_tong;
					sGet.ProtocolID = enumC2S_TONG_GET_MANAGER_INFO;
					sGet.m_dwParam = nPlayerIdx;
					sGet.m_nParam1 = pInfo->m_nParam1;
					sGet.m_nParam2 = pInfo->m_nParam2;
					sGet.m_nParam3 = pInfo->m_nParam3;
					if (m_pTongClient)
						m_pTongClient->SendPackToServer((const void*)&sGet, sizeof(sGet));
				}
				break;
			case enumTONG_APPLY_INFO_ID_MEMBER:
				{
					int nRet = 0;
					sGet.m_nSelfIdx = nPlayerIdx;
					sGet.m_nInfoID = pInfo->m_btInfoID;
					sGet.m_nParam1 = pInfo->m_nParam1;
					sGet.m_nParam2 = pInfo->m_nParam2;
					sGet.m_nParam3 = pInfo->m_nParam3;
					sGet.m_szName[0] = 0;
					if (m_pCoreServerShell)
						nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_GET_INFO, (unsigned int)&sGet, 0);
					if (nRet == 0)
						break;

					STONG_GET_MEMBER_INFO_COMMAND	sGet;
					sGet.ProtocolFamily = pf_tong;
					sGet.ProtocolID = enumC2S_TONG_GET_MEMBER_INFO;
					sGet.m_dwParam = nPlayerIdx;
					sGet.m_nParam1 = pInfo->m_nParam1;
					sGet.m_nParam2 = pInfo->m_nParam2;
					sGet.m_nParam3 = pInfo->m_nParam3;
					if (m_pTongClient)
						m_pTongClient->SendPackToServer((const void*)&sGet, sizeof(sGet));
				}
				break;
			case enumTONG_APPLY_INFO_ID_ONE:
				break;
			case enumTONG_APPLY_INFO_ID_TONG_HEAD:
				{
					DWORD	dwTongNameID = 0;

					sGet.m_nSelfIdx = nPlayerIdx;
					sGet.m_nInfoID = pInfo->m_btInfoID;
					sGet.m_nParam1 = pInfo->m_nParam1;
					sGet.m_nParam2 = pInfo->m_nParam2;
					sGet.m_nParam3 = pInfo->m_nParam3;
					sGet.m_szName[0] = 0;

					if (m_pCoreServerShell)
						dwTongNameID = m_pCoreServerShell->GetGameData(SGDI_TONG_GET_INFO, (unsigned int)&sGet, 0);
					if (dwTongNameID == 0)
						break;

					STONG_GET_TONG_HEAD_INFO_COMMAND	sGet;
					sGet.ProtocolFamily	= pf_tong;
					sGet.ProtocolID		= enumC2S_TONG_GET_HEAD_INFO;
					sGet.m_dwParam		= nPlayerIdx;
					sGet.m_dwNpcID		= pInfo->m_nParam1;
					sGet.m_dwTongNameID	= dwTongNameID;
					if (m_pTongClient)
						m_pTongClient->SendPackToServer((const void*)&sGet, sizeof(sGet));
				}
				break;
			}
		}
		break;
	case enumTONG_COMMAND_ID_APPLY_INSTATE:
		{
			TONG_APPLY_INSTATE_COMMAND	*pApply = (TONG_APPLY_INSTATE_COMMAND*)pData;
			if (pApply->m_wLength + 1 != sizeof(TONG_APPLY_INSTATE_COMMAND))
				break;
			int nRet = 0;
			if (m_pCoreServerShell)
				nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_INSTATE_POWER, (unsigned int)pApply, nPlayerIdx);
			if (nRet == 0)
				break;

			STONG_INSTATE_COMMAND	sInstate;
			sInstate.ProtocolFamily	= pf_tong;
			sInstate.ProtocolID		= enumC2S_TONG_INSTATE;
			sInstate.m_btCurFigure	= pApply->m_btCurFigure;
			sInstate.m_btCurPos		= pApply->m_btCurPos;
			sInstate.m_btNewFigure	= pApply->m_btNewFigure;
			sInstate.m_btNewPos		= pApply->m_btNewPos;
			sInstate.m_dwParam		= nPlayerIdx;
			sInstate.m_dwTongNameID	= pApply->m_dwTongNameID;
			memset(sInstate.m_szName, 0, sizeof(sInstate.m_szName));
			memcpy(sInstate.m_szName, pApply->m_szName, pApply->m_wLength + 1 + sizeof(pApply->m_szName) - sizeof(TONG_APPLY_INSTATE_COMMAND));

			if (m_pTongClient)
				m_pTongClient->SendPackToServer((const void*)&sInstate, sizeof(sInstate));
		}
		break;

	case enumTONG_COMMAND_ID_APPLY_KICK:
		{
			TONG_APPLY_KICK_COMMAND	*pKick = (TONG_APPLY_KICK_COMMAND*)pData;
			if (pKick->m_wLength + 1 != sizeof(TONG_APPLY_KICK_COMMAND))
				break;
			int nRet = 0;
			if (m_pCoreServerShell)
				nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_KICK_POWER, (unsigned int)pKick, nPlayerIdx);
			if (nRet == 0)
				break;

			STONG_KICK_COMMAND	sKick;
			sKick.ProtocolFamily	= pf_tong;
			sKick.ProtocolID		= enumC2S_TONG_KICK;
			sKick.m_dwParam			= nPlayerIdx;
			sKick.m_dwTongNameID	= pKick->m_dwTongNameID;
			sKick.m_btFigure		= pKick->m_btFigure;
			sKick.m_btPos			= pKick->m_btPos;
			memcpy(sKick.m_szName, pKick->m_szName, sizeof(pKick->m_szName));

			if (m_pTongClient)
				m_pTongClient->SendPackToServer((const void*)&sKick, sizeof(sKick));
		}
		break;
	case enumTONG_COMMAND_ID_APPLY_LEAVE:
		{
			TONG_APPLY_LEAVE_COMMAND	*pLeave = (TONG_APPLY_LEAVE_COMMAND*)pData;
			if (pLeave->m_wLength + 1 != sizeof(TONG_APPLY_LEAVE_COMMAND))
				break;
			int nRet = 0;
			if (m_pCoreServerShell)
				nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_LEAVE_POWER, (unsigned int)pLeave, nPlayerIdx);
			if (nRet == 0)
				break;

			STONG_LEAVE_COMMAND	sLeave;
			sLeave.ProtocolFamily	= pf_tong;
			sLeave.ProtocolID		= enumC2S_TONG_LEAVE;
			sLeave.m_dwParam		= nPlayerIdx;
			sLeave.m_dwTongNameID	= pLeave->m_dwTongNameID;
			sLeave.m_btFigure		= pLeave->m_btFigure;
			sLeave.m_btPos			= pLeave->m_btPos;
			memcpy(sLeave.m_szName, pLeave->m_szName, sizeof(pLeave->m_szName));

			if (m_pTongClient)
				m_pTongClient->SendPackToServer((const void*)&sLeave, sizeof(sLeave));
		}
		break;
	case enumTONG_COMMAND_ID_APPLY_CHANGE_MASTER:
		{
			TONG_APPLY_CHANGE_MASTER_COMMAND	*pChange = (TONG_APPLY_CHANGE_MASTER_COMMAND*)pData;
			if (pChange->m_wLength + 1 != sizeof(TONG_APPLY_CHANGE_MASTER_COMMAND))
				break;
			int nRet = 0;
			if (m_pCoreServerShell)
				nRet = m_pCoreServerShell->GetGameData(SGDI_TONG_CHANGE_MASTER_POWER, (unsigned int)pChange, nPlayerIdx);
			if (nRet == 0)
				break;

			STONG_CHANGE_MASTER_COMMAND	sChange;
			sChange.ProtocolFamily	= pf_tong;
			sChange.ProtocolID		= enumC2S_TONG_CHANGE_MASTER;
			sChange.m_btFigure		= pChange->m_btFigure;
			sChange.m_btPos			= pChange->m_btPos;
			sChange.m_dwParam		= nPlayerIdx;
			sChange.m_dwTongNameID	= pChange->m_dwTongNameID;
			memcpy(sChange.m_szName, pChange->m_szName, sizeof(pChange->m_szName));
			if (m_pTongClient)
				m_pTongClient->SendPackToServer((const void*)&sChange, sizeof(sChange));
		}
		break;
	default:
		break;
	}
}

BOOL KSwordOnLineSever::CheckPlayerID(unsigned long netidx, DWORD nameid)
{
	if (netidx < 0 || netidx >= m_nMaxPlayer)
		return FALSE;

	int idx = m_pGameStatus[netidx].nPlayerIndex;
	if (idx <= 0 || idx >= MAX_PLAYER)
		return FALSE;

	if (nameid != m_pCoreServerShell->GetGameData(SGDI_CHARACTER_ID, idx, 0))
		return FALSE;

	return TRUE;
}

