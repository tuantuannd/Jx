// S3PEquipmentsDAO.h: interface for the S3PEquipmentsDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPMENTSDAO_H__1204DE95_4AD5_4714_8E68_0D332E031BD0__INCLUDED_)
#define AFX_S3PEQUIPMENTSDAO_H__1204DE95_4AD5_4714_8E68_0D332E031BD0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PEquipmentsDAO : public S3PTableDAO  
{
public:
	S3PEquipmentsDAO(S3PDBConnection * pConn);
	virtual ~S3PEquipmentsDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();
protected:
	S3PDBConnection * m_pConn;

};

#endif // !defined(AFX_S3PEQUIPMENTSDAO_H__1204DE95_4AD5_4714_8E68_0D332E031BD0__INCLUDED_)
