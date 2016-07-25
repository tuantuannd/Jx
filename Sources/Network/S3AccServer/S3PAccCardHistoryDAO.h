//-----------------------------------------//
//                                         //
//  File		: S3PAccCardHistoryDAO.h   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PACCCARDHISTORYDAO_H__A3496AAC_0B7F_4723_93B7_0E1F0D849A7D__INCLUDED_)
#define AFX_S3PACCCARDHISTORYDAO_H__A3496AAC_0B7F_4723_93B7_0E1F0D849A7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PAccCardHistoryDAO : public S3PTableDAO  
{
public:
	S3PAccCardHistoryDAO( S3PDBConnection* pConn );
	virtual ~S3PAccCardHistoryDAO();

	virtual std::string GetTableName();
	virtual S3PDBConnection* GetConnection();
protected:
	S3PDBConnection* m_pConn;
};

#endif // !defined(AFX_S3PACCCARDHISTORYDAO_H__A3496AAC_0B7F_4723_93B7_0E1F0D849A7D__INCLUDED_)
