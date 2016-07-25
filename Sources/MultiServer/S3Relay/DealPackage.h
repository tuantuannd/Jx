// DealPackage.h: interface for the CDealPackage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEALPACKAGE_H__A8BD275E_8454_49B3_9EE3_F8314D9B3F32__INCLUDED_)
#define AFX_DEALPACKAGE_H__A8BD275E_8454_49B3_9EE3_F8314D9B3F32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetConnect.h"

class CDealPackage  
{
public:
	CDealPackage();
	virtual ~CDealPackage();

public:
	BOOL Initialize();
	BOOL Uninitialize();

private:
	enum {PROTOCOLCOUNT = 256};
	typedef BOOL (CDealPackage::*LPFNDEALPROC)(const CNetConnect* pConn, const void* pData, unsigned len);

	LPFNDEALPROC m_Protocol2Proc[PROTOCOLCOUNT];

public:
	BOOL DealPackage(const CNetConnect* pConn, const void* pData, unsigned len);

private:
	BOOL Proc_c2sRegisterFamily(const CNetConnect* pConn, const void* pData, unsigned len);
	BOOL Proc_c2sEnterGame(const CNetConnect* pConn, const void* pData, unsigned len);
	BOOL Proc_c2sLeaveGame(const CNetConnect* pConn, const void* pData, unsigned len);
	BOOL Proc_c2sRelay(const CNetConnect* pConn, const void* pData, unsigned len);
};

#endif // !defined(AFX_DEALPACKAGE_H__A8BD275E_8454_49B3_9EE3_F8314D9B3F32__INCLUDED_)
