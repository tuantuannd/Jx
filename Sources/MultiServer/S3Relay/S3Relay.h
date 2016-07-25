
#if !defined(AFX_S3RELAY_H__B8C0717F_28A0_45B1_B760_36FCC946AD70__INCLUDED_)
#define AFX_S3RELAY_H__B8C0717F_28A0_45B1_B760_36FCC946AD70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"


///////////////////////////////////////////////////////////

#define UM_CONSIGNCLIENTSD	(WM_USER + 0x0100)

///////////////////////////////////////////////////////////



inline const char* _ip2a(DWORD ip) { in_addr ia; ia.s_addr = ip; return inet_ntoa(ia); }
inline DWORD _a2ip(const char* cp) { return inet_addr(cp); }



int gTrace(LPCSTR fmt, ...);
void gTrackRecvData(const void* pData, size_t size);
void gTrackSendData(const void* pData, size_t size);



#define rTRACE	gTrace
#define rTRACKRECVDATA	gTrackRecvData
#define rTRACKSENDDATA	gTrackSendData


#if defined(_DEBUG) || defined(_TESTING)
	#define dTRACE	rTRACE
	#define dTRACKRECVDATA	rTRACKRECVDATA
	#define dTRACKSENDDATA	rTRACKSENDDATA
#else
	#define dTRACE	1 ? 0 : rTRACE
	#define dTRACKRECVDATA	1 ? 0 : rTRACKRECVDATA
	#define dTRACKSENDDATA	1 ? 0 : rTRACKSENDDATA
#endif



#define RELEASEDO(s)	{s;}

#if defined(_DEBUG) || defined(_TESTING)
	#define DEBUGDO(s)	RELEASEDO(s)
#else
	#define DEBUGDO(s)	1 ? 0 : RELEASEDO(s)
#endif




const UINT timer_log = 1;
const UINT timer_rootclient = 2;
const UINT timer_gatewayclient = 3;
const UINT timer_dbclient = 4;
const UINT timer_friendudtdb = 5;


#endif // !defined(AFX_S3RELAY_H__B8C0717F_28A0_45B1_B760_36FCC946AD70__INCLUDED_)
