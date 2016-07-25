/********************************************************************
	created:	2003/02/18
	file base:	GameClient
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_GAMECLIENT_H__
#define __INCLUDE_GAMECLIENT_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

//[ Include in .\..\ESClient
#include "ESClient\SocketClient.h"
#include "ESClient\tstring.h"
//]


class CGameClient : public OnlineGameLib::Win32::CSocketClient
{
public:

	CGameClient(
		const OnlineGameLib::Win32::_tstring &addressToConnectServer,
		unsigned short portToConnectServer,
		size_t maxFreeBuffers,
		size_t bufferSize = 1024
		);

	CGameClient(
		size_t maxFreeBuffers,
		size_t bufferSize = 1024
		);	
	
	virtual ~CGameClient();

private:

	virtual SOCKET CreateConnectionSocket( 
			const OnlineGameLib::Win32::_tstring &addressToConnectServer,
			unsigned short port);

	virtual void OnStartConnections();
	virtual void OnStopConnections();
	virtual void OnShutdownInitiated();
	virtual void OnShutdownComplete();
	
	virtual void OnConnect();
	virtual void OnClose();
	virtual void ReadCompleted( OnlineGameLib::Win32::CIOBuffer *pBuffer );

	virtual void OnError( const OnlineGameLib::Win32::_tstring &message );
	
	virtual void PreWrite( 
			OnlineGameLib::Win32::CIOBuffer *pBuffer, 
			const char *pData, 
			size_t dataLength );

	// No copies do not implement
	CGameClient( const CGameClient &rhs );
	CGameClient &operator=( const CGameClient &rhs );

};

#endif //__INCLUDE_GAMECLIENT_H__