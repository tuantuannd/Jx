#include "Stdafx.h"
#include "GameServer.h"

OnlineGameLib::Win32::CBuffer::Allocator CGameServer::m_theGlobalAllocator( 1024 * 64, 20 );

/*
 * CGamePlayer Global Function
 */
bool CGameServer::SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers )
{
	return CGameServer::m_theGlobalAllocator.ReSet( bufferSize, maxFreeBuffers );
}

LONG CGameServer::m_slnIdentityCounts = 0L;

/*
 * class CGameServer
 */
CGameServer::CGameServer( UINT nIdentityID /* = -1 */ )
				: m_lnIdentityID( nIdentityID )
{
	LONG lnID = ::InterlockedExchangeAdd( &m_slnIdentityCounts, 1 );

	m_lnIdentityID = ( ( UINT )( -1 ) == m_lnIdentityID ) ? lnID : m_lnIdentityID;
}

CGameServer::~CGameServer()
{
	::InterlockedExchangeAdd( &m_slnIdentityCounts, -1 );
}

int CGameServer::Run()
{
	
	return 0;
}

bool CGameServer::Create()
{
	return true;
}

bool CGameServer::Destroy()
{
	return true;
}