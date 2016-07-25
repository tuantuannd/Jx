// S3PTestCardInfoDAO.h: interface for the S3PTestCardInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTCARDINFODAO_H__61D64007_9186_408A_80C9_71FD10C65587__INCLUDED_)
#define AFX_S3PTESTCARDINFODAO_H__61D64007_9186_408A_80C9_71FD10C65587__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

#include <string>

class S3PTestCardInfoDAO : public TestCase  
{
public:
	S3PTestCardInfoDAO(std::string name) : TestCase (name) {};
	virtual ~S3PTestCardInfoDAO();
	void			setUp ();
	static Test		*suite ();

protected:
	void testAdd();
	void testUpdate();
	void testDelete();
};

#endif // !defined(AFX_S3PTESTCARDINFODAO_H__61D64007_9186_408A_80C9_71FD10C65587__INCLUDED_)
