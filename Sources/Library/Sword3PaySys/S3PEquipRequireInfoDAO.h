// S3PEquipRequireInfoDAO.h: interface for the S3PEquipRequireInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPREQUIREINFODAO_H__E6BBDD4E_9599_492C_AB18_F4D121799978__INCLUDED_)
#define AFX_S3PEQUIPREQUIREINFODAO_H__E6BBDD4E_9599_492C_AB18_F4D121799978__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PEquipRequireInfoDAO : public S3PTableDAO  
{
public:
	S3PEquipRequireInfoDAO(S3PDBConnection * pConn);
	virtual ~S3PEquipRequireInfoDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();
protected:
	S3PDBConnection * m_pConn;

};

#endif // !defined(AFX_S3PEQUIPREQUIREINFODAO_H__E6BBDD4E_9599_492C_AB18_F4D121799978__INCLUDED_)
