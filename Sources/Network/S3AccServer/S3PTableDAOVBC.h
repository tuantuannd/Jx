// S3PTableDAOVBC.h: interface for the S3PTableDAOVBC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTABLEDAOVBC_H__423474B6_0A91_4B88_ACBD_E28E09C8F326__INCLUDED_)
#define AFX_S3PTABLEDAOVBC_H__423474B6_0A91_4B88_ACBD_E28E09C8F326__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include <string>
#include "S3PResult.h"	// Added by ClassView
#include "S3PRow.h"
#include "S3PDBConnection.h"
#include "S3PDB_MSSQLServer_Connection.h"

class S3PTableDAOVBC  
{
public:
	S3PTableDAOVBC();
	virtual ~S3PTableDAOVBC();
public:
	virtual int AddGroup(const std::list<ColumnAndValue*> & group) = 0;
	virtual int Query(std::string q, S3PResult & result) = 0;
	virtual int Add(S3PRow * row) = 0;
	virtual int Update(S3PRow * row, S3PRow * where = NULL) = 0;
	virtual int Delete(S3PRow * where = NULL) = 0;
	virtual bool HasItem( S3PRow* where ) = 0;
	virtual std::string GetTableName() = 0;
	virtual int GetInsertID() = 0;
	virtual S3PDBConnection * GetConnection() = 0;
};

#endif // !defined(AFX_S3PTABLEDAOVBC_H__423474B6_0A91_4B88_ACBD_E28E09C8F326__INCLUDED_)
