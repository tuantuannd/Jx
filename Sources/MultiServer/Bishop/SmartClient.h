/********************************************************************
	created:	2003/08/22
	file base:	SmartClient
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_SMARTCLIENT__
#define __INCLUDE_SMARTCLIENT__

#pragma once

#include "IClient.h"
#include "RainbowInterface.h"

#include "Library.h"
#include "Buffer.h"
#include "tstring.h"
#include "CriticalSection.h"

#include <list>

class CSmartClient
{
public:

	CSmartClient();
	virtual ~CSmartClient();

	bool Open( const char * const pAddressToConnectServer, 
		unsigned short usPortToConnectServer, 
		HWND hwndContain,
		const char *pName,
		const char *pPwd,
		bool bReLink = true );

	void Close();

	bool Valid() { return ( NULL != m_pClient ); };

	bool Send( const void * const pData, const size_t &datalength );
	const void *Recv( size_t &datalength );	

	static DWORD WINAPI	WorkingThreadFunction( void *pV );

	HWND								m_hwndContain;

protected:

	bool CreateConnectThread();
	IClient* _Connect();
	void _Verify(IClient* pClient, bool bDisconnect);
	static void __stdcall EventNotify( LPVOID lpParam, 
				const unsigned long &ulnEventType );

	void _EventNotify( const unsigned long &ulnEventType );

	enum enumServerLoginErrorCode
	{
		enumConnectFailed	= 0xA1,
		enumUsrNamePswdErr,
		enumIPPortErr,
		enumException
	};

private:

	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	static OnlineGameLib::Win32::CLibrary	m_theRainbowLib;

	OnlineGameLib::Win32::CCriticalSection	m_csHistroyList;
	typedef std::list< OnlineGameLib::Win32::CBuffer * > stdList;
	stdList m_theHistroyList;

	HANDLE m_hWorkingThread;

	bool m_bReLink;		//使用本类需要自动重连功能
	
	/*
	 * Config
	 */
	IClient		*m_pClient;

	OnlineGameLib::Win32::_tstring		m_sUserName;
	OnlineGameLib::Win32::_tstring		m_sUserPwd;

	OnlineGameLib::Win32::_tstring		m_sSvrIP;
	unsigned short						m_nSvrPort;
	OnlineGameLib::Win32::CBuffer::Allocator	m_theAllocator;

	OnlineGameLib::Win32::CCriticalSection	m_csStatus;
	enum enumClientStatus
	{
		enumClientNormal = 0x00,
		enumClientConnect,
		enumClientVerify,
		enumClientError,
	};

	enumClientStatus m_ClientStatus;

	bool IsNormal();
	bool IsConnecting();
	bool IsVerifying();
	bool IsError();

	void SwitchToConnecting();		//切换状态到重连,如有必要会启动重连线程
	bool SetClientStatus(enumClientStatus nNewS);
};

extern CSmartClient g_theSmartClient;

#endif // __INCLUDE_SMARTCLIENT__