// NetClient.cpp: implementation of the CNetClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "NetClient.h"
#include "NetCenter.h"

#include "S3Relay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetClientDup::CNetClientDup(const CNetClient& netclient)
	: m_pClient(netclient.GetClient()), m_SvrIP(netclient.GetSvrIP())
{
	_SafeAddRef(m_pClient);
}

BOOL CNetClientDup::SendPackage(const void* pData, size_t size) const
{
	dTRACKSENDDATA(pData, size);

	if(!m_pClient)
		return FALSE;
	m_pClient->SendPackToServer(pData, size);
	return TRUE;
}

CNetClientDup& CNetClientDup::operator =(const CNetClient& src)
{
	_SafeRelease(m_pClient);
	m_pClient = src.GetClient();
	_SafeAddRef(m_pClient);
	m_SvrIP = src.GetSvrIP();
	return *this;
}



CNetClient::CNetClient(CNetCenter* pNetCenter, BOOL bAutoFree)
	: m_pClient(NULL), m_ready(FALSE), m_pNetCenter(pNetCenter), m_bAutoFree(bAutoFree),
	m_SvrIP(0), m_SvrPort(0), m_hOver(NULL), m_bReqDisconn(FALSE), m_shutdown(0L)
{
	assert(m_pNetCenter);
	
}

CNetClient::~CNetClient()
{
	//assert(m_pClient);
	//_SafeRelease(m_pClient);
}


void CNetClient::DirectClearup()
{
	if (m_hStop)
	{
		::CloseHandle(m_hStop);
		m_hStop = NULL;
	}

	if (!m_bBlockSD)
	{
		if (m_hProcessor)
		{
			m_hProcessor = NULL;
			::CloseHandle(m_hProcessor);
		}
	}

	if (m_pClient)
	{
		try
		{
			if (m_bReqDisconn)
			{
				m_pClient->Shutdown();
				m_pClient->Cleanup();
			}

			m_pClient->Release();
		}
		catch (...)
		{
			//assert(FALSE);
		}
		m_pClient = NULL;
	}

	if (m_hOver)
	{
		::CloseHandle(m_hOver);
		m_hOver = NULL;
	}

	m_SvrAddr.resize(0);
	m_SvrIP = 0;
	m_SvrPort = 0;

	m_bReqDisconn = FALSE;
	m_bBlockSD = FALSE;
}

void CNetClient::InterShutdown()
{
	m_ready = FALSE;
	assert(m_pClient);

	m_bBlockSD = FALSE;
	m_bReqDisconn = TRUE;

#ifdef _WORKMODE_SINGLETHREAD
	DirectClearup();
	TryFreeThis();
#else
	assert(m_hStop);
	::SetEvent(m_hStop);	//clearup at thread terminate
#endif
}

DWORD CNetClient::Main(LPVOID lpParam)
{
	HRESULT hr = m_pClient->ConnectTo(m_SvrAddr.c_str(), m_SvrPort);
	if (FAILED(hr))
	{
		m_pClient->Shutdown();
		m_pClient->Cleanup();
		m_pClient->Release();
		m_pClient = NULL;

		OnStartupFail();

		if (m_hOver)
			::SetEvent(m_hOver);	//the Startup() do Clearup
		else
		{
			DirectClearup();
			TryFreeThis();
		}

		return -1;
	}

	//SetEvent(m_hOver);	//unblock in OnCreate

#ifdef _WORKMODE_SINGLETHREAD
	return 0;
#endif

#ifndef _WORKMODE_SINGLETHREAD
	EnterLoop();
#endif

	while (!IsAskingStop())
	{
		assert(m_pClient);

		DoRoute();

		::Sleep(breathe_interval);
	}

#ifndef _WORKMODE_SINGLETHREAD
	LeaveLoop();
#endif

	//final clearup
	DirectClearup();
	if (!m_bBlockSD)
		TryFreeThis();

	return 0;
}


//static
void __stdcall CNetClient::ClientEventNotify (
			LPVOID lpParam,
			const unsigned long &ulnEventType)
{
	CNetClient* pThis = (CNetClient*)lpParam;
	assert(pThis);

	switch(ulnEventType)
	{
	case enumServerConnectCreate:
		pThis->_NotifyServerEventCreate();
		break;
	case enumServerConnectClose:
		pThis->_NotifyServerEventClose();
		break;
	}
}


