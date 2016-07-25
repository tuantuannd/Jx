#include "GameClient.h"

//[ Include in .\..\ESClient
#include "ESClient\Utils.h"
#include "ESClient\Win32Exception.h"
#include "ESClient\Socket.h"
#include "ESClient\Exception.h"
//]

//[ Include in .\..\Protocol
#include "Protocol\Protocol.h"
//]

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
			: OnlineGameLib::Win32::CSocketClient(addressToConnectServer, portToConnectServer, maxFreeBuffers, bufferSize),
			m_pRecvBuf( NULL ),
			m_dwRecvSize( 0 )
{
	m_hHasRecvdata = CreateEvent(
		NULL,			// no security attributes
        TRUE,			// manual-reset event
        FALSE,			// initial state is signaled
        "ReadEvent"		// object name
		);
}

CGameClient::CGameClient(
			size_t maxFreeBuffers,
			size_t bufferSize /* = 1024 */
			)
			: OnlineGameLib::Win32::CSocketClient(maxFreeBuffers, bufferSize),
			m_pRecvBuf( NULL ),
			m_dwRecvSize( 0 )
{
	m_hHasRecvdata = CreateEvent(
		NULL,			// no security attributes
        TRUE,			// manual-reset event
        FALSE,			// initial state is signaled
        "ReadEvent"		// object name
		);
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
		pBuffer = ProcessDataStream( pBuffer );
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

void CGameClient::ProcessCommand( const OnlineGameLib::Win32::CIOBuffer *pBuffer)
{
	const BYTE *pPackData = pBuffer->GetBuffer();
	const size_t used = pBuffer->GetUsed();

	bool ok = false;

	WORD wDataLen = 0;
	PACK_HEADER ph = {0};

	if ( used > PACK_HEADER_LEN )
	{
		memcpy( &ph, pPackData, PACK_HEADER_LEN );

		wDataLen = ph.wDataLen;

		ok = true;
	}
	
	if ( ok )
	{
		const BYTE *pData = pPackData + PACK_HEADER_LEN;
		const size_t datalength = wDataLen;

		/*
		 * TODO : Process data receive from server
		 */
		
		// ==================== BEGIN =======================
		Output( _T("Package[length:") + ToString( wDataLen ) + _T("]\n") + DumpData( pData, datalength, 40 ) );

		m_pRecvBuf = pData;
		m_dwRecvSize = datalength;
		SetEvent( m_hHasRecvdata );
		// ====================  END  =======================
	}
	else
	{
		Output( "found error and close this connection!" );
		
		StopConnections();
	}
}

size_t CGameClient::GetMinimumMessageSize() const
{
	/*
	 * The smallest possible package we accept is pack-header
	 * once we have this many bytes we can start with try and work out
	 * what we have...
	 */
	
	return PACK_HEADER_LEN;
}

size_t CGameClient::GetMessageSize( 
			const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const
{
	const BYTE *pData = pBuffer->GetBuffer();	
	const size_t used = pBuffer->GetUsed();
	
	PACK_HEADER ph = {0};

	/*
	 * First, verify the flag of a message
	 */
	if ( used > PACK_HEADER_LEN )
	{
		memcpy( &ph, pData, PACK_HEADER_LEN );

		if ( PACK_BEGIN_FLAG == ph.cPackBeginFlag && 
			 PACK_END_FLAG == ph.cPackEndFlag )
		{
			WORD wCRC = MAKE_CRC_DATE( PACK_BEGIN_FLAG, PACK_END_FLAG, ph.wDataLen );

			if ( ph.wCRCData == wCRC )
			{
				return ph.wDataLen + PACK_HEADER_LEN;
			}
		}		
	}
	
	return 0;
}

CIOBuffer *CGameClient::ProcessDataStream( OnlineGameLib::Win32::CIOBuffer *pBuffer)
{
	bool done;
	
//	DEBUG_ONLY( Output( _T("ProcessDataStream:\n") + DumpData( pBuffer->GetBuffer(), pBuffer->GetUsed(), 40 ) ) );
	
	do
	{
		done = true;
		
		const size_t used = pBuffer->GetUsed();
		
		if ( used >= GetMinimumMessageSize() )
		{
			const size_t messageSize = GetMessageSize( pBuffer );
			
			if ( messageSize == 0 )
			{
				/*
				 * havent got a complete message yet.
				
				 * we null terminate our messages in the buffer, so we need to reserve
				 * a byte of the buffer for this purpose...
				 */
				
				if ( used == ( pBuffer->GetSize() - 1 ) )
				{
					Output( _T("Too much data!") );
					
					/*
					 * Write this message and then shutdown the sending side of the socket.
					 */
					Output( "found error and close this connection!" );
		
					StopConnections();
					
					/*
					 * throw the rubbish away
					 */
					pBuffer->Empty();
					
					done = true;
				}
			}
			else if ( used == messageSize )
			{
				Output( _T("Got complete, distinct, message") );
				/*
				 * we have a whole, distinct, message
				 */
				
				pBuffer->AddData(0);   // null terminate the command string;
				
				ProcessCommand( pBuffer );
				
				pBuffer->Empty();
				
				done = true;
			}
			else if (used > messageSize)
			{
				Output(_T("Got message plus extra data"));
				/*
				 * we have a message, plus some more data
				 * 
				 * allocate a new buffer, copy the extra data into it and try again...
				 */
				
				CIOBuffer *pMessage = pBuffer->SplitBuffer( messageSize );
				
				pMessage->AddData(0);   // null terminate the command string;
				
				ProcessCommand( pMessage );
				
				pMessage->Release();
				
				/*
				 * loop again, we may have another complete message in there...
				 */
				
				done = false;
			}
		}
	}
	while ( !done );
	
	/*
	 * not enough data in the buffer, reissue a read into the same buffer to collect more data
	 */
	return pBuffer;
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
