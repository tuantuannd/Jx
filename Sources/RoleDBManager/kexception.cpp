#include "stdafx.h"
#include "KException.h"

/*
 * KException
 */
KException::KException( const char*  where, const  char *message )
{
	strcpy(m_where ,where);
	strcpy(m_message, message);
}

void KException::MessageBox( HWND hWnd /* = NULL */ ) const 
{ 
   /*
    * call to unqualified virtual function
	*/

   ::MessageBox( hWnd, m_message, m_where, MB_ICONSTOP );
}

