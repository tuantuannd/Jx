/********************************************************************
	created:	2003/02/15
	file base:	GameServerWorkerThread
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_ONLINEGAME_GAMESERVERWORKERTHREAD_H__
#define __INCLUDE_ONLINEGAME_GAMESERVERWORKERTHREAD_H__
//#ifdef def_MSSQLSERVER_DB
#include "..\KStdAfx.h"
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include "..\S3PDBSocketPool.h"
#include "..\NetWork\IOCPServer\SocketServer.h"

/*
 * Classes defined in other files...
 */
namespace OnlineGameLib {
namespace Win32 {

//[ Include in .\..\IPCPServer
class CIOBuffer;
//]

} // End of namespace OnlineGameLib
} // End of namespace Win32


class CGameServerWorkerThread : public OnlineGameLib::Win32::CSocketServer::WorkerThread
{
public:
	
	explicit CGameServerWorkerThread( OnlineGameLib::Win32::CIOCompletionPort &iocp );
	
	virtual ~CGameServerWorkerThread();
	
private:
	
	virtual void ReadCompleted(
				OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
				OnlineGameLib::Win32::CIOBuffer *pBuffer);
	
	virtual void OnBeginProcessing();	
	virtual void OnEndProcessing();
	
	virtual void OnError( const OnlineGameLib::Win32::_tstring &message );
	
	size_t GetMinimumMessageSize() const;	
	size_t GetMessageSize( const OnlineGameLib::Win32::CIOBuffer *pBuffer ) const;
	
	OnlineGameLib::Win32::CIOBuffer *ProcessDataStream(
				OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
				OnlineGameLib::Win32::CIOBuffer *pBuffer) const;
	
	void ProcessCommand(
				OnlineGameLib::Win32::CSocketServer::Socket *pSocket,
				const OnlineGameLib::Win32::CIOBuffer *pBuffer) const;
	
	/*
	 * No copies do not implement
	 */
	CGameServerWorkerThread( const CGameServerWorkerThread &rhs );
	CGameServerWorkerThread &operator=( const CGameServerWorkerThread &rhs ); 
};
//#endif
#endif //__INCLUDE_ONLINEGAME_GAMESERVERWORKERTHREAD_H__