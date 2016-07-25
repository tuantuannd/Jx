#include "GameServerWorkerThread.h"
//#ifdef def_MSSQLSERVER_DB
//[ Include in .\..\IPCPServer
#include "..\NetWork\IOCPServer\Utils.h"
#include "..\NetWork\IOCPServer\tstring.h"
#include "..\NetWork\IOCPServer\Exception.h"
//]

//[ Include in .\..\Protocol
#include "..\Protocol\Protocol.h"
//]

/*
 * Using directives
 */
using OnlineGameLib::Win32::CIOCompletionPort;
using OnlineGameLib::Win32::CIOBuffer;
using OnlineGameLib::Win32::CSocketServer;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::_tstring;
//using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::ToString;
using OnlineGameLib::Win32::DumpData;

CGameServerWorkerThread::CGameServerWorkerThread(
				 CIOCompletionPort &iocp)
			   : CSocketServer::WorkerThread(iocp)
{
}

CGameServerWorkerThread::~CGameServerWorkerThread()
{
}

void CGameServerWorkerThread::OnBeginProcessing()
{
	Output( _T("OnBeginProcessing") );
}

void CGameServerWorkerThread::ReadCompleted(
					CSocketServer::Socket *pSocket,
					CIOBuffer *pBuffer)
{
	try
	{
		pBuffer = ProcessDataStream( pSocket, pBuffer );
		
		pSocket->Read( pBuffer );
	}
	catch(const OnlineGameLib::Win32::CException &e)
	{
		Output( _T("ReadCompleted - Exception - ") + e.GetWhere() + _T(" - ") + e.GetMessage() );
		pSocket->Shutdown();
	}
	catch(...)
	{
		Output( _T("ReadCompleted - Unexpected exception") );
		pSocket->Shutdown();
	}
}

void CGameServerWorkerThread::OnEndProcessing()
{
	Output( _T("OnEndProcessing") );
}

void CGameServerWorkerThread::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

void CGameServerWorkerThread::ProcessCommand(
				   CSocketServer::Socket *pSocket,
				   const CIOBuffer *pBuffer) const
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
		
		// ==================== BEGIN =======================
		Output( _T("Package[length:") + ToString( wDataLen ) + _T("]\n") + DumpData( pData, datalength, 40 ) );
		
		S3PDBSocketPool::Process( pSocket, ( IBYTE* )pData, datalength );
		//douhao 采集socket层接收的数据
		
		// ====================  END  =======================
	}
	else
	{
		Output( "found error and close this socket!" );
		
		pSocket->Close();
	}
}

size_t CGameServerWorkerThread::GetMinimumMessageSize() const
{
	/*
	 * The smallest possible package we accept is pack-header
	 * once we have this many bytes we can start with try and work out
	 * what we have...
	 */
	
	return PACK_HEADER_LEN;
}

size_t CGameServerWorkerThread::GetMessageSize( const CIOBuffer *pBuffer ) const
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

CIOBuffer *CGameServerWorkerThread::ProcessDataStream(
							CSocketServer::Socket *pSocket,
							CIOBuffer *pBuffer) const
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
					Output( "found error and close this socket!" );
		
					pSocket->Close();
					
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
				
				ProcessCommand( pSocket, pBuffer );
				
				pBuffer->Empty();
				
				done = true;
			}
			else if ( used > messageSize )
			{
				Output( _T("Got message plus extra data") );
				/*
				 * we have a message, plus some more data
				 * 
				 * allocate a new buffer, copy the extra data into it and try again...
				 */
				
				CIOBuffer *pMessage = pBuffer->SplitBuffer( messageSize );
				
				pMessage->AddData(0);   // null terminate the command string;
				
				ProcessCommand( pSocket, pMessage );
				
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
//#endif