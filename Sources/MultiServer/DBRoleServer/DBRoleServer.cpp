// DBRoleServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include <winsock2.h>
#include <process.h>

#include "kprotocoldef.h"

#include "IDBRoleServer.h"

#include "console.h"
#include "Event.h"
#include "macro.h"
#include "Inifile.h"
#include "Utils.h"
#include "tstring.h"
#include "Library.h"
#include "Buffer.h"
#include "CriticalSection.h"

#include "IServer.h"
#include "HeavenInterface.h"

#include "ClientNode.h"

#include <iostream>
#include <strstream>
#include <string>
#include <list>

using namespace std;

using OnlineGameLib::Win32::Console::clrscr;
using OnlineGameLib::Win32::Console::setcolor;
using OnlineGameLib::Win32::CEvent;
using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CLibrary;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::CPackager;
using OnlineGameLib::Win32::CBuffer;
using OnlineGameLib::Win32::CCriticalSection;

static CLibrary g_theHeavenLibrary( "heaven.dll" );

static CEvent g_hQuitEvent( NULL, true, false );
static CEvent g_hStartWorkEvent( NULL, true, false );

static unsigned short g_nServerPort = 5001;
static IServer *g_pServer = NULL;

static CCriticalSection					g_csPlayer;

typedef map< size_t, CClientNode * > stdPlayerMap;
static 	stdPlayerMap	g_thePlayer;

static CPackager				g_theRecv;
static CPackager				g_theSend;

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
		{
			CClientNode *pNode = CClientNode::AddNode( g_pServer, ulnID );
			
			if ( pNode )
			{
				CCriticalSection::Owner lock( g_csPlayer );

				g_thePlayer.insert( stdPlayerMap::value_type( ulnID, pNode ) );
			}
			
			cout << "NO." << ulnID << " was connectted" << endl;
		}
		break;

	case enumClientConnectClose:
		{
			{
				CCriticalSection::Owner lock( g_csPlayer );

				stdPlayerMap::iterator it;
				if ( g_thePlayer.end() != ( it = g_thePlayer.find( ulnID ) ) )
				{
					g_thePlayer.erase( ulnID );
				}
			}
			
			CClientNode::DelNode( ulnID );
			
			cout << "NO." << ulnID << " was disconnectted" << endl;
		}
		break;
	}
}

DWORD WINAPI ThreadProcess( void *pParam )
{
	IServer *pServer = ( IServer * )pParam;

	ASSERT( pServer );

	g_hStartWorkEvent.Wait();

	try
	{		
		while ( !g_hQuitEvent.Wait( 0 ) )
		{
			{
				CCriticalSection::Owner lock( g_csPlayer );
				
				stdPlayerMap::iterator it;
				for ( it = g_thePlayer.begin(); it != g_thePlayer.end(); it ++ )
				{
					UINT index = ( *it ).first;
					CClientNode *pClientNode = ( *it ).second;
					
					size_t datalength = 0;
					const char *pData = ( const char * )pServer->GetPackFromClient( index, datalength );
					
					while ( pClientNode && pData && datalength )
					{
						pClientNode->AppendData( pData, datalength );
						
						pData = ( const char * )pServer->GetPackFromClient( index, datalength );
					}
				}
			}
			
			::Sleep( 1 );
		}
	}
	catch(...)
	{
		::MessageBox( NULL, "ThreadProcess was error!", "Warning", MB_OK );
	}

	return 0L;
}

int main(int argc, char* argv[])
{
	clrscr();

	cout << "Welcome to the DBRoleServer" << endl;

	if ( !InitDBInterface() )
	{
		cout << "Setup dbserver is failed!" << endl;
		exit( 0 );
	}

	/*
	 * Open this server to client
	 */
	pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )( g_theHeavenLibrary.GetProcAddress( "CreateInterface" ) );

	IServerFactory *pServerFactory = NULL;

	if ( pFactroyFun && SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
	{
		pServerFactory->SetEnvironment( 1000, 10, 20, 1024 * 64 );
		
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

	CClientNode::Start( g_pServer );

	g_hStartWorkEvent.Set();

	cout << "Please 'exit' to end this application!" << endl;

	bool quit = false; 

	string sCmd;

	while ( sCmd.compare( "exit" ) && sCmd.compare( "Exit" ) )
	{
		cin >> sCmd;
	}

	CClientNode::End();

	g_hQuitEvent.Set();
	
	DWORD result = ::WaitForSingleObject( hThread, 50000 );

	if ( result == WAIT_TIMEOUT )
	{
		::TerminateThread( hThread, ( DWORD )( -1 ) );
	}

	if ( hThread != INVALID_HANDLE_VALUE )
	{
		SAFE_CLOSEHANDLE( hThread );
	}	

	g_pServer->CloseService();

	SAFE_RELEASE( g_pServer );

	ReleaseDBInterface();
	
	return 0;
}

