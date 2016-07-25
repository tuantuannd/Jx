//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSocketClient2.h
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   2003-9-16 15:35:35
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _KSOCKETCLIENT2_H_
#define _KSOCKETCLIENT2_H_  1

#include "winsock2.h"
#include "IClient.h"

#define KSOCKETCLIENT_MAX_BUFFER_SIZE   (64 * 1024)

class KSocketClient2 : public IClient
{
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
    
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
    
    virtual ULONG STDMETHODCALLTYPE Release( void);

    // Initialize server object and start up it
    virtual HRESULT STDMETHODCALLTYPE Startup();

	// Stop this object and destroy it
	virtual HRESULT STDMETHODCALLTYPE Cleanup();

	// Connect to a specified server
	virtual HRESULT STDMETHODCALLTYPE ConnectTo(
        const char * const &pAddressToConnectServer,
        unsigned short usPortToConnectServer
    );

	virtual HRESULT STDMETHODCALLTYPE RegisterMsgFilter(
		LPVOID lpParam,
		CALLBACK_CLIENT_EVENT pfnEventNotify
	);
	
	// Send some data to the current server
	virtual HRESULT STDMETHODCALLTYPE SendPackToServer(
        const void * const	pData,
        const size_t		&datalength
	);

	
	// Get some data from the current server
	virtual const void * STDMETHODCALLTYPE GetPackFromServer(
	    size_t  &datalength
	);
	
	// Disconnect a specified connection
	virtual HRESULT STDMETHODCALLTYPE Shutdown();

private:
    ULONG   m_ulRefCount;

    void *m_pvCallBackParam;
    CALLBACK_CLIENT_EVENT m_pfnCallBack;

    unsigned char m_InBuffer [KSOCKETCLIENT_MAX_BUFFER_SIZE];
    unsigned char m_OutBuffer[KSOCKETCLIENT_MAX_BUFFER_SIZE];


    SOCKET          m_Socket;
    int             m_nReturnBufferFlag;
    unsigned char  *m_pbyInBufferEnd;
    int             m_nRemainSize;


public:
    KSocketClient2();
    ~KSocketClient2();


};





#endif  // _KSOCKETCLIENT2_H_