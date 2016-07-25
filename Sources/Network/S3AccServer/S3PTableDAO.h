// S3PTableDAO.h: interface for the S3PTableDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTABLEDAO_H__B9DCB685_90F6_459E_BE31_6DCD9B10D915__INCLUDED_)
#define AFX_S3PTABLEDAO_H__B9DCB685_90F6_459E_BE31_6DCD9B10D915__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include "S3P_MSSQLServer_TableDAO.h"
#ifdef def_MYSQL_DB
#include <string>
#include <sqlplus.hh>
#include "S3PResult.h"	// Added by ClassView
#include "S3PRow.h"
#include "S3PTableDAOVBC.h"
#include "S3PDBConnection.h"
#include "S3PDB_MSSQLServer_Connection.h"

//class S3PDBConnection;
class S3P_mySQL_TableDAO : S3PTableDAOVBC
{
public:
	static int Query(S3PDBConnection * pConn, string q, S3PResult & result);
	int AddGroup(const std::list<ColumnAndValue*> & group);
	int Query(std::string q, S3PResult & result);
	int Add(S3PRow * row);
	int Update(S3PRow * row, S3PRow * where = NULL);
	int Delete(S3PRow * where = NULL);
	bool HasItem( S3PRow* where );
	virtual std::string GetTableName() = 0;
	S3P_mySQL_TableDAO();
	virtual ~S3P_mySQL_TableDAO();
	int GetInsertID();
	virtual S3PDBConnection * GetConnection() = 0;

protected:
	ResNSel m_resAdd;
};
#endif
#endif // !defined(AFX_S3PTABLEDAO_H__B9DCB685_90F6_459E_BE31_6DCD9B10D915__INCLUDED_)
