/********************************************************************
	created:	2003/05/11
	file base:	GameServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_GAMESERVER_H__
#define __INCLUDE_GAMESERVER_H__

#include "Buffer.h"

#include "IGServer.h"

/*
 * class CGameServer
 */
class CGameServer : public IGServer
{
public:
	
	CGameServer( UINT nIdentityID = ( UINT )( -1 ) );
	virtual ~CGameServer();

	virtual int  Run();
	
	virtual bool Create();
	virtual bool Destroy();

	static bool SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers );

private:

	static OnlineGameLib::Win32::CBuffer::Allocator	m_theGlobalAllocator;

	static LONG m_slnIdentityCounts;
	LONG		m_lnIdentityID;
	
};

#endif // __INCLUDE_GAMESERVER_H__