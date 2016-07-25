#include "JXClient.h"
#include <assert.h>

//[ Include in .\..\IPCPServer
#include "IOCPServer\Exception.h"
#include "IOCPServer\Utils.h"
#include "IOCPServer\ManualResetEvent.h"
//]

#include "Protocol.h"

#include "GameClient.h"

using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CManualResetEvent;
using OnlineGameLib::Win32::GetTimeStamp;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::DumpData;

static CGameClient	g_theJXClient( MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE );

static CConnectManager g_theManager( MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE );

CConnectManager::CConnectManager(
			size_t maxFreeBuffers, 
			size_t bufferSize /*1024*/ )
		: OnlineGameLib::Win32::CIOBuffer::Allocator(bufferSize, maxFreeBuffers),
		m_pfnCallbackClientEvent(NULL)
{
	m_pReadBuffer = Allocate();
	m_pRecvBuffer = Allocate();
}

CConnectManager::~CConnectManager()
{
	m_pReadBuffer->Release();
	m_pRecvBuffer->Release();
	
	try
	{
		Flush();
	}
	catch(...)
	{
	}
}

void CConnectManager::ConnectServer()
{
	if ( m_pfnCallbackClientEvent )
	{
		m_pfnCallbackClientEvent( SERVER_CONNECT_CREATE	);
	}
}

void CConnectManager::DisConnetServer()
{
	if ( m_pfnCallbackClientEvent )
	{
		m_pfnCallbackClientEvent( SERVER_CONNECT_CLOSE );
	}
}

void CConnectManager::RecvFromServer( const char *pData, size_t dataLength )
{
	CCriticalSection::Owner locker( m_csReadAction );

	m_pRecvBuffer->AddData( pData, dataLength );
}

const char *CConnectManager::GetFromClient( size_t &dataLength )
{
	CCriticalSection::Owner locker( m_csReadAction );
	
	m_pReadBuffer->Empty();

	m_pRecvBuffer = ProcessDataStream( m_pRecvBuffer );

	const BYTE *pPackData = m_pReadBuffer->GetBuffer();
	dataLength = m_pReadBuffer->GetUsed();	
	
	//[
#ifdef NETWORK_DEBUG	

	if ( dataLength > 0 )
	{
		static const int sc_nFlagLen = sizeof(DWORD) * 2 + PACK_HEADER_LEN + sizeof(BYTE);

		assert( dataLength > sc_nFlagLen && "Get error buffer from server!" );
		dataLength -= sc_nFlagLen;
	}

#endif //NETWORK_DEBUG
	//]	

	return reinterpret_cast<const char*>( pPackData );
}

void CConnectManager::ProcessCommand( const OnlineGameLib::Win32::CIOBuffer *pBuffer)
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
		
		//Output( _T("Get a package[length:") + ToString( wDataLen ) + _T("]\n") + DumpData( pData, datalength, 50 ) );

		m_pReadBuffer->AddData( pData, datalength );
		
	}
	else
	{
		Output( "found error and close this connection!" );
		
		Shutdown();
	}
}

size_t CConnectManager::GetMinimumMessageSize() const
{
	/*
	 * The smallest possible package we accept is pack-header
	 * once we have this many bytes we can start with try and work out
	 * what we have...
	 */
	
	return PACK_HEADER_LEN;
}

size_t CConnectManager::GetMessageSize( 
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

OnlineGameLib::Win32::CIOBuffer *CConnectManager::ProcessDataStream( 
						OnlineGameLib::Win32::CIOBuffer *pBuffer)
{
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
	
				Shutdown();
				
				/*
				 * throw the rubbish away
				 */
				pBuffer->Empty();					
			}
		}
		else if ( used == messageSize )
		{
			Output( _T("Got a complete message and begin to process it") );
			/*
			 * we have a whole, distinct, message
			 */
			
			ProcessCommand( pBuffer );
			
			pBuffer->Empty();
		}
		else if (used > messageSize)
		{
			Output(_T("Got message plus extra data"));
			/*
			 * we have a message, plus some more data
			 * 
			 * allocate a new buffer, copy the extra data into it and try again...
			 */
			
			OnlineGameLib::Win32::CIOBuffer *pMessage = pBuffer->SplitBuffer( messageSize );
			
			ProcessCommand( pMessage );
			
			pMessage->Release();
			
		}
	}
	
	/*
	 * Reissue a read into the same buffer to collect more data
	 */
	return pBuffer;
}

bool ClientStartup( 
		JX_CLIENT_
		)
{
	g_theJXClient.Start();

	return true;
}

void ClientCleanup( JX_CLIENT_ )
{
	try
	{
		g_theJXClient.StopConnections();
		
		g_theJXClient.WaitForShutdownToComplete();
	}
	catch( const CException &e )
	{
		Output(_T("Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage());
	}
	catch(...)
	{
		Output(_T("Unexpected exception"));
	}
}

bool ConnectTo( 
		JX_CLIENT_
		const OnlineGameLib::Win32::_tstring &addressToConnectServer,
		unsigned short portToConnectServer )
{
	bool ok = false;

	try
	{
		g_theJXClient.ConnectTo( addressToConnectServer, portToConnectServer );
		
		g_theJXClient.StartConnections();

		ok = true;
	}
	catch( const CException &e )
	{
		Output(_T("Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage());
	}
	catch(...)
	{
		Output(_T("Unexpected exception"));
	}

	return ok;
}

void InstallCallBack( JX_CLIENT_ CALLBACK_CLIENT_EVENT pfn )
{
	g_theManager.InstallCallBack( pfn );
}

void Client_Start()
{
	g_theJXClient.StartConnections();
}

void Client_Pause()
{
	g_theJXClient.StopConnections();
}

void Client_Begin()
{

}

void Client_End()
{

}

/*
 * Client usage
 */
inline void RecvFromServer( 
		JX_CLIENT_ 
		const char *pData, 
		size_t dataLength )
{
	g_theManager.RecvFromServer( pData, dataLength );
}

inline const char *GetFromServer( size_t &dataLength )
{
	return g_theManager.GetFromClient( dataLength );
}
 
inline void SendToServer( 
		JX_CLIENT_ 
		void *pData, 
		size_t dataLength )
{
	g_theJXClient.Write( reinterpret_cast<const char*>( pData ), dataLength );
}

void Shutdown( JX_CLIENT_ )
{
	//g_theJXClient.InitiateShutdown();
	g_theJXClient.StopConnections();
}

void ConnectServer( JX_CLIENT_ )
{
	g_theManager.ConnectServer();
}

void DisConnetServer( JX_CLIENT_ )
{
	g_theManager.DisConnetServer();
}