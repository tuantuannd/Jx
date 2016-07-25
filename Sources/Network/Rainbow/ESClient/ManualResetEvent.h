/********************************************************************
	created:	2003/02/14
	file base:	ManualResetEvent
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_MANUALRESETEVENT_H__
#define __INCLUDE_MANUALRESETEVENT_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include "Event.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

class CManualResetEvent : public CEvent
{
public:
	
	explicit CManualResetEvent( bool initialState = false );	
	explicit CManualResetEvent( const _tstring &name, bool initialState = false );
	
private:
	
	/*
	 * No copies do not implement
	 */
	CManualResetEvent( const CManualResetEvent &rhs );
	CManualResetEvent &operator=( const CManualResetEvent &rhs );
};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_MANUALRESETEVENT_H__