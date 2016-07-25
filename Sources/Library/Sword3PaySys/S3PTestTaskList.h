// S3PTestTaskList.h: interface for the S3PTestTaskList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTESTTASKLIST_H__501451ED_0390_4E3E_9927_78563FB058C0__INCLUDED_)
#define AFX_S3PTESTTASKLIST_H__501451ED_0390_4E3E_9927_78563FB058C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"
#include <string>
class S3PTestTaskList : public TestCase  
{
public:
	S3PTestTaskList(std::string name) : TestCase(name){}
	virtual ~S3PTestTaskList();

	static Test * suite();

protected:
	void test();
};

#endif // !defined(AFX_S3PTESTTASKLIST_H__501451ED_0390_4E3E_9927_78563FB058C0__INCLUDED_)
