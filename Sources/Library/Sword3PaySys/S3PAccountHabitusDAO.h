//-----------------------------------------//
//                                         //
//  File		: S3PAccountHabitusDAO.h   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PACCOUNTHABITUSDAO_H__7F307DA2_7B76_4592_BA36_79857EE425D0__INCLUDED_)
#define AFX_S3PACCOUNTHABITUSDAO_H__7F307DA2_7B76_4592_BA36_79857EE425D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PAccountHabitusDAO : public S3PTableDAO  
{
public:
	S3PAccountHabitusDAO( S3PDBConnection* pConn );
	virtual ~S3PAccountHabitusDAO();

	virtual std::string GetTableName();
	virtual S3PDBConnection* GetConnection();
protected:
	S3PDBConnection* m_pConn;
};

#endif // !defined(AFX_S3PACCOUNTHABITUSDAO_H__7F307DA2_7B76_4592_BA36_79857EE425D0__INCLUDED_)
