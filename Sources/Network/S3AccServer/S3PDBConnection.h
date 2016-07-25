//-----------------------------------------//
//                                         //
//  File		: S3PDBConnection.h        //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBCONNECTION_H_
#define _S3PDBCONNECTION_H_

//#pragma warning( disable : 4786 4800 4355 )

#include "KStdAfx.h"
#include <string>

#include <sqlplus.hh>
#include <custom.hh>

#include "S3PDBConVBC.h"
#include "S3PDBConnectionPool.h"

class S3PDB_mySQL_Connection : public S3PDBConVBC
{
public:
	S3PDB_mySQL_Connection();
	~S3PDB_mySQL_Connection();
	virtual bool OpenConnect(_LPDATABASEINFO) = 0;
	virtual void CloseConnect() = 0;
	virtual bool QuerySql(const char* lpszSql, S3PResultVBC** ppResult) = 0;
	virtual bool Do(const char* lpszSql);
	virtual std::string GetDate();
	virtual std::string GetDateTime();
	
protected:

	Result m_res;
	ResNSel m_res2;
	DWORD m_pRes;

	int m_iDBIdentifier;
	DWORD m_dwConID;
	Connection* m_pCon;
};

#endif