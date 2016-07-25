#include "stdafx.h"
#include "IniFile.h"
#include "Utils.h"
#include "Macro.h"

#define TCS_STRINGBUFFER	2048
#define TCS_KEYBUFFER		256		// This buffer length for store key value
#define TCS_INTBUFFER		16		// This buffer length for store integer value

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CIniFile::CIniFile()
{
	memset( m_szFileName, 0, MAX_PATH );
}

CIniFile::CIniFile( LPCTSTR pFileName )
{
	ASSERT( pFileName );
	
	strcpy( m_szFileName, pFileName );
}

CIniFile::~CIniFile()
{
}

void CIniFile::SetFile( LPCTSTR pFileName )
{
	ASSERT( pFileName );
	
	strcpy( m_szFileName, pFileName );	
}

int	 CIniFile::ReadInteger( LPCTSTR lpSectionName, LPCTSTR lpKeyName, int nDefault )
{
	char szStr[TCS_KEYBUFFER];
	szStr[0] = 0;
	
	ReadString( lpSectionName, lpKeyName, szStr, TCS_KEYBUFFER, NULL );

	if( 0 != szStr[0] )
	{
		return atoi( szStr );
	}

	return nDefault;	
}

BOOL CIniFile::WriteInteger( LPCTSTR lpSectionName, LPCTSTR lpKeyName, int nValue )
{
	char szStrOut[TCS_INTBUFFER];

	wsprintf( szStrOut, "%d", nValue );

	return ::WritePrivateProfileString( lpSectionName, lpKeyName, szStrOut, m_szFileName );
}

DWORD CIniFile::ReadHexNum( LPCTSTR lpSectionName, LPCTSTR lpKeyName, DWORD dwDefault )
{
	char szStr[TCS_KEYBUFFER];
	szStr[0] = 0;
	
	ReadString( lpSectionName, lpKeyName, szStr, TCS_KEYBUFFER, NULL );

	if( 0 != szStr[0] )
	{
		return HexStringToDWORD( reinterpret_cast< LPCTSTR >( szStr ) );
	}

	return dwDefault;	
}

BOOL CIniFile::WriteHexNum( LPCTSTR lpSectionName, LPCTSTR lpKeyName, DWORD dwValue )
{
	char szStrOut[TCS_INTBUFFER];

	wsprintf( szStrOut, "%8.8X", dwValue );

	return ::WritePrivateProfileString( lpSectionName, lpKeyName, szStrOut, m_szFileName );
}

DWORD CIniFile::ReadString( LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPTSTR lpString, DWORD dwSize, LPCTSTR lpDefault )
{
	ASSERT( lpSectionName && lpKeyName );

	return ::GetPrivateProfileString( lpSectionName, lpKeyName, lpDefault, lpString, dwSize, m_szFileName );
}

BOOL CIniFile::WriteString( LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpString )
{
	ASSERT( lpSectionName && lpKeyName );

	return ::WritePrivateProfileString( lpSectionName, lpKeyName, lpString, m_szFileName );
}

BOOL CIniFile::ReadSections( _VETSTR& stdSections )
{
	LPVOID pvData = NULL;

	HGLOBAL hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, 16385 );

	ASSERT( hGlobal );

	pvData = ::GlobalLock( hGlobal );

	ASSERT( pvData );

	stdSections.clear();

	if ( ::GetPrivateProfileString( NULL, NULL, NULL, ( char * )pvData, 16384, m_szFileName ) )
	{
        char *pSection = ( char * ) pvData;

        while ( 0 != *pSection )
		{
			stdSections.push_back( pSection );

			pSection += strlen( pSection ) + 1;
		}
	}

	::GlobalUnlock( hGlobal );
	::GlobalFree( hGlobal );

	return ( BOOL )( stdSections.size() > 0 );
}

BOOL CIniFile::ReadSection( LPCTSTR lpSection, _VETSTR& stdKey )
{
	LPVOID pvData = NULL;

	HGLOBAL hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, 16385 );

	ASSERT( hGlobal );

	pvData = ::GlobalLock( hGlobal );

	ASSERT( pvData );

	stdKey.clear();
	
	if ( ::GetPrivateProfileString( lpSection, NULL, NULL, ( char * ) pvData, 16384, m_szFileName ) )
	{
        char *pKey = ( char * )pvData;

        while ( 0 != *pKey )
		{
			stdKey.push_back( pKey );

			pKey += strlen( pKey ) + 1;
		}
	}

	::GlobalUnlock( hGlobal );
	::GlobalFree( hGlobal );

	return ( BOOL )( stdKey.size() > 0 );
}

} // End of namespace OnlineGameLib
} // End of namespace Win32	
	