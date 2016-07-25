/********************************************************************
	created:	2002/10/16
	file base:	global
	file ext:	cpp
	author:		liupeng
	
	purpose:	The file contains definitions, macros, and structures 
				to help me write gameengine

*********************************************************************/
#include "stdafx.h"
#include "macro.h"
#include "stdio.h"

#ifdef _DEBUG

void _trace(char *fmt, ...)
{
	char string[1024] = {0};
	char out[1024] = {0};

	va_list body;
	va_start(body, fmt);

	vsprintf(string, fmt, body);
	va_end(body);

	if ( strlen( string ) )
	{
		sprintf( out, "[TRACE]: %s", string );
		OutputDebugString(out);
	}
}

#endif