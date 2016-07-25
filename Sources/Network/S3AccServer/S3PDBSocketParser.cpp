//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketParser.cpp    //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBSocketParser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PDBSocketParser::S3PDBSocketParser()
{
	Init();
}

S3PDBSocketParser::~S3PDBSocketParser()
{
	Release();
}

BOOL S3PDBSocketParser::Encode( const IBYTE* pRawData,
							   unsigned long ulRawSize,
							   IBYTE* pEncodedData,
							   unsigned long& ulEncodedSize )
{
	BOOL bRet = FALSE;
	if ( NULL != pRawData )
	{
		ulEncodedSize = ulRawSize;
		if ( NULL != pEncodedData )
		{
			memcpy( pEncodedData, pRawData, ulEncodedSize );
		}
		bRet = TRUE;
	}
	return bRet;
}

BOOL S3PDBSocketParser::Decode(IBYTE *lpData, DWORD &dwSize, IBYTE *lpRetData )
{
	// Not completed...
	if ( NULL != lpRetData )
		memcpy( lpRetData, lpData, dwSize );
	return TRUE;
}

BOOL S3PDBSocketParser::Parse( IBYTE* lpData, DWORD dwSize, BOOL bEncrypt )
{
	assert( ( NULL != lpData ) );
	BOOL bRet = FALSE;

	IBYTE* lpDataTemp = NULL;
	DWORD dwSizeTemp = 0;
	if ( bEncrypt )
	{
		dwSizeTemp = dwSize;
		if ( !( Decode( lpData, dwSizeTemp, NULL ) ) )
		{
			bRet = FALSE;
			return bRet;
		}
		if ( 0 == dwSizeTemp )
		{
			bRet = FALSE;
			return bRet;
		}

		lpDataTemp = new IBYTE[dwSizeTemp];
		if ( NULL != lpDataTemp )
		{	
			dwSizeTemp = dwSize;
			if ( !( Decode( lpData, dwSizeTemp, lpDataTemp ) ))
			{
				bRet = FALSE;
				return bRet;
			}
		}
		else
		{
			bRet = FALSE;
			return bRet;
		}
	}
	else
	{
		dwSizeTemp = dwSize;
		if ( 0 == dwSizeTemp )
		{
			bRet = FALSE;
			return bRet;
		}
		lpDataTemp = new IBYTE[dwSizeTemp];
		if ( NULL != lpDataTemp )
		{
			memcpy( lpDataTemp, lpData, dwSizeTemp );
		}
		else
		{
			bRet = FALSE;
			return bRet;
		}
	}

	if ( dwSizeTemp > 0 )
	{
		if ( LOGIN_A_TAG == lpDataTemp[0] )
		{
			DWORD dwDataSize = 0;
			DWORD dwParam = 0;
			memcpy( &dwDataSize, &lpDataTemp[1], sizeof( DWORD ) );
			memcpy( &dwParam, &lpDataTemp[1+sizeof( DWORD )], sizeof( DWORD ) );
			if ( dwDataSize >= KLOGINSTRUCTHEADSIZE )
			{
				m_dwParam = dwParam;
				m_dwSize = dwDataSize - KLOGINSTRUCTHEADSIZE;
				if ( m_dwSize > 0 )
				{
					m_lpData = new IBYTE[m_dwSize];
					if ( NULL != m_lpData )
					{
						memcpy( m_lpData, &lpDataTemp[1+2*sizeof( DWORD )], m_dwSize );
						bRet = TRUE;
					}
				}
				else
				{
					m_lpData = NULL;
					bRet = TRUE;
				}
			}
		}
	}

	if ( NULL != lpDataTemp )
	{
		delete lpDataTemp;
	}
	return bRet;
}

void S3PDBSocketParser::Init()
{
	m_dwParam = 0;
	m_lpData = NULL;
	m_dwSize = 0;
}

void S3PDBSocketParser::Release()
{
	if ( NULL != m_lpData )
	{
		delete []m_lpData;
		m_lpData = NULL;
	}
	m_dwSize = 0;
	m_dwParam = 0;
}
