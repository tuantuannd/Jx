/********************************************************************
	created:	2003/02/13
	file base:	CriticalSection
	file ext:	cpp
	author:		liupeng
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "CriticalSection.h"
#include "Utils.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CCriticalSection::CCriticalSection()
{
	::InitializeCriticalSection( &m_crit );
}
      
CCriticalSection::~CCriticalSection()
{
	::DeleteCriticalSection( &m_crit );
}

#if ( _WIN32_WINNT >= 0x0400 )
bool CCriticalSection::TryEnter()
{
	return BOOL_to_bool( ::TryEnterCriticalSection( &m_crit ) );
}
#endif

/*
 * CCriticalSection::Owner
 */

CCriticalSection::Owner::Owner( CCriticalSection &crit )
		: m_crit( crit )
{
	m_crit.Enter();
}

CCriticalSection::Owner::~Owner()
{
	m_crit.Leave();
}
      
} // End of namespace OnlineGameLib
} // End of namespace Win32