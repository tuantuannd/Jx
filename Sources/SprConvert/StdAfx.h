// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F728717E_AADA_4BA4_9F44_540F2E687817__INCLUDED_)
#define AFX_STDAFX_H__F728717E_AADA_4BA4_9F44_540F2E687817__INCLUDED_
#ifndef WINVER               
#define WINVER 0x0500        
#endif 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <assert.h>

// TODO: reference additional headers your program requires here
#include "..\engine\src\KEngine.h"

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F728717E_AADA_4BA4_9F44_540F2E687817__INCLUDED_)
