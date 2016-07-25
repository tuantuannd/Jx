// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D37121BB_0F27_47FA_833A_9584641D625F__INCLUDED_)
#define AFX_STDAFX_H__D37121BB_0F27_47FA_833A_9584641D625F__INCLUDED_

#if _MSC_VER > 1300
#pragma once
#endif // _MSC_VER > 1000


// TODO: reference additional headers your program requires here
#ifndef WINVER                // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0403        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif                        

#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE            // Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0500    // Change this to the appropriate value to target IE 5.0 or later.
#endif


#pragma warning(disable : 4786)
#ifndef _STANDALONE
#define WIN32_LEAN_AND_MEAN // Fixed By MrChucong@gmail.com
#include "windows.h"
#endif
#include <winsock2.h>
#include "KEngine.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D37121BB_0F27_47FA_833A_9584641D625F__INCLUDED_)
