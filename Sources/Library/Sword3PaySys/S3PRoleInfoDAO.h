// S3PRoleInfoDAO.h: interface for the S3PRoleInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PROLEINFODAO_H__271F83AD_B7C0_4F8C_9A6A_18D9B5FF943C__INCLUDED_)
#define AFX_S3PROLEINFODAO_H__271F83AD_B7C0_4F8C_9A6A_18D9B5FF943C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PRoleInfoDAO : public S3PTableDAO  
{
public:
	S3PRoleInfoDAO(S3PDBConnection * pConn);
	virtual ~S3PRoleInfoDAO();
	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PROLEINFODAO_H__271F83AD_B7C0_4F8C_9A6A_18D9B5FF943C__INCLUDED_)
