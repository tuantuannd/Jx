// S3P_MSSQLSERVER_TableDAO.h: interface for the S3P_MSSQLSERVER_TableDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3P_MSSQLSERVER_TABLEDAO_H__398CBE34_C342_4DB1_BF74_CA06EA3610ED__INCLUDED_)
#define AFX_S3P_MSSQLSERVER_TABLEDAO_H__398CBE34_C342_4DB1_BF74_CA06EA3610ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAOVBC.h"

#ifdef def_MSSQLSERVER_DB
class S3P_MSSQLServer_TableDAO : public S3PTableDAOVBC  
{
public:
	S3P_MSSQLServer_TableDAO();
	virtual ~S3P_MSSQLServer_TableDAO();
public:
	static int Query(S3PDBConnection * pConn, string q, S3PResult & result);
	virtual int AddGroup(const std::list<ColumnAndValue*> & group);
	virtual int Query(std::string q, S3PResult & result);
	virtual int Add(S3PRow * row);
	virtual int Update(S3PRow * row, S3PRow * where = NULL);
	virtual int Delete(S3PRow * where = NULL);
	virtual bool HasItem( S3PRow* where );
	virtual std::string GetTableName() = 0;
	virtual int GetInsertID();
	virtual S3PDBConnection * GetConnection() = 0;
protected:
	_RecordsetPtr m_resAdd;
};
#endif

#endif // !defined(AFX_S3P_MSSQLSERVER_TABLEDAO_H__398CBE34_C342_4DB1_BF74_CA06EA3610ED__INCLUDED_)
