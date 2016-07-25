//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSocketClient2.cpp
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   2003-9-16 15:35:46
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "winsock2.h"
#include "IClient.h"
#include "KSocketClient2.h"

#define SOCKET_VERSION_REQUIRED     0x202

KSocketClient2::KSocketClient2()
{
    WSADATA WSAData = { 0 };

    WSAStartup(SOCKET_VERSION_REQUIRED, &WSAData);
    
    m_ulRefCount = 0;
    AddRef();

    m_pvCallBackParam   = NULL;
    m_pfnCallBack       = NULL;

    m_Socket            = INVALID_SOCKET;
    m_nReturnBufferFlag = false;
    m_pbyInBufferEnd    = m_InBuffer;
    m_nRemainSize       = KSOCKETCLIENT_MAX_BUFFER_SIZE;

}

KSocketClient2::~KSocketClient2()
{
    WSACleanup();

}

ULONG STDMETHODCALLTYPE KSocketClient2::AddRef(void)
{
    return InterlockedIncrement((LPLONG)&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE KSocketClient2::Release(void)
{
    ULONG ulRefCount = InterlockedDecrement((LPLONG)&m_ulRefCount);
    if (0 == ulRefCount)
    {
        delete this;
        return 0;
    }

    return ulRefCount;
}

HRESULT STDMETHODCALLTYPE KSocketClient2::QueryInterface( 
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	/*
	 * By definition all COM objects support the IUnknown interface
	 */
	if( riid == IID_IUnknown )
	{
		AddRef();

		*ppvObject = dynamic_cast< IUnknown * >( this );
	}
	else if ( riid == IID_IESClient )
	{
		AddRef();

		*ppvObject = dynamic_cast< IClient * >( this );	
	}
	else
	{
		*ppvObject = NULL;

		return E_NOINTERFACE;
	}

	return S_OK;
}

// Initialize server object and start up it
HRESULT STDMETHODCALLTYPE KSocketClient2::Startup()
{
    m_Socket            = INVALID_SOCKET;
    m_nReturnBufferFlag = false;
    m_pbyInBufferEnd    = m_InBuffer;
    m_nRemainSize       = KSOCKETCLIENT_MAX_BUFFER_SIZE;

    m_pvCallBackParam   = NULL;
    m_pfnCallBack       = NULL;

    return S_OK;
}

// Stop this object and destroy it
HRESULT STDMETHODCALLTYPE KSocketClient2::Cleanup()
{
    HRESULT hrResult  = E_FAIL;

    hrResult = Shutdown();

    m_pvCallBackParam   = NULL;
    m_pfnCallBack       = NULL;

    return hrResult;
}

// Connect to a specified server
HRESULT STDMETHODCALLTYPE KSocketClient2::ConnectTo(
    const char * const &pAddressToConnectServer,
    unsigned short usPortToConnectServer
)
{
    HRESULT hrResult = E_FAIL;
    int nRetCode = false;

    struct sockaddr_in ServerAddr = { 0 };
    int nServerAddrSize = sizeof(ServerAddr);

    if (!pAddressToConnectServer)
        goto Exit0;
    
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(pAddressToConnectServer);
    if (ServerAddr.sin_addr.s_addr == INADDR_NONE)
        goto Exit0;
    
    ServerAddr.sin_port = htons(usPortToConnectServer);

    m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_Socket == INVALID_SOCKET)
        goto Exit0;

    nRetCode = connect(m_Socket, (struct sockaddr *)&ServerAddr, nServerAddrSize);
    if (nRetCode == SOCKET_ERROR)
        goto Exit0;

    if (m_pfnCallBack)
    {
    	m_pfnCallBack(m_pvCallBackParam, enumServerConnectCreate);

    }

    hrResult = S_OK;
Exit0:
    if (FAILED(hrResult))
    {
        if (m_Socket != INVALID_SOCKET)
        {
            closesocket(m_Socket);
            m_Socket = INVALID_SOCKET;
        }
    }

    return hrResult;
}

HRESULT STDMETHODCALLTYPE KSocketClient2::RegisterMsgFilter(
	LPVOID lpParam,
	CALLBACK_CLIENT_EVENT pfnEventNotify
)
{
    m_pvCallBackParam   = lpParam;
    m_pfnCallBack       = pfnEventNotify;
    return S_OK;
}

// Send some data to the current server
HRESULT STDMETHODCALLTYPE KSocketClient2::SendPackToServer(
    const void * const	pData,
    const size_t		&datalength
)
{
    HRESULT hrResult = E_FAIL;
    int  nSendSize = 0;
    int  nRetCode = false;
    unsigned char *pbyBuffer = NULL;

    if (!pData)
        goto Exit0;

    if (datalength > (KSOCKETCLIENT_MAX_BUFFER_SIZE - sizeof(WORD)))
        goto Exit0;

    nSendSize = datalength + sizeof(WORD);
    memcpy(m_OutBuffer + sizeof(WORD), pData, datalength);

    *((WORD *)m_OutBuffer) = (WORD)nSendSize;

    pbyBuffer = m_OutBuffer;

    while (nSendSize > 0)
    {
        nRetCode = send(m_Socket, (char *)pbyBuffer, nSendSize, 0);
        if (
            (nRetCode == SOCKET_ERROR) ||
            (nRetCode == 0)
        )
        {
            Shutdown();     // notify shutdown
            goto Exit0;
        }


        nSendSize -= nRetCode;
        pbyBuffer += nRetCode;
    }

    hrResult = S_OK;
Exit0:
    return hrResult;
}


// Get some data from the current server
const void * STDMETHODCALLTYPE KSocketClient2::GetPackFromServer(
	size_t  &datalength
)
{
    static struct timeval s_timeout = { 0, 0 };    
    int nRetCode = false;

    if (m_nReturnBufferFlag)
    {
        unsigned char *pbyNextInBuffer = (m_InBuffer + (unsigned)(*(WORD *)m_InBuffer));
        int nCopySize = m_pbyInBufferEnd - pbyNextInBuffer; 
        
        memcpy(m_InBuffer, pbyNextInBuffer, nCopySize);

        m_pbyInBufferEnd    =  m_InBuffer + nCopySize;
        m_nRemainSize       =  KSOCKETCLIENT_MAX_BUFFER_SIZE - nCopySize;
        m_nReturnBufferFlag = false;
    }

    while (true)
    {
        if (m_Socket == INVALID_SOCKET)
            return NULL;

        if (!
            (
                (m_pbyInBufferEnd < (m_InBuffer + sizeof(WORD))) ||
                (m_pbyInBufferEnd < (m_InBuffer + (unsigned)(*(WORD *)m_InBuffer)))
            )
        )
        {
            m_nReturnBufferFlag = true;

            datalength = (size_t)(*(WORD *)m_InBuffer) - sizeof(WORD);
            return m_InBuffer + sizeof(WORD);
        }

        fd_set ReadFDSET;
        ReadFDSET.fd_count = 1;
        ReadFDSET.fd_array[0] = m_Socket;

        nRetCode = select(1, &ReadFDSET, NULL, NULL, &s_timeout);
        if (nRetCode == SOCKET_ERROR)
        {
            Shutdown();     // notify shutdown
            return NULL;
        }

        if (nRetCode == 0)  // if timeout
        {
            return NULL;
        }

        // if Read Ready
        nRetCode = recv(m_Socket, (char *)m_pbyInBufferEnd, m_nRemainSize, 0);
        if (
            (nRetCode == SOCKET_ERROR) ||
            (nRetCode == 0)
        )
        {
            Shutdown();     // notify shutdown
            return NULL;
        }

        m_pbyInBufferEnd += nRetCode;
        m_nRemainSize    -= nRetCode;
    }

    return NULL;
}

// Disconnect a specified connection
HRESULT STDMETHODCALLTYPE KSocketClient2::Shutdown()
{
    // maybe lock
    SOCKET sckSocket = InterlockedExchange((LPLONG)&m_Socket, INVALID_SOCKET);

    if (sckSocket != INVALID_SOCKET)
    {
		LINGER lingerStruct;
	
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;
	
        // notify accept thread to exit
        setsockopt( 
            sckSocket, 
			SOL_SOCKET, 
			SO_LINGER, 
			(char *)&lingerStruct, 
			sizeof(lingerStruct)
        );
        
        closesocket(sckSocket);
        sckSocket = INVALID_SOCKET;

        if (m_pfnCallBack)
        {
    		m_pfnCallBack(m_pvCallBackParam, enumServerConnectClose);

        }
    }

    return S_OK;
}
