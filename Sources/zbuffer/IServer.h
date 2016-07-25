#ifndef ISERVER_H
#define ISERVER_H

#include "zport.h"
#include "xbuffer.h"

enum enumClientConnectInfo {
	enumClientConnectCreate = 0x100,
	enumClientConnectClose
};

typedef void (*CALLBACK_SERVER_EVENT)(void * lpParam, const unsigned long &ulnID, const unsigned long &ulnEventType);

//进行侦听的线程
class IServer;
class ZListenThread : public ZThread {
	IServer *parent;
public:
	ZListenThread(IServer *the_parent) : ZThread() {
		parent = the_parent;
	}
	int action();
};

class ZSendThread : public ZThread {
public:
	IServer *parent;
	int index;
	ZSendThread()
	{
		parent = NULL;
		index = -1;
	}
	int action();
};

class ZRecvThread : public ZThread {
public:
	IServer *parent;
	int index;
	ZRecvThread()
	{
		parent = NULL;
		index = -1;
	}
	int action();
};

class ZCloseCallBackThread : public ZThread
{
	IServer *parent;
public:
	ZCloseCallBackThread(IServer *the_parent) : ZThread() {
		parent = the_parent;
	}
	int action();
};

class ZCreateCallBackThread : public ZThread
{
	IServer *parent;
public:
	ZCreateCallBackThread(IServer *the_parent) : ZThread() {
		parent = the_parent;
	}
	int action();
};

#define MAX_IN_BUFFER	8192

#define STATE_IDLE					0
#define STATE_SENDING				1
#define STATE_BLOCKED				2
#define STATE_CLOSING				3

class client_info {
public:
#ifdef WIN32
	OVERLAPPED read_overlapped;
	OVERLAPPED write_overlapped;

	WSABUF  m_wsaInBuffer;						//读数据的缓冲区
	WSABUF	m_wsaOutBuffer;						//发送数据使用的缓冲区
#endif
	SOCKET sock;
	struct sockaddr addr;
	int index;
	ZBuffer* buffer;
	//ZMutex mutex;
	
	unsigned long server_key;
	unsigned long client_key;
	short state;
	unsigned char conn_flag;
	unsigned char recv_flag;
	client_info() {
		state = STATE_IDLE;
		conn_flag = 0;
		recv_flag = 0;
#ifdef WIN32
		ZeroMemory(&read_overlapped, sizeof(OVERLAPPED));
		ZeroMemory(&write_overlapped, sizeof(OVERLAPPED));
#endif
	}
	~client_info() {
		delete buffer;
	}
};

/*#ifndef _COND_SEND_
class send_signal_info
{
public:
	unsigned short flagset;
	unsigned short retrycount;
	
public:
	void setFlag()
	{
		flagset = true;
		retrycount++;
	}
	void clearFlag()
	{
		flagset = false;
		retrycount = 0;
	}
	send_signal_info()
	{
		flagset = false;
		retrycount = 0;
	}
};
#endif
*/
class IServer {
public:
	CALLBACK_SERVER_EVENT call_back;
	void *parameter;

	ZPerf sendPerf;
	ZPerf recvPerf;

#ifdef WIN32
	HANDLE CompletionPort;
#else
	ZMutex mutex;
//	#ifndef _EPOLL_
	//sigset_t *sigsets;
//		#ifndef _COND_SEND_
//		send_signal_info *sigflags;
//		#else
		//pthread_cond_t cond_send_data;
		//ZMutex mutex_send_data;
		//pthread_t *thr_id;
//		#endif
	//int *pids;
	int pid;
	//int current_thread;
//	#else
//	Epoll* pEpoll;
//	#endif
	ZSendThread *send_threads;
	ZRecvThread *recv_threads;
	ZCloseCallBackThread* close_callback_thread;
	ZCreateCallBackThread* create_callback_thread;
#endif
	int max_thread;						//最大并发线程数目
	SOCKET listen_socket;
	ZListenThread *listen_thread;

	client_info *clients;
	int max_client;

	IServer(int number, int thread_number = 4, int max_send = 300 * 1024, int max_receive = 32 * 1024);
	~IServer();
	int getConnection(SOCKET sock);
	void closeConnection(client_info *client);
	bool sendData(client_info *client);
	bool receiveData(client_info *client);

//对外公开的接口
	int Startup();
	int Cleanup();
	int Release();
	int Open(const unsigned long &ulnAddressToListenOn, const unsigned short &usnPortToListenOn);
	int OpenService(const unsigned long &ulnAddressToListenOn, const unsigned short &usnPortToListenOn) {
		return Open(ulnAddressToListenOn, usnPortToListenOn);
	}
	int CloseService();
	int RegisterMsgFilter(void * lpParam, CALLBACK_SERVER_EVENT pfnEventNotify);
	int PreparePackSink();
	int PackDataToClient(const unsigned long &ulnClientID, const void *pData, unsigned long datalength);
	int SendPackToClient();
	int SendPackToClient(int index);
	int SendData(const unsigned long &ulnClientID, const void *pData, unsigned long datalength);
	const void *GetPackFromClient(unsigned long ulnClientID, unsigned int &datalength);
	int ShutdownClient(const unsigned long &ulnClientID);
	const char * GetClientInfo(const unsigned long &ulnClientID);
};

#endif
