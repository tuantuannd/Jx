#include "GameClient.h"

//[ Include in .\..\ESClient
#include "ESClient\Utils.h"
#include "ESClient\Win32Exception.h"
#include "ESClient\Socket.h"
#include "ESClient\Exception.h"
//]

#include "Protocol.h"

#include "JXClient.h"

#pragma comment (lib, "ESClient.lib")
/*
 * Using directives
 */
using OnlineGameLib::Win32::CIOBuffer;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::CWin32Exception;
using OnlineGameLib::Win32::CSocket;
using OnlineGameLib::Win32::DumpData;


CGameClient::CGameClient(
			const OnlineGameLib::Win32::_tstring &addressToConnectServer,
			unsigned short portToConnectServer,
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */
			)
		  : OnlineGameLib::Win32::CSocketClient(addressToConnectServer, portToConnectServer, maxFreeBuffers, bufferSize)
{

}

CGameClient::CGameClient(
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */
			)
		: OnlineGameLib::Win32::CSocketClient(maxFreeBuffers, bufferSize)
{
}

CGameClient::~CGameClient()
{	
	try
	{
		ReleaseBuffers();
	}
	catch(...)
	{
	}
}

SOCKET CGameClient::CreateConnectionSocket( 
					  const OnlineGameLib::Win32::_tstring &addressToConnectServer,
					  unsigned short port)
{
	SOCKET s = ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, 0 );
	
	if ( INVALID_SOCKET == s )
	{
		throw CWin32Exception( _T("CSocket::CreateListeningSocket()"), ::WSAGetLastError() );
	}
	
	CSocket connectionSocket( s );
	
	CSocket::InternetAddress localAddress( addressToConnectServer, port );
	
	connectionSocket.Connect( localAddress );
	
	return connectionSocket.Detatch();	
}

void CGameClient::OnStartConnections()
{
	Output( _T("OnStartConnections") );
}

void CGameClient::OnStopConnections()
{
	DisConnetServer();	

	Output( _T("OnStopConnections") );
}

void CGameClient::OnShutdownInitiated()
{
	Output( _T("OnShutdownInitiated") );
}

void CGameClient::OnShutdownComplete()
{
	Output( _T("OnShutdownComplete") );
}

void CGameClient::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

void CGameClient::OnConnect()
{
	ConnectServer();

	Output( _T("OnConnect") );
}

void CGameClient::OnClose()
{
	Output( _T("OnClose") );
}

void CGameClient::ReadCompleted( OnlineGameLib::Win32::CIOBuffer *pBuffer )
{
	try
	{	
		const BYTE *pPackData = pBuffer->GetBuffer();
		const size_t used = pBuffer->GetUsed();

		RecvFromServer( reinterpret_cast<const char*>( pPackData ), used );
		
		pBuffer->Empty();
	}
	catch(const CException &e)
	{
		Output( _T("ReadCompleted - Exception - ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
		StopConnections();
	}
	catch(...)
	{
		Output( _T("ReadCompleted - Unexpected exception") );
		StopConnections();
	}
}


void CGameClient::PreWrite( 
			OnlineGameLib::Win32::CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength )
{
	/*
	 * TODO : You can change protocol that it is used to split package
	 */
	
	if ( pBuffer && dataLength > 0 )
	{
		PACK_HEADER ph = {0};
		
		ph.cPackBeginFlag = PACK_BEGIN_FLAG;
		ph.cPackEndFlag = PACK_END_FLAG;
		
		ph.wDataLen = dataLength;
		ph.wCRCData = MAKE_CRC_DATE( PACK_BEGIN_FLAG, PACK_END_FLAG, dataLength );

		pBuffer->AddData( reinterpret_cast<const char *>(&ph), PACK_HEADER_LEN );
	}
}
