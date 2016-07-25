//------------------------------------------//
//											//
//  File		: S3PAccountUser.h			//
//	Author		: Yang Xiaodong				//
//	Modified	: 12/19/2002				//
//											//
//------------------------------------------//

#if !defined(AFX_S3PACCOUNTUSER_H__C322886C_8CF8_4ED9_BFC3_C2F5693438EE__INCLUDED_)
#define AFX_S3PACCOUNTUSER_H__C322886C_8CF8_4ED9_BFC3_C2F5693438EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PAccountUserDAO : public S3PTableDAO  
{
public:
	S3PAccountUserDAO( S3PDBConnection* pConn );
	virtual ~S3PAccountUserDAO();
	virtual std::string GetTableName();
	virtual S3PDBConnection * GetConnection();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PACCOUNTUSER_H__C322886C_8CF8_4ED9_BFC3_C2F5693438EE__INCLUDED_)
