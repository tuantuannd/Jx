// Bishop.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Application.h"

#include "Macro.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	/*
	 * Add this funtion by liupeng on 2003
	 * We can find some error when start a console tracer
	 *
	 * Please check 'project-setting'
	 */
#ifdef	CONSOLE_DEBUG

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

	cprintf( "Startup application!\n" );

#endif

	CBishopApp app( hInstance );

	int nRet = app.Run();

#ifdef CONSOLE_DEBUG

	cprintf( "End of application!\n" );

	if ( bOpenTracer )
	{
		FreeConsole();
	}

#endif
	
	return nRet;
}



