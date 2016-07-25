// S3PDB_MSSQLServer_Connection.h: interface for the S3PDB_MSSQLServer_Connection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PDB_MSSQLSERVER_CONNECTION_H__BFEECC7C_6B4A_466E_80BF_26AD4F94E506__INCLUDED_)
#define AFX_S3PDB_MSSQLSERVER_CONNECTION_H__BFEECC7C_6B4A_466E_80BF_26AD4F94E506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include <string>
#include "S3PDBConVBC.h"
#include "S3P_MSSQLServer_Result.h"

#include "msado15.tlh"

class S3PDB_MSSQLServer_Connection : public S3PDBConVBC  
{
public:
	S3PDB_MSSQLServer_Connection();
	~S3PDB_MSSQLServer_Connection();
	virtual bool OpenConnect(_LPDATABASEINFO);
	virtual void CloseConnect();
	virtual bool QuerySql(const char* lpszSql, S3PResultVBC** ppResult);
	virtual bool Do(const char* lpszSql);
	
	
protected:
	_ConnectionPtr m_pCon;
	_CommandPtr m_pCmd;
	S3P_MSSQLServer_Result m_Result;
	S3P_MSSQLServer_Result m_Result2;
	bool GetFreeResult(S3P_MSSQLServer_Result** ppResult);

	bool QueryResult(const char* lpszSql, S3P_MSSQLServer_Result* pRes);
	bool DoResult(const char* lpszSql);

	bool QueryCmd(const char* lpszSql, S3P_MSSQLServer_Result* pRes);
	bool DoCmd(const char* lpszSql);
};

#endif // !defined(AFX_S3PDB_MSSQLSERVER_CONNECTION_H__BFEECC7C_6B4A_466E_80BF_26AD4F94E506__INCLUDED_)
