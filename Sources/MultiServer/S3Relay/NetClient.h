// NetClient.h: interface for the CNetClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETCLIENT_H__92C76923_17D1_46D4_ADE1_A1BDE2E5788A__INCLUDED_)
#define AFX_NETCLIENT_H__92C76923_17D1_46D4_ADE1_A1BDE2E5788A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../Multiserver/Rainbow/Interface/IClient.h"
#include "KThread.h"

class CNetClient : protected KThread   //!workmode_singlethread
{
public:
	CNetClient(class CNetCenter* pNetCenter, BOOL bAutoFree);
	virtual ~CNetClient();

public:
	BOOL Startup(size_t maxFreeBuffers, size_t bufferSize, const char * pAddressToConnectServer, unsigned short usPortToConnectServer, BOOL block = TRUE);
	BOOL Shutdown();

	BOOL IsReady() const { return m_ready && m_pClient != NULL; }

private:
	std::string m_SvrAddr;
	DWORD m_SvrIP;
	unsigned short m_SvrPort;

public:
	DWORD GetSvrIP() const {return m_SvrIP;}
	unsigned short GetSvrPort() const {return m_SvrPort;}

private:
	static void __stdcall ClientEventNotify (
			LPVOID lpParam,
			const unsigned long &ulnEventType);

private:
	IClient* m_pClient;
	BOOL m_ready;
	CNetCenter* m_pNetCenter;
	BOOL m_bAutoFree;

	HANDLE m_hOver;
	BOOL m_bReqDisconn;
	BOOL m_bBlockSD;

	LONG m_shutdown;

private:
	void CheckSDSignal();
	void DirectClearup();

protected:
	void InterShutdown();	//!workmode_singlethread

public:
	CNetCenter* GetCenter() const {return m_pNetCenter;}
	IClient* GetClient() const {return m_ready ? m_pClient : NULL;}

protected:
	virtual void OnStartupFail() {}
	virtual void OnShutdownFail() {}

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

private:
	void _NotifyServerEventCreate();
	void _NotifyServerEventClose();

protected:
	virtual void OnServerEventCreate() {}
	virtual void OnServerEventClose() {}

private:
	void TryFreeThis() {if (m_bAutoFree) {assert(!m_hProcessor); DeleteThis();}}

protected:
	virtual void DeleteThis() {assert(false);}

private:
	BOOL DoRoute();

public:
	BOOL Route();
};


class CNetClientDup 
{
public:
	CNetClientDup() : m_pClient(NULL), m_SvrIP(0) {}
	CNetClientDup(const CNetClientDup& netclntdup) : m_pClient(netclntdup.GetClient()), m_SvrIP(netclntdup.m_SvrIP) {_SafeAddRef(m_pClient);}
	CNetClientDup(const CNetClient& netclient);
	virtual ~CNetClientDup() {_SafeRelease(m_pClient);}

	BOOL IsValid() const {return m_pClient != NULL;}
	IClient* GetClient() const {return m_pClient;}

	DWORD GetSvrIP() const {return m_SvrIP;}
	BOOL SendPackage(const void* pData, size_t size) const;

	void Clearup() {_SafeRelease(m_pClient); m_SvrIP = 0;}

	CNetClientDup& operator =(const CNetClientDup& src)
	{ if (&src == this) return *this; _SafeRelease(m_pClient); m_pClient = src.m_pClient; _SafeAddRef(m_pClient); m_SvrIP = src.m_SvrIP; return *this; }
	CNetClientDup& operator =(const CNetClient& src);

private:
	IClient* m_pClient;
	DWORD m_SvrIP;
};


#endif // !defined(AFX_NETCLIENT_H__92C76923_17D1_46D4_ADE1_A1BDE2E5788A__INCLUDED_)
