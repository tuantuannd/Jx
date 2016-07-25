// S3PTestAccount.cpp: implementation of the S3PTestAccount class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestAccount.h"
#include "S3PAccount.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
S3PTestAccount::~S3PTestAccount()
{

}

void S3PTestAccount::testLogin()
{
	S3PAccount account;
	DWORD dwSize;
	account.QueryGameserverList( "YangXiaodong", "yxd", NULL, dwSize );
}

void S3PTestAccount::setUp()
{
}

Test* S3PTestAccount::suite()
{
	TestSuite *testSuite = new TestSuite ("Test account Login");

	testSuite->addTest( new TestCaller <S3PTestAccount> ("testLogin", testLogin) );
    
	return testSuite;
}