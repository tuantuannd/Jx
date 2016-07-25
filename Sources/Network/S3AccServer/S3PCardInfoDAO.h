// S3PCardInfoDAO.h: interface for the S3PCardInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PCARDINFODAO_H__7B421D69_1E40_495D_96F9_8789944EDBEA__INCLUDED_)
#define AFX_S3PCARDINFODAO_H__7B421D69_1E40_495D_96F9_8789944EDBEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"
#include "S3PResult.h"

class S3PCardInfoDAO : public S3PTableDAO  
{
public:
	S3PCardInfoDAO(S3PDBConnection * pConn);
	virtual ~S3PCardInfoDAO();

	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();

protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PCARDINFODAO_H__7B421D69_1E40_495D_96F9_8789944EDBEA__INCLUDED_)
