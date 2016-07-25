// S3PTaskListDAO.h: interface for the S3PTaskListDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTASKLISTDAO_H__9893DA02_9545_4D1B_B83C_2C7F79B295D4__INCLUDED_)
#define AFX_S3PTASKLISTDAO_H__9893DA02_9545_4D1B_B83C_2C7F79B295D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"
class S3PDBConnection;

class S3PTaskListDAO : public S3PTableDAO  
{
public:
	S3PTaskListDAO(S3PDBConnection * pConn);
	virtual ~S3PTaskListDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();

protected:
	S3PDBConnection *m_pConn;
};

#endif // !defined(AFX_S3PTASKLISTDAO_H__9893DA02_9545_4D1B_B83C_2C7F79B295D4__INCLUDED_)
