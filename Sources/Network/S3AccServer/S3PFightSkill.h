// S3PFightSkill.h: interface for the S3PFightSkill class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PFIGHTSKILL_H__C14C1882_19B9_4655_8CE3_C3482538D489__INCLUDED_)
#define AFX_S3PFIGHTSKILL_H__C14C1882_19B9_4655_8CE3_C3482538D489__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PFightSkill : public S3PManipulator  
{
public:
	S3PFightSkill(int iid);
	virtual ~S3PFightSkill();
	S3PFightSkill();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;
};

#endif // !defined(AFX_S3PFIGHTSKILL_H__C14C1882_19B9_4655_8CE3_C3482538D489__INCLUDED_)
