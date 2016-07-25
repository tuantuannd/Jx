#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include "KRoleInfomation.h"
#include "KRoleDBServer.h"
#include "KException.h"
#include "S3DBINTERFACE.H"
#include "KDBProcessThread.h"
#include "KRoleDBHeader.h"
#include "..\..\Headers\KProtocolDef.h"

#include "..\MultiServer\Common\tstring.h"

using OnlineGameLib::Win32::_tstring;

#pragma warning(disable:4786)
#include <crtdbg.h>
#include <list>
#include "stdio.h"
using namespace std;

static const int g_snMaxPlayerCount = 5000;
static const int g_snPrecision = 10;
static const int g_snMaxBuffer = 15;
static const int g_snBufferSize = 1024;

IServer *g_pNetServer = NULL;
static HMODULE hDllModule;

DWORD g_dwPerSaveTime = 100;//每次存档的时间
DWORD g_dwMaxUnModifyTime;
list<TCmd*> g_MainThreadCmdList;
list<TGetRoleInfoFromDB*> g_DBLoadThreadCmdList;
list<TCmd*> g_NetServiceThreadCmdList;
extern BOOL InitNetEngine();
extern RoleDBServer::KNetServer g_NetServer;


unsigned char g_LoadDBThreadMark = MARK1;
unsigned char g_SaveDBThreadMark = MARK1;
unsigned char g_NetServerMark = MARK1;;
unsigned char g_MainLoopMark = MARK1;
int  g_nMistakeCount = 0;
size_t g_dwSendLen = 0;
size_t g_dwRecvLen = 0;

OnlineGameLib::Win32::CCriticalSection g_RoleInfoMutex;
OnlineGameLib::Win32::CCriticalSection g_RoleInfoSetMutex;

OnlineGameLib::Win32::CCriticalSection g_MainMsgListMutex;
OnlineGameLib::Win32::CCriticalSection g_NetMsgListMutex;
OnlineGameLib::Win32::CCriticalSection g_GetRoleInfoMutex;

TThreadData g_ThreadData[4];


char * GetRoleNameFromRoleBuffer(char * pRoleBuffer)
{
	if (pRoleBuffer == NULL)
		return NULL;
	try
	{
		TRoleData *pRoleData = (TRoleData*) pRoleBuffer;
		return pRoleData->BaseInfo.szName;
	}
	catch(...)
	{
		g_nMistakeCount ++;
		throw KException("GetRoleNameFromRoleBuffer", "内存有问题");
	}
}

int InitServer()
{
	int nReturn;
	S3DBI_InitDBInterface();
	nReturn = InitNetEngine();
	return nReturn;
}

DWORD ReleaseServer()
{
	
	for(int i = 0; i < 4; i ++)
		TerminateThread(g_ThreadData[i].hThread,0);

	g_pNetServer->CloseService();
	g_pNetServer->Cleanup();
	g_pNetServer->Release();
	::FreeLibrary( hDllModule );
	
	S3DBI_ReleaseDBInterface();
	return 1;
}

typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
														 REFIID	riid,
														 void	**ppv
														 );

void __stdcall ServerEventNotify(
								 LPVOID lpParam,
								 const unsigned long &ulnID,
								 const unsigned long &ulnEventType )
{

	switch( ulnEventType )
	{
	case enumClientConnectCreate:
		g_NetServer.RegisterClient(ulnID);
		break;
	case enumClientConnectClose:
		g_NetServer.DestoryClient(ulnID);
		break;
	}
	
	
}

BOOL InitNetEngine()
{
	
	hDllModule = ::LoadLibrary( "heaven.dll" );
	
	if ( hDllModule )
	{
		pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )GetProcAddress( hDllModule, "CreateInterface" );
		
		IServerFactory *pServerFactory = NULL;
		
		if ( SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
		{
			pServerFactory->SetEnvironment( g_snMaxPlayerCount, g_snPrecision, g_snMaxBuffer, g_snBufferSize  );
			
			pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &g_pNetServer ) );
			
			pServerFactory->Release();
		}
	}
	else
		return FALSE;
	
	g_pNetServer->Startup();
	
	g_pNetServer->RegisterMsgFilter( reinterpret_cast< void * >( g_pNetServer ), ServerEventNotify );
	
	g_pNetServer->OpenService( 0, 5001 );
	
	return 1;
}
