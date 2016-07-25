#include "stdafx.h"
#include "library.h"
#include "Macro.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CLibrary::CLibrary( const char * const lpFileName )
{
	m_hInst = NULL;
	
	Load( lpFileName );
}

CLibrary::CLibrary()
{
	m_hInst = NULL;
}

CLibrary::~CLibrary()
{
	Free();
}

bool CLibrary::Load( const char * const lpFileName )
{
	ASSERT( lpFileName );

	m_sFileName = lpFileName;

	m_hInst = ::LoadLibrary( lpFileName );

	return m_hInst ? true : false;
}

bool CLibrary::Free()
{
	if ( m_hInst )
	{
		::FreeLibrary( m_hInst );
		m_hInst = NULL;
	}

	return true;
}

} // End of namespace OnlineGameLib
} // End of namespace Win32