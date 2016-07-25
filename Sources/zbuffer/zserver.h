#ifndef ZSERVER_H
#define ZSERVER_H

#include "zport.h"
#include "zbuffer.h"

enum enumClientConnectInfo {
	enumClientConnectCreate = 0x100,
	enumClientConnectClose
};

typedef void (*CALLBACK_SERVER_EVENT)(void * lpParam, const unsigned long &ulnID, const unsigned long &ulnEventType);

//进行侦听的线程
class ZServer;
class ZListenThread : public ZThread {
	ZServer *parent;
public:
	ZListenThread(ZServer *the_parent) : ZThread() {
		parent = the_parent;
	}
	int action();
};

#define MAX_IN_BUFFER	8192

class client_info {
public:
#ifdef WIN32
	OVERLAPPED read_overlapped;
	OVERLAPPED write_overlapped;

	WSABUF  m_wsaInBuffer;						//读数据的缓冲区
	WSABUF	m_wsaOutBuffer;						//发送数据使用的缓冲区
#endif
	unsigned char m_byInBuffer[MAX_IN_BUFFER];
	SOCKET sock;
	struct sockaddr addr;
	int read_index;
	int write_index;
	
	unsigned long server_key;
	unsigned long client_key;

	client_info() {
		read_index = write_index = -1;
#ifdef WIN32
		ZeroMemory(&read_overlapped, sizeof(OVERLAPPED));
		ZeroMemory(&write_overlapped, sizeof(OVERLAPPED));
#endif
	}
};

class ZServer {
public:
	CALLBACK_SERVER_EVENT call_back;
	void *parameter;

#ifdef WIN32
	HANDLE CompletionPort;
#else
  sigset_t sigset;
  int pid;
#endif
	SOCKET listen_socket;
	ZListenThread *listen_thread;

	client_info *clients;
	int max_client;
	ZBuffer *buffer;

	ZServer(int number);
	~ZServer();
	int getConnection(SOCKET sock);
	
	bool sendData(client_info *client);
	bool receiveData(client_info *client);

//对外公开的接口
	int Startup();
	int Cleanup();
	int Release();
	int Open(const unsigned long &ulnAddressToListenOn, const unsigned short &usnPortToListenOn);
	int CloseService();
	int RegisterMsgFilter(void * lpParam, CALLBACK_SERVER_EVENT pfnEventNotify);
	int PreparePackSink();
	int PackDataToClient(const unsigned long &ulnClientID, const void *pData, unsigned long datalength);
	int SendPackToClient();
	int SendPackToClient(int index);
	int SendData(const unsigned long &ulnClientID, const void *pData, unsigned long datalength);
	const void *GetPackFromClient(unsigned long ulnClientID, unsigned long &datalength);
	int ShutdownClient(const unsigned long &ulnClientID);
};

#endif
