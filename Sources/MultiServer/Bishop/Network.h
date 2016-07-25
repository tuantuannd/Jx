/********************************************************************
	created:	2003/05/05
	file base:	Network
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_NETWORK_H__
#define __INCLUDE_NETWORK_H__

#pragma once

#include "IClient.h"
#include "RainbowInterface.h"

#include "IServer.h"
#include "HeavenInterface.h"

#include "Library.h"
#include "Buffer.h"
#include "tstring.h"
#include "CriticalSection.h"

class CNetwork
{
public:

	CNetwork();
	virtual ~CNetwork();

	bool	Create();
	void	Destroy();

//	IClient *CreateAccSvrClient( const char * const pAddressToConnectServer, 
//		unsigned short usPortToConnectServer );

	const char *GetAccSvrIP() { return m_sAccSvrIP.c_str(); }
	unsigned short GetAccSvrPort() { return m_nAccSvrPort; }
	
	const char *GetRoleSvrIP() { return m_sRoleSvrIP.c_str(); }
	void SetRoleSvrIP( const char *pAddress ) { if ( pAddress ) { m_sRoleSvrIP = pAddress; } }
	unsigned short GetRoleSvrPort() { return m_nRoleSvrPort; }
	void SetRoleSvrPort( unsigned short nRoleSvrPort ) { m_nRoleSvrPort = nRoleSvrPort; }

	unsigned short GetClientOpenPort() { return m_nClientOpenPort; }
	void SetClientOpenPort( unsigned short nClientOpenPort ) { m_nClientOpenPort = nClientOpenPort; }

	unsigned short GetGameSvrOpenPort() { return m_nGameSvrOpenPort; }
	void SetGameSvrOpenPort( unsigned short nGameSvrOpenPort ) { m_nGameSvrOpenPort = nGameSvrOpenPort; }

protected:

	typedef HRESULT ( __stdcall * pfnCreateServerInterface )( REFIID riid, void **ppv );
	typedef HRESULT ( __stdcall * pfnCreateClientInterface )( REFIID riid, void **ppv );

	static OnlineGameLib::Win32::CLibrary	m_theHeavenLib;
	static OnlineGameLib::Win32::CLibrary	m_theRainbowLib;

//	static void __stdcall AccSvrEventNotify( LPVOID lpParam, const unsigned long &ulnEventType );
//	void	DistroyAccSvrClient();
	
private:

	/*
	 * Account server config
	 */
//	IClient		*m_pAccSvrClient;

	OnlineGameLib::Win32::_tstring		m_sAccSvrIP;
	unsigned short	m_nAccSvrPort;

	/*
	 * Role server config
	 */
	OnlineGameLib::Win32::_tstring		m_sRoleSvrIP;
	unsigned short	m_nRoleSvrPort;

	/*
	 * Client config
	 */
	unsigned short	m_nClientOpenPort;

	/*
	 * Game server config
	 */
	unsigned short	m_nGameSvrOpenPort;
	
	
};

#endif // __INCLUDE_NETWORK_H__