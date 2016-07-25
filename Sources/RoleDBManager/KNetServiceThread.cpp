#include "stdafx.h"
#include "KDBProcessThread.h"
//	
DWORD WINAPI 	RoleNetWorkServiceThreadFun(void * pParam)
{

	g_NetServer.Init();
	while(1)
	{
		g_NetServerMark ++;
		if (g_NetServerMark > MAX_MARK_COUNT) g_NetServerMark = 0;
		try
		{
			g_NetServer.Receive();
			g_NetServer.Send();
			Sleep(1);
		}
		catch(...)
		{
			g_nMistakeCount ++;
		}
	}
	
}
