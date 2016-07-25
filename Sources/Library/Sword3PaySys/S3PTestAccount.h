// S3PTestAccount.h: interface for the S3PTestAccount class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTACCOUNT_H__AFE9EB5D_E03F_42E1_8282_2C176A9EA68D__INCLUDED_)
#define AFX_S3PTESTACCOUNT_H__AFE9EB5D_E03F_42E1_8282_2C176A9EA68D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

class S3PTestAccount : public TestCase  
{
public:
	S3PTestAccount( std::string name ) : TestCase ( name ) {};
	virtual ~S3PTestAccount();

	void			setUp ();
	static Test		*suite ();

protected:
	void testLogin();
};

#endif // !defined(AFX_S3PTESTACCOUNT_H__AFE9EB5D_E03F_42E1_8282_2C176A9EA68D__INCLUDED_)
