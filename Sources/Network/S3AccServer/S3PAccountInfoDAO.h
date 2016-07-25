//-----------------------------------------//
//                                         //
//  File		: S3PAccountInfoDAO.h      //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PACCOUNTINFODAO_H__72109E19_CACF_422A_A152_29690FB5FE12__INCLUDED_)
#define AFX_S3PACCOUNTINFODAO_H__72109E19_CACF_422A_A152_29690FB5FE12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PAccountInfoDAO : public S3PTableDAO  
{
public:
	S3PAccountInfoDAO( S3PDBConnection * pConn );
	virtual ~S3PAccountInfoDAO();

	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PACCOUNTINFODAO_H__72109E19_CACF_422A_A152_29690FB5FE12__INCLUDED_)
