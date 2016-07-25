// S3PTableInfoCatch.h: interface for the S3PTableInfoCatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTABLEINFOCATCH_H__7E2D361E_5261_44DC_8BFE_65617E88796A__INCLUDED_)
#define AFX_S3PTABLEINFOCATCH_H__7E2D361E_5261_44DC_8BFE_65617E88796A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "KStdAfx.h"

#include <map>
#include <vector>
#include <string>
#include "S3PDBConnection.h"
#include "S3PDB_MSSQLServer_Connection.h"

//class S3PDBConnection;
class S3PTableInfoCatch  
{
protected:
	static S3PTableInfoCatch * m_pInstance;

	std::map<std::string,std::map<std::string, std::string> *> m_initedMap;
protected:
	S3PTableInfoCatch();
public:
	static S3PTableInfoCatch * Instance();
	static void Release();
	virtual ~S3PTableInfoCatch();

	std::map<std::string, std::string> * GetColumnInfo(std::string tableName, S3PDBConnection * pConn);
};

#endif // !defined(AFX_S3PTABLEINFOCATCH_H__7E2D361E_5261_44DC_8BFE_65617E88796A__INCLUDED_)
