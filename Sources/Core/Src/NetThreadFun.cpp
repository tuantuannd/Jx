#define _WIN32_WINNT 0x0400
#ifdef _SERVER
#include "KCore.h"
#include "KThread.h"
#include "KPlayer.h"
#include "KNetServer.h"
#include "CoreServerDataDef.h"

void* NetReceiveThreadFun(void* pParam)
{
	LARGE_INTEGER	t4, t5;
	while(1)
	{
		g_pNetServer->CheckConnect();
		QueryPerformanceCounter(&t4);

		for (int i = 0; i < MAX_CLIENT_NUMBER; i++)
		{
			g_pNetServer->RecieveMessage(i, RECIEVE_BUFFER_SIZE);
		}
		g_pNetServer->RecvDataToTemp();

		QueryPerformanceCounter(&t5);
		g_DebugLog("ReceiveTime: %d", t5.QuadPart - t4.QuadPart);
		SwitchToThread();

	}
	
	return NULL;
}

void* NetSendThreadFun(void* pParam)
{
	LARGE_INTEGER	t1, t2;
	while(1)
	{
		QueryPerformanceCounter(&t1);
		g_pNetServer->CopyTempToDest();
		for (int i = 0; i < MAX_CLIENT_NUMBER; i++)
		{
			g_pNetServer->SendDest(i);
		}		
		QueryPerformanceCounter(&t2);
		g_DebugLog("SendTime:%d", t2.QuadPart - t1.QuadPart);
		SwitchToThread();
	
	}
	return NULL;
}
#endif
