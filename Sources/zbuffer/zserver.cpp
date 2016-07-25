#ifdef WIN32
#include <winsock2.h>
#define socklen_t	int
#endif
#include "IServer.h"
#include "Cipher.h"
#include "KSG_EncodeDecode.h"

#include <time.h>
#include <string.h>
#include <errno.h>

#define MAX_SIGNAL	20
#define	SIG_SENDDATA	SIGRTMIN + MAX_SIGNAL + 1

static unsigned gs_holdrand = time(NULL);

static inline unsigned _Rand() {
    gs_holdrand = gs_holdrand * 214013L + 2531011L;
    return gs_holdrand;
}

static inline void RandMemSet(int nSize, unsigned char *pbyBuffer) {
    while (nSize--) {
        *pbyBuffer++ = (unsigned char)_Rand();
    }
}


int ZListenThread::action() {
	int index;
	while(true) {
//响应请求-----------------
//		SOCKET s = WSAAccept(parent->listen_socket, NULL, NULL, NULL, 0);
		struct sockaddr client_addr;
		socklen_t length = sizeof(client_addr);
		SOCKET s = accept(parent->listen_socket, &client_addr, &length);
		if (s == INVALID_SOCKET)
		{
#ifdef WIN32
			Sleep(10);
#else
			usleep(10);
#endif
			continue;
		}
//分配一个连接的资源
		index = parent->getConnection(s);
		if(index== -1) {
			printf("server max connection reached, client attempt fail.\n");
			closesocket(s);
		}
		else {
//平台相关,创建完成端口或者事件
#ifdef WIN32
			CreateIoCompletionPort((HANDLE)s, parent->CompletionPort, (DWORD)&parent->clients[index], 0);
#else
  fcntl(s, F_SETSIG, SIGRTMIN + (index % MAX_SIGNAL));
  fcntl(s, F_SETOWN, parent->pids[index % parent->max_thread]);
  int flags = fcntl(s, F_GETFL);
  flags |= O_NONBLOCK | O_ASYNC;
  fcntl(s, F_SETFL, flags);
//  fcntl(s, F_SET
#endif
//发送key值到客户端
			memcpy(&parent->clients[index].addr, &client_addr, length);
			parent->clients[index].server_key = _Rand();
			parent->clients[index].client_key = _Rand();
			ACCOUNT_BEGIN account_begin;
            RandMemSet(sizeof(account_begin), (unsigned char *)&account_begin);
			account_begin.ProtocolType = CIPHER_PROTOCOL_TYPE;
			account_begin.Mode         = 0;
			account_begin.ServerKey    = ~parent->clients[index].server_key;
			account_begin.ClientKey    = ~parent->clients[index].client_key;
			parent->PackDataToClient(index, &account_begin, sizeof(account_begin));
			int length;
			parent->buffer->sendPacket(parent->clients[index].write_index, length);
			parent->sendData(&parent->clients[index]);
#ifdef WIN32
			parent->receiveData(&parent->clients[index]);
#endif
			if(parent->call_back)
				parent->call_back(parent->parameter, index, enumClientConnectCreate);		//notify
			printf("client [%d] created. (%u, %u)\n", index, parent->clients[index].server_key, parent->clients[index].client_key);
		}
	}
	return 0;
}


// flying add this functino
// Data shoule be sent in a thread, so that the main-process will not be blocked.
int do_sendData(IServer* server, client_info* client) {
	int data_length;
	char *data_buffer = server->buffer->getSendData(client->write_index, data_length);
	if(!data_buffer) return -1;
	int nRetVal = send(client->sock, data_buffer, data_length, 0);
	if(nRetVal != -1) {
		server->buffer->sendData(client->write_index, nRetVal);
		if(nRetVal != data_length) {
			server->sendData(client);
		}
		return 1;
	}
	else {
		printf("send error %d\n", errno);
		if(errno == EAGAIN) {
			server->sendData(client);
		}
		else {
			printf("sending failed....close connection [%d].\n", client->index);
			server->closeConnection(client);
		}
	}
	return 1;
}


