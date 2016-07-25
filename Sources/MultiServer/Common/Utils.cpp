#include "stdafx.h"
#include "Utils.h"
#include "Exception.h"
#include "CriticalSection.h"
#include "Macro.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <conio.h>

#ifdef _UNICODE
	typedef std::wfstream _tfstream;
#else
	typedef std::fstream _tfstream;
#endif

#include "Lmcons.h"     // define UNLEN

#define HOSTNAME_SIZE	MAX_PATH

/*
 * Using directives
 */
using std::auto_ptr;
using std::endl;

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CCriticalSection s_criticalSection;

static _tfstream s_debugOut;

static std::string s_logFileName = "\\common.log";

void SetLogFileName( const _tstring &name )
{
	USES_CONVERSION;

	if ( s_debugOut.is_open() )
	{
		s_debugOut.close();
	}

	s_logFileName = T2A( const_cast< PTSTR >( name.c_str() ) );
}

void Message( const char * const pInfo )
{
	if ( pInfo )
	{
		::MessageBeep( -1 );

		::MessageBox( NULL, pInfo, "Info", MB_ICONINFORMATION );
	}
}

void Output( const _tstring &message )
{
	return;
	
#ifdef _DEBUG
	CCriticalSection::Owner lock( s_criticalSection );

#ifdef _UNICODE
	std::wcout << ToString( GetCurrentThreadId() ) << _T(": ") << message << endl;
#else
	std::cout << ToString( GetCurrentThreadId() ) << _T(": ") << message << endl;
#endif

	const _tstring msg = ToString(GetCurrentThreadId()) + _T(": ") + message + _T("\n");

	::OutputDebugString( msg.c_str() );

/*	if (!s_debugOut.is_open())
	{
		s_debugOut.open(s_logFileName.c_str(), std::ios_base::out | std::ios_base::app);

		s_debugOut << _T("=======================================") << endl;
	}
*/
	s_debugOut <<  ToString( GetCurrentThreadId() ) << _T(": ") << message << endl;
#else
   // NULL
#endif //_DEBUG
}

void Trace( const _tstring &progID, const _tstring &message )
{
#ifdef _DEBUG
	const _tstring msg = ToString(GetCurrentThreadId()) + _T(" [") + progID + _T("] ") + message + _T("\n");

	cprintf( msg.c_str() );
#endif
}

_tstring GetLastErrorMessage( DWORD last_error )
{
	TCHAR errmsg[512];

	if ( !FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 
				  0,
				  last_error,
				  0,
				  errmsg, 
				  511,
				  NULL) )
	{
		/*
		 * if we fail, call ourself to find out why and return that error 
		 */
		return ( GetLastErrorMessage( GetLastError() ) );  
	}
  
	return ( _tstring )errmsg;
}


_tstring HexToString( const BYTE *pBuffer, size_t iBytes )
{
	_tstring result;
     
	for ( size_t i = 0; i < iBytes; i++ )
	{
		BYTE c ;
		
		BYTE b = pBuffer[i] >> 4;
		
		if ( 9 >= b )
		{
			c = b + '0';
		}
		else
		{
			c = ( b - 10 ) + 'A';
		}
		
		result += (TCHAR)c;
		
		b = pBuffer[i] & 0x0f;
		
		if ( 9 >= b )
		{
			c = b + '0';
		}
		else
		{
			c = ( b - 10 ) + 'A';
		}
		
		result += ( TCHAR )c;
	}

   return result;
}

void StringToHex( const _tstring &ts, BYTE *pBuffer, size_t nBytes )
{
	USES_CONVERSION;

	const std::string s = T2A( const_cast< PTSTR >( ts.c_str() ) );

	for ( size_t i = 0; i < nBytes; i++ )
	{
		const size_t stringOffset = i * 2;

		BYTE val = 0;

		const BYTE b = s[stringOffset];

		if ( isdigit(b) )
		{
			val = ( BYTE )( (b - '0') * 16 );
		}
		else 
		{
			val = ( BYTE )( ( ( toupper(b) - 'A') + 10 ) * 16 ); 
		}
		
		const BYTE b1 = s[stringOffset + 1];
		
		if ( isdigit( b1 ) )
		{
			val += b1 - '0' ; 
		}
		else 
		{
			val += ( BYTE )( ( toupper(b1) - 'A' ) + 10 );
		}
		
		pBuffer[i] = val;
	}
}

DWORD HexStringToDWORD( LPCTSTR lpString )
{
	int nLen = strlen( lpString );

	DWORD dwValue = 0;

	for( int i = 0; i < nLen; i++ )
	{
		dwValue = dwValue*16;

		char c = *( lpString+i );

		if ( c >= '0' && c <= '9' )
		{
			dwValue = dwValue + c - '0';
		}
		else if( c >= 'a' && c <= 'f' )
		{
			dwValue = dwValue + c - 'a' + 10;
		}
		else if( c >= 'A' && c <= 'F' )
		{
			dwValue = dwValue + c - 'A' + 10;
		}
		else
		{
			ASSERT( FALSE );
		}
	}

	return dwValue;
}

DWORD HashStr2ID( const char * const pStr )
{
	if ( pStr )
	{
		DWORD dwID = 0;
		
		for ( int i = 0; pStr[i]; i++ )
		{
			dwID = ( dwID + ( i + 1 ) * pStr[i] ) % 0x8000000B * 0xFFFFFFEF;
		}

		return ( dwID ^ 0x12345678 );	
	}

	return ( DWORD )( -1 );
}

unsigned long net_aton( const char *pAddr )
{
	static char szAddr[16];
	static char seps[] = " .";

	char *token = NULL;

	unsigned long lnAddr = 0;
	unsigned long lnSeg = 0;
	int nPos = 0;

	if ( !pAddr )
	{
		return 0;
	}

	size_t len = strlen( pAddr );
	len = ( len > 15 ) ? 15 : len;

	memcpy( szAddr, pAddr, len );
	szAddr[len] = '\0';

	token = strtok( szAddr, seps );

	while( token != NULL && nPos < 4 )
	{
		/*
		 * While there are tokens in "string"
		 */
		lnSeg = atoi( token );
		lnAddr |= ( lnSeg & 0xFF ) << ( nPos * 8 );

		/*
		 * Get next token
		 */
		token = strtok( NULL, seps );

		nPos ++;
   }

	return lnAddr;
}

const char *net_ntoa( unsigned long lnAddr )
{
	static char szAddr[16];

	unsigned long lnSeg = lnAddr;

	sprintf( szAddr, 
		"%u.%u.%u.%u", 
		lnAddr & 0xFF,
		( lnAddr >> 8 ) & 0xFF,
		( lnAddr >> 16 ) & 0xFF,
		( lnAddr >> 24 ) & 0xFF );

	szAddr[15] = '\0';

	return szAddr;
}

_tstring GetCurrentDirectory()
{
	DWORD size = ::GetCurrentDirectory(0, 0);
	
	auto_ptr<TCHAR> spBuf( new TCHAR[size] );
	
	if ( 0 == ::GetCurrentDirectory( size, spBuf.get() ) )
	{
		throw CException( _T("GetCurrentDirectory()"), _T("Failed to get current directory") );
	}
	
	return _tstring( spBuf.get() );
}

_tstring GetAppFullPath( HINSTANCE hInst )
{
	static bool gotName = false;
	
	static _tstring name;
	
	if (!gotName)
	{
		TCHAR moduleFileName[MAX_PATH + 1] ;
		DWORD moduleFileNameLen = MAX_PATH ;
		
		if ( ::GetModuleFileName( hInst, moduleFileName, moduleFileNameLen ) )
		{
			int nLen = strlen( moduleFileName );

			while ( nLen -- > 0 )
			{
				if ( '\\' == moduleFileName[nLen] )
				{
					moduleFileName[nLen+1] = '\0';
					
					break;
				}
			}
			name = moduleFileName;
		}
		
		gotName = true;
	}
	
	return name;	
}

_tstring GetDateStamp()
{
	SYSTEMTIME systime;
	GetSystemTime( &systime );
	
	static TCHAR buffer[7];
	
	_stprintf(buffer, _T("%02d%02d%02d"),
				systime.wDay,
				systime.wMonth,
				( 1900 + systime.wYear) % 100);
	
	return buffer;
}

_tstring GetTimeStamp()
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	
	static TCHAR buffer[9];
	
	_stprintf( buffer, _T("%02d:%02d:%02d"),
			systime.wHour,
			systime.wMinute,
			systime.wSecond );
	
	return buffer;
}

_tstring ToHex( BYTE c )
{
	TCHAR hex[3];
	
	const int val = c;
	
	_stprintf( hex, _T("%02X"), val );
	
	return hex;
}

_tstring DumpData( const BYTE * const pData, size_t dataLength, size_t lineLength /* = 0 */ )
{
	const size_t bytesPerLine = lineLength != 0 ? (lineLength - 1) / 3 : 0;
	
	_tstring result;
	
	_tstring hexDisplay;
	_tstring display;
	
	size_t i = 0;
	
	while ( i < dataLength )
	{
		const BYTE c = pData[i++];
		
		hexDisplay += ToHex( c ) + _T(" ");
		
		if ( isprint( c ) )
		{
			display += (TCHAR)c;
		}
		else
		{
			display += _T('.');
		}
		
		if ( ( bytesPerLine && ( i % bytesPerLine == 0 && i != 0 ) ) || i == dataLength )
		{
			if ( i == dataLength && ( bytesPerLine && ( i % bytesPerLine != 0 ) ) )
			{
				for ( size_t pad = i % bytesPerLine; pad < bytesPerLine; pad++ )
				{
					hexDisplay += _T("   ");
				}
			}

			result += hexDisplay + _T(" - ") + display + _T("\n");
			
			hexDisplay = _T("");
			display = _T("");
		}
	}
	
	return result;
}

_tstring GetComputerName()
{
	static bool gotName = false;
	
	static _tstring name = _T("UNAVAILABLE");
	
	if ( !gotName )
	{
		TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1] ;
		DWORD computerNameLen = MAX_COMPUTERNAME_LENGTH ;
		
		if ( ::GetComputerName( computerName, &computerNameLen ) )
		{
			name = computerName;
		}
		
		gotName = true;
	}
	
	return name;
}

_tstring GetModuleFileName( HINSTANCE hModule /* = 0 */ )
{
	static bool gotName = false;
	
	static _tstring name = _T("UNAVAILABLE");
	
	if (!gotName)
	{
		TCHAR moduleFileName[MAX_PATH + 1] ;
		DWORD moduleFileNameLen = MAX_PATH ;
		
		if ( ::GetModuleFileName( hModule, moduleFileName, moduleFileNameLen ) )
		{
			name = moduleFileName;
		}
		
		gotName = true;
	}
	
	return name;
}

_tstring GetUserName()
{
	static bool gotName = false;
	
	static _tstring name = _T("UNAVAILABLE");
	
	if ( !gotName )
	{
		TCHAR userName[UNLEN + 1] ;
		DWORD userNameLen = UNLEN;
		
		if ( ::GetUserName( userName, &userNameLen ) )
		{
			name = userName;
		}
		
		gotName = true;
	}
	
	return name;
}

_tstring StripLeading( const _tstring &source, const char toStrip )
{
	const TCHAR *pSrc = source.c_str();
	
	while ( pSrc && *pSrc == toStrip )
	{
		++pSrc;
	}
	
	return pSrc;
}

_tstring StripTrailing( const _tstring &source, const char toStrip )
{
	size_t i = source.length();
	const _TCHAR *pSrc = source.c_str() + i;
	
	--pSrc;
	
	while ( i && *pSrc == toStrip )
	{
		--pSrc;
		--i;
	}
	
	return source.substr( 0, i );
}

#pragma comment(lib, "Version.lib")

typedef struct LANGANDCODEPAGE
{
	WORD wLanguage;
	WORD wCodePage;

}LCP, NEAR *PLCP, FAR *LPLCP;

_tstring GetFileVersion()
{
	static bool gotName = false;
	static _tstring version;
	
	if ( gotName )
	{
		return version;
	}
	
	const _tstring moduleFileName = GetModuleFileName( NULL );
	
	LPTSTR pModuleFileName = const_cast< LPTSTR >( moduleFileName.c_str() );
	
	DWORD zero = 0;
	
	DWORD verSize = ::GetFileVersionInfoSize( pModuleFileName, &zero );
	
	if ( verSize != 0 )
	{
		auto_ptr< BYTE > spBlock( new BYTE[verSize] );
		
		LPLCP	lpTranslate = NULL;
		UINT	nTranslateSize = 0;
		
		/*
		 * How many language in this product?
		 */
		if ( ::GetFileVersionInfo( pModuleFileName, 0, verSize, spBlock.get() ) )
		{		   
			if ( ::VerQueryValue( spBlock.get(), 
				const_cast< LPSTR >( _T("\\VarFileInfo\\Translation") ),
				( void ** )( &lpTranslate ),
				&nTranslateSize ) )
			{
				if ( ( nTranslateSize / sizeof( LCP ) ) > 0 )
				{
					CHAR szSubBlock[64];
				
					/*
					 * get version information from first language.
					 */
					wsprintf( szSubBlock, 
						TEXT( "\\StringFileInfo\\%04x%04x\\FileVersion" ),
						lpTranslate[0].wLanguage,
						lpTranslate[0].wCodePage);
					
					auto_ptr<BYTE> spBuffer( new BYTE[verSize] );
					
					if ( ::GetFileVersionInfo( pModuleFileName, 0, verSize, spBuffer.get() ) )
					{
						LPTSTR pVersion = 0;
						UINT verLen = 0;
						
						if ( ::VerQueryValue( spBuffer.get(), 
							szSubBlock,
							( void ** )( &pVersion ),
							&verLen ) )
						{
							version = pVersion;
							
							gotName = true;
						}
					}
				}
			}
		}
	}
	
	return version;
}

/*
 *	Get local computer name.  Something like: "mycomputer.myserver.net"
 */
bool GetLocalName( LPTSTR strName, UINT nSize )
{
	if ( strName && nSize > 0 )
	{
		char strHost[HOSTNAME_SIZE] = { 0 };

		/*
		 * get host name, if fail, SetLastError is set
		 */
		if ( SOCKET_ERROR != gethostname( strHost, sizeof( strHost ) ) )
		{
			struct hostent* hp;

			hp = gethostbyname( strHost );

			if ( hp )
			{
				strcpy( strHost, hp->h_name );
			}

			/*
			 * check if user provide enough buffer
			 */
			if ( strlen(strHost) > nSize )
			{
				::SetLastError( ERROR_INSUFFICIENT_BUFFER );

				return false;
			}

			/*
			 * Unicode conversion
			 */
#ifdef _UNICODE
			return ( 0 != MultiByteToWideChar( CP_ACP, 
				0,
				strHost, 
				-1,
				strName, 
				nSize, 
				NULL, 
				NULL ) );
#else
			_tcscpy( strName, strHost );

			return true;
#endif
		}
	}
	else
	{
		::SetLastError( ERROR_INVALID_PARAMETER );
	}

	return false;
}


/*
 * Get TCP address of local computer in dot format ex: "127.0.0.0"
 */
bool GetLocalAddress( LPTSTR strAddress, UINT nSize, int nAdapter /* 0 */ )
{
	/*
	 * Get computer local address
	 */
	if ( strAddress && nSize > 0 )
	{
		CHAR strHost[HOSTNAME_SIZE] = { 0 };

		/*
		 * get host name, if fail, SetLastError is called
		 */
		if ( SOCKET_ERROR != gethostname( strHost, sizeof( strHost ) ) )
		{
			struct hostent* hp;

			hp = gethostbyname( strHost );

			if ( hp && hp->h_addr_list[nAdapter] )
			{
				/*
				 * Address is four bytes (32-bit)
				 */
				if ( hp->h_length < 4 )
				{
					return false;
				}

				/*
				 * Convert address to . format
				 */
				strHost[0] = 0;

				/*
				 * Create Address string
				 */
				sprintf( strHost, "%u.%u.%u.%u",
					( UINT )( ( ( PBYTE ) hp->h_addr_list[nAdapter])[0] ),
					( UINT )( ( ( PBYTE ) hp->h_addr_list[nAdapter])[1] ),
					( UINT )( ( ( PBYTE ) hp->h_addr_list[nAdapter])[2] ),
					( UINT )( ( ( PBYTE ) hp->h_addr_list[nAdapter])[3] ) );

				/*
				 * check if user provide enough buffer
				 */
				if ( strlen( strHost ) > nSize )
				{
					::SetLastError( ERROR_INSUFFICIENT_BUFFER );

					return false;
				}

				/*
				 * Unicode conversion
				 */
#ifdef _UNICODE
				return ( 0 != MultiByteToWideChar( CP_ACP, 
					0, 
					strHost, 
					-1, 
					strAddress,
					nSize, 
					NULL, 
					NULL ) );
#else
				_tcscpy( strAddress, strHost );

				return true;
#endif
			}
		}
	}
	else
	{
		::SetLastError( ERROR_INVALID_PARAMETER );
	}

	return false;
}

} // End of namespace OnlineGameLib
} // End of namespace Win32