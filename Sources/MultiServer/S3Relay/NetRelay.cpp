// NetRelay.cpp: implementation of the CNetRelay class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "S3Relay.h"
#include "NetRelay.h"

#include "../../Multiserver/Heaven/Interface/IServer.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetRelay::CNetRelay()
	: m_hDataExist(NULL)
{
}

CNetRelay::~CNetRelay()
{
	assert(!m_hDataExist);

	Clearup();
}




DWORD CNetRelay::Main(LPVOID lpParam)
{
	for (;;)
	{
		const HANDLE hdls[] = {m_hStop, m_hDataExist};
		DWORD wait = ::WaitForMultipleObjects(sizeof(hdls)/sizeof(hdls[0]), hdls, FALSE, INFINITE);
		if (wait != WAIT_OBJECT_0 + 1)	// !(data exist)
			break;
		Breathe();
	}

	return 0;
}


void CNetRelay::Breathe()
{
	Lock();	// <<<
	if (m_queuePckg.empty())
	{
		assert(::WaitForSingleObject(m_hDataExist, 0) == WAIT_TIMEOUT);
		::ResetEvent(m_hDataExist);

		Unlock();	// >>>
		return;
	}
	else
	{
		PACKAGE pckg = m_queuePckg.front();
		m_queuePckg.pop_front();

		if (m_queuePckg.empty())
			::ResetEvent(m_hDataExist);


		Unlock();	// >>>


		//send ...

		assert(pckg.pData && pckg.size > 0);

		g_pNetServer->SendPackage(pckg.idFrom, pckg.ipTo, pckg.portTo, pckg.pData, pckg.size);

		g_pMemory->Free(pckg.pData);


		in_addr ia;
		ia.s_addr = pckg.ipTo;
		gTrace("%s -- send size: %d", inet_ntoa(ia), pckg.size);
	}
}

void CNetRelay::Clearup()
{
	for (PACKAGEQUEUE::iterator it = m_queuePckg.begin(); it != m_queuePckg.end(); it++)
		g_pMemory->Free((*it).pData);

	m_queuePckg.clear();
}


int CNetRelay::RelayPackage(unsigned long idFrom, DWORD ipTo, unsigned short portTo, void* pData, unsigned size)
{
	if (pData == NULL || size < sizeof(RELAY_COMMAND))
		return FALSE;

	RELAY_COMMAND* pRelayCmd = (RELAY_COMMAND*)pData;
	assert(pRelayCmd->ProtocolType == c2s_relay);

	if (pRelayCmd->wLength < sizeof(RELAY_COMMAND) - sizeof(pRelayCmd->ProtocolType))
		return FALSE;


	PACKAGE pckg;
	pckg.idFrom = idFrom;
	pckg.ipTo = ipTo;
	pckg.portTo = portTo;
	pckg.pData = pData;
	pckg.size = size;


	Lock();

	m_queuePckg.push_back(pckg);

	if (m_hDataExist)
		::SetEvent(m_hDataExist);

	Unlock();

	return TRUE;
}

BOOL CNetRelay::Stop()
{
	BOOL ret = KThread::Stop();
	if (!ret)
		return FALSE;

	if (m_hDataExist)
	{
		::CloseHandle(m_hDataExist);
		m_hDataExist = NULL;
	}

	return TRUE;
}

HANDLE CNetRelay::Start()
{
	if (!m_hDataExist)
		m_hDataExist = ::CreateEvent(NULL, TRUE, !m_queuePckg.empty(), NULL);
	if (!m_hDataExist)
		return NULL;

	HANDLE hProc = KThread::Start();
	if (!hProc)
	{
		::CloseHandle(m_hDataExist);
		m_hDataExist = NULL;
		return NULL;
	}

	return hProc;
}