//实际的工作线程,平台相关
#ifdef WIN32
DWORD WINAPI work_thread(LPVOID id) {
#else
void *work_thread(LPVOID id) {
#endif
	IServer *parent = (IServer *)id;
#ifdef WIN32
	DWORD transferred;
	LPOVERLAPPED overlapped;
	DWORD flags;
	DWORD dwIoSize;
	client_info *client;
	while(true) {
		bool bIORet = GetQueuedCompletionStatus(parent->CompletionPort, &dwIoSize, (LPDWORD) &client, &overlapped, INFINITE);
		DWORD dwIOError = GetLastError();
		if(!bIORet && dwIOError != WAIT_TIMEOUT ) {
			closesocket(client->sock);
			parent->buffer->clear(client->read_index);
			parent->buffer->clear(client->write_index);
			client->sock = INVALID_SOCKET;
			if(parent->call_back)
				parent->call_back(parent->parameter, client->index, enumClientConnectClose);		//notify
			continue;
		}
		if(overlapped == &client->write_overlapped) {
			if(dwIoSize > 0) {
				parent->buffer->sendData(client->write_index, dwIoSize);
			}
			parent->sendData(client);
		}
		else {
			if(dwIoSize > 0) {
				parent->buffer->receiveData(client->read_index, (const char *)client->m_byInBuffer, dwIoSize);
			}
			parent->receiveData(client);
		}
	}
#else
  siginfo_t siginfo;
  int thread_index = parent->current_thread++;
  parent->pids[thread_index] = getpid();
  printf("thread %d start\n", thread_index);
  while(true) {
    int signum = sigwaitinfo(&parent->sigsets[thread_index], &siginfo);
    if(signum == -1) break;
    if(signum == SIGIO) {
      printf("overflow\n");
    }
	// insert this branch to send data out
	if (signum == SIG_SENDDATA) {
		client_info* client = (client_info*)siginfo.si_value.sival_ptr;
		do_sendData(parent, client);
	}
    else if(signum >= SIGRTMIN) {
		client_info *client;
		int index = signum - SIGRTMIN;
		while(index < parent->max_client) {
			client = &parent->clients[index];
			if(client->sock == siginfo.si_fd) {
				if(siginfo.si_band & 1) {			//有等待读入的数据
					parent->receiveData(client);
				}
				break;
			}
			index += MAX_SIGNAL;
		}
    }
  }
#endif
}

void IServer::closeConnection(client_info *client) {
	if(client->sock == INVALID_SOCKET) return;
	closesocket(client->sock);
	buffer->clear(client->read_index);
	buffer->clear(client->write_index);
	client->sock = INVALID_SOCKET;
	if(call_back) call_back(parameter, client->index, enumClientConnectClose);
}

bool IServer::sendData(client_info *client) {
	//int nRetryTime = 0;
//平台相关代码,实际发出重叠的IO请求,发送数据---------------------------------------
	int size;
#ifdef WIN32
	client->m_wsaOutBuffer.buf = buffer->getSendData(client->write_index, (int &)client->m_wsaOutBuffer.len);
	if(client->m_wsaOutBuffer.buf) {
		ULONG ulFlags = MSG_PARTIAL;
		ZeroMemory(&client->write_overlapped, sizeof(OVERLAPPED));
		int nRetVal = WSASend(client->sock, &client->m_wsaOutBuffer, 1, &client->m_wsaOutBuffer.len, 
			ulFlags, &client->write_overlapped, NULL);
		if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING ) {		//出现错误
			closeConnection(client);
		}
		else return true;
	}
#else
	// kill a signal
	union sigval info;
	info.sival_ptr = (void*)client;
	sigqueue(pids[client->index % max_thread], SIG_SENDDATA, info);
#endif
	return false;
}

bool IServer::receiveData(client_info *client) {
//平台相关代码,实际发出重叠的IO请求,接收数据---------------------------------------
#ifdef WIN32
	ULONG ulFlags = MSG_PARTIAL;
	DWORD dwIoSize;
	client->m_wsaInBuffer.buf = (char *)client->m_byInBuffer;
	client->m_wsaInBuffer.len = MAX_IN_BUFFER;
	UINT nRetVal = WSARecv(client->sock, &client->m_wsaInBuffer, 1, &dwIoSize, &ulFlags, &client->read_overlapped, NULL);
	if(nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
	    printf("receive data pending? close connection [%d].\n", client->index);
		closeConnection(client);
		return false;
	}
#else
  int ret = recv(client->sock, client->m_byInBuffer, MAX_IN_BUFFER, 0);
  if(ret <= 0 && errno != EWOULDBLOCK) {
    printf("receive data pending? close connection [%d].\n", client->index);
	closeConnection(client);
    return false;
  }
  buffer->receiveData(client->read_index, (const char *)client->m_byInBuffer, ret);
  #endif
  return true;
}

IServer::IServer(int number, int thread_number) {
	buffer = new ZBuffer(number * 800000, number * 2);
	clients = new client_info[number];
	max_client = number;
	int index;
	for(index = 0; index < max_client; index++) {
		clients[index].sock = INVALID_SOCKET;
		clients[index].index = index;
	}
	call_back = NULL;
	max_thread = thread_number;
	current_thread = 0;
	pids = new int[max_thread];
	sigsets = new sigset_t[max_thread];
	listen_thread = new ZListenThread(this);
}

IServer::~IServer() {
	listen_thread->stop();
	delete[] pids;
	delete[] sigsets;
	delete listen_thread;
	delete[] clients;
	delete buffer;
}

int IServer::getConnection(SOCKET sock) {
	int index;
	for(index = 0; index < max_client; index++) {
		if(clients[index].sock == INVALID_SOCKET) {
			clients[index].sock = sock;
			clients[index].read_index = buffer->getConnection();
			clients[index].write_index = buffer->getConnection();
			return index;
		}
	}
	return -1;
}

