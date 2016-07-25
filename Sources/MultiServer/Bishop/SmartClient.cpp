#include "stdafx.h"
#include "SmartClient.h"

#include "Macro.h"

#include "KProtocolDef.h"
#include "AccountLoginDef.h"
#include "Buffer.h"

#include "msg_define.h"
#include "Macro.h"
#include "../Engine/Src/KWin32.h"
#include "../Engine/Src/KSG_MD5_String.h"
#include "inoutmac.h"
#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")


using OnlineGameLib::Win32::CPackager;

using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::CBuffer;

OnlineGameLib::Win32::CLibrary	CSmartClient::m_theRainbowLib( "rainbow.dll" );

static const DWORD g_dwServerIdentify = 0xAEFC07B5;

CSmartClient g_theSmartClient;

CSmartClient::CSmartClient()
	: m_hWorkingThread( NULL )
	, m_pClient( NULL )
	, m_hwndContain( NULL )
	, m_nSvrPort( 0 )
	, m_theAllocator( 1024, 500 )
{
}

CSmartClient::~CSmartClient()
{
	if ( m_pClient )
	{
		m_pClient->Cleanup();
	}

	SAFE_RELEASE( m_pClient );
}

bool CSmartClient::Open( const char * const pAddressToConnectServer, 
		unsigned short usPortToConnectServer, 
		HWND hwndContain,
		const char *pName,
		const char *pPwd,
		bool bReLink /* true */ )
{
	if ( !pName || !pPwd )
	{
		return false;
	}

	m_sUserName = pName;
	m_sUserPwd = pPwd;

	m_sSvrIP = pAddressToConnectServer;
	m_nSvrPort = usPortToConnectServer;

	m_hwndContain = hwndContain;

	/*
	 *	m_bReLink == true ( default )
	 */
	m_bReLink = bReLink;

	m_ClientStatus = enumClientConnect;

	IClient* pClient = _Connect();
	if (pClient)
	{
		_Verify(pClient, false);
		SAFE_RELEASE(pClient);
	}

	bool ok = CreateConnectThread();

	return ok;
}

void CSmartClient::Close()
{
	SetClientStatus(enumClientError);	//停止线程
}

void __stdcall CSmartClient::EventNotify( LPVOID lpParam, 
						const unsigned long &ulnEventType )
{
	CSmartClient *pThis = reinterpret_cast< CSmartClient * >( lpParam );

	ASSERT( pThis );

	try
	{
		pThis->_EventNotify( ulnEventType );
	}
	catch(...)
	{
		TRACE( "CSmartClient::EventNotify exception!" );
	}
}

void CSmartClient::_EventNotify( const unsigned long &ulnEventType )
{
	switch( ulnEventType )
	{
	case enumServerConnectCreate:
		::PostMessage( m_hwndContain, WM_SERVER_STATUS, ACCOUNTSERVER_NOTIFY, CONNECTED );
		break;

	case enumServerConnectClose:
		{
			::PostMessage( m_hwndContain, WM_SERVER_STATUS, ACCOUNTSERVER_NOTIFY, DICONNECTED );
			SwitchToConnecting();
		}
		break;
	}
}

bool CSmartClient::CreateConnectThread()
{
	/*
	 * Start a working thread
	 */
	DWORD dwThreadID = 0;

	m_hWorkingThread = ::CreateThread( NULL, 
		0, 
		WorkingThreadFunction, 
		( void * )this, 
		0, 
		&dwThreadID );
			
	if ( m_hWorkingThread == INVALID_HANDLE_VALUE )
	{
		return false;
		
	}

	
	return true;
}

DWORD WINAPI CSmartClient::WorkingThreadFunction( void *pV )
{
	CSmartClient *pThis = reinterpret_cast< CSmartClient * >( pV );

	ASSERT( pThis );
	IClient* pClient = NULL;

	try
	{
		while (!pThis->IsError())
		{
			if (pThis->IsConnecting())
			{
				if (pThis->m_pClient)
				{
					pThis->m_pClient->Cleanup();
					pThis->m_pClient = NULL;
				}
				SAFE_RELEASE( pClient );
				pClient = pThis->_Connect();
			}
			else if (pThis->IsVerifying())
			{
				pThis->_Verify(pClient, true);
				SAFE_RELEASE( pClient );
			}
			Sleep(10);
		}
	}
	catch(...)
	{
		::MessageBox( NULL, "Run a working thread is failed!", "CSmartClient class", MB_OK );
	}
	::PostMessage(pThis->m_hwndContain, WM_CLOSE, NULL, NULL);
	return 0;
}

