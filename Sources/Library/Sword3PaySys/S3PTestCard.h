// S3PTestCard.h: interface for the S3PTestCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTCARD_H__1AD6EB59_D5AC_4D2C_8C50_C7BD0F4168D2__INCLUDED_)
#define AFX_S3PTESTCARD_H__1AD6EB59_D5AC_4D2C_8C50_C7BD0F4168D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

#include <string>

class S3PTestCard : public TestCase  
{
public:
	S3PTestCard(std::string name) : TestCase (name) {};
	virtual ~S3PTestCard();
	void			setUp ();
	static Test		*suite ();

protected:
	void testCardInit();
	void testCardSetInfo();
	void testRemove();
	void testGetCardList();
};

#endif // !defined(AFX_S3PTESTCARD_H__1AD6EB59_D5AC_4D2C_8C50_C7BD0F4168D2__INCLUDED_)
