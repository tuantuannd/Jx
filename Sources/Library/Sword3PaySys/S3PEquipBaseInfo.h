// S3PEquipBaseInfo.h: interface for the S3PEquipBaseInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPBASEINFO_H__02F78BFE_DBBC_4803_88BB_F3849FB58CE3__INCLUDED_)
#define AFX_S3PEQUIPBASEINFO_H__02F78BFE_DBBC_4803_88BB_F3849FB58CE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PEquipBaseInfo : public S3PManipulator  
{
public:
	S3PEquipBaseInfo(int iid);
	virtual ~S3PEquipBaseInfo();
	S3PEquipBaseInfo();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;
};

#endif // !defined(AFX_S3PEQUIPBASEINFO_H__02F78BFE_DBBC_4803_88BB_F3849FB58CE3__INCLUDED_)