IClient* CSmartClient::_Connect()
{
	if ( !IsConnecting() )
		return NULL;

#ifdef	CONSOLE_DEBUG
					_cprintf( "Begin connect\n");
#endif	

	IClient *pClient = NULL;
	/*
	 * There is connectted the heaven by the rainbow
	 */

	pfnCreateClientInterface pClientFactroyFun = ( pfnCreateClientInterface )( m_theRainbowLib.GetProcAddress( _T( "CreateInterface" ) ) );
	IClientFactory *pClientFactory = NULL;
	
	if ( pClientFactroyFun && SUCCEEDED( pClientFactroyFun( IID_IClientFactory, reinterpret_cast< void ** >( &pClientFactory ) ) ) )
	{
		pClientFactory->SetEnvironment( 1024 * 1024 );
		
		pClientFactory->CreateClientInterface( IID_IESClient, reinterpret_cast< void ** >( &pClient ) );
		
		SAFE_RELEASE( pClientFactory );
	}
	
	if ( pClient )
	{
		pClient->Startup();
		
		pClient->RegisterMsgFilter( reinterpret_cast< void * >( this ), EventNotify );
		
		if ( FAILED( pClient->ConnectTo( m_sSvrIP.c_str(), m_nSvrPort ) ) )
		{
			pClient->Cleanup();
			
			SAFE_RELEASE( pClient );
				
#ifdef	CONSOLE_DEBUG
					_cprintf( "Reconnect Failed\n");
#endif	
			::Sleep( 5000 );
			
			return NULL;
		}
		else
		{
			SetClientStatus(enumClientVerify);	
		}
	}
	
	return pClient;
}

void CSmartClient::_Verify(IClient* pClient, bool bDisconnect)
{
	if (!IsVerifying())
		return;

	if (pClient == NULL)
		return;

#ifdef	CONSOLE_DEBUG
					_cprintf( "Begin verify\n");
#endif	
	
	/*
	* Send a login command
	*/
	const size_t datalength = sizeof( KServerAccountUserLoginInfo ) + 1;
	
	BYTE *pData = new BYTE[datalength];
	
	KServerAccountUserLoginInfo serlogin;
	serlogin.Size = sizeof( KServerAccountUserLoginInfo );
	
	serlogin.Type = ServerAccountUserLoginInfo;
	serlogin.Version = ACCOUNT_CURRENT_VERSION;
	serlogin.Operate = g_dwServerIdentify;
	
	size_t lenTag, lenScr;
	
	lenTag = sizeof( serlogin.Account );
	lenScr = m_sUserName.size() + 1;
	
	lenTag = ( lenTag > lenScr ) ? lenScr : lenTag;
	
	memcpy( serlogin.Account, m_sUserName.c_str(), lenTag );
	serlogin.Account[lenTag - 1] = '\0';
	
	lenTag = sizeof( serlogin.Password );
	
	KSG_StringToMD5String(serlogin.Password, m_sUserPwd.c_str() /*szPassword*/);
	
	serlogin.Address = -1;
	
	// add by spe 2003/08/08
	gGetMacAndIPAddress(NULL, NULL, serlogin.MacAddress, &serlogin.Address);
	
	if ( bDisconnect )
	{
		*pData = c2s_gatewayverifyagain;
	}
	else
	{
		*pData = c2s_gatewayverify;
	}
	memcpy( pData + 1, &serlogin, sizeof( KServerAccountUserLoginInfo ) );
	
	pClient->SendPackToServer( ( const void * )pData, datalength );
	
	SAFE_DELETE_ARRAY( pData );
			
	/*
	* Wait for write back
	*/
	while ( pClient )
	{
		if ( !IsVerifying() )
		{
			break;	//因为正常的状态变化引起的退出
		}
#ifdef	CONSOLE_DEBUG
					_cprintf( "Wait for verify\n");
#endif	

		size_t nLen = 0;
		
		const void *pData = pClient->GetPackFromServer( nLen );
	
		if ( !pData || 0 == nLen )
		{
			::Sleep( 1 );
			
			continue;
		}
		
		BYTE cProtocol = CPackager::Peek( pData );
		
		switch ( cProtocol )
		{
		case s2c_gatewayverify:
			{
				KAccountUserReturn* pReturn = ( KAccountUserReturn * )( ( ( char * )pData ) + 1 );
				
				nLen--;
				if ( pReturn &&
					( g_dwServerIdentify == pReturn->Operate ) && 
					( nLen == sizeof( KAccountUserReturn ) ) )
				{
					if ( ACTION_SUCCESS == pReturn->nReturn )
					{
						::PostMessage( m_hwndContain, WM_SERVER_LOGIN_SUCCESSFUL, 0L, 0L );
						pClient->QueryInterface( IID_IESClient, reinterpret_cast< void ** >( &m_pClient ) );

						if (SetClientStatus(enumClientNormal))
						{
							Send(NULL, 0);
#ifdef	CONSOLE_DEBUG
					_cprintf( "Succeed verify\n");
#endif	

						}
					}
					else if ( E_ACCOUNT_OR_PASSWORD == pReturn->nReturn )
					{
						::PostMessage( m_hwndContain, WM_SERVER_LOGIN_FAILED, enumUsrNamePswdErr, 0L );

						SetClientStatus(enumClientError);
					}
					else if ( E_ADDRESS_OR_PORT == pReturn->nReturn )
					{
						::PostMessage( m_hwndContain, WM_SERVER_LOGIN_FAILED, enumIPPortErr, 0L );

						SetClientStatus(enumClientError);
					}
					else
					{
						::PostMessage( m_hwndContain, WM_SERVER_LOGIN_FAILED, enumException, 0L );

						SetClientStatus(enumClientError);
					}
					
				}
			}
			break;
			
		default:
			
			ASSERT( FALSE );

			SetClientStatus(enumClientError);
			
			break;
		}
	}	
}

