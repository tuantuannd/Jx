// S3PTask.h: interface for the S3PTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTASK_H__3CA150AB_6C85_490E_8BFA_5C35EE69011E__INCLUDED_)
#define AFX_S3PTASK_H__3CA150AB_6C85_490E_8BFA_5C35EE69011E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PTask : public S3PManipulator 
{
public:
	S3PTask(int iid);
	virtual ~S3PTask();
	S3PTask();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;
};

#endif // !defined(AFX_S3PTASK_H__3CA150AB_6C85_490E_8BFA_5C35EE69011E__INCLUDED_)
