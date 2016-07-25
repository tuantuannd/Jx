#ifndef ZCLIENT_H
#define ZCLIENT_H

//应该使用线程的方法,Startup启动线程等待信号,不同的IO模型使用不同的信号通知这些线程,处理的时候
//根据处理器数目的不同启动不同数目的线程和缓冲区,处理异常情况
#include "zport.h"
#include "zbuffer.h"
#include "Cipher.h"

#include "..\MultiServer\Rainbow\Interface\IClient.h"

/*enum enumServerConnectInfo {
	enumServerConnectCreate = 0x100,
	enumServerConnectClose
};*/

typedef void (CALLBACK *CALLBACK_CLIENT_EVENT )(void *lpParam, const unsigned long &ulnEventType);

class ZClient;
class ZClientThread : public ZThread {
	ZClient *parent;
public:
	ZClientThread(ZClient *the_parent) : ZThread() {
		parent = the_parent;
	}
	int action();
};

class ZClient {
protected:
	ZClientThread *thread;
public:
	bool bConnected;
	bool bStop;
	unsigned int m_uServerKey;
	unsigned int m_uClientKey;
	int read_index;
	int write_index;
	CALLBACK_CLIENT_EVENT call_back;
	void *call_back_param;
	int nSocket;
	ZBuffer *buffer;
	ZClient(ZBuffer *the_buffer);
	~ZClient();
	void Startup();						//启动一个线程发送
	void Cleanup();
	void Shutdown();

	bool ConnectTo(const char * const &pAddressToConnectServer, unsigned short usPortToConnectServer);
	void RegisterMsgFilter(void * lpParam, CALLBACK_CLIENT_EVENT pfnEventNotify);

	bool SendPackToServer( const void * const pData, const unsigned long datalength);
	void *GetPackFromServer(unsigned int &datalength );
	void startPerf() {
		buffer->startPerf();
	}
	void stopPerf() {
		buffer->stopPerf();
	}
};

#endif