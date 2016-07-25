//-----------------------------------------//
//                                         //
//  File		: S3PGamerIDDAO.h		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 3/21/2003                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PGAMERIDDAO_H__D8CEE543_EFF8_480A_B434_C7EC8A352534__INCLUDED_)
#define AFX_S3PGAMERIDDAO_H__D8CEE543_EFF8_480A_B434_C7EC8A352534__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PGamerIDDAO : public S3PTableDAO  
{
public:
	S3PGamerIDDAO( S3PDBConnection * pConn );
	virtual ~S3PGamerIDDAO();
	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PGAMERIDDAO_H__D8CEE543_EFF8_480A_B434_C7EC8A352534__INCLUDED_)
