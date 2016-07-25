// ServerClone.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include <winsock2.h>
#include <process.h>

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "macro.h"
#include "Inifile.h"
#include "Utils.h"
#include "tstring.h"
#include "Library.h"
#include "Buffer.h"
#include "CriticalSection.h"

#include "..\..\Sword3PaySys\S3AccServer\AccountLoginDef.h"
#include "..\..\RoleDBManager\kroledbheader.h"
#include "S3DBInterface.h"
#include "KProtocolDef.h"
#include "KProtocol.h"

using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CCriticalSection;

static const int g_snMaxPlayerCount = 5000;
static const int g_snPrecision = 1;
static const int g_snMaxBuffer = 50;
static const int g_snBufferSize = 1024;

#define CONFIG_FILENAME			_T( "config.ini" )

const char *g_szGameSvrIP = "192.168.26.1";
unsigned short g_nServerPort = 5688;
IServer *g_pServer = NULL;

const char *g_szGatewayIP = "192.168.26.1";
const unsigned short g_nGatewayPort = 5632;
IClient *g_pClient = NULL;

GUID g_theLastUserGuid;
_tstring g_sRoleName;

/*
 * Global variable
 */
_tstring	g_sAppFullPath;

BYTE g_MapID[] = { 
		0,
		1,
		2,
		20,
		50,
		60,
		70,
		255 };

CPackager			m_theRecv;

CLibrary g_theHeavenLibrary( "heaven.dll" );
CLibrary g_theRainbowLibrary( "rainbow.dll" );

#include <iostream>
#include <strstream>
#include <list>
#include <map>

using namespace std;

CCriticalSection g_csPlayer;
list< UINT > g_thePlayer;

/*
 * Using directives
 */
using std::auto_ptr;
using std::endl;

typedef HRESULT ( __stdcall * pfnCreateClientInterface )( 
			REFIID riid, 
			void **ppv 
		);

typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
			REFIID	riid,
			void	**ppv
		);

void __stdcall ServerEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CCriticalSection::Owner locker( g_csPlayer );

	switch( ulnEventType )
	{
	case enumClientConnectCreate:

		g_thePlayer.push_back( ulnID );
		
		cout << "NO." << ulnID << " was connectted" << endl;
		break;
	
	case enumClientConnectClose:

		g_thePlayer.remove( ulnID );

		cout << "NO." << ulnID << " was disconnectted" << endl;
			
		tagLeaveGame lg;
		lg.cProtocol = c2s_leavegame;
		memcpy( lg.szRoleName, g_sRoleName.c_str(), g_sRoleName.size() );
		lg.szRoleName[g_sRoleName.size()] = '\0';

		g_pClient->SendPackToServer( ( const void * )&lg, sizeof( tagLeaveGame ) );

		/*
		 * c2s_logout
		 */
		
		break;
	}
}

void SmallPackProcess( IClient *pClient, const void *pData, size_t dataLength );
void LargePackProcess( IClient *pClient, const void *pData, size_t dataLength );

DWORD WINAPI ThreadProcess( void *pParam )
{
	IServer *pServer = ( IServer * )pParam;

	ASSERT( pServer );

	while ( true )
	{
		CCriticalSection::Owner locker( g_csPlayer );
	
		list< UINT >::iterator it;
		for ( it = g_thePlayer.begin(); it != g_thePlayer.end(); it ++ )
		{
			UINT index = ( *it );
			
			size_t datalength = 0;
			const char *pData = ( const char * )pServer->GetPackFromClient( index, datalength );

			if ( pData && datalength )
			{
				BYTE cProtocol = CPackager::Peek( pData );

				switch ( cProtocol )
				{
				case c2s_logiclogin:
					{
						tagLogicLogin *pLL = ( tagLogicLogin * )pData;

						if ( 0 == memcmp( &g_theLastUserGuid, &pLL->guid, sizeof( GUID ) ) )
						{
							cout << "Found a player is logging in this system who is " << g_sRoleName.c_str() << endl;

							/*
							 * Begin play game and notify gateway
							 */
							tagEnterGame eg;
							eg.cProtocol = c2s_entergame;
							strcpy( ( char * )eg.szRoleName, g_sRoleName.c_str() );
	
							g_pClient->SendPackToServer( ( const void * )&eg, sizeof( tagEnterGame ) );
						}
					}
					break;

				default:
					break;
				}
			}
		}

		::Sleep( 1 );
	}

	return 0L;
}

