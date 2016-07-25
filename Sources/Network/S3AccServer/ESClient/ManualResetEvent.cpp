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

CManualResetEvent::CManualResetEvent( const _tstring &name, bool initialState /* = false */ )
   : CEvent( 0, true, initialState, name )
{
   
}

} // End of namespace OnlineGameLib
} // End of namespace Win32