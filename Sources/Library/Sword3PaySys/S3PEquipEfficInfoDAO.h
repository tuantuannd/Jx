// S3PEquipEfficInfoDAO.h: interface for the S3PEquipEfficInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPEFFICINFODAO_H__D3E826CD_ACF9_47A4_92A2_79F29CEFB5CB__INCLUDED_)
#define AFX_S3PEQUIPEFFICINFODAO_H__D3E826CD_ACF9_47A4_92A2_79F29CEFB5CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PEquipEfficInfoDAO : public S3PTableDAO  
{
public:
	S3PEquipEfficInfoDAO(S3PDBConnection *pConn);
	virtual ~S3PEquipEfficInfoDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();
protected:
	S3PDBConnection * m_pConn;

};

#endif // !defined(AFX_S3PEQUIPEFFICINFODAO_H__D3E826CD_ACF9_47A4_92A2_79F29CEFB5CB__INCLUDED_)
