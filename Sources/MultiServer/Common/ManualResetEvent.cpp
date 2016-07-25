#include "stdafx.h"
#include "ManualResetEvent.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CManualResetEvent::CManualResetEvent( bool initialState /* = false */ )
   : CEvent( 0, true, initialState )
{

}

CManualResetEvent::CManualResetEvent( const char *pName, bool initialState /* = false */ )
   : CEvent( 0, true, initialState, pName )
{
   
}

} // End of namespace OnlineGameLib
} // End of namespace Win32