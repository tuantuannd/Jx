// S3PEquipBaseInfoDAO.h: interface for the S3PEquipBaseInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPBASEINFODAO_H__38806389_B52E_478B_B60D_489B159B0EAB__INCLUDED_)
#define AFX_S3PEQUIPBASEINFODAO_H__38806389_B52E_478B_B60D_489B159B0EAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PEquipBaseInfoDAO : public S3PTableDAO  
{
public:
	S3PEquipBaseInfoDAO(S3PDBConnection *pConn);
	virtual ~S3PEquipBaseInfoDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();
protected:
	S3PDBConnection * m_pConn;

};

#endif // !defined(AFX_S3PEQUIPBASEINFODAO_H__38806389_B52E_478B_B60D_489B159B0EAB__INCLUDED_)
