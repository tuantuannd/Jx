#include "stdlib.h"
#include "stdio.h"
#include "KStdAfx.h"
#include "S3PDBConnector.h"
#include "S3PDBSocketPool.h"
#include "S3PDBSocketServer.h"
#include "S3PRelockAccount.h"

#include "TestRunner.h"

#include "S3PTestRoleInfoDAO.h"
#include "S3PTestCardInfoDAO.h"
#include "S3PTestAccountInfoDAO.h"
#include "S3PTestCard.h"
#include "S3PTableInfoCatch.h"
#include "S3PTestTaskList.h"

#include "S3PTestRole.h"

static DWORD WINAPI AddAccount( LPVOID lParam )
{
	DWORD dwRet = 0;
	int iParam = ( int )lParam;
	//if ( 0 < iParam )
	{
		for ( int i = iParam; i <= 20000; i += 10 )
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
int main()
{
	int iRet = 0;
	TCHAR szExePath[MAX_PATH+1];
	TCHAR szINIPath[MAX_PATH+1];
	KPIGetExePath( szExePath, MAX_PATH );
	_tcscpy( szINIPath, szExePath );
	_tcscat( szINIPath, "DataBase.ini" );

	S3PTableInfoCatch::Instance();
	S3PDBConnectionPool::Init( szINIPath );
	S3PDBConnectionPool::Instance();
	S3PDBConnector::Instance();

	/*
	// 单元测试代码
	//------>BEGIN
	cout << endl << "＝＝＝＝＝＝＝＝＝测试开始＝＝＝＝＝＝＝＝" << endl;
	TestRunner  runner;
    runner.addTest("RoleInfo",S3PTestRoleInfoDAO::suite());
	runner.addTest("CardInfo",S3PTestCardInfoDAO::suite());
	runner.addTest( "AccountInfo", S3PTestAccountInfoDAO::suite() );
	runner.addTest( "Card",S3PTestCard::suite() );
	runner.addTest("Role",S3PTestRole::suite());

    runner.run("RoleInfo");
	runner.run("CardInfo");
	runner.run("AccountInfo");
	runner.run("Card");
	runner.run("Role");
	cout << endl << "＝＝＝＝＝＝＝＝＝＝＝测试结束＝＝＝＝＝＝＝＝＝" << endl;
	//<------END
	*/
	// 工作代码
	// 下面将会有三个线程运行，正确运行的情况下，
	// 应该只要一个线程退出其它线程都应退出（对此
	// 处理还未完善）
	//------>BEGIN
	BOOL bFlag = FALSE;
	S3PDBSocketPool sPool;
	char szSetupFilePath[MAX_PATH+1];
	_tcscpy( szSetupFilePath, szExePath );
	_tcscat( szSetupFilePath, "setup.ini" );
	short siPort = def_DEFAULT_PORT;
	char szPort[32];
	KPIGetPrivateProfileString( "Local", "port", "", szPort, 32, szSetupFilePath );
	siPort = atoi( szPort );
	S3PDBSocketServer sServer( siPort, &sPool );
	HANDLE hSServer = NULL;
	S3PRelockAccount* pRelocker = S3PRelockAccount::Instance();
	if ( NULL == pRelocker )
	{
		cout << endl << "Program exited due to some errors" << endl;
		return iRet;
	}
	pRelocker->Start();
	//DWORD dwThreadID;
	//for ( int i = 0; i < 1; i++ )
	//{
	//	CreateThread( 0, 0, AddAccount, ( LPVOID )i, 0, &dwThreadID );
	//}
	cout << endl << "Enter \"q\" to quit:";
	while ( TRUE )
	{
		if ( !bFlag )
		{
			hSServer = sServer.Start();
			if ( NULL != hSServer )
			{
				bFlag = TRUE;
			}
		}
		char c;
		cin >> c;
		if ( ( 'q' == c ) || ( 'Q' == c ) )
		{
			cout << "Exiting...";
			sServer.Stop();
			S3PRelockAccount::Instance()->Stop();
			cout << endl << "Shutdown completed." << endl;
			break;
		}
	}
	//<------END

	S3PDBConnectionPool::ReleaseInstance();
	S3PDBConnector::ReleaseInstance();
	S3PTableInfoCatch::Release();

	return iRet;
}
