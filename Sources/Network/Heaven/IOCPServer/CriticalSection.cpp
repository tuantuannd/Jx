/********************************************************************
	created:	2003/02/13
	file base:	CriticalSection
	file ext:	cpp
	author:		liupeng
	
	purpose:	
*********************************************************************/
#include "CriticalSection.h"
#include "Utils.h"

/* 
 * Member not defined
 *		CCriticalSection::CCriticalSection
 *		CCriticalSection::operator= 
 *		Owner::Owner
 *		Owner::operator=
 */


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

void CCriticalSection::Enter()
{
	::EnterCriticalSection( &m_crit );
}

void CCriticalSection::Leave()
{
	::LeaveCriticalSection( &m_crit );
}

///////////////////////////////////////////////////////////////////////////////
// CCriticalSection::Owner
///////////////////////////////////////////////////////////////////////////////

CCriticalSection::Owner::Owner(
   CCriticalSection &crit)
   : m_crit(crit)
{
	m_crit.Enter();
}

CCriticalSection::Owner::~Owner()
{
	m_crit.Leave();
}

} // End of namespace OnlineGameLib
} // End of namespace Win32