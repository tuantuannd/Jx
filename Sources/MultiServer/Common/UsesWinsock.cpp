#include "stdafx.h"
#include "UsesWinsock.h"
#include "Win32Exception.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CUsesWinsock::CUsesWinsock()
{
	WORD wVersionRequested = 0x202;
	
	if ( 0 != ::WSAStartup( wVersionRequested, &m_data ) )
	{
		throw CWin32Exception( _T("CUsesWinsock::CUsesWinsock()"), ::WSAGetLastError() );
	}
}
      
CUsesWinsock::~CUsesWinsock()
{
	::WSACleanup();
}

} // End of namespace OnlineGameLib
} // End of namespace Win32