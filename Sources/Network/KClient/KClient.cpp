// testESClient.cpp : Defines the entry point for the console application.
//

#include <winsock2.h>

#include "ESClient\Exception.h"
#include "ESClient\Utils.h"
#include "ESClient\ManualResetEvent.h"

#include "GameClient.h"

//[ Include in .\..\Protocol
#include "Protocol\Protocol.h"
//]

using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CManualResetEvent;

int main(int argc, char* argv[])
{
	try
	{
		CGameClient client( 10, 1024 );

		client.ConnectTo( "127.0.0.1", 5001 );

		client.StartConnections();
	
		char szBuffer[1000] = {0};
		
		memset( szBuffer, '.', 1000 );
		memcpy( szBuffer, "BEGIN", strlen("BEGIN") );
		memcpy( szBuffer+1000-strlen("END"), "END", 3 );
		
		client.Write( szBuffer, 1000 );
	
		CManualResetEvent shutdownEvent(_T("OnlineGameClientShutdown"), false);
		CManualResetEvent pauseResumeEvent(_T("OnlineGameClientPauseResume"), false);
		
		HANDLE handlesToWaitFor[2];
		
		handlesToWaitFor[0] = shutdownEvent.GetEvent();
		handlesToWaitFor[1] = pauseResumeEvent.GetEvent();

		bool accepting = true;
		bool done = false;
		
		while (!done)
		{
			DWORD waitResult = ::WaitForMultipleObjects(2, handlesToWaitFor, false, INFINITE);

			if (waitResult == WAIT_OBJECT_0)
			{
				done = true;
			}
			else if (waitResult == WAIT_OBJECT_0 + 1)
			{
				if (accepting)
				{
					client.StopConnections();
				}
				else
				{
					client.StartConnections();
				}
				
				accepting = !accepting;
			}
			else
			{
				Output(_T("Unexpected result from WaitForMultipleObjects - exiting"));
				done = true;
			}
		}
			
		client.WaitForShutdownToComplete();
	}
	catch(const CException &e)
	{
		Output(_T("Exception: ") + e.GetWhere() + _T(" - ") + e.GetMessage());
	}
	catch(...)
	{
		Output(_T("Unexpected exception"));
	}
	
	return 0;
}

