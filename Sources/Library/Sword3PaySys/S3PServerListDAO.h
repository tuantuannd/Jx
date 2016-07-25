//-----------------------------------------//
//                                         //
//  File		: S3PServerListDAO.h	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PSERVERLISTDAO_H__D2C953B0_525D_40E9_B866_8B8D345637E6__INCLUDED_)
#define AFX_S3PSERVERLISTDAO_H__D2C953B0_525D_40E9_B866_8B8D345637E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PServerListDAO : public S3PTableDAO  
{
public:
	S3PServerListDAO( S3PDBConnection* pConn );
	virtual ~S3PServerListDAO();

	virtual std::string GetTableName();
	S3PDBConnection* GetConnection();
protected:
	S3PDBConnection* m_pConn;
};

#endif // !defined(AFX_S3PSERVERLISTDAO_H__D2C953B0_525D_40E9_B866_8B8D345637E6__INCLUDED_)
