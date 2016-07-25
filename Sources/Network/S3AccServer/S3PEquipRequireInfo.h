// S3PEquipRequireInfo.h: interface for the S3PEquipRequireInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPREQUIREINFO_H__C4278CA9_8C51_44CC_AAEA_EBDC2BE4EA83__INCLUDED_)
#define AFX_S3PEQUIPREQUIREINFO_H__C4278CA9_8C51_44CC_AAEA_EBDC2BE4EA83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PEquipRequireInfo : public S3PManipulator  
{
public:
	S3PEquipRequireInfo(int iid);
	virtual ~S3PEquipRequireInfo();
	S3PEquipRequireInfo();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;

};

#endif // !defined(AFX_S3PEQUIPREQUIREINFO_H__C4278CA9_8C51_44CC_AAEA_EBDC2BE4EA83__INCLUDED_)
