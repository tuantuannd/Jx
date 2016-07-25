// S3PLifeSkill.h: interface for the S3PLifeSkill class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PLIFESKILL_H__3636BE72_88F3_4196_8B7C_71352C8A87DE__INCLUDED_)
#define AFX_S3PLIFESKILL_H__3636BE72_88F3_4196_8B7C_71352C8A87DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PLifeSkill : public S3PManipulator  
{
public:
	S3PLifeSkill(int iid);
	virtual ~S3PLifeSkill();
	S3PLifeSkill();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;
};

#endif // !defined(AFX_S3PLIFESKILL_H__3636BE72_88F3_4196_8B7C_71352C8A87DE__INCLUDED_)
