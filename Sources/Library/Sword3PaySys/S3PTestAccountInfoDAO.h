// S3PTestAccountInfoDAO.h: interface for the S3PTestAccountInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTACCOUNTINFODAO_H__AFF0C444_D171_4510_915B_FBA12FB6B5D1__INCLUDED_)
#define AFX_S3PTESTACCOUNTINFODAO_H__AFF0C444_D171_4510_915B_FBA12FB6B5D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

#include <string>

class S3PTestAccountInfoDAO : public TestCase  
{
public:
	S3PTestAccountInfoDAO(std::string name) : TestCase (name) {};
	virtual ~S3PTestAccountInfoDAO();
	void			setUp ();
	static Test		*suite ();

protected:
	void testAdd();
	void testUpdate();
	void testDelete();
};

#endif // !defined(AFX_S3PTESTACCOUNTINFODAO_H__AFF0C444_D171_4510_915B_FBA12FB6B5D1__INCLUDED_)