int init_socket = 0;
int IServer::Startup() {
	if(!init_socket) {
#ifdef WIN32
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
	int index;
	int signal_number;
	for(index = 0; index < max_thread; index++) {
		sigemptyset(&sigsets[index]);
		sigaddset(&sigsets[index], SIGIO);
		sigaddset(&sigsets[index], SIG_SENDDATA);
		signal_number = SIGRTMIN + index;
		while(signal_number < SIGRTMIN + MAX_SIGNAL) {
			sigaddset(&sigsets[index], signal_number);
			signal_number += max_thread;
		}
		sigprocmask(SIG_BLOCK, &sigsets[index], NULL);
	}
#endif
		init_socket++;
	}
	return 0;
}

int IServer::Cleanup() {
	if(init_socket--) {
#ifdef WIN32
		WSACleanup();
#else
	int index;
	for(index = 0; index < max_thread; index++) {
		sigprocmask(SIG_UNBLOCK, &sigsets[index], NULL);
	}
#endif
	}
	return 0;
}

int IServer::Release() {
	return 0;
}

int IServer::Open(const unsigned long &ulnAddressToListenOn, const unsigned short &usnPortToListenOn) {
//在此创建套接字,绑定,启动侦听线程
#ifdef WIN32
	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	int index;
	DWORD thread_id;
	for(index = 0; index < max_thread; index++) {
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, work_thread, this, 0, &thread_id);
		CloseHandle(hThread);
	}
	listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    pthread_t work;
	int index;
	for(index = 0; index < max_thread; index++) {
		int ret = pthread_create(&work, NULL, work_thread, this);
	}
#endif
	if(listen_socket == INVALID_SOCKET) return 0;
	struct sockaddr_in addr;
	
	memset((void*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(usnPortToListenOn);
	addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(listen_socket, (const sockaddr *)&addr, sizeof(addr))) {
		return 0;
	}
	if(listen(listen_socket, 5)) return 0;
	listen_thread->start();
	return 1;
}

int IServer::CloseService() {
	closesocket(listen_socket);
	return 0;
}

int IServer::RegisterMsgFilter(void * lpParam, CALLBACK_SERVER_EVENT pfnEventNotify) {
	call_back = pfnEventNotify;
	parameter = lpParam;
	return 0;
}

int IServer::PreparePackSink() {		//准备开始数据封装
	return 0;
}

int IServer::PackDataToClient(const unsigned long &ulnClientID, const void * pData,	unsigned long datalength) {
	if(clients[ulnClientID].sock != INVALID_SOCKET) {
		buffer->packData(clients[ulnClientID].write_index, (const char *)pData, datalength);
	}
	return 0;
}

int IServer::SendPackToClient() {
	int index;
	for(index = 0; index < max_client; index++) {
		if(clients[index].sock != INVALID_SOCKET) {
			int length;
			char *data = buffer->sendPacket(clients[index].write_index, length);
			if(data && clients[index].server_key) {
				KSG_EncodeBuf(length, (unsigned char *)data, (unsigned int *)&clients[index].server_key);
				sendData(&clients[index]);
			}
			return length;
		}
	}
	return 0;
}

int IServer::SendPackToClient(int index) {
	if (index < 0)
	{
		return SendPackToClient();
	}
	if(clients[index].sock != INVALID_SOCKET) {
		int length;
		char *data = buffer->sendPacket(clients[index].write_index, length);
		if(data) {
			KSG_EncodeBuf(length, (unsigned char *)data, (unsigned int *)&clients[index].server_key);
			sendData(&clients[index]);
		}
		return length;
	}
	return 0;
}

int IServer::SendData(const unsigned long &ulnClientID,	const void * pData, unsigned long datalength) {
	PackDataToClient(ulnClientID, pData, datalength);
	SendPackToClient(ulnClientID);
//发送一个消息
	return 0;
}
	
const void *IServer::GetPackFromClient(unsigned long ulnClientID, unsigned int &datalength) {
	if(clients[ulnClientID].sock != INVALID_SOCKET) {
		char *data = buffer->getPacket(clients[ulnClientID].read_index, (int &)datalength);
		if(data) {
			KSG_DecodeBuf(datalength, (unsigned char *)data, (unsigned int *)&clients[ulnClientID].client_key);
			return data;
		}
	}
	return NULL; 
}

int IServer::ShutdownClient(const unsigned long &ulnClientID) { 
	if(clients[ulnClientID].sock != INVALID_SOCKET) {
		closesocket(clients[ulnClientID].sock);
		buffer->clear(clients[ulnClientID].read_index);
		buffer->clear(clients[ulnClientID].write_index);
		clients[ulnClientID].sock = INVALID_SOCKET;
		printf("shut down client [%d]\n", ulnClientID); 
		if(call_back)
			call_back(parameter, ulnClientID, enumClientConnectClose);		//notify
	}
	return 0; 
}
