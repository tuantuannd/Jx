#include "GameServerWorkerThread.h"

//[ Include in .\..\IPCPServer
#include "IOCPServer\Utils.h"
#include "IOCPServer\tstring.h"
#include "IOCPServer\Exception.h"
//]

#include "JXServer.h"

#pragma comment (lib, "IOCPServer.lib")
/*
 * Using directives
 */
using OnlineGameLib::Win32::CIOCompletionPort;
using OnlineGameLib::Win32::CIOBuffer;
using OnlineGameLib::Win32::CSocketServer;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::OutPutInfo;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
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
//	Output( _T("OnBeginProcessing") );
}

void CGameServerWorkerThread::ReadCompleted(
					CSocketServer::Socket *pSocket,
					CIOBuffer *pBuffer)
{
	try
	{
		unsigned long ulnID = pSocket->GetUserData();

		const BYTE *pPackData = pBuffer->GetBuffer();
		const size_t used = pBuffer->GetUsed();

		RecvFromClient( ulnID, reinterpret_cast<const char*>( pPackData ), used );

		pBuffer->Empty();
		
		pSocket->Read( pBuffer );
	}
	catch(const CException &e)
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
//	Output( _T("OnEndProcessing") );
}

void CGameServerWorkerThread::OnError( const OnlineGameLib::Win32::_tstring &message )
{
	Output( _T("OnError - ") + message );
}

