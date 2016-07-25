#include <stdafx.h>
//#include "IClient.h"
#include "..\..\LINUX\IClient.h"
#include "KSG_EncodeDecode.h"
#include <winsock2.h>
#define MAX_TEMP_SIZE		65000
int init_client_socket = 0;

int IClientThread::action() {
//select进行发送和接收
	fd_set read_set, write_set;
	struct timeval time_out;
	time_out.tv_sec = 0;
	time_out.tv_usec = 2;
	//char temp_buffer[MAX_TEMP_SIZE];														//64k的临时缓冲区
	int length = 0;
	while(true) {
		if(parent->bStop) 
			break;
	    FD_ZERO(&read_set);
		FD_SET(parent->nSocket, &read_set);
		if (select(parent->nSocket + 1, &read_set, NULL, NULL, &time_out) != 0) {					//有数据等待读
			int bufsize = 0;
			char* recv_buffer = parent->buffer->getReceiveBuffer(bufsize);
			if (!recv_buffer)
				break;
			length = recv(parent->nSocket, recv_buffer, bufsize, 0);
			if(length < 0) {																//出错了，连接中断
				//int error = WSAGetLastError();
				break;
			}
			else {
//				if (length > 0)
//					length = length;
				parent->buffer->receiveData(length);
			}
		}
		if(parent->bStop) break;
		char *send_ptr = parent->buffer->getSendData(length);
		if(send_ptr) {
			FD_ZERO(&write_set);
			FD_SET(parent->nSocket, &write_set);
			//printf("send data %d\n", length);
			if(select(parent->nSocket + 1, NULL, &write_set, NULL, &time_out) != 0) {					//有数据等待写
				int length1 = send(parent->nSocket, send_ptr, length, 0);
				if(length1 < 0) {
					break;
				}
				else {
					if (length1 != length)
						length1 = length1;
					parent->buffer->sendData(length1);
				}
			}
		}
#ifndef __linux
		Sleep(1);
#else
		usleep(1);
#endif
	}
	if(parent->call_back) parent->call_back(parent->call_back_param, enumServerConnectClose);
	parent->buffer->clear();
	parent->m_uServerKey = parent->m_uClientKey = 0;
	return 0;
}

IClient::IClient(int max_send, int max_receive) {
	if(!init_client_socket) {
#ifndef __linux
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(1, 1), &wsa_data);
#endif
		init_client_socket++;
	}
	buffer = new ZBuffer(max_send, max_receive);
	call_back = NULL;
	m_uServerKey = m_uClientKey = 0;
	bStop = false;
	thread = new IClientThread(this);
	bConnected = false;
}

IClient::~IClient() {
	delete thread;
	if(--init_client_socket) {
#ifndef __linux		
		WSACleanup();
#endif
	}
}

int IClient::Startup() {
	return 1;
}

int IClient::Cleanup() {
	return 1;
}

int IClient::Shutdown() {
	bStop = true;
	if (bConnected) {
		shutdown(nSocket, 2);
		closesocket(nSocket);
		bConnected = false;
	}
	return 0;
}


bool IClient::ConnectTo(const char * const &pAddressToConnectServer, unsigned short usPortToConnectServer) {
	int nReturn;
	if (bConnected)
		return false;
	nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (nSocket == -1)
		return false;
    struct sockaddr_in addrServer;
	
	memset((void*)&addrServer, 0, sizeof(addrServer));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(usPortToConnectServer);
	addrServer.sin_addr.s_addr = inet_addr(pAddressToConnectServer);
	//memset(&addrServer.sin_zero, 0, 8);

	nReturn = connect(nSocket,(struct sockaddr *)&addrServer, sizeof(addrServer));
	if(nReturn < 0) 
		return false;
/*	read_index = buffer->getConnection();
	if (read_index == -1)
	{
		return false;
	}
	write_index = buffer->getConnection();
	if (write_index == -1) 
		return false;*/
	if (call_back) 
		call_back(call_back_param, enumServerConnectCreate);
	bStop = false;
	thread->start();
	bConnected = true;
	return true;
}

void IClient::RegisterMsgFilter(void * lpParam, CALLBACK_CLIENT_EVENT pfnEventNotify) {
	call_back = pfnEventNotify;
	call_back_param = lpParam;
}

bool IClient::SendPackToServer( const void * const pData, const unsigned long datalength) {
	int count = 0;
	while(!m_uClientKey) {												//如果还没有密钥，则收第一个包(包含密钥)
#ifndef __linux
		Sleep(1);
#else
		usleep(1);
#endif
		unsigned int length;
		GetPackFromServer(length);
		if(++count == 1000)
			return false;
	}
	buffer->packData((const char *)pData, datalength);
	int length;
	char *data = buffer->completePacket(length);
	if(!data) return false;
	unsigned& uKey = m_uClientKey;
	KSG_EncodeBuf(length, (unsigned char *)data, &uKey);
	buffer->sendPacket();
	return true;
}

void *IClient::GetPackFromServer(unsigned int &datalength ) {
	if(m_uServerKey == 0) {			//第一个数据包
		ACCOUNT_BEGIN *pAccountBegin = (ACCOUNT_BEGIN * )buffer->getPacket((int &)datalength);
		if(pAccountBegin) {
			printf("first packet %d %d\n", pAccountBegin->ServerKey, pAccountBegin->ClientKey);
			if(pAccountBegin->ProtocolType == CIPHER_PROTOCOL_TYPE) {
				m_uServerKey = ~(pAccountBegin->ServerKey);
				m_uClientKey = ~(pAccountBegin->ClientKey);
			}
		}
		return NULL;
	}
	else {
		char *data = (char*)buffer->getPacket((int &)datalength);
		if(data) {
/*			printf("-----BEFORE uServerKey = %u\ndatalen = %u, data = ", m_uServerKey, datalength);
			for (int i = 0; i < 32; i++)
				printf("[%02X] ", ((BYTE *)data)[i]);
			printf("\n");*/
			unsigned& uKey = m_uServerKey;
			KSG_DecodeBuf(datalength, (unsigned char *)data, &uKey);
/*			printf("-----AFTER uServerKey = %u\ndatalen = %u, data = ", m_uServerKey, datalength);
			for (int i = 0; i < 32; i++)
				printf("[%02X] ", ((BYTE *)data)[i]);
			printf("\n");*/
		}
		return data;
	}
}
