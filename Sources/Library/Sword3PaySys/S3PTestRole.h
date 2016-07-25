// S3PTestRole.h: interface for the S3PTestRole class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTROLE_H__0E6F3810_2E8A_444E_873A_5200BE6F6048__INCLUDED_)
#define AFX_S3PTESTROLE_H__0E6F3810_2E8A_444E_873A_5200BE6F6048__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

class S3PTestRole : public TestCase  
{
public:
	S3PTestRole(std::string name) : TestCase (name) {};
	virtual ~S3PTestRole();
 	static Test		*suite ();

protected:
	void testAdd();
	void testLoad();
};

#endif // !defined(AFX_S3PTESTROLE_H__0E6F3810_2E8A_444E_873A_5200BE6F6048__INCLUDED_)
