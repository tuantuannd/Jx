//-----------------------------------------//
//                                         //
//  File		: KStdAfx.cpp              //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/14/2002                //
//                                         //
//-----------------------------------------//
#include "tchar.h"

#include "GlobalFun.h"
#include "KStdAfx.h"
#include "process.h"

static const TCHAR szEnter1[] = "\r\n";
static const UINT uiEnter1 = _tcslen( szEnter1 );
static const TCHAR szEnter2[] = "\n\r";
static const UINT uiEnter2 = _tcslen( szEnter2 );
static const TCHAR szEnter3[] = "\r";
static const UINT uiEnter3 = _tcslen( szEnter3 );
static const TCHAR szEnter4[] = "\n";
static const UINT uiEnter4 = _tcslen( szEnter4 );
static const TCHAR szTabKey[] = { 0x09, 0 };
static const UINT uiTabkey = _tcslen( szTabKey );
static const UINT uiTab2SpaceNum = 4;
static const TCHAR szSpaceKey[] = " ";
static const UINT uiSpaceKey = _tcslen( szSpaceKey );

static const TCHAR szLBracket[] = "[";
static const UINT uiLBracket = _tcslen( szLBracket );
static const TCHAR szRBracket[] = "]";
static const UINT uiRBracket = _tcslen( szRBracket );
static const TCHAR szEqual[] = "=";
static const UINT uiEqual = _tcslen( szEqual );

static BOOL IsEnterKey( LPCTSTR lpChar,
						BOOL* pEnterType = NULL )
{
	BOOL bRet = FALSE;
	
	if ( 0 == _tcsncmp( lpChar, szEnter1, uiEnter1 ) )
	{
		if ( NULL != pEnterType )
		{
			*pEnterType = TRUE;
		}
		bRet = TRUE;
	}
	else if ( 0 == _tcsncmp( lpChar, szEnter2, uiEnter2 ) )
	{
		if ( NULL != pEnterType )
		{
			*pEnterType = TRUE;
		}
		bRet = TRUE;
	}
	else if ( 0 == _tcsncmp( lpChar, szEnter3, uiEnter3 ) )
	{
		if ( NULL != pEnterType )
		{
			*pEnterType = FALSE;
		}
		bRet = TRUE;
	}
	else if ( 0 == _tcsncmp( lpChar, szEnter4, uiEnter4 ) )
	{
		if ( NULL != pEnterType )
		{
			*pEnterType = FALSE;
		}
		bRet = TRUE;
	}

	return bRet;
}

static long GetFirstLineLen( LPCTSTR lpszSource )
{
	long liRet = 0;
	if ( NULL != lpszSource )
	{
		LPTSTR lpszAt = ( LPTSTR )lpszSource;
		BOOL bHasEnterKey = FALSE;
		while ( 0 != lpszAt[0] )
		{
			if ( (0 == _tcsncmp( lpszAt, szEnter1, uiEnter1 ) )
				|| ( 0 == _tcsncmp( lpszAt, szEnter2, uiEnter2 ) )
				|| ( 0 == _tcsncmp( lpszAt, szEnter3, uiEnter3 ) )
				|| ( 0 == _tcsncmp( lpszAt, szEnter4, uiEnter4 ) ) )
			{
				bHasEnterKey = TRUE;
				liRet = lpszAt - lpszSource;
				break;
			}
			lpszAt++;
		}
		if ( !bHasEnterKey )
		{
			liRet = lpszAt - lpszSource;
		}
	}
	return liRet;
}

