// testIOCPServer.cpp : Defines the entry point for the console application.
//
#include <winsock2.h>

#include "IOCPServer\Exception.h"
#include "IOCPServer\Utils.h"
#include "IOCPServer\ManualResetEvent.h"

#include "GameServer.h"
#include "JXServer.h"

using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CManualResetEvent;

int main(int argc, char* argv[])
{
	ServerStartup( INADDR_ANY, 5001, true );

	while ( true )
	{
		SERVER(Begin);

		SendToClient( 0, "test", 4 );
		::Sleep( 1 );

		SERVER(End);
	}	

	ServerCleanup();
	
	return 0;
}

