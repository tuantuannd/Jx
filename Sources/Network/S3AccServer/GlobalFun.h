//-----------------------------------------//
//                                         //
//  File		: KStdAfx.h                //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/14/2002                //
//                                         //
//-----------------------------------------//
#ifndef _GLOBALFUN_H_
#define _GLOBALFUN_H_

#include "GlobalDTD.h"
#include <WTypes.h>
#include <stdio.h>


BOOL KPIWritePrivateProfileString( LPCTSTR lpszAppName,	/* section name */
								  LPCTSTR lpszKeyName,	/* key name */
								  LPCTSTR lpszString,	/* string to add */
								  LPCTSTR lpszFileName	/* initialization file */ );

DWORD KPIGetPrivateProfileString( LPCTSTR lpszAppName,	/* section name(Input) */
							  LPCTSTR lpszKeyName,		/* key name(Input) */
							  LPCTSTR lpszDefault,		/* default string(Input) */
							  LPTSTR lpszReturnedString,/* destination buffer(Output) */
							  DWORD dwSize,				/* size of destination buffer(Input) */
							  LPCTSTR lpszFileName		/* initialization file name(Input) */ );

HANDLE KPICreateThread( LPTHREAD_START_ROUTINE lpStartAddress,	/* thread function */
					   LPVOID lpParameter,						/* thread argument */
					   LPDWORD lpThreadId						/* thread identifier */ );

BOOL KPICloseHandle( HANDLE hObject );

DWORD KPIWaitForSingleObject( HANDLE hHandle, DWORD dwMilliseconds );

BOOL KPITerminateThread( HANDLE hThread, DWORD dwExitCode );

SOCKET KPICreateUDPSocket( int iPort );

HANDLE KPICreateEvent( LPSECURITY_ATTRIBUTES lpEventAttributes,	/* SD */
					  BOOL bManualReset,						/* reset type */
					  BOOL bInitialState,						/* initial state */
					  LPCTSTR lpName							/* object name */ );

BOOL KPISetEvent( HANDLE hEvent/* handle to event */ );

BOOL KPIResetEvent( HANDLE hEvent/* handle to event */ );

void KPIGetExePath( LPSTR lpExePath, DWORD dwSize );

HANDLE KPICreateMutex( LPSECURITY_ATTRIBUTES lpMutexAttributes,	/* SD */
					  BOOL bInitialOwner,						/* initial owner */
					  LPCTSTR lpName							/* object name */ );

BOOL KPIReleaseMutex( HANDLE hMutex );


unsigned long KPIHash( char* pStr, unsigned long ulModel, unsigned long ulBaseNum );

void KPIPrintToFile( const TCHAR* lpszFilePath, const TCHAR* lpszFmt, ... );
void KPIPrintToFile( FILE* pFile, const TCHAR* lpszFmt, ... );

int gTrace(LPCSTR fmt, ...);

#endif	// _GLOBALFUN_H_