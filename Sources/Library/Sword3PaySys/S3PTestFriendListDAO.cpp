// S3PTestFriendListDAO.cpp: implementation of the S3PTestFriendListDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestFriendListDAO.h"

#include "S3PDBConnection.h"
#include "S3PRow.h"
#include "S3PFriendListDAO.h"
#include "S3PDBConnector.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTestFriendListDAO::~S3PTestFriendListDAO()
{

}

Test *S3PTestFriendListDAO::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test FriendList Table Manipulator");

	testSuite->addTest (new TestCaller <S3PTestFriendListDAO> ("testAdd", TestAdd));
    
	return testSuite;
}

void S3PTestFriendListDAO::TestAdd()
{
	S3PDBConnection* pRoleCon =	
		S3PDBConnector::Instance()->ApplyDBConnection( def_ROLESECTIONNAME );

	if (NULL != pRoleCon)
	{
		S3PFriendListDAO fl(pRoleCon);
		ColumnAndValue cav;
		
		cav["cUserCode"] = "123";
		cav["cFriendCode"] = "321";
		
		S3PRow row(fl.GetTableName(), &cav, pRoleCon);
		if ( fl.Add(&row) <= 0 )
		{
			assert(false);
		}
	}
}