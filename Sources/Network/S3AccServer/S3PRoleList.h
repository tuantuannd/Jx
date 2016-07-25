// S3PRoleList.h: interface for the S3PRoleList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PROLELIST_H__F6DD88CE_DECF_46D1_A7A3_3417A55E4345__INCLUDED_)
#define AFX_S3PROLELIST_H__F6DD88CE_DECF_46D1_A7A3_3417A55E4345__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PRole.h"

class S3PRoleList : public S3PTableObjList<S3PRole>  
{
public:
	S3PRoleList(std::string cAccName);
	virtual ~S3PRoleList();

};

#endif // !defined(AFX_S3PROLELIST_H__F6DD88CE_DECF_46D1_A7A3_3417A55E4345__INCLUDED_)
