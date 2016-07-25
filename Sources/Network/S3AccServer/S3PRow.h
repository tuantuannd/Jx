// S3PRow.h: interface for the S3PRow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PROW_H__792EA5AF_C922_4DA3_838C_8F515F7CC3A0__INCLUDED_)
#define AFX_S3PROW_H__792EA5AF_C922_4DA3_838C_8F515F7CC3A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> ColumnAndValue;

class S3PDBConnection;

class S3PRow  
{
public:
	int GetExpLikeInsertValue(std::string & exp);
	int GetExpLikeInsertKey(std::string & exp);
	S3PRow(const std::string & tableName, ColumnAndValue * pCAV, S3PDBConnection * pConn);
	virtual ~S3PRow();

	int GetExpLikeWhereAnd(std::string & exp);
	int GetExpLikeUpdate(std::string & exp);
	int GetExpLikeInsert(std::string & exp);
protected:
	S3PRow(){};

	BOOL Init();

	BOOL IsNeedQuote(const std::string & column);
	int Quote(const std::string & value, std::string & outValue);
	int FixValue(const std::string & key, const std::string & value, std::string & outValue);

protected:
	S3PDBConnection * m_pConn;
	ColumnAndValue  * m_pCAV;
	std::string       m_tableName;

	std::map<std::string, std::string> * m_columnInfoMap;
};

#endif // !defined(AFX_S3PROW_H__792EA5AF_C922_4DA3_838C_8F515F7CC3A0__INCLUDED_)