static BOOL GetStrInStrPos( LPCTSTR lpszSouce,
						   LPCTSTR lpszSearch,
						   long &liOffset )
{
	BOOL bRet = FALSE;
	if ( ( NULL == lpszSouce )
		|| ( NULL == lpszSearch ) )
	{
		bRet = FALSE;
	}
	else
	{
		long liSource = _tcslen( lpszSouce );
		long liSearch = _tcslen( lpszSearch );
		LPTSTR lpszAt = ( LPTSTR )( lpszSouce );
		for ( long i = 0; i < liSource; i++ )
		{
			if ( 0 == _tcsncmp( lpszAt, lpszSearch, liSearch ) )
			{
				bRet = TRUE;
				liOffset = i;
				break;
			}
			lpszAt++;
		}
	}
	return bRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: GetPrivateProfileKeyPos
//	Return	: int
//			  = -1	---- Error;
//			  = 0	---- Has no the section;
//			  = 1	---- Finds out the key successfully;
//			  = 2	---- Only finds out the section.
//	Platform	: Unconcerned.
//----------------------------------------------------------
static int GetPrivateProfileKeyPos( LPCTSTR lpszBuf,
									LPCTSTR lpszAppName,
									LPCTSTR lpszKeyName,
									long &liOffset,
									long &liSize )
{
	int iRet = -1;
	if ( ( NULL == lpszBuf )
		|| ( NULL == lpszAppName )
		|| ( NULL == lpszKeyName ) )
	{
		iRet = -1;
	}
	else
	{
		BOOL bSearchFlag = FALSE;
		LPTSTR lpszAt = ( LPTSTR )lpszBuf;
		long liAppName = _tcslen( lpszAppName );
		LPTSTR lpszModifiedAppName = new TCHAR[liAppName+3];
		if ( NULL != lpszModifiedAppName )
		{
			_tcscpy( lpszModifiedAppName, szLBracket );
			_tcscpy( lpszModifiedAppName + uiLBracket, lpszAppName );
			_tcscpy( lpszModifiedAppName + uiLBracket + liAppName, szRBracket );
			
			long liAppNameOffset;
			bSearchFlag = GetStrInStrPos( lpszBuf, lpszModifiedAppName, liAppNameOffset );
			if ( bSearchFlag )
			{
				lpszAt +=
					liAppNameOffset + liAppName + uiLBracket + uiRBracket;
				long liKeyNameOffset;
				bSearchFlag = GetStrInStrPos( lpszAt, lpszKeyName, liKeyNameOffset );
				if ( bSearchFlag )
				{
					liOffset =
						liAppNameOffset + liKeyNameOffset + liAppName + uiLBracket + uiRBracket;
					liSize = GetFirstLineLen( lpszAt + liKeyNameOffset );
					iRet = 1;
				}
				else
				{
					liOffset =
						liAppNameOffset + liAppName + uiLBracket + uiRBracket;
					liSize = 0;
					iRet = 2;
				}
			}
			else
			{
				iRet = 0;
			}
			delete []lpszModifiedAppName;
		}
	}
	return iRet;
}

static LPTSTR GetKeyValue( LPCTSTR lpszKeyLine, long &liKeyValueSize )
{
	LPTSTR lpszRet = NULL;
	if ( NULL != lpszKeyLine )
	{
		long liKeySize = GetFirstLineLen( lpszKeyLine );
		LPTSTR lpszAt = ( LPTSTR )lpszKeyLine;
		BOOL bHasValue = FALSE;
		long i;
		for ( i = 0; i < liKeySize; i++ )
		{
			if ( 0 == _tcsncmp( lpszAt, szEqual, uiEqual ) )
			{
				bHasValue = TRUE;
				break;
			}
			lpszAt++;
		}
		if ( bHasValue )
		{
			for ( i += uiEqual, lpszAt += uiEqual; i < liKeySize; )
			{
				if ( 0 == _tcsncmp( lpszAt, szSpaceKey, uiSpaceKey ) )
				{
					lpszAt += uiSpaceKey;
					i += uiSpaceKey;
					continue;
				}
				else
				{
					if ( ( lpszAt - lpszKeyLine ) < liKeySize )
					{
						lpszRet = lpszAt;
						liKeyValueSize =
							liKeySize - ( lpszAt - lpszKeyLine );
					}
					break;
				}
			}
		}
	}
	return lpszRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: KPIWritePrivateProfileString
//	Return	: BOOL
//	Description	: copies a string into the
//				  specified section of an
//				  initialization file.
//	Platform	: Unconcerned.
//----------------------------------------------------------
BOOL KPIWritePrivateProfileString( LPCTSTR lpszAppName,  /* section name */
								  LPCTSTR lpszKeyName,  /* key name */
								  LPCTSTR lpszString,   /* string to add */
								  LPCTSTR lpszFileName  /* initialization file */ )
{
	BOOL bRet = TRUE;
	if ( ( NULL == lpszAppName )
		|| ( NULL == lpszKeyName )
		|| ( NULL == lpszString )
		|| ( NULL == lpszFileName ) )
	{
		bRet = FALSE;
	}
	else
	{
		try
		{
			FILE* pFile = fopen( lpszFileName, "rb" );
			long liFileSize = 0;
			char* lpszFileBuf = NULL;
			if ( NULL != pFile )
			{
				fseek( pFile, 0, SEEK_END );
				liFileSize = ftell( pFile );
				if ( liFileSize > 0 )
				{
					lpszFileBuf = new char[liFileSize+1];
					if ( NULL != lpszFileBuf )
					{
						fseek( pFile, 0, SEEK_SET );
						fread( lpszFileBuf, 1, liFileSize, pFile );
						lpszFileBuf[liFileSize] = 0;
					}
					else
					{
						fclose( pFile );
						bRet = FALSE;
						return bRet;
					}
				}
				fclose( pFile );
			}

			pFile = fopen( lpszFileName, "wb" );
			if ( NULL != pFile )
			{
				if ( 0 == liFileSize )
				{
					fwrite( szLBracket, 1, uiLBracket, pFile );
					fwrite( lpszAppName, 1, _tcslen( lpszAppName ), pFile );
					fwrite( szRBracket, 1, uiRBracket, pFile );
					fwrite( szEnter1, 1, uiEnter1, pFile );

					fwrite( lpszKeyName, 1, _tcslen( lpszKeyName ), pFile );
					fwrite( szEqual, 1, uiEqual, pFile );
					fwrite( lpszString, 1, _tcslen( lpszString ), pFile );
					fwrite( szEnter1, 1, uiEnter1, pFile );
				}
				else
				{
					long liOffset, liSize;
					int iResult =
						GetPrivateProfileKeyPos( lpszFileBuf,
						lpszAppName,
						lpszKeyName,
						liOffset,
						liSize );
					if ( 0 == iResult )
					{
						fwrite( lpszFileBuf, 1, liFileSize, pFile );
						if ( ( !IsEnterKey( &( lpszFileBuf[liFileSize-2] ) ) )
							&& ( !IsEnterKey( &( lpszFileBuf[liFileSize-3] ) ) ) )
						{
							fwrite( szEnter1, 1, uiEnter1, pFile );
						}

						fwrite( szLBracket, 1, uiLBracket, pFile );
						fwrite( lpszAppName, 1, _tcslen( lpszAppName ), pFile );
						fwrite( szRBracket, 1, uiRBracket, pFile );
						fwrite( szEnter1, 1, uiEnter1, pFile );
						
						fwrite( lpszKeyName, 1, _tcslen( lpszKeyName ), pFile );
						fwrite( szEqual, 1, uiEqual, pFile );
						fwrite( lpszString, 1, _tcslen( lpszString ), pFile );
						fwrite( szEnter1, 1, uiEnter1, pFile );
						bRet = TRUE;
					}
					else if ( 1 == iResult )
					{
						fwrite( lpszFileBuf, 1, liOffset, pFile );
						long liKeyLen =
							GetFirstLineLen( &( lpszFileBuf[liOffset] ) );
						LPTSTR lpszKeyTemp = new TCHAR[liKeyLen+1];
						memcpy( lpszKeyTemp, &( lpszFileBuf[liOffset] ), liKeyLen * sizeof( TCHAR ) );
						lpszKeyTemp[liKeyLen] = 0;
						long liKeyValueSize;
						if ( NULL != GetKeyValue( lpszKeyTemp, liKeyValueSize ) )
						{
							fwrite( lpszKeyTemp, 1, liKeyLen - liKeyValueSize, pFile );
							fwrite( lpszString, 1, _tcslen( lpszString ), pFile );
							fwrite( &( lpszFileBuf[liOffset+liKeyLen] ), 1, liFileSize - liOffset - liKeyLen, pFile );
						}
						else
						{
							fwrite( &( lpszFileBuf[liOffset] ), 1, liFileSize - liOffset, pFile );
						}

						delete []lpszKeyTemp;
						bRet = TRUE;
					}
					else if ( 2 == iResult )
					{
						fwrite( lpszFileBuf, 1, liOffset, pFile );
						fwrite( szEnter1, 1, uiEnter1, pFile );
						fwrite( lpszKeyName, 1, _tcslen( lpszKeyName ), pFile );
						fwrite( szEqual, 1, uiEqual, pFile );
						fwrite( lpszString, 1, _tcslen( lpszString ), pFile );
						if ( !IsEnterKey( &( lpszFileBuf[liOffset] ) ) )
						{
							fwrite( szEnter1, 1, uiEnter1, pFile );
						}
						fwrite( &( lpszFileBuf[liOffset] ), 1, liFileSize - liOffset, pFile );
						bRet = TRUE;
					}
					else
					{
						bRet = FALSE;
					}
				}
				
				fclose( pFile );
			}
			else
			{
				bRet = FALSE;
			}

			if ( NULL != lpszFileBuf )
			{
				delete []lpszFileBuf;
			}
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	
	return bRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: KPIGetPrivateProfileString
//	Return	: DWORD
//			  The return value is the number of
//			  characters copied to the buffer,
//			  not including the terminating null
//			  character.
//	Description	: retrieves a string from
//				  the specified section in
//				  an initialization file.
//	Platform	: Unconcerned.
//----------------------------------------------------------
DWORD KPIGetPrivateProfileString( LPCTSTR lpszAppName,		/* section name(Input) */
								 LPCTSTR lpszKeyName,		/* key name(Input) */
								 LPCTSTR lpszDefault,		/* default string(Input) */
								 LPTSTR lpszReturnedString,	/* destination buffer(Output) */
								 DWORD dwSize,				/* size of destination buffer(Input) */
								 LPCTSTR lpszFileName		/* initialization file name(Input) */ )
{
	DWORD dwRet = 0;

	if ( ( NULL == lpszAppName )
		|| ( NULL == lpszKeyName )
		|| ( NULL == lpszDefault )
		|| ( NULL == lpszFileName ) )
	{
		return dwRet;
	}

	BOOL bDefault = FALSE;

	try
	{
		FILE* pFile = fopen( lpszFileName, "rb" );
		if ( NULL != pFile )
		{
			fseek( pFile, 0, SEEK_END );
			long liFileSize = ftell( pFile );
			char* lpszFileBuf = NULL;
			if ( liFileSize > 0 )
			{
				lpszFileBuf = new char[liFileSize+1];
				if ( NULL != lpszFileBuf )
				{
					fseek( pFile, 0, SEEK_SET );
					fread( lpszFileBuf, 1, liFileSize, pFile );
					lpszFileBuf[liFileSize] = 0;

					long liOffset, liSize;
					int iResult =
						GetPrivateProfileKeyPos( lpszFileBuf,
						lpszAppName,
						lpszKeyName,
						liOffset,
						liSize );
					if ( 1 == iResult )
					{
						long liKeyLen =
							GetFirstLineLen( &( lpszFileBuf[liOffset] ) );
						LPTSTR lpszKeyTemp = new TCHAR[liKeyLen+1];
						memcpy( lpszKeyTemp, &( lpszFileBuf[liOffset] ), liKeyLen * sizeof( TCHAR ) );
						lpszKeyTemp[liKeyLen] = 0;
						long liKeyValueSize;
						LPTSTR lpszKeyValue =
							GetKeyValue( lpszKeyTemp, liKeyValueSize );
						if ( NULL != lpszKeyValue )
						{
							dwRet = liKeyValueSize;
							DWORD dwCopyLen =
								dwSize <= liKeyValueSize ? dwSize : liKeyValueSize;
							if ( NULL != lpszReturnedString )
							{
								memcpy( lpszReturnedString, lpszKeyValue, dwCopyLen );
								if ( dwSize > dwCopyLen )
								{
									lpszReturnedString[dwCopyLen] = 0;
								}
							}
						}
						else
						{
							bDefault = TRUE;
						}

						delete []lpszKeyTemp;
					}
					else
					{
						bDefault = TRUE;
					}
					delete []lpszFileBuf;
				}
			}
			fclose( pFile );
		}
		else
		{
			bDefault = TRUE;
		}
	}
	catch(...)
	{
		bDefault = TRUE;
	}

	if (bDefault)
	{
		dwRet = _tcslen( lpszDefault );
		DWORD dwCopyLen =
			dwSize <= dwRet ? dwSize : dwRet;
		if ( NULL != lpszReturnedString )
		{
			memcpy( lpszReturnedString, lpszDefault, dwCopyLen );
			if ( dwSize > dwCopyLen )
			{
				lpszReturnedString[dwCopyLen] = 0;
			}
		}
	}
	
	return dwRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: KPIWaitForSingleObject
//	Return	: DWORD
//			  = 0	---- failed;
//			  = 1	---- The state of the specified
//					     object is signaled;
//			  = 2	---- time out;
//			  = ...	---- Not defined.
//	Platform	: Windows 9X/NT/2000/Later
//----------------------------------------------------------
DWORD KPIWaitForSingleObject( HANDLE hHandle, DWORD dwMilliseconds )
{
	DWORD dwRet = 0;
	DWORD dwResult = WaitForSingleObject( hHandle, dwMilliseconds );
	switch ( dwResult )
	{
	case WAIT_FAILED:
		dwRet = 0;
		break;
	case WAIT_OBJECT_0:
		dwRet = 1;
		break;
	case WAIT_TIMEOUT:
		dwRet = 2;
		break;
	default:
		dwRet = 0;
		break;
	}
	return dwRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: KPICloseHandle
//	Return	: BOOL
//	Platform	: Windows 9X/NT/2000/Later
//----------------------------------------------------------
BOOL KPICloseHandle( HANDLE hObject )
{
	BOOL bRet = FALSE;
	bRet = CloseHandle( hObject );
	return bRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: KPITerminateThread
//	Return	: BOOL
//	Platform	: Windows 9X/NT/2000/Later
//----------------------------------------------------------
BOOL KPITerminateThread( HANDLE hThread, DWORD dwExitCode )
{
	BOOL bRet = FALSE;
	bRet = TerminateThread( hThread, dwExitCode );
	return bRet;
}

//----------------------------------------------------------
//	< Function >
//	Name	: KPICreateThread
//	Return	: HANDLE
//			  Returns 0 if function is failed;
//			  Returns the handle of new thread if
//			  function is successful.
//	Description	: Creates a thread to execute within
//				  the virtual address space of the
//				  calling process.
//	Platform	: Windows 9X/NT/2000/Later
//----------------------------------------------------------
HANDLE KPICreateThread( LPTHREAD_START_ROUTINE lpStartAddress,	/* thread function */
					   LPVOID lpParameter,						/* thread argument */
					   LPDWORD lpThreadId						/* thread identifier */ )
{
	typedef unsigned ( __stdcall *start_address )( void * );

	HANDLE hRet = NULL;

	DWORD dwThreadId = 0;
	if (lpThreadId == NULL)
		lpThreadId = &dwThreadId; 
	
	hRet = 
		(HANDLE)_beginthreadex( NULL, 0, (start_address)lpStartAddress, lpParameter, 0, (unsigned*)lpThreadId);

	return hRet;
}

SOCKET KPICreateUDPSocket( int iPort )
{
	SOCKET s = INVALID_SOCKET;

	// The follow code is used to initialize
	// the environment for Windows OS platform
	// and should be removed or changed when
	// replanted to other platforms.
	//------>BEGIN
	WORD wVersionRequired = MAKEWORD(1,1);
	WSADATA WSAdata;
	if ( 0 != WSAStartup( wVersionRequired, &WSAdata ) )
	{
		s = INVALID_SOCKET;
		return s;
	}
	//<------END

	s = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( INVALID_SOCKET == s )
	{
		return s;
	}
	
	SOCKADDR_IN local;
	local.sin_family = AF_INET;
	local.sin_port = htons( ( short )( iPort ) );
	local.sin_addr.s_addr = htonl( INADDR_ANY );
	if ( bind( s, ( SOCKADDR* )( &local ), sizeof( local ) )
		== SOCKET_ERROR )
	{
		closesocket( s );
		s = INVALID_SOCKET;
		return s;
	}

	WSACleanup();	// Needed on Windows OS platforms.

	return s;
}

HANDLE KPICreateEvent( LPSECURITY_ATTRIBUTES lpEventAttributes,	/* SD */
					  BOOL bManualReset,						/* reset type */
					  BOOL bInitialState,						/* initial state */
					  LPCTSTR lpszName							/* object name */ )
{
	HANDLE hRet = NULL;

	hRet = CreateEvent( lpEventAttributes, bManualReset, bInitialState, lpszName );

	return hRet;
}

BOOL KPISetEvent( HANDLE hEvent/* handle to event */ )
{
	BOOL bRet = FALSE;

	bRet = SetEvent( hEvent );

	return bRet;
}

BOOL KPIResetEvent( HANDLE hEvent/* handle to event */ )
{
	BOOL bRet = FALSE;

	bRet = ResetEvent( hEvent );

	return bRet;
}

void KPIGetExePath( LPSTR lpExePath, DWORD dwSize )
{
	if ( lpExePath )
	{
		memset( lpExePath, 0, dwSize );
		GetModuleFileName( NULL, lpExePath, dwSize );
		DWORD dwEndAt;
		dwEndAt = strlen( lpExePath ) - 1;
		while ( dwEndAt >= 1 )
		{
			if ( lpExePath[dwEndAt-1] == '\\' )
			{
				lpExePath[dwEndAt] = 0;
				break;
			}
			dwEndAt--;
		}
	}
}

HANDLE KPICreateMutex( LPSECURITY_ATTRIBUTES lpMutexAttributes,	/* SD */
					  BOOL bInitialOwner,						/* initial owner */
					  LPCTSTR lpName							/* object name */ )
{
	return CreateMutex( lpMutexAttributes, bInitialOwner, lpName );
}

BOOL KPIReleaseMutex( HANDLE hMutex )
{
	return ReleaseMutex( hMutex );
}

unsigned long KPIHash( char* pStr, unsigned long ulModel, unsigned long ulBaseNum )
{
	assert( ( NULL != pStr ) && ( 0 < ulModel ) );
    unsigned long i = 0;
    unsigned long j = 1;
    while ( *pStr )
	{
		i += ( ( unsigned char )*pStr ) * j++;
		pStr++;
	}
    return ( i % ulModel + ulBaseNum );
}

void KPIPrintToFile( const TCHAR* lpszFilePath, const TCHAR* lpszFmt, ... )
{
	if ( ( NULL == lpszFilePath )
		|| ( NULL == lpszFmt ) )
	{
		return;
	}

	FILE* pF = fopen( lpszFilePath, "a+b" );
	if ( NULL != pF )
	{
		fseek( pF, 0, SEEK_END );

		va_list args;
		va_start( args, lpszFmt );
		vfprintf( pF, lpszFmt, args );
		va_end ( args );

		fclose( pF );
	}
}

void KPIPrintToFile( FILE* pFile, const TCHAR* lpszFmt, ... )
{
	if ( ( NULL == pFile )
		|| ( NULL == lpszFmt ) )
	{
		return;
	}

	va_list args;
	va_start( args, lpszFmt );
	vfprintf( pFile, lpszFmt, args );
	va_end ( args );
}
