#include "jxserver.h"

#include <winsock2.h>
//#include "kengine.h"
//#include "kdebug.h"

//[ Include in .\..\IPCPServer
#include "IOCPServer\Exception.h"
#include "IOCPServer\Utils.h"
#include "IOCPServer\ManualResetEvent.h"
//]

#include "Protocol.h"

#include "GameServer.h"

using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CManualResetEvent;
using OnlineGameLib::Win32::GetTimeStamp;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::CCriticalSection;
using OnlineGameLib::Win32::DumpData;
using OnlineGameLib::Win32::Trace2File;

//[
#ifdef  NETWORK_DEBUG
static DWORD gs_dwPackID = 0;
#endif
//

/*
 * Server
 */

/*
 * The still variable better than dynamic variable, because of don't to think that
 * it can call through to unqualified virtual function
 */

static CGameServer g_theJXServer( MAX_SOCKET_KEEPINPOOL, MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE );

static CClientManager g_theClientManager( MAX_BUFFER_KEEPINPOOL, EXTEND_BUFFER_SIZE );

/*
 * Function
 */
bool ServerStartup( 
		JX_SERVER_
		unsigned long ulnAddressToListenOn,
		unsigned short usnPortToListenOn,
		bool bStartAtOnce
		)
{
	bool ok = false;

	try
	{
		g_theJXServer.Open( ulnAddressToListenOn, usnPortToListenOn );
		
		if ( bStartAtOnce )
		{
			g_theJXServer.StartAcceptingConnections();
		}

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

void ServerCleanup( JX_SERVER_ )
{
	try
	{		
		g_theJXServer.StopAcceptingConnections();
		
		g_theJXServer.WaitForShutdownToComplete();
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

void InstallCallBack( JX_SERVER_ CALLBACK_SERVER_EVENT pfn )
{
	g_theClientManager.InstallCallBack( pfn );
}

void Server_Start()
{
	g_theJXServer.StartAcceptingConnections();
}

void Server_Pause()
{
	g_theJXServer.StopAcceptingConnections();
}

void Server_Begin()
{
	g_theClientManager.PrepareToSend();
}

void Server_End()
{
	g_theClientManager.BeginToSend();
}

inline void SendToClient(
		JX_SERVER_ 
		const unsigned long ulnID, 
		void *pData,
		size_t dataLength )
{
	g_theClientManager.SendToClient( 
		ulnID,
		reinterpret_cast<const char*>( pData ), 
		dataLength );
}

void ShutdownClient(
		JX_SERVER_ 
		const unsigned long ulnID )
{
	g_theClientManager.ShutdownClient( ulnID );
}

CClientManager::CClientManager(
		size_t maxFreeBuffers,
		size_t bufferSize /* = 1024 */)
	: OnlineGameLib::Win32::CIOBuffer::Allocator(bufferSize, maxFreeBuffers),
	m_pfnCallBackServerEvent( NULL )
{
	int i=0;

	for ( i=0; i<MAX_CLIENT_CANBELINKED; i++ )
	{
		m_freeClientStack.push( i );
	}

	for ( i=0; i<MAX_CLIENT_CANBELINKED; i++ )
	{
		m_clientContext[i].pSocket = NULL;

		m_clientContext[i].pRecvBuffer = Allocate();
		m_clientContext[i].pReadBuffer = Allocate();

		m_clientContext[i].pWriteBuffer = Allocate();
	}
}

CClientManager::~CClientManager()
{
	CCriticalSection::Owner lock( m_csAction );

	/*
     * Repeat until stack is empty
	 */
	while ( !m_freeClientStack.empty() )
	{
		m_freeClientStack.pop();
	}

	m_usedClientStack.erase( m_usedClientStack.begin(), m_usedClientStack.end() );

	for ( int i=0; i<MAX_CLIENT_CANBELINKED; i++ )
	{
		m_clientContext[i].pReadBuffer->Release();
		m_clientContext[i].pRecvBuffer->Release();

		m_clientContext[i].pWriteBuffer->Release();
	}

	try
	{
		Flush();
	}
	catch(...)
	{
	}
}

unsigned long CClientManager::AddClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
		OnlineGameLib::Win32::CIOBuffer *pAddress )
{
	CCriticalSection::Owner lock( m_csAction );

	unsigned long index = -1;

	if ( !m_freeClientStack.empty() && pSocket )
	{
		/*
		 * Get ID from the free table
		 */
		index = m_freeClientStack.top();
		m_freeClientStack.pop();

		/*
		 * Set ID into the used table
		 */
		m_usedClientStack.push_back( index );

		/*
		 * Store information
		 */
		pSocket->SetUserData( index );
		m_clientContext[index].pSocket = pSocket;

		if ( pAddress )
		{
			m_clientContext[index].SocketAddrIn.SetAddr( 
						reinterpret_cast< SOCKADDR_IN * >( pAddress ) );
		}
		
		if ( m_pfnCallBackServerEvent )
		{
			m_pfnCallBackServerEvent( index, CLIENT_CONNECT_CREATE );
		}

		Output( GetTimeStamp() + _T("Add a client [ID:") + 
			ToString( index ) + _T( "] current total is " ) +
			ToString( m_usedClientStack.size() )
			);
	}

	return index;
}

unsigned long CClientManager::DelClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket )
{
	CCriticalSection::Owner lock( m_csAction );

	unsigned long index = -1;

	if ( !m_usedClientStack.empty() && pSocket )
	{
		index = pSocket->GetUserData();
		m_clientContext[index].pSocket = NULL;

		m_clientContext[index].SocketAddrIn.Clear();

		m_clientContext[index].pReadBuffer->Empty();
		m_clientContext[index].pRecvBuffer->Empty();
		m_clientContext[index].pWriteBuffer->Empty();

		/*
		 * Call back ID into the free table
		 */
		m_freeClientStack.push( index );
		
		/*
		 * Erase ID from the used table
		 */
		m_usedClientStack.remove( index );

		if ( m_pfnCallBackServerEvent )
		{
			m_pfnCallBackServerEvent( index, CLIENT_CONNECT_CLOSE );
		}

		Output( GetTimeStamp() + _T("Del a client [ID:") + 
			ToString( index ) + _T( "] current total is " ) + 
			ToString( m_usedClientStack.size() ) );
	}

	return index;
}

void CClientManager::SendToClient(
		const unsigned long ulnID, 
		const char *pData,
		size_t dataLength )
{
	if ( ulnID >= 0 && ulnID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
			m_clientContext[ulnID].pSocket;
		if ( pSocket )
		{
			CCriticalSection::Owner locker( m_clientContext[ulnID].csWriteAction );

			try
			{
				m_clientContext[ulnID].pWriteBuffer->AddData( pData, dataLength );
			}
			catch(const CException &e)
			{
				Output(_T("Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage());
				pSocket->Shutdown();
			}
			catch(...)
			{
				Output(_T("CClientManager::SendToClient - Unexpected exception"));
				pSocket->Shutdown();
			}
		}
	}
}

void CClientManager::RecvFromClient(
		   const unsigned long ulnID, 
		   const char *pData,
		   size_t dataLength )
{
	if ( ulnID >= 0 && ulnID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
			m_clientContext[ulnID].pSocket;
		if ( pSocket )
		{
			CCriticalSection::Owner	locker( m_clientContext[ulnID].csReadAction );

			m_clientContext[ulnID].pRecvBuffer->AddData( pData, dataLength );			
		}
	}	
}

const char *CClientManager::GetFromClient(
	const unsigned long ulnID, 
	size_t &dataLength )
{
	if ( ulnID >= 0 && ulnID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
			m_clientContext[ulnID].pSocket;
		if ( pSocket )
		{
			CCriticalSection::Owner	locker( m_clientContext[ulnID].csReadAction );
			
			m_clientContext[ulnID].pReadBuffer->Empty();

			m_clientContext[ulnID].pRecvBuffer = ProcessDataStream(
				ulnID,
				m_clientContext[ulnID].pRecvBuffer );

			const BYTE *pPackData = m_clientContext[ulnID].pReadBuffer->GetBuffer();
			dataLength = m_clientContext[ulnID].pReadBuffer->GetUsed();
			 
			return reinterpret_cast<const char*>( pPackData );
		}
	}

	dataLength = 0;
	return NULL;
}

void CClientManager::ShutdownClient( const unsigned long ulnID )
{
	if ( ulnID >= 0 && ulnID < MAX_CLIENT_CANBELINKED )
	{
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket = 
			m_clientContext[ulnID].pSocket;
		if ( pSocket )
		{
			pSocket->Close();
			pSocket = NULL;

			m_clientContext[ulnID].pReadBuffer->Empty();
			m_clientContext[ulnID].pRecvBuffer->Empty();

			m_clientContext[ulnID].pWriteBuffer->Empty();
		}
	}
}

void CClientManager::PrepareToSend()
{
	//CCriticalSection::Owner lock( m_csAction );
}

void CClientManager::BeginToSend()
{
	CCriticalSection::Owner lock( m_csAction );

	for ( LIST::iterator i = m_usedClientStack.begin(); 
			i != m_usedClientStack.end();
			i ++ )
	{
		unsigned long ulnID = *i;

		CCriticalSection::Owner locker( m_clientContext[ulnID].csWriteAction );
		
		const BYTE *pPackData = m_clientContext[ulnID].pWriteBuffer->GetBuffer();
		const size_t used = m_clientContext[ulnID].pWriteBuffer->GetUsed();

		if ( used > 0 )
		{
		//[
#ifdef  NETWORK_DEBUG
//{
		// sizeof(DWORD) == 4
		static const DWORD sc_dwFlag = 0xABCD0000; // End of stream
		static const int sc_nFlagLen = sizeof(DWORD) * 2;

		DWORD dwEOSFlag[2];
		dwEOSFlag[0] = gs_dwPackID ++;
		dwEOSFlag[1] = sc_dwFlag | ( 0xFFFF & used );

		m_clientContext[ulnID].pWriteBuffer->AddData( reinterpret_cast<const char*>( &dwEOSFlag ), sc_nFlagLen );

		m_clientContext[ulnID].pSocket->Write( reinterpret_cast<const char*>( pPackData ), used + sc_nFlagLen );
	
/*	#define NETWORK_DEBUG_TRACE2FILE
	#undef NETWORK_DEBUG_TRACE2FILE	
	#ifdef NETWORK_DEBUG_TRACE2FILE
	//{
		Trace2File( _T("Package[ID:") + ToString( gs_dwPackID - 1 ) + _T("]\n") + DumpData( pPackData, used, 40 ) );
	//}
	#else
	//{

		g_DebugLog( "[NETWORK] NO.[ID:%d]Pack - Len:%d", gs_dwPackID - 1, used );
	//}
	#endif // NETWORK_DEBUG_TRACE_MININFO
	*/
//}
#else	// NETWORK_DEBUG
//{
		m_clientContext[ulnID].pSocket->Write( reinterpret_cast<const char*>( pPackData ), used );
//}
#endif  // NETWORK_DEBUG
		//]

/*			Output(
				_T("ID:") + 
				ToString(ulnID) + 
				_T(" - Package[length:") + 
				ToString( used ) + 
				_T("]\n") + 
				DumpData( pPackData, used, 40 ) 
				);*/
		}

		m_clientContext[ulnID].pWriteBuffer->Empty();
	}
}

size_t CClientManager::GetClientCount()
{
	CCriticalSection::Owner lock( m_csAction );

	return m_usedClientStack.size();
}

ULONG CClientManager::GetClientAddrIP( const unsigned long &ulnID )
{
	CCriticalSection::Owner lock( m_csAction );
	
	if ( ulnID >= 0 && ulnID < MAX_CLIENT_CANBELINKED )
	{
		if ( !m_clientContext[ulnID].SocketAddrIn.IsNull() )
		{
			return m_clientContext[ulnID].SocketAddrIn.GetIPAddr();
		}
	}

	return ( ULONG )( -1 );
}

inline size_t GetClientCount()
{
	return g_theClientManager.GetClientCount();
}

inline ULONG GetClientAddrIP( const unsigned long &ulnID )
{
	return 	g_theClientManager.GetClientAddrIP( ulnID );
}

inline unsigned long AddClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
		OnlineGameLib::Win32::CIOBuffer *pAddress )
{
	return g_theClientManager.AddClient( pSocket, pAddress );
}
inline unsigned long DelClient( 
		OnlineGameLib::Win32::CSocketServer::Socket *pSocket )
{
	return g_theClientManager.DelClient( pSocket );
}

void CClientManager::ProcessCommand(
				   const unsigned long ulnID,
				   const OnlineGameLib::Win32::CIOBuffer *pBuffer) const
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
		 * TODO : Process data receive from client
		 */
		
		//Output( _T("Get a package[length:") + ToString( wDataLen ) + _T("]\n") + DumpData( pData, datalength, 40 ) );

		m_clientContext[ulnID].pReadBuffer->AddData( pData, datalength );				
	}
	else
	{
		Output( "found error and close this socket!" );
		
		m_clientContext[ulnID].pSocket->Close();
	}
}

size_t CClientManager::GetMinimumMessageSize() const
{
	/*
	 * The smallest possible package we accept is pack-header
	 * once we have this many bytes we can start with try and work out
	 * what we have...
	 */
	
	return PACK_HEADER_LEN;
}

size_t CClientManager::GetMessageSize(
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

OnlineGameLib::Win32::CIOBuffer *CClientManager::ProcessDataStream(
				const unsigned long ulnID,
				OnlineGameLib::Win32::CIOBuffer *pBuffer) const
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
				Output( "found error and close this socket!" );
	
				m_clientContext[ulnID].pSocket->Close();
				
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
			ProcessCommand( ulnID, pBuffer );
			
			pBuffer->Empty();
		}
		else if ( used > messageSize )
		{
			Output( _T("Got message plus extra data") );
			/*
			 * we have a message, plus some more data
			 * 
			 * allocate a new buffer, copy the extra data into it and try again...
			 */				
			OnlineGameLib::Win32::CIOBuffer *pMessage = 
				pBuffer->SplitBuffer( messageSize );
			
			ProcessCommand( ulnID, pMessage );
			
			pMessage->Release();			

		}
	}

	/*
	 * Reissue a read into the same buffer to collect more data
	 */
	return pBuffer;
}

inline void RecvFromClient( 
		JX_SERVER_ 
		const unsigned long ulnID,
		const char *pData, 
		size_t dataLength )
{
	g_theClientManager.RecvFromClient( ulnID, pData, dataLength );
}

inline const char *GetFromClient(
	const unsigned long ulnID, 
	size_t &dataLength )
{
	return g_theClientManager.GetFromClient( ulnID, dataLength );
}