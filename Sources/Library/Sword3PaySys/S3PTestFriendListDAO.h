// S3PTestFriendListDAO.h: interface for the S3PTestFriendListDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTFRIENDLISTDAO_H__5F5C8119_A9B9_4EE1_83E7_AEE507AF4724__INCLUDED_)
#define AFX_S3PTESTFRIENDLISTDAO_H__5F5C8119_A9B9_4EE1_83E7_AEE507AF4724__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

#include <string>

class S3PTestFriendListDAO : public TestCase  
{
public:
	S3PTestFriendListDAO(std::string name) : TestCase (name) {};
	virtual ~S3PTestFriendListDAO();
	static Test		*suite ();
public:
	void TestAdd();
};

#endif // !defined(AFX_S3PTESTFRIENDLISTDAO_H__5F5C8119_A9B9_4EE1_83E7_AEE507AF4724__INCLUDED_)
