// KException.h: interface for the KException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEXCEPTION_H__EABC0D47_03E6_44C2_A492_38D2A02530E5__INCLUDED_)
#define AFX_KEXCEPTION_H__EABC0D47_03E6_44C2_A492_38D2A02530E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "windows.h"
class KException  
{
	public:
	KException (const char * where, const char *message );
	virtual ~KException() {}

	void MessageBox( HWND hWnd = NULL ) const; 

	char m_where[100];
	char m_message[100];
};

#endif // !defined(AFX_KEXCEPTION_H__EABC0D47_03E6_44C2_A492_38D2A02530E5__INCLUDED_)
