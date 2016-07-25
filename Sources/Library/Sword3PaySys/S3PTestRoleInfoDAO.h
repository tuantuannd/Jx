// S3PTestRoleInfoDAO.h: interface for the S3PTestRoleInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTROLEINFODAO_H__F9F4A30A_4970_44B2_B2BC_33FB46496403__INCLUDED_)
#define AFX_S3PTESTROLEINFODAO_H__F9F4A30A_4970_44B2_B2BC_33FB46496403__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4786 4800 4355 )

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

#include <string>

class S3PTestRoleInfoDAO : public TestCase  
{
public:
	S3PTestRoleInfoDAO(std::string name) : TestCase (name) {};
	virtual ~S3PTestRoleInfoDAO();

	void			setUp ();
	static Test		*suite ();

protected:
	void testAddGroup();
	void testQuery();
	void testAdd();
	void testUpdate();
	void testDelete();
};

#endif // !defined(AFX_S3PTESTROLEINFODAO_H__F9F4A30A_4970_44B2_B2BC_33FB46496403__INCLUDED_)