BOOL CNetClient::Startup(size_t maxFreeBuffers, size_t bufferSize, const char * pAddressToConnectServer, unsigned short usPortToConnectServer, BOOL block/* = TRUE*/)
{
	if (m_pClient)	//already startup !!!
		return FALSE;

	BOOL started = FALSE;

	assert(!m_ready);
	m_ready = FALSE;

	m_shutdown = 0L;

	IClientFactory* pClientFactory = NULL;

	if (FAILED(g_libRainbow.CreateInterface(IID_IClientFactory, reinterpret_cast< void ** >(&pClientFactory))))
		goto on_fail;
	
	pClientFactory->SetEnvironment(bufferSize);
	pClientFactory->CreateClientInterface(IID_IESClient, reinterpret_cast< void ** >(&m_pClient));

	pClientFactory->Release();

	if (FAILED(m_pClient->Startup()))
		goto on_fail;

	m_pClient->RegisterMsgFilter((LPVOID)this, ClientEventNotify);

	m_SvrAddr = pAddressToConnectServer;
	m_SvrIP = _a2ip(pAddressToConnectServer);
	m_SvrPort = usPortToConnectServer;


	m_bBlockSD = FALSE;
	m_bReqDisconn = FALSE;


#ifdef _WORKMODE_SINGLETHREAD

	if (block)
	{
		HRESULT hr = m_pClient->ConnectTo(m_SvrAddr.c_str(), m_SvrPort);
		if (FAILED(hr))
		{
			//m_pClient->Shutdown();
			m_pClient->Cleanup();
			m_pClient->Release();
			m_pClient = NULL;

			OnStartupFail();

			goto on_fail;
		}
	}
	else
	{
		if (!Start())
			goto on_fail;

		started = TRUE;

		::WaitForSingleObject(m_hProcessor, INFINITE);
		if (!m_pClient)
			goto on_fail;
	}

#else

	assert(!m_hOver);
	if (block)
	{
		m_hOver = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_hOver)
			goto on_fail;
	}

	if (!Start())
		goto on_fail;

	started = TRUE;

	if (block)
	{
		::WaitForSingleObject(m_hOver, INFINITE);
		if (m_pClient == NULL)
		{
			::WaitForSingleObject(m_hProcessor, INFINITE);
			goto on_fail;
		}
	}

#endif

	return TRUE;

on_fail:
	try
	{
		if (started)
			Stop();

		DirectClearup();
		TryFreeThis();
	}
	catch (...)
	{
		assert(FALSE);
	}

	return FALSE;
}

BOOL CNetClient::Shutdown()
{
	m_ready = FALSE;

	if (m_pClient)
	{
#ifdef _WORKMODE_SINGLETHREAD
		m_bBlockSD = TRUE;
		m_bReqDisconn = TRUE;
		DirectClearup();
#else

		m_bBlockSD = TRUE;
		m_bReqDisconn = TRUE;
		Stop();	//clearup at thread terminate
#endif

		TryFreeThis();
	}


	return TRUE;
}

void CNetClient::_NotifyServerEventCreate()
{
	m_ready = TRUE;

	m_pNetCenter->_NotifyServerEventCreate(this);
	OnServerEventCreate();

	//free block
	if (m_hOver)
		::SetEvent(m_hOver);
}

void CNetClient::_NotifyServerEventClose()
{
	if (m_shutdown)
		return;

	++ m_shutdown;

	m_ready = FALSE;

	OnServerEventClose();
	m_pNetCenter->_NotifyServerEventClose(this);

	//clearup at thread terminate if necessary
	if (!m_bReqDisconn)
	{
#ifdef _WORKMODE_SINGLETHREAD
		gConsignClientSD(m_pClient);
		m_pClient->Release();
		m_pClient = NULL;
		DirectClearup();
		TryFreeThis();
#else
		InterShutdown();
#endif
	}

	-- m_shutdown;
}


BOOL CNetClient::SendPackage(const void* pData, size_t size)
{
	if (!m_ready || m_pClient == NULL)
		return FALSE;

	dTRACKSENDDATA(pData, size);
	m_pClient->SendPackToServer(pData, size);
	return TRUE;
}


BOOL CNetClient::DoRoute()
{
	if (!m_ready || m_pClient == NULL)
		return FALSE;

	BeginRoute();

	for ( ; ; )
	{
		size_t size = 0;
		const void* pData = m_pClient->GetPackFromServer(size);
		if (!pData || size <= 0)
			break;

		dTRACKRECVDATA(pData, size);
		RecvPackage(pData, size);
	}

	EndRoute();

	return TRUE;
}

BOOL CNetClient::Route()
{
	return DoRoute();
}
