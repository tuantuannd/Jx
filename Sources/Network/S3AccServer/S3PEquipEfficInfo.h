// S3PEquipEfficInfo.h: interface for the S3PEquipEfficInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPEFFICINFO_H__A2E52C6E_C3BC_46A6_A386_2ACAF99B0B4A__INCLUDED_)
#define AFX_S3PEQUIPEFFICINFO_H__A2E52C6E_C3BC_46A6_A386_2ACAF99B0B4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PEquipEfficInfo : public S3PManipulator  
{
public:
	S3PEquipEfficInfo(int iid);
	virtual ~S3PEquipEfficInfo();
	S3PEquipEfficInfo();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;

};

#endif // !defined(AFX_S3PEQUIPEFFICINFO_H__A2E52C6E_C3BC_46A6_A386_2ACAF99B0B4A__INCLUDED_)
