// S3PTestAccountInfoDAO.cpp: implementation of the S3PTestAccountInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestAccountInfoDAO.h"

#include "S3PDBConnection.h"
#include "S3PRow.h"
#include "S3PAccountInfoDAO.h"
#include "S3PDBConnector.h"
#include "S3PAccount.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
S3PTestAccountInfoDAO::~S3PTestAccountInfoDAO()
{
}

void S3PTestAccountInfoDAO::setUp()
{
}
 
Test* S3PTestAccountInfoDAO::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test AccountInfo Table Manipulator");

	testSuite->addTest( new TestCaller <S3PTestAccountInfoDAO> ("testAdd", testAdd) );
    //testSuite->addTest( new TestCaller <S3PTestAccountInfoDAO> ("testUpdate", testUpdate) );
    //testSuite->addTest( new TestCaller <S3PTestAccountInfoDAO> ("testDelete", testDelete) );
    
	return testSuite;
}

static DWORD WINAPI AddAccount( LPVOID lParam )
{
	DWORD dwRet = 0;
	int iParam = ( int )lParam;
	//if ( 0 < iParam )
	{
		for ( int i = iParam; i <= 2000; i += 10 )
		{
			char szAccount[LOGIN_ACCOUNT_MAX_LEN+1];
			szAccount[LOGIN_ACCOUNT_MAX_LEN] = 0;
			char szPassword[LOGIN_PASSWORD_MAX_LEN+1];
			szPassword[LOGIN_PASSWORD_MAX_LEN] = 0;
			char szRealName[LOGIN_REALNAME_MAX_LEN+1];
			szRealName[LOGIN_REALNAME_MAX_LEN] = 0;
			
			sprintf( szAccount, "%d", i );
			sprintf( szPassword, "123456" );
			sprintf( szRealName, "Yang" );
			int iResult = S3PAccount::AddAccount( szAccount, szPassword, szRealName );
		}
	}
	return dwRet;
}
void S3PTestAccountInfoDAO::testAdd()
{
	DWORD dwThreadID;
	for ( int i = 0; i < 1; i++ )
	{
		CreateThread( 0, 0, AddAccount, ( LPVOID )i, 0, &dwThreadID );
	}
	/*
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );

	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		
		cav["cAccName"] = "杨晓东";
		cav["cPassWord"] = "yxd";
		cav["cRealName"] = "杨晓东";
		//cav["dBirthDay"] = "1978-2-21";
		//cav["cArea"] = "Beijing";
		//cav["cIDNum"] = "10000";
		//cav["dRegDate"] = "2002-8-22";
		//cav["cPhone"] = "13900000000";
		//cav["iGameID"] = "30";
		//cav["dBeginDate"] = "2002-8-22";
		//cav["dEndDate"] = "2002-9-22";
		
		S3PRow row( account.GetTableName(), &cav, pAccountCon );
		if ( false == account.HasItem( &row ) )
		{
			if ( account.Add( &row ) <= 0 )
			{
				assert( false );
			}
		}
		pAccountCon->Close();
	}*/
}

void S3PTestAccountInfoDAO::testUpdate()
{
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		cav["cAccName"] = "Yxd";
		cav["cRealName"] = "没有";
		cav["cPhone"] = "No!";

		ColumnAndValue where;
		where["iGameID"] = "0";
		
		S3PRow row( account.GetTableName(), &cav, pAccountCon );
		S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );

		if ( account.Update( &row, &rowWhere ) <= 0 )
		{
			assert( false );
		}
		pAccountCon->Close();
	}
}

void S3PTestAccountInfoDAO::testDelete()
{
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );

		ColumnAndValue where;
		where["cAccName"] = "YangXiaodong";
		where["cPhone"] = "13900000000";
		
		S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );

		if ( account.Delete( &rowWhere ) <= 0 )
		{
			assert( false );
		}
		pAccountCon->Close();
	}
}