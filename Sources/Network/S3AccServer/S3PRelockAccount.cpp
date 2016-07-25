//-----------------------------------------//
//                                         //
//  File		: S3PRelockAccount.h	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/28/2002                //
//                                         //
//-----------------------------------------//
#include "S3PRow.h"
#include "S3PAccountInfoDAO.h"
#include "S3PDBConnector.h"

#include "S3PRelockAccount.h"

S3PRelockAccount* S3PRelockAccount::m_pInstance = NULL;
HANDLE S3PRelockAccount::m_hEnable = NULL;
DWORD S3PRelockAccount::m_dwCycle = 60000;		// One minute

S3PRelockAccount::S3PRelockAccount()
{
	Init();
}

S3PRelockAccount::~S3PRelockAccount()
{

}

S3PRelockAccount* S3PRelockAccount::Instance()
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new S3PRelockAccount;
	}
	return m_pInstance;
}

void S3PRelockAccount::ReleaseInstance()
{
	if ( NULL != m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

DWORD WINAPI S3PRelockAccount::Relock( LPVOID lpParam )
{
	DWORD dwRet = 0;
	if ( NULL == lpParam )
	{
		return dwRet;
	}

	_LPRELOCKERPARAM lpRelocker = ( _LPRELOCKERPARAM )lpParam;
	if ( NULL == lpRelocker->hEnable )
	{
		return dwRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	
	if ( NULL != pAccountCon )
	{
		while ( TRUE )
		{
			DWORD dwResult =
				KPIWaitForSingleObject( lpRelocker->hEnable, lpRelocker->dwCycle );
			if ( 1 == dwResult )	// The thread is commanded to exit.
			{
				break;
			}
			else if ( 2 == dwResult )	// Time out
			{
				// Relocks specified data section
				//------>BEGIN
				pAccountCon->Do( "update account_info set iGameId = 0 where iGameId <> 0 and iTimeCount = 0" );
				pAccountCon->Do( "update account_info set iTimeCount = 0 where iGameId <> 0 and iTimeCount <> 0" );
				//<------END
			}
		}
		pAccountCon->Close();
	}
	S3PDBConnectionPool::Instance()->ReleaseInstance();

	return dwRet;
}

HANDLE S3PRelockAccount::Start()
{
	HANDLE hRet = NULL;
	if ( NULL != m_hRelocker )	// The thread has been running.
	{
		return hRet;
	}

	if ( NULL == m_hEnable )
	{
		m_hEnable = KPICreateEvent( NULL, TRUE, TRUE, def_RELOCKEVENTNAME );
	}
	if ( NULL != m_hEnable )
	{
		KPIResetEvent( m_hEnable );
	}

	m_param.dwCycle = m_dwCycle;
	m_param.hEnable = m_hEnable;
	m_hRelocker = KPICreateThread( S3PRelockAccount::Relock, &m_param, &m_dwRelockerID );
	hRet = m_hRelocker;

	return hRet;
}

BOOL S3PRelockAccount::Stop()
{
	BOOL bRet = FALSE;

	if ( NULL == m_hRelocker )
	{
		return bRet;
	}

	KPISetEvent( m_hEnable );

//	DWORD dwResult = KPIWaitForSingleObject( m_hRelocker, 3000 );
	DWORD dwResult = KPIWaitForSingleObject( m_hRelocker, INFINITE );
	if ( 0 == dwResult )
	{
		bRet = FALSE;
	}
	else if ( 1 == dwResult )	// The thread has ended.
	{
		m_hRelocker = NULL;
		bRet = TRUE;
	}
	else if ( 2 == dwResult )	// Time out.
	{
		if ( TRUE == KPITerminateThread( m_hRelocker, 0 ) )
		{
			m_hRelocker = NULL;
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

void S3PRelockAccount::Init()
{
	m_hRelocker = NULL;
	m_param.dwCycle = m_dwCycle;
	m_param.hEnable = NULL;
}
