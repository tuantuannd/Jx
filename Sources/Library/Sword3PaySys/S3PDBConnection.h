//-----------------------------------------//
//                                         //
//  File		: S3PDBConnection.h        //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBCONNECTION_H_
#define _S3PDBCONNECTION_H_

#pragma warning( disable : 4786 4800 4355 )

#include "KStdAfx.h"
#include <string>
#include <sqlplus.hh>
#include <custom.hh>
#include "S3PDBConnectionPool.h"

class S3PDBConnection
{
	friend class S3PDBConnector;
	typedef struct tag_S3PDBCONINFO
	{
		int iDBIdentifier;
		DWORD dwConID;
		Connection* pCon;
	}_S3PDBCONINFO, *_LPS3PDBCONINFO;
public:
	virtual BOOL Connect( int iDBIdentifier );
	virtual BOOL QueryBySql( LPCTSTR lpszSql );
	virtual BOOL Do( LPCTSTR lpszSql );
	virtual DWORD GetRes();
	virtual void Close();
	virtual std::string GetDate();
	virtual std::string GetDateTime();
	~S3PDBConnection();
	
protected:
	S3PDBConnection();

	virtual void Init();
	Result m_res;
	ResNSel m_res2;
	DWORD m_pRes;

	_S3PDBCONINFO m_info;
};

#endif