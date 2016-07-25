// S3PFriend.h: interface for the S3PFriend class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PFRIEND_H__750048AF_916F_4843_9FCA_0B61129D60A2__INCLUDED_)
#define AFX_S3PFRIEND_H__750048AF_916F_4843_9FCA_0B61129D60A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

class S3PFriend : public S3PManipulator  
{
public:
	S3PFriend(int iid);
	virtual ~S3PFriend();
	S3PFriend();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

public:
	int m_iid;

};

#endif // !defined(AFX_S3PFRIEND_H__750048AF_916F_4843_9FCA_0B61129D60A2__INCLUDED_)
