// S3Server.cpp : Defines the entry point for the application.
#include "KEngine.h"
#include "..\MultiServer\Heaven\Interface\IServer.h"
#include "SwordOnLineServer.h"
#include "ServerLocalControlWnd.h"

/*
 * Add this macro by liupeng on 2003.4.22
 * This macro is helper that can judge some legal character
 */
#define _private_IS_SPACE(c)   ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t' || (c) == 'x')
#define IS_SPACE(c)	_private_IS_SPACE(c)


KSwordOnLineSever		l_Server;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	if (!l_Server.Init())
		return 0;
	if (KServerLocalControlWnd::Create(hInstance, &l_Server) == false)
		return 0;
	
	/*
	 * Add this funtion by liupeng on 2003.4.22
	 * We can find some error when start a console tracer
	 */
#ifdef	TRUE

	bool bOpenTracer = false;

    while( lpCmdLine[0] == '-' || lpCmdLine[0] == '/' )
    {
        lpCmdLine++;
		
        switch ( *lpCmdLine++ )
        {
		case 'c':
        case 'C':
            bOpenTracer = true;
            break;
        }
		
        while( IS_SPACE( *lpCmdLine ) )
        {
            lpCmdLine++;
        }
    }
	
	if ( bOpenTracer ) 
	{
		AllocConsole();
	}

#endif // End of this function

	//----ÏûÏ¢Ñ­»·----
	MSG	Msg;
	while (true)
	{
		if (PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&Msg, NULL, 0, 0))
				break;
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else if(l_Server.Breathe())
			KServerLocalControlWnd::UpdateData();
	}

#ifdef TRUE

	if ( bOpenTracer )
	{
		FreeConsole();
	}

#endif

	return 0;
}

void __stdcall ServerCallBack(LPVOID lpParam, const unsigned long &ulnID, const unsigned long &uEventType)
{
	if (ulnID > l_Server.GetMaxPlayer())
		return;
	
	switch(uEventType)
	{
	case enumClientConnectCreate:
		//m_NetConnectStatus[ulnID] = NET_CONNECTED;
		l_Server.SetNetStatus(ulnID, NET_CONNECTED);
		break;
	case enumClientConnectClose:
		l_Server.SetNetStatus(ulnID, NET_UNCONNECT);
		break;
	default:
		break;
	}
}