int main(int argc, char* argv[])
{
	/*
	 * Show some information for administrator
	 */
	cout << "+-----------------------------+" << endl;
	cout << "|  Welcome to JX.MMOG server  |" << endl;
	cout << "+-----------------------------+" << endl << endl;

	g_sAppFullPath = GetAppFullPath( NULL );

	cout << "The current directory : " 
		<< endl 
		<< _strlwr( const_cast< char * >( g_sAppFullPath.c_str() ) )
		<< endl << endl;

	_tstring sConfigFile;

	sConfigFile = g_sAppFullPath + CONFIG_FILENAME;

	CIniFile iniFile;

	iniFile.SetFile( sConfigFile.c_str() );

	g_nServerPort = iniFile.ReadInteger( "Server", "GameServPort", g_nServerPort );

	/*
	 * Open this server to player
	 */
	pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )( g_theHeavenLibrary.GetProcAddress( "CreateInterface" ) );

	IServerFactory *pServerFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( g_snMaxPlayerCount, g_snPrecision, g_snMaxBuffer, g_snBufferSize  );
		
		pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &g_pServer ) );
		
		pServerFactory->Release();
	}

	if ( !g_pServer )
	{
		cout << "Initialization failed! Don't find a correct heaven.dll" << endl;

		return -1;
	}

	g_pServer->Startup();

	g_pServer->RegisterMsgFilter( reinterpret_cast< void * >( g_pServer ), ServerEventNotify );

	if ( FAILED( g_pServer->OpenService( INADDR_ANY, g_nServerPort ) ) )
	{
		cout << "Startup server failed!" << endl;
	}

	DWORD dwThreadID = 0L;

	IServer *pCloneServer = NULL;
	g_pServer->QueryInterface( IID_IIOCPServer, ( void ** )&pCloneServer );

	HANDLE hThread = ::CreateThread( NULL, 0, ThreadProcess, ( void * )pCloneServer, 0, &dwThreadID );

	if ( INVALID_HANDLE_VALUE != hThread )
	{
		::CloseHandle( hThread ); 
	}

	cout << "Startup server successful!" << " Open port is " << g_nServerPort << endl;

	/*
	 * Connect to gateway
	 */
	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( g_theRainbowLibrary.GetProcAddress( "CreateInterface" ) );

	IClientFactory *pClientFactory = NULL;

	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 10, 1024 * 64 );

		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &g_pClient ) );

		pClientFactory->Release();
	}

	if ( !g_pClient )
	{
		cout << "Initialization failed! Don't find a correct rainbow.dll" << endl;

		return -1;
	}

	g_pClient->Startup();

	cout << "Gateway IP : " << g_szGatewayIP << " - Port : " << g_nGatewayPort << endl;

	if ( FAILED( g_pClient->ConnectTo( g_szGatewayIP, g_nGatewayPort ) ) )
	{
		cout << "Connect to gateway is failed!" << endl;
	}

	cout << "Connect gateway successful!" << endl;
	
	while ( true )
	{
		size_t dataLength = 0;
		const void *pData = g_pClient->GetPackFromServer( dataLength );

		if ( pData && dataLength )
		{
			BYTE cProtocol = CPackager::Peek( pData );
			
			if ( cProtocol < s2c_micropackbegin )
			{
				LargePackProcess( g_pClient, pData, dataLength );
			}
			else if ( cProtocol > s2c_micropackbegin )
			{
				SmallPackProcess( g_pClient, pData, dataLength );
			}
			else
			{
				ASSERT( FALSE && "Error!" );
			}		
		}

		::Sleep( 1 );
	}

	g_pClient->Cleanup();

	g_pClient->Release();
	
	g_pServer->CloseService();
	
	g_pServer->Cleanup();
	
	g_pServer->Release();	

	return 0;
}

