// NetSockDupEx.h: interface for the CNetSockDupEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETSOCKDUPEX_H__B385E71F_A852_41C4_BB1B_1199612E19B5__INCLUDED_)
#define AFX_NETSOCKDUPEX_H__B385E71F_A852_41C4_BB1B_1199612E19B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetConnect.h"
#include "NetClient.h"

class CNetSockDupEx  
{
public:
	CNetSockDupEx();
	CNetSockDupEx(const CNetConnectDup& rNetConnectDup);
	CNetSockDupEx(const CNetClientDup& rNetClientDup);
	CNetSockDupEx(const CNetSockDupEx& rNetSockDupEx);

	virtual ~CNetSockDupEx();

public:
	enum SOCKTYPE {socktype_none, socktype_Connect, socktype_Client};

public:
	BOOL IsValid() const {return m_NetConnectDup.IsValid() || m_NetClientDup.IsValid();}
	SOCKTYPE GetSockType() const;
	BOOL SendPackage(const void* pData, size_t size) const;
	DWORD GetSockIP()const;
	unsigned long GetSockParam() const;

	const CNetConnectDup& GetConnectDup() const {return m_NetConnectDup;}
	const CNetClientDup& GetClientDup() const {return m_NetClientDup;}

	void Clearup() {m_NetConnectDup.Clearup(); m_NetClientDup.Clearup();}

	CNetSockDupEx& operator= (const CNetConnectDup& rNetConnectDup);
	CNetSockDupEx& operator= (const CNetClientDup& rNetClientDup);
	CNetSockDupEx& operator= (const CNetSockDupEx& rNetSockDupEx);

private:
	//most only one is valid
	CNetConnectDup m_NetConnectDup;
	CNetClientDup m_NetClientDup;
};

#endif // !defined(AFX_NETSOCKDUPEX_H__B385E71F_A852_41C4_BB1B_1199612E19B5__INCLUDED_)
