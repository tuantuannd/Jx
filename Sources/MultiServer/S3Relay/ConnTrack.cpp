// ConnTrack.cpp: implementation of the CConnTrack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ConnTrack.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConnTrack::CConnTrack()
{

}

CConnTrack::~CConnTrack()
{

}


BOOL CConnTrack::Initialize()
{
	Clearup();

	return TRUE;
}

BOOL CConnTrack::Uninitialize()
{
	return TRUE;
}

BOOL CConnTrack::Clearup()
{
	m_csAccess.Lock();

	m_mapFamily.clear();

	m_csAccess.Unlock();

	return TRUE;
}


BOOL CConnTrack::RegConnect(unsigned long conn, BYTE fami, BYTE meth)
{
	m_csAccess.Lock();

	METHODMAP& methods = m_mapFamily[fami];
	CONNSET& conns = methods[meth];

	conns.insert(conn);

	m_csAccess.Unlock();

	return TRUE;
}

BOOL CConnTrack::UnregConnect(unsigned long conn, BYTE fami, BYTE meth)
{
	m_csAccess.Lock();

	FAMILYMAP::iterator itFami = m_mapFamily.find(fami);
	if (itFami != m_mapFamily.end())
	{
		METHODMAP& methods = (*itFami).second;

		METHODMAP::iterator itMeth = methods.find(meth);
		if (itMeth != methods.end())
		{
			CONNSET& conns = (*itMeth).second;
			conns.erase(conn);
		}
	}

	m_csAccess.Unlock();

	return TRUE;
}

BOOL CConnTrack::ClearConnect(unsigned long conn)
{
	m_csAccess.Lock();

	for (FAMILYMAP::iterator itFami = m_mapFamily.begin(); itFami != m_mapFamily.end(); itFami++)
	{
		METHODMAP& methods = (*itFami).second;

		for (METHODMAP::iterator itMeth = methods.begin(); itMeth != methods.end(); itMeth++)
		{
			CONNSET& conns = (*itMeth).second;
			conns.erase(conn);
		}
	}

	m_csAccess.Unlock();

	return TRUE;
}


CConnTrack::CONNECTS CConnTrack::DupConns(BYTE fami, BYTE meth)
{
	CONNECTS connsRet;

	m_csAccess.Lock();

	FAMILYMAP::iterator itFami = m_mapFamily.find(fami);
	if (itFami != m_mapFamily.end())
	{
		METHODMAP& methods = (*itFami).second;

		METHODMAP::iterator itMeth = methods.find(meth);
		if (itMeth != methods.end())
		{
			CONNSET& conns = (*itMeth).second;

			connsRet.reserve(conns.size());	//<==ÓÅ»¯
			for (CONNSET::iterator itConn = conns.begin(); itConn != conns.end(); itConn++)
				connsRet.push_back((*itConn));
		}
	}

	m_csAccess.Unlock();

	return connsRet;
}

BOOL CConnTrack::IsConnAt(unsigned long conn, BYTE fami, BYTE meth)
{
	BOOL bFind = FALSE;

	m_csAccess.Lock();

	FAMILYMAP::iterator itFami = m_mapFamily.find(fami);
	if (itFami != m_mapFamily.end())
	{
		METHODMAP& methods = (*itFami).second;

		METHODMAP::iterator itMeth = methods.find(meth);
		if (itMeth != methods.end())
		{
			CONNSET& conns = (*itMeth).second;

			bFind = conns.find(conn) != conns.end();
		}
	}

	m_csAccess.Unlock();

	return bFind;
}

BOOL CConnTrack::IsConnOnFamily(unsigned long conn, BYTE fami)
{
	BOOL bFind = FALSE;

	m_csAccess.Lock();

	FAMILYMAP::iterator itFami = m_mapFamily.find(fami);
	if (itFami != m_mapFamily.end())
	{
		METHODMAP& methods = (*itFami).second;

		for (METHODMAP::iterator itMeth = methods.begin(); itMeth != methods.end(); itMeth++)
		{
			CONNSET& conns = (*itMeth).second;

			bFind = conns.find(conn) != conns.end();

			if (bFind)
				break;
		}
	}

	m_csAccess.Unlock();

	return bFind;
}

BOOL CConnTrack::IsConnOnMethod(unsigned long conn, BYTE meth)
{
	BOOL bFind = FALSE;

	m_csAccess.Lock();

	for (FAMILYMAP::iterator itFami = m_mapFamily.begin(); itFami != m_mapFamily.end(); itFami++)
	{
		METHODMAP& methods = (*itFami).second;

		METHODMAP::iterator itMeth = methods.find(meth);
		if (itMeth != methods.end())
		{
			CONNSET& conns = (*itMeth).second;

			bFind = conns.find(conn) != conns.end();

			if (bFind)
				break;
		}
	}

		
	m_csAccess.Unlock();

	return bFind;
}
