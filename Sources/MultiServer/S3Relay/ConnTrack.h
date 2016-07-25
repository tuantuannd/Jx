// ConnTrack.h: interface for the CConnTrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNTRACK_H__26F8DBDF_E103_4050_8302_078018B7A4C8__INCLUDED_)
#define AFX_CONNTRACK_H__26F8DBDF_E103_4050_8302_078018B7A4C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <set>
#include <map>
#include <vector>
#include "KThread.h"

class CConnTrack  
{
public:
	CConnTrack();
	virtual ~CConnTrack();

private:
	BOOL Clearup();

public:
	BOOL Initialize();
	BOOL Uninitialize();

public:
	BOOL RegConnect(unsigned long conn, BYTE fami, BYTE meth);
	BOOL UnregConnect(unsigned long conn, BYTE fami, BYTE meth);
	BOOL ClearConnect(unsigned long conn);

private:
	typedef std::set<unsigned long> CONNSET;
	typedef std::map<BYTE, CONNSET> METHODMAP;
	typedef std::map<BYTE, METHODMAP> FAMILYMAP;

	FAMILYMAP m_mapFamily;

	K_CS m_csAccess;

public:
	typedef std::vector<unsigned long>	CONNECTS;

	CONNECTS DupConns(BYTE fami, BYTE meth);
	BOOL IsConnAt(unsigned long conn, BYTE fami, BYTE meth);
	BOOL IsConnOnFamily(unsigned long conn, BYTE fami);
	BOOL IsConnOnMethod(unsigned long conn, BYTE meth);
};

#endif // !defined(AFX_CONNTRACK_H__26F8DBDF_E103_4050_8302_078018B7A4C8__INCLUDED_)
