// NetConnect.h: interface for the CNetConnect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETCONNECT_H__1DDF0ACA_C466_4BB0_8A1D_C4B55012953E__INCLUDED_)
#define AFX_NETCONNECT_H__1DDF0ACA_C466_4BB0_8A1D_C4B55012953E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../Multiserver/Heaven/Interface/IServer.h"
#include "KThread.h"

class CNetConnect : public KThread,	//!workmode_singlethread
					public IMessageProcess	//workmode_multithread2
{
public:
	CNetConnect(class CNetServer* pNetServer, unsigned long id);
	virtual ~CNetConnect();

public:
	CNetServer* GetServer() const {return m_pNetServer;}
	unsigned long GetID() const {return m_id;}

private:
	CNetServer* m_pNetServer;
	unsigned long m_id;

	LONG m_disconnect;

private:
	DWORD m_IP;
	unsigned short m_port;

public:
	DWORD GetIP() const {return m_IP;}
	DWORD GetPort() const {return m_port;}

	HRESULT OnMessage( VOID *, size_t );

	BOOL IsReady() const;

protected:
	virtual DWORD Main(LPVOID lpParam);
#ifndef _WORKMODE_SINGLETHREAD
	virtual void EnterLoop() {}
	virtual void LeaveLoop() {}
#endif
	virtual void BeginRoute() {}
	virtual void EndRoute() {}
	virtual void RecvPackage(const void* pData, size_t size) {}

public:
	BOOL SendPackage(const void* pData, size_t size);

public:
	void _NotifyClientConnectCreate();
	void _NotifyClientConnectClose();

protected:
	virtual void OnClientConnectCreate() {}
	virtual void OnClientConnectClose() {}

private:
	BOOL DoRoute();

public:
	BOOL Route();
};


class CNetConnectDup 
{
public:
	CNetConnectDup() : m_pServer(NULL), m_IP(0), m_id(-1) {}
	CNetConnectDup(const CNetConnectDup& netconndup) : m_pServer(netconndup.GetServer()), m_IP(netconndup.m_IP), m_id(netconndup.GetID()) {_SafeAddRef(m_pServer);}
	CNetConnectDup(const CNetConnect& netconnect);
	virtual ~CNetConnectDup() {_SafeRelease(m_pServer);}

	BOOL IsValid() const {return m_pServer != NULL;}
	IServer* GetServer() const {return m_pServer;}
	unsigned long GetID() const {return m_id;}

	DWORD GetIP() const {return m_IP;}
	BOOL SendPackage(const void* pData, size_t size) const;

	void Clearup() {_SafeRelease(m_pServer); m_IP = 0; m_id = -1;}

	CNetConnectDup& operator =(const CNetConnectDup& src)
	{ if (&src == this) return *this; _SafeRelease(m_pServer); m_pServer = src.m_pServer; _SafeAddRef(m_pServer); m_IP = src.m_IP; m_id = src.m_id; return *this; }
	CNetConnectDup& operator =(const CNetConnect& src);

private:
	IServer* m_pServer;
	DWORD m_IP;
	unsigned long m_id;
};


#endif // !defined(AFX_NETCONNECT_H__1DDF0ACA_C466_4BB0_8A1D_C4B55012953E__INCLUDED_)
