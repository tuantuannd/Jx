// PlayerTrack.cpp: implementation of the CPlayerTrack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlayerTrack.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerTrack::CPlayerTrack()
{

}

CPlayerTrack::~CPlayerTrack()
{

}


BOOL CPlayerTrack::Initialize()
{
	Clearup();

	return TRUE;
}

BOOL CPlayerTrack::Uninitialize()
{
	return FALSE;
}

BOOL CPlayerTrack::Clearup()
{
	m_csAccess.Lock();


	m_mapAcc.clear();
	//m_mapRole.clear();
	m_mapConn.clear();


	m_csAccess.Unlock();
	
	return TRUE;
}



BOOL CPlayerTrack::Login(const std::_tstring& acc, const std::_tstring& role, DWORD nameid, unsigned long conn, int idx)
{
	assert(!acc.empty() && !role.empty());

	m_csAccess.Lock();


	ACCVALUE accval;
	accval.role = role;
	accval.nameid = nameid;
	accval.conn = conn;
	accval.index = idx;
	m_mapAcc.insert(ACCMAP::value_type(acc, accval));

	/*
	ROLEVALUE roleval;
	roleval.acc = acc;
	roleval.nameid = nameid;
	roleval.conn = conn;
	roleval.index = idx;
	m_mapRole.insert(ROLEMAP::value_type(role, roleval));
	*/

	PLAYERSET& rPlys = m_mapConn[conn];
	PLAYERVALUE plyval;
	plyval.acc = acc;
	plyval.role = role;
	plyval.nameid = nameid;
	plyval.index = idx;
	rPlys.insert(plyval);


	m_csAccess.Unlock();

	return TRUE;
}

BOOL CPlayerTrack::Logout(const std::_tstring& acc)
{
	assert(!acc.empty());

	m_csAccess.Lock();


	ACCMAP::iterator itAcc = m_mapAcc.find(acc);
	if (itAcc != m_mapAcc.end())
	{
		ACCVALUE accval = (*itAcc).second;

		m_mapAcc.erase(itAcc);
		//m_mapRole.erase(accval.role);


		CONNMAP::iterator itConn = m_mapConn.find(accval.conn);
		if (itConn != m_mapConn.end())
		{
			PLAYERSET& rPlys = (*itConn).second;

			PLAYERVALUE plyval;
			plyval.acc = acc;
			plyval.role = accval.role;
			plyval.nameid = accval.nameid;
			plyval.index = accval.index;

			rPlys.erase(plyval);

			//erase rPlys ???
			if (rPlys.empty())
				m_mapConn.erase(itConn);
		}
	}


	m_csAccess.Unlock();

	return TRUE;
}

BOOL CPlayerTrack::ClearOnSvr(unsigned long conn)
{
	m_csAccess.Lock();


	CONNMAP::iterator itConn = m_mapConn.find(conn);
	if (itConn != m_mapConn.end())
	{
		PLAYERSET& rPlys = (*itConn).second;

		for (PLAYERSET::iterator it = rPlys.begin(); it != rPlys.end(); it++)
		{
			m_mapAcc.erase((*it).acc);
			//m_mapRole.erase((*it).role);
		}
		m_mapConn.erase(itConn);
	}


	m_csAccess.Unlock();

	return TRUE;
}

