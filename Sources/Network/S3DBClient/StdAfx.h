// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C02FFA4F_4B38_427E_9C3D_3BAE1541E175__INCLUDED_)
#define AFX_STDAFX_H__C02FFA4F_4B38_427E_9C3D_3BAE1541E175__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#define WIN32_LEAN_AND_MEAN
#include <Winsock2.h>
#include "Logindef.h"
#include "GlobalDTD.h"
#include "GlobalFun.h"

#include <winsock2.h>

#include "..\ESClient\Exception.h"
#include "..\ESClient\Utils.h"
#include "..\ESClient\ManualResetEvent.h"
#include "GameClient.h"
//[ Include in .\..\Protocol
#include "Protocol\Protocol.h"
//]
using OnlineGameLib::Win32::_tstring;
//using OnlineGameLib::Win32::CException;
using OnlineGameLib::Win32::Output;
using OnlineGameLib::Win32::CManualResetEvent;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C02FFA4F_4B38_427E_9C3D_3BAE1541E175__INCLUDED_)
