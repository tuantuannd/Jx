// S3PCardHistoryDAO.h: interface for the S3PCardHistoryDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PCARDHISTORYDAO_H__EC8A684F_8228_48C2_BDF3_B176440E63EF__INCLUDED_)
#define AFX_S3PCARDHISTORYDAO_H__EC8A684F_8228_48C2_BDF3_B176440E63EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PCardHistoryDAO : public S3PTableDAO  
{
public:
	S3PCardHistoryDAO(S3PDBConnection * pConn);
	virtual ~S3PCardHistoryDAO();

	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PCARDHISTORYDAO_H__EC8A684F_8228_48C2_BDF3_B176440E63EF__INCLUDED_)
