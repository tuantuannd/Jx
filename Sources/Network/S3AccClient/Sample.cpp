// Sample.cpp: implementation of the CSample class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "S3AccClientSample.h"
#include "S3PDBSocketParser.h"
#include "Sample.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSample* CSample::m_pInstance = NULL;
CSample::CSample()
{
}

CSample::~CSample()
{
}

CSample* CSample::Instance()
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new CSample;
		if ( NULL != m_pInstance )
		{
			if ( !m_pInstance->CreateClientSocket() )
			{
				delete m_pInstance;
			}
		}
	}
	return m_pInstance;
}

void CSample::ReleaseInstance()
{
	if ( NULL != m_pInstance )
	{
		m_pInstance->ReleaseClientSocket();
		delete m_pInstance;
	}
}

BOOL CSample::CreateClientSocket()
{
	BOOL bRet = FALSE;
	try
	{
		m_pS = new CGameClient( 10, 1024 );
		if ( NULL != m_pS )
		{
			bRet = TRUE;
		}
	}
	catch ( ... )
	{
		bRet = FALSE;
	}
	return bRet;
}

void CSample::ReleaseClientSocket()
{
	if ( m_pS )
	{
		delete m_pS;
		m_pS = NULL;
	}
}

BOOL CSample::Connect( LPCTSTR lpszAddr, short siPort )
{
	BOOL bRet = FALSE;
	if ( NULL != m_pS )
	{
		try
		{
			m_pS->ConnectTo( lpszAddr, siPort );
			m_pS->StartConnections();
			bRet = TRUE;
		}
		catch ( ... )
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

BOOL CSample::Start()
{
	BOOL bRet = FALSE;
	if ( NULL != m_pS )
	{
		try
		{
			m_pS->Start();
			bRet = TRUE;
		}
		catch ( ... )
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

void CSample::Login( const char *lpszAccName,
					const char *lpszPassword,
					const DWORD dwServerID )
{
	ASSERT(NULL!=lpszAccName);
	ASSERT(NULL!=lpszPassword);
	ASSERT(strlen(lpszAccName)<=LOGIN_ACCOUNT_MAX_LEN);
	ASSERT(strlen(lpszPassword)<=LOGIN_PASSWORD_MAX_LEN);
	ASSERT(NULL!=m_pS);
	
	KLoginAccountInfo loginStruct;
	memset( &loginStruct, 0, sizeof( KLoginAccountInfo ) );
	strcpy( loginStruct.Account, lpszAccName );
	strcpy( loginStruct.Password, lpszPassword );
	loginStruct.Param = LOGIN_A_LOGIN;
	loginStruct.Size = sizeof( KLoginAccountInfo ) + sizeof( DWORD );
	DWORD dwTempSize = 1 + sizeof( KLoginAccountInfo ) + sizeof( DWORD );
	IBYTE* pTemp = new IBYTE[dwTempSize];
	if ( NULL != pTemp )
	{
		// 数据打包
		//------>BEGIN
		pTemp[0] = LOGIN_A_TAG;
		memcpy( &pTemp[1], &loginStruct, sizeof( KLoginAccountInfo ) );
		memcpy( &pTemp[1 + sizeof( KLoginAccountInfo )], &dwServerID, sizeof( DWORD ) );
		//<------END
		unsigned long ulEncodedSize;
		if ( S3PDBSocketParser::Encode( pTemp,
			dwTempSize,
			NULL,
			ulEncodedSize ) ) // 计算加密后数据包尺寸
		{
			IBYTE* pSendBuf = new IBYTE[ulEncodedSize];
			if ( NULL != pSendBuf )
			{
				if ( S3PDBSocketParser::Encode( pTemp,
					dwTempSize,
					pSendBuf,
					ulEncodedSize ) ) // 加密数据包
				{
					try
					{
						m_pS->Write( pSendBuf, ulEncodedSize );

						DWORD dwWaitResult =
							WaitForSingleObject( m_pS->m_hHasRecvdata, 6000 );
						if ( WAIT_OBJECT_0 == dwWaitResult )
						{
							ResetEvent( m_pS->m_hHasRecvdata );
							m_pS->m_pRecvBuf; // 接收到的数据
							m_pS->m_dwRecvSize; // 接收到的数据的尺寸
							// 其它数据处理
							//------>BEGIN

							// ...

							//<------END
						}
						else
						{
							// 错误处理
						}

					}
					catch ( ... )
					{
						// 错误处理
					}
				}
				delete []pSendBuf;
				pSendBuf = NULL;
			}
		}

		delete []pTemp;
		pTemp = NULL;
	}
}

void CSample::QueryGSList()
{
}

void CSample::DecAccountDeposit(DWORD dwGamerID, DWORD dwServerID, DWORD dwDecSeconds)
{
}

void CSample::Logout(DWORD dwGamerID, DWORD dwServerID)
{
}
