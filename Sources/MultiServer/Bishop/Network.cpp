#include "Stdafx.h"

#include <objbase.h>
#include <initguid.h>

#include "Network.h"
#include "Macro.h"
#include "Inifile.h"
#include "Utils.h"
#include "tstring.h"

using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CCriticalSection;

OnlineGameLib::Win32::CLibrary				CNetwork::m_theHeavenLib( "heaven.dll" );
OnlineGameLib::Win32::CLibrary				CNetwork::m_theRainbowLib( "rainbow.dll" );

#define MAX_STRING_LEN	100

CNetwork::CNetwork()
	: /*m_pAccSvrClient( NULL )
	, */m_nAccSvrPort( 0 )
	, m_nRoleSvrPort( 0 )
	, m_nClientOpenPort( 0 )
	, m_nGameSvrOpenPort( 0 )
{

}

CNetwork::~CNetwork()
{

}

bool CNetwork::Create()
{
	CIniFile theConfigFile;

	_tstring sAppPath = GetAppFullPath( NULL );

	_tstring sConfigFile;

	sConfigFile = sAppPath + "Bishop.cfg";

	theConfigFile.SetFile( sConfigFile.c_str() );

	m_sRoleSvrIP.resize( MAX_STRING_LEN );

	DWORD dwLen = theConfigFile.ReadString( "Network", "RoleSvrIP", const_cast< char * >( m_sRoleSvrIP.c_str() ), MAX_STRING_LEN, "" );
	m_sRoleSvrIP.resize( dwLen + 1 /* '\0' */ );

	m_nRoleSvrPort = theConfigFile.ReadInteger( "Network", "RoleSvrPort", 0 );

	m_sAccSvrIP.resize( MAX_STRING_LEN );

	dwLen = theConfigFile.ReadString( "Network", "AccSvrIP", const_cast< char * >( m_sAccSvrIP.c_str() ), MAX_STRING_LEN, "" );
	m_sAccSvrIP.resize( dwLen + 1 /* '\0' */ );

	m_nAccSvrPort = theConfigFile.ReadInteger( "Network", "AccSvrPort", 0 );

	m_nClientOpenPort = theConfigFile.ReadInteger( "Network", "ClientOpenPort", 0 );
	m_nGameSvrOpenPort = theConfigFile.ReadInteger( "Network", "GameSvrOpenPort", 0 );

	return true;
}

void CNetwork::Destroy()
{
/*	if ( m_pAccSvrClient )
	{
		m_pAccSvrClient->Cleanup();
	}

	SAFE_RELEASE( m_pAccSvrClient );
*/
	CIniFile theConfigFile;

	_tstring sAppPath = GetAppFullPath( NULL );

	_tstring sConfigFile;

	sConfigFile = sAppPath + "Bishop.cfg";

	theConfigFile.SetFile( sConfigFile.c_str() );

	theConfigFile.WriteString( "Network", "AccSvrIP", m_sAccSvrIP.c_str() );
	theConfigFile.WriteInteger( "Network", "AccSvrPort", m_nAccSvrPort );

	theConfigFile.WriteString( "Network", "RoleSvrIP", m_sRoleSvrIP.c_str() );
	theConfigFile.WriteInteger( "Network", "RoleSvrPort", m_nRoleSvrPort );

	theConfigFile.WriteInteger( "Network", "ClientOpenPort", m_nClientOpenPort );
	theConfigFile.WriteInteger( "Network", "GameSvrOpenPort", m_nGameSvrOpenPort );
}

//IClient *CNetwork::CreateAccSvrClient( const char * const pAddressToConnectServer, 
//									  unsigned short usPortToConnectServer )
//{
//	if ( m_pAccSvrClient )
//	{
//		IClient *pClonClient = NULL;
//		m_pAccSvrClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pClonClient ) );
//
//		return pClonClient;
//	}
//
//	/*
//	 * There is connectted the heaven by the rainbow
//	 */
//	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( m_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
//	
//	IClientFactory *pClientFactory = NULL;
//
//	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
//	{
//		pClientFactory->SetEnvironment( 1024 * 1024 );
//		
//		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pAccSvrClient ) );
//		
//		SAFE_RELEASE( pClientFactory );
//	}
//
//	if ( m_pAccSvrClient )
//	{
//		m_pAccSvrClient->Startup();
//		
//		m_pAccSvrClient->RegisterMsgFilter( reinterpret_cast< void * >( this ), AccSvrEventNotify );
//
//		if ( FAILED( m_pAccSvrClient->ConnectTo( pAddressToConnectServer, usPortToConnectServer ) ) )
//		{
//			m_pAccSvrClient->Cleanup();
//
//			SAFE_RELEASE( m_pAccSvrClient );
//
//			return NULL;
//		}
//		
//		/*
//		 * Recorder
//		 */
//		m_sAccSvrIP = pAddressToConnectServer;
//		m_nAccSvrPort = usPortToConnectServer;
//
//		IClient *pClonClient = NULL;
//		m_pAccSvrClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &pClonClient ) );
//
//		return pClonClient;
//	}
//
//	return NULL;
//}

/*void __stdcall CNetwork::AccSvrEventNotify( LPVOID lpParam, const unsigned long &ulnEventType )
{
	CNetwork *pThis = reinterpret_cast< CNetwork * >( lpParam );

	ASSERT( lpParam );

	switch ( ulnEventType )
	{
	case enumServerConnectCreate:

		::MessageBeep( -1 );

		break;

	case enumServerConnectClose:

		if ( pThis )
		{
			pThis->DistroyAccSvrClient();
		}
		
		break;
	}
}

void CNetwork::DistroyAccSvrClient()
{
	SAFE_RELEASE( m_pAccSvrClient );
}*/
