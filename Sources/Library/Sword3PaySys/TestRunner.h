// TestRunner.h: interface for the TestRunner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTRUNNER_H__F8BE488A_9076_45BD_A028_667E82E40927__INCLUDED_)
#define AFX_TESTRUNNER_H__F8BE488A_9076_45BD_A028_667E82E40927__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4786 4800 4355 )

#include <iostream>
#include <vector>
#include <string>

using namespace std;

#include "KStdAfx.h"
#include "TextTestResult.h"


class Test;

typedef pair<string, Test *>           mapping;
typedef vector<pair<string, Test *> >   mappings;


class TestRunner  
{
protected:
    bool                                m_wait;
    vector<pair<string,Test *> >        m_mappings;

public:
	TestRunner    () : m_wait (false) {}
    ~TestRunner   ();

    void        run           (string testCase);
    void        addTest       (string name, Test *test)
    { m_mappings.push_back (mapping (name, test)); }

protected:
    void        run (Test *test);
    void        printBanner ();

};

#endif // !defined(AFX_TESTRUNNER_H__F8BE488A_9076_45BD_A028_667E82E40927__INCLUDED_)