bool CSmartClient::Send( const void * const pData, const size_t &datalength )
{
	if (IsNormal())
	{
		if (m_pClient)
		{
			/*
			 *	Send the histroy information to server
			 */
			{
				CCriticalSection::Owner locker( m_csHistroyList );

				stdList::iterator it;
				for ( it = m_theHistroyList.begin();
					it != m_theHistroyList.end();
					it ++ )
					{
						CBuffer *pBuffer = *it;

						if ( pBuffer )
						{
							m_pClient->SendPackToServer( 
								( const void * )( pBuffer->GetBuffer() ), 
								pBuffer->GetUsed() );

							SAFE_RELEASE( pBuffer );
						}
					}

				m_theHistroyList.clear();
			}

			if (pData && datalength)
				m_pClient->SendPackToServer( pData, datalength );

			return true;
		}
	}
	else if (pData && datalength > 0)
	{
		/* Store data into buffer */
		CCriticalSection::Owner locker( m_csHistroyList );

		CBuffer *pBuffer = m_theAllocator.Allocate();

		ASSERT( pBuffer );

		pBuffer->AddData((BYTE*) pData, datalength );

		m_theHistroyList.push_back( pBuffer );
	}

	return false;
}

const void * CSmartClient::Recv( size_t &datalength )
{
	if (IsNormal())
	{
		if ( m_pClient )
		{
			return m_pClient->GetPackFromServer( datalength );
		}
	}

	datalength = 0;
	return NULL;
}

bool CSmartClient::IsNormal()
{
	CCriticalSection::Owner locker(m_csStatus);
	return (m_ClientStatus == enumClientNormal);
}

bool CSmartClient::IsConnecting()
{
	CCriticalSection::Owner locker(m_csStatus);
	return (m_ClientStatus == enumClientConnect);
}

bool CSmartClient::IsVerifying()
{
	CCriticalSection::Owner locker(m_csStatus);
	return (m_ClientStatus == enumClientVerify);
}

bool CSmartClient::IsError()
{
	CCriticalSection::Owner locker(m_csStatus);
	return (m_ClientStatus == enumClientError);
}

void CSmartClient::SwitchToConnecting()		//切换状态到重连,如有必要会停止重连线程
{
	if (m_bReLink)
		SetClientStatus(enumClientConnect);
	else
		SetClientStatus(enumClientError);	;	//停止线程
}

bool CSmartClient::SetClientStatus(enumClientStatus nNewS)
{
	CCriticalSection::Owner locker(m_csStatus);

	if ( nNewS == enumClientError )
	{
		m_ClientStatus = nNewS;
		return true;
	}

	if (m_ClientStatus == enumClientNormal && nNewS == enumClientConnect)
	{
		m_ClientStatus = nNewS;
		return true;
	}

	if (m_ClientStatus == enumClientConnect &&
		(nNewS == enumClientConnect || nNewS == enumClientVerify)
		)
	{
		m_ClientStatus = nNewS;
		return true;
	}

	if (m_ClientStatus == enumClientVerify &&
		(nNewS == enumClientConnect || nNewS == enumClientNormal)
		)
	{
		m_ClientStatus = nNewS;
		return true;
	}

	return false;
}