void LargePackProcess( IClient *pClient, const void *pData, size_t dataLength )
{
	ASSERT( pData && dataLength );

	CBuffer *pBuffer = m_theRecv.PackUp( pData, dataLength );

	if ( pBuffer )
	{
		BYTE cProtocol = CPackager::Peek( pBuffer->GetBuffer() );
		
		switch ( cProtocol )
		{
		case s2c_syncgamesvr_roleinfo_cipher:
			{
				tagGuidableInfo *pGI = ( tagGuidableInfo * )pBuffer->GetBuffer();

				GUID guid;
				memcpy( &guid, &( pGI->guid ), sizeof( GUID ) );
				
				if ( pGI->datalength > 0 )
				{
					const TRoleData *pRoleData = ( const TRoleData * )( pGI->szData );
				
					cout << "AccName : " << pRoleData->BaseInfo.caccname;
					cout << " - Name : " << pRoleData->BaseInfo.szName << endl;
					
					/*
					 * Send a command that can permit player enter the gameserver
					 */
					tagPermitPlayerLogin ppl;
					
					ppl.cProtocol = c2s_permitplayerlogin;
					memcpy( &( ppl.guid ), &guid, sizeof( GUID ) );

					memcpy( &g_theLastUserGuid, &guid, sizeof( GUID ) );

					g_sRoleName = pRoleData->BaseInfo.szName;
					
					/*
					 * Succeeded : content is role name
					 * Failed	 : content is null
					 */
					strcpy( ( char * )( ppl.szRoleName ), ( const char * )( pRoleData->BaseInfo.szName ) );
					
					pClient->SendPackToServer( ( const void * )&ppl, sizeof( tagPermitPlayerLogin ) );
				}
				else
				{
					cout << "Don't find any valid data!" << endl;
				}
			}
			break;

		default:
			break;			
		}
		SAFE_RELEASE( pBuffer );
	}
}

void SmallPackProcess( IClient *pClient, const void *pData, size_t dataLength )
{
	ASSERT( pData && dataLength );

	BYTE cProtocol = CPackager::Peek( pData );
	
	switch ( cProtocol )
	{
	case s2c_querymapinfo:
		{
			tagUpdateMapID *pUMI;
			
			pUMI = ( tagUpdateMapID * )new BYTE[ sizeof( g_MapID ) + sizeof( tagUpdateMapID ) ];
			memset( pUMI, 0, sizeof( g_MapID ) + sizeof( tagUpdateMapID ) );
			
			pUMI->cProtocol = c2s_updatemapinfo;
			pUMI->cMapCount = sizeof( g_MapID ) / sizeof( BYTE );
			memcpy( pUMI->szMapID, g_MapID, sizeof( g_MapID ) );
			
			pClient->SendPackToServer( ( const void * )pUMI, sizeof( g_MapID ) + sizeof( tagUpdateMapID ) );
			
			SAFE_DELETE( pUMI );
		}
		break;
		
	case s2c_querygameserverinfo:
		{
			tagGameSvrInfo ni;
			
			ni.cProtocol = c2s_updategameserverinfo;
			
			ni.nIPAddr = OnlineGameLib::Win32::net_aton( g_szGameSvrIP );
			ni.nPort = g_nServerPort;
			
			pClient->SendPackToServer( ( const void * )&ni, sizeof( tagGameSvrInfo ) );
			
		}
		break;
	}
}
