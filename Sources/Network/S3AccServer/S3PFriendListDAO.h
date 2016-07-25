// S3PFriendListDAO.h: interface for the S3PFriendListDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PFRIENDLISTDAO_H__F872A100_5845_455D_9048_C209492E2B28__INCLUDED_)
#define AFX_S3PFRIENDLISTDAO_H__F872A100_5845_455D_9048_C209492E2B28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PDBConnection;
class S3PFriendListDAO  : public S3PTableDAO  
{
public:
	S3PFriendListDAO(S3PDBConnection * pConn);
	virtual ~S3PFriendListDAO();
	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PFRIENDLISTDAO_H__F872A100_5845_455D_9048_C209492E2B28__INCLUDED_)
