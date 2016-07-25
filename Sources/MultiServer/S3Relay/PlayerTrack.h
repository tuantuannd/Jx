// PlayerTrack.h: interface for the CPlayerTrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERTRACK_H__2E58B67D_CA53_47CA_979E_22BE551A1131__INCLUDED_)
#define AFX_PLAYERTRACK_H__2E58B67D_CA53_47CA_979E_22BE551A1131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <set>
#include "KThread.h"

class CPlayerTrack : K_CS 
{
public:
	CPlayerTrack();
	virtual ~CPlayerTrack();

private:
	BOOL Clearup();

public:
	BOOL Initialize();
	BOOL Uninitialize();

private:
	struct ACCVALUE
	{
		std::_tstring role;
		DWORD nameid;
		unsigned long conn;
		int index;
	};
	typedef std::map<std::_tstring, ACCVALUE> ACCMAP;
	ACCMAP m_mapAcc;

	/*
	struct ROLEVALUE
	{
		std::_tstring acc;
		DWORD nameid;
		unsigned long conn;
		int index;
	};
	typedef std::map<std::_tstring, ROLEVALUE> ROLEMAP;
	ROLEMAP m_mapRole;
	*/

	struct PLAYERVALUE
	{
		std::_tstring acc;
		std::_tstring role;
		DWORD nameid;
		int index;
		bool operator <(const PLAYERVALUE& src) const
		{ return index < src.index && nameid < src.nameid && role < src.role && acc < src.acc; }
	};
	typedef std::set<PLAYERVALUE> PLAYERSET;
	typedef std::map<unsigned long, PLAYERSET> CONNMAP;
	CONNMAP m_mapConn;

	K_CS m_csAccess;

public:
	BOOL Login(const std::_tstring& acc, const std::_tstring& role, DWORD nameid, unsigned long conn, int idx);
	BOOL Logout(const std::_tstring& acc);
	BOOL ClearOnSvr(unsigned long conn);
};

#endif // !defined(AFX_PLAYERTRACK_H__2E58B67D_CA53_47CA_979E_22BE551A1131__INCLUDED_)
