//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2002 by Kingsoft
//
// File:	KNetServer.cpp
// Date:	2002.07.10
// Code:	QiuXin
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KEngine.h"
#include "KHashTable.h"
#include "KNetServer.h"
#include "KCodec.h"
#include "KCodecLha.h"
#include "KMutex.h"
#include "crtdbg.h"

//---------------------------------------------------------------------------
#define N_SOCKET_MIN_PORTNUM   (7000)
#define N_SOCKET_MAX_PORTNUM   (7999)
#define N_SOCKET_PORTRANGE     (N_SOCKET_MAX_PORTNUM-N_SOCKET_MIN_PORTNUM)

#define NET_DEBUG

enum    //s2c系统协议，最大到31
{
	s2c_passwordchange = 1,
	s2c_replyping,
	s2c_shakehand,
};
enum  //c2s系统协议，最大到31
{
	c2s_replyshakehand = 1,
	c2s_sendping,
	c2s_passwordrecieved,	
};


KNetServer* g_pNetServer = NULL;
KMutex      Mutex[MAX_CLIENT_NUMBER];

//--------------------------------------------------------------------
//  GetPortNumFromName()
//--------------------------------------------------------------------
short KNetServer::GetPortNumFromName(char *pname)
{
	ULONG pnum = g_Hash(pname) + N_SOCKET_MIN_PORTNUM;
    return (short)pnum;
} 

//---------------------------------------------------------------------------
// 函数:	KNetServer
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KNetServer::KNetServer(void)
{
    g_pNetServer=this;
	m_Socket=0;
	n_ClientNum=0;
    tval.tv_sec=0;
    tval.tv_usec=100;

	//for debug only
	n_SendSize=0;
	n_RecordIndex=0;

}
//---------------------------------------------------------------------------
// 函数:	~KNetServer
// 功能:	分构
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KNetServer::~KNetServer(void)
{
	Close();
	g_FreeCodec((KCodec**)&pCodec, CODEC_LHA);
}

//---------------------------------------------------------------------------
// 函数:	Create
// 功能:	构造
// 参数:	char* 
// 返回:	void
//---------------------------------------------------------------------------
BOOL KNetServer::Create(char* port_name)
{
	WORD version = MAKEWORD(2, 0);//ver2.0
	struct WSAData wsa_data;
    int res;

    pCodec=NULL;	


    g_InitCodec((KCodec**)&pCodec, CODEC_LHA);
     
	
	
    for(int i=0;i<MAX_CLIENT_NUMBER;i++)
    {
		memset((char *)Client_data_array+i*sizeof(CLIENT_DATA),0,sizeof(CLIENT_DATA));
		memcpy(Client_data_array[i].SendBuffer.id,"PACK",4);
    }
	
	if (WSAStartup(version, &wsa_data))
	{
		g_DebugLog("KNetServer::Create(): WSAStartup() failed!");
		return FALSE;
	}
	
	
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_Socket == INVALID_SOCKET)
	{
		g_DebugLog("KNetServer::Create(): socket() failed!");
		return FALSE;
    }
	
	
    m_HostAddr.sin_family = AF_INET;
    m_HostAddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(m_HostAddr.sin_zero),0,sizeof(m_HostAddr.sin_zero));
	short port_num = GetPortNumFromName(port_name);
	m_HostAddr.sin_port = htons(port_num);
    res = bind(m_Socket,(struct sockaddr *)&(m_HostAddr),sizeof(m_HostAddr));
	if (res == SOCKET_ERROR)
	{
		g_DebugLog("KNetServer::Create(): max number of servers started!");
		return FALSE;
    }
	res = listen(m_Socket,MAX_CLIENT_NUMBER);
	if (res == SOCKET_ERROR)
	{
		g_DebugLog("KNetServer::Create(): listen failed!");
		return FALSE;
    }
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	CheckConnect
// 功能:	检查有无新增加的客户端连接
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer:: CheckConnect()
{
	int i;
	fd_set  Set;
	FD_ZERO(&Set);
	FD_SET(m_Socket, &Set);
	
	int s = select(m_Socket + 1, &Set, NULL,NULL, &tval);
	
	if (s > 0)
	{
		for(i=0;i< MAX_CLIENT_NUMBER; i ++)
			if(!Client_data_array[i].Socket) break;
			
			if(i>=MAX_CLIENT_NUMBER) return;
			
			sockaddr_in sockaddr;
			int		addrlen = sizeof(sockaddr);
			
			//增加该连结
			Client_data_array[i].Socket = accept(m_Socket, (struct sockaddr *)&sockaddr, &addrlen);
			if (Client_data_array[i].Socket > 0)
			{
				Client_data_array[i].LogStatus=LogStatus_Connected;
				memcpy(&Client_data_array[i].m_ClientAddr,&sockaddr,sizeof(sockaddr_in));
				n_ClientNum ++;
			}			
	}
}

//---------------------------------------------------------------------------
// 函数:	Encipher
// 功能:	将要发送给某个客户端封包加密（压缩前）
// 参数:	int nClientIndex
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::Encipher(int nClientIndex)
{
	int i,index=0;
	CLIENT_DATA  *pTemp=&Client_data_array[nClientIndex];
	BYTE *pTemp1=pTemp->SendBuffer.Data;
	BYTE *pTemp2=pTemp->Cipher;
    WORD Num=pTemp->SendBuffer.dwSize;

	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return ;

	if(!pTemp->CipherVerify) return;
    for(i=0;i<(int)Num;i++) 
	{
     *pTemp1^=*(pTemp2+index);
	 index=(index+1)%16;
	}
}
//---------------------------------------------------------------------------
// 函数:	MakeNewCipher
// 功能:	生成新的密钥备用
// 参数:	int nClientIndex
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::MakeNewCipher(int nClientIndex)
{
	int i;
	BYTE *pTemp=Client_data_array[nClientIndex].NextCipher;

	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return ;
   
    for(i=0;i<16;i++) 
	{
     *pTemp=(BYTE)(rand()%256);
      pTemp++;
	}
}
//---------------------------------------------------------------------------
// 函数:	RecieveData
// 功能:	将要发送给某个客户端的数据放在相关的封包内
// 参数:	int nClientIndex, BYTE *pBuffer,DWORD dwSize
// 返回:	int
//---------------------------------------------------------------------------
int KNetServer:: RecieveData(int nClientIndex, BYTE *pBuffer,DWORD dwSize)
{
	BYTE TempBuf[SEND_BUFFER_SIZE];
	SEND_DATA*  pTemp;
	int res=0;
	
	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return -1;
	
	if(!pBuffer||!dwSize) res = -1;
	_ASSERT(*pBuffer > 0);

	pTemp=&Client_data_array[nClientIndex].SendBuffer;
	if((pTemp->dwSize+dwSize)>SEND_BUFFER_SIZE) 
	{
		SendMessage(nClientIndex);
		res=1;
	}
	if((*pBuffer)<32&&pTemp->DataNum) //系统协议放在最前面
    {
     memcpy(TempBuf,pTemp->Data, pTemp->dwSize); 
	 memcpy(pTemp->Data,pBuffer, dwSize);
	 memcpy(pTemp->Data+dwSize,TempBuf,pTemp->dwSize);
    }
	else
	 memcpy(pTemp->Data + Client_data_array[nClientIndex].CurOffset, pBuffer, dwSize);
    
	

	pTemp->DataNum++;
	pTemp->dwSize+=(WORD)dwSize;
	Client_data_array[nClientIndex].CurOffset+=(WORD)dwSize;
	return res;
}

//---------------------------------------------------------------------------
// 函数:	SendMessage
// 功能:	向某一客户端发数据包
// 参数:	int nClientIndex
// 返回:	int
//---------------------------------------------------------------------------

int KNetServer::SendMessage(int nClientIndex)
{
	SEND_DATA*  pTemp;
	int len; 
	SOCKET socket;
	DWORD  dwRealSize;
	TCodeInfo tCodeInfo;
	BYTE      SwapBuf[SEND_BUFFER_SIZE+SEND_BUFFER_SIZE/10+1024];

	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return -100;
	socket=Client_data_array[nClientIndex].Socket;
	if(socket<=0) return -100;
	
    pTemp=&Client_data_array[nClientIndex].SendBuffer;
	if(!pTemp->DataNum) return -300;
	if(!pTemp->dwSize) return -400;
	
	if (Client_data_array[nClientIndex].LogStatus == LogStatus_UnConnected)
	{
		pTemp->DataNum = 0;
		pTemp->dwSize = 0;
		Client_data_array[nClientIndex].CurOffset=0;
		return 0;
	}
	
	fd_set clientSet;
	FD_ZERO(&clientSet);
	fd_set errSet;
	FD_ZERO(&errSet);
	
	
	FD_SET(socket, &clientSet);
	FD_SET(socket, &errSet);
	
	select(socket+ 1, NULL,&clientSet,&errSet,&tval);
	if (!FD_ISSET(socket, &clientSet)) 
	{
		pTemp->DataNum = 0;
		pTemp->dwSize = 0;
		pTemp->isCompressed=0;
		Client_data_array[nClientIndex].CurOffset=0;
		g_DebugLog("socket could not send ");
		return -200;
	}

    Mutex[nClientIndex].Lock();
    //加密
	 Encipher(nClientIndex); 

	//压缩发送包
	 

	if(pTemp->dwSize>MIN_COMPRESS_SIZE)
	{
		tCodeInfo.lpData=pTemp->Data;
		tCodeInfo.dwDataLen=pTemp->dwSize;
		tCodeInfo.lpPack=SwapBuf;
		if(((KCodecLha *)pCodec)->Encode(&tCodeInfo)==TRUE&&tCodeInfo.dwPackLen<pTemp->dwSize)
		{
			pTemp->dwCompSize=(WORD)tCodeInfo.dwPackLen;
			pTemp->isCompressed=1;
			memcpy(pTemp->Data,SwapBuf,tCodeInfo.dwPackLen);
		}
		else
			pTemp->dwCompSize=pTemp->dwSize;
	}
    else
		pTemp->dwCompSize=pTemp->dwSize;
	
	if (pTemp->dwSize > 32)
		g_DebugLog("[net]Compress Percent:%d.%d, size:%d", pTemp->dwCompSize * 100 / pTemp->dwSize, (pTemp->dwCompSize * 10000 / pTemp->dwSize) % 100, pTemp->dwSize);
	dwRealSize=pTemp->dwCompSize+sizeof(SEND_DATA)-SEND_BUFFER_SIZE;
	len=send(socket, (char *)pTemp,dwRealSize,0);


	Mutex[nClientIndex].Unlock();

	
	if(len<0)
    {
		Client_data_array[nClientIndex].IsolateNum++;
		pTemp->DataNum = 0;
		pTemp->dwSize = 0;
		pTemp->isCompressed=0;
		Client_data_array[nClientIndex].CurOffset=0;
		
		if(Client_data_array[nClientIndex].IsolateNum>=MAX_ISOLATE_NUM)
			Client_data_array[nClientIndex].LogStatus= LogStatus_ConnectFail;
		g_DebugLog("socket send fail:%d", len);
		return(len);
    }
	
    Client_data_array[nClientIndex].IsolateNum=0;
	pTemp->DataNum = 0;
	pTemp->dwSize = 0;
    pTemp->isCompressed=0;
	Client_data_array[nClientIndex].CurOffset=0;

//记录发送情况
	n_SendSize+= dwRealSize;
	n_SendRecord[n_RecordIndex]= (WORD)dwRealSize;
    n_RecordIndex=(n_RecordIndex+1)%16;
/*
#ifdef NET_DEBUG
	if(!n_RecordIndex)
	{
     DWORD nAvg= 0;
     for ( int i=0; i<16; i++ )
      nAvg+=(DWORD)n_SendRecord[i];
	  nAvg>>=4;
     g_DebugLog("Total message send is %d\n",n_SendSize);
	 g_DebugLog("Avg message size is %d\n",nAvg);
	}
#endif	 
*/
	return(len);
}
//---------------------------------------------------------------------------
// 函数:	SendDest
// 功能:	向某一客户端发数据包（发送线程使用）
// 参数:	int nClientIndex
// 返回:	int
//---------------------------------------------------------------------------
int KNetServer::SendDest(int nClientIndex)
{
	SEND_DATA*  pTemp;
	int len; 
	SOCKET socket;
	DWORD  dwRealSize;
	TCodeInfo tCodeInfo;
	BYTE      SwapBuf[SEND_BUFFER_SIZE+SEND_BUFFER_SIZE/10+1024];

	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return -100;
	socket=Client_data_array[nClientIndex].Socket;
	if(socket<=0) return -100;
	
    pTemp=&Client_data_array[nClientIndex].SendDest;
	if(!pTemp->DataNum) return -300;
	if(!pTemp->dwSize) return -400;
	
	if (Client_data_array[nClientIndex].LogStatus == LogStatus_UnConnected)
	{
		pTemp->DataNum = 0;
		pTemp->dwSize = 0;
		Client_data_array[nClientIndex].CurOffset=0;
		return 0;
	}
	
	fd_set clientSet;
	FD_ZERO(&clientSet);
	fd_set errSet;
	FD_ZERO(&errSet);
	
	
	FD_SET(socket, &clientSet);
	FD_SET(socket, &errSet);
	
	len=select(socket+ 1, NULL,&clientSet,&errSet,&tval);
	if(!len||len==SOCKET_ERROR) return -200;
	if (!FD_ISSET(socket, &clientSet)) 
	{
		pTemp->DataNum = 0;
		pTemp->dwSize = 0;
		pTemp->isCompressed=0;
		Client_data_array[nClientIndex].CurOffset=0;
		g_DebugLog("socket could not send ");
		return -200;
	}

    //加密
	 Encipher(nClientIndex); 

	//压缩发送包
	 

	if(pTemp->dwSize>MIN_COMPRESS_SIZE)
	{
		tCodeInfo.lpData=pTemp->Data;
		tCodeInfo.dwDataLen=pTemp->dwSize;
		tCodeInfo.lpPack=SwapBuf;
		if(((KCodecLha *)pCodec)->Encode(&tCodeInfo)==TRUE&&tCodeInfo.dwPackLen<pTemp->dwSize)
		{
			pTemp->dwCompSize=(WORD)tCodeInfo.dwPackLen;
			pTemp->isCompressed=1;
			memcpy(pTemp->Data,SwapBuf,tCodeInfo.dwPackLen);
		}
		else
			pTemp->dwCompSize=pTemp->dwSize;
	}
    else
		pTemp->dwCompSize=pTemp->dwSize;

	if (pTemp->dwSize > 32)
		g_DebugLog("[net]Compress Percent:%d.%d, size:%d", pTemp->dwCompSize * 100 / pTemp->dwSize, (pTemp->dwCompSize * 10000 / pTemp->dwSize) % 100, pTemp->dwSize);

	dwRealSize=pTemp->dwCompSize+sizeof(SEND_DATA)-SEND_BUFFER_SIZE;
	len=send(socket, (char *)pTemp,dwRealSize,0);

	if(len<0)
    {
		Client_data_array[nClientIndex].IsolateNum++;
		pTemp->DataNum = 0;
		pTemp->dwSize = 0;
		pTemp->isCompressed=0;
		Client_data_array[nClientIndex].CurOffset=0;
		
		if(Client_data_array[nClientIndex].IsolateNum>=MAX_ISOLATE_NUM)
			Client_data_array[nClientIndex].LogStatus= LogStatus_ConnectFail;
		g_DebugLog("socket send fail:%d", len);
		return(len);
    }
	
    Client_data_array[nClientIndex].IsolateNum=0;
	pTemp->DataNum = 0;
	pTemp->dwSize = 0;
    pTemp->isCompressed=0;
	Client_data_array[nClientIndex].CurOffset=0;

//记录发送情况
	n_SendSize+= dwRealSize;
	n_SendRecord[n_RecordIndex]= (WORD)dwRealSize;
    n_RecordIndex=(n_RecordIndex+1)%16;
/*
#ifdef NET_DEBUG
	if(!n_RecordIndex)
	{
     DWORD nAvg= 0;
     for ( int i=0; i<16; i++ )
      nAvg+=(DWORD)n_SendRecord[i];
	  nAvg>>=4;
     g_DebugLog("Total message send is %d\n",n_SendSize);
	 g_DebugLog("Avg message size is %d\n",nAvg);
	}
#endif	 
*/
	return(len);
}


//---------------------------------------------------------------------------
// 函数:	RecieveMessage
// 功能:	接收某一客户端的数据包
// 参数:	int nClientIndex, DWORD dwSize
// 返回:	int
//---------------------------------------------------------------------------
int KNetServer:: RecieveMessage(int nClientIndex, DWORD dwSize)
{
	int len;
	SOCKET socket;
	timeval tval_recv;          
	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return -100;
	socket=Client_data_array[nClientIndex].Socket;
	if(socket<=0) return -100;

    tval_recv.tv_sec=0;
    tval_recv.tv_usec=0;
	fd_set clientSet;
	FD_ZERO(&clientSet);
	fd_set errSet;
	FD_ZERO(&errSet);
	
	
	FD_SET(socket, &clientSet);
	FD_SET(socket, &errSet);
	len=select(socket+ 1, &clientSet,NULL,&errSet,&tval_recv);
	if(!len||len==SOCKET_ERROR) return -200;
	if (!FD_ISSET(socket, &clientSet)) return -200;

	len=recv(socket,(char *)Client_data_array[nClientIndex].RecieveBuffer,dwSize,0);
	Client_data_array[nClientIndex].LastRecvSize=len;
	return(len);
}

//---------------------------------------------------------------------------
// 函数:	MessagePreProcess
// 功能:	处理某一客户端的数据包中系统维护部分
// 参数:	int nClientIndex
// 返回:	int
//---------------------------------------------------------------------------
int KNetServer:: MessagePreProcess(int nClientIndex)
{
	int i,nSize;
	CLIENT_DATA  *pTemp=&Client_data_array[nClientIndex];
	BYTE Buffer[32];
	WORD *pTemp0=(WORD *)pTemp->RecieveBuffer;
    BYTE *pTemp1;

	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return -1;

    int nTime=pTemp->SendBuffer.CountNum-pTemp->m_nEnterLoopRate;
	
    if(pTemp->nShakeHandNum > 256) //10秒不连通就宣布断线
	 pTemp->LogStatus= LogStatus_ConnectFail;

    //25个循环发一次握手信息
    if(!(nTime%25))
	{
		Buffer[0]=s2c_shakehand;
		RecieveData(nClientIndex, Buffer,4);
        pTemp->nShakeHandNum++;
	}
	//10分钟更换一次密钥25*60*10=15000
	

    if(!(nTime%15000))
    {
		MakeNewCipher(nClientIndex);
		Buffer[0]=s2c_passwordchange;
		memcpy(Buffer+4,pTemp->NextCipher,16);
		RecieveData(nClientIndex, Buffer,20);
    }

    nSize=pTemp->dwDestSize;
	if(nSize<=2) return 0;
    while(TRUE)
	{
		pTemp1=(BYTE *)pTemp0 +2;
		switch(*pTemp1)
		{
		case c2s_replyshakehand:
			pTemp->IsolateNum=0;
            pTemp->nShakeHandNum=0;
			break;
		case c2s_sendping:
			memcpy(Buffer,pTemp->RecieveBuffer,8);
			Buffer[0]=s2c_replyping;
			RecieveData(nClientIndex, Buffer,8);
			break;
		case c2s_passwordrecieved:
			memcpy(pTemp->Cipher,pTemp->NextCipher,16);
			pTemp->CipherVerify=0;
			for(i=0;i<16;i++)
				pTemp->CipherVerify+=(WORD)pTemp->Cipher[i];
			pTemp->SendBuffer.CipherVerify=pTemp->CipherVerify;
			break;
		default:
			break;
		}
		int protocol_size= *pTemp0;  
		if(protocol_size<=2) break;
		nSize-= protocol_size;
		if(nSize<=2) break;
		pTemp0=(WORD *)((BYTE *)pTemp0 + protocol_size);
	}
	return 0;
}

//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::SetCountNum(int nClientIndex, int nCountNum)
{

 //_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
 if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return;
 if(!Client_data_array[nClientIndex].SendBuffer.CountNum)
    Client_data_array[nClientIndex].m_nEnterLoopRate=nCountNum;
    Client_data_array[nClientIndex].SendBuffer.CountNum=nCountNum;
}
//---------------------------------------------------------------------------
// 函数:	ClearRecvInfo
// 功能:	清空输入信息
// 参数:	nClientIndex
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::ClearRecvInfo( int nClientIndex)
{
	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return ;
    Client_data_array[nClientIndex].dwDestSize=0;
}

//--------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// 函数:	DelOneClient
// 功能:	断开一个客户端连接
// 参数:	nClientIndex
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::DelOneClient(int nClientIndex)
{
	_ASSERT(nClientIndex >=0 && nClientIndex < MAX_CLIENT_NUMBER);
	if(nClientIndex<0||nClientIndex>=MAX_CLIENT_NUMBER) return ;
	Client_data_array[nClientIndex].Socket= 0;
	Client_data_array[nClientIndex].LogStatus =LogStatus_UnConnected;
	n_ClientNum --;
}
//---------------------------------------------------------------------------
// 函数:	CopyDataToTemp
// 功能:	把发送数据传输到中间区
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::CopyDataToTemp()
{
	int i;
    CLIENT_DATA*  pTemp;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
      pTemp=&Client_data_array[i];
	  if(!pTemp->Socket) continue;
      if(!pTemp->SendBuffer.DataNum) continue;
      if(pTemp->SendTemp.DataNum) continue;

	  Mutex[i].Lock();
	  memcpy(&pTemp->SendTemp,&pTemp->SendBuffer,sizeof(SEND_DATA));
	  pTemp->SendBuffer.DataNum = 0;
	  pTemp->SendBuffer.dwSize = 0;
	  pTemp->SendBuffer.isCompressed=0;
	  Client_data_array[i].CurOffset=0;
      Mutex[i].Unlock();
	}
}

//---------------------------------------------------------------------------
// 函数:	CopyTempToDest
// 功能:	把发送数据传输到中间区
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::CopyTempToDest()
{
	int i;
    CLIENT_DATA*  pTemp;
    
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
      pTemp=&Client_data_array[i];
	  if(!pTemp->Socket) continue;
      if(!pTemp->SendTemp.DataNum) continue;
	  Mutex[i].Lock();
	  memcpy(&pTemp->SendDest,&pTemp->SendTemp,sizeof(SEND_DATA));
	  memset(&pTemp->SendTemp,0,sizeof(SEND_DATA));
      Mutex[i].Unlock();
	}
}
//---------------------------------------------------------------------------
// 函数:	RecvDataToTemp
// 功能:	把接受数据传输到中间区
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::RecvDataToTemp()
{
	int i;
    CLIENT_DATA*  pTemp;
    BYTE*   pTemp1;
	WORD*   pSize;
    int    nLeftSize;	
    
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
      pTemp=&Client_data_array[i];
	  if(!pTemp->Socket) continue;
      if(pTemp->LastRecvSize <= 2) continue;

	  pTemp1=pTemp->RecieveBuffer;
	  nLeftSize=(WORD)pTemp->LastRecvSize;
      while(TRUE)
	 {
		pSize=(WORD *)pTemp1;  
		if((*pSize)<=2) break;
		nLeftSize = nLeftSize-(*pSize);
		if(nLeftSize<0) break;
		pTemp1 = pTemp1 +(*pSize);
        if(nLeftSize<2) break; 
	  }
	  pTemp->LastRecvSize=0;
      nLeftSize=pTemp1-pTemp->RecieveBuffer;
	  if((pTemp->dwTempSize+nLeftSize)>=8*RECIEVE_BUFFER_SIZE) continue;
	  Mutex[i].Lock();
	  memcpy(pTemp->RecieveTemp+pTemp->dwTempSize,pTemp->RecieveBuffer,nLeftSize);
//	g_DebugLog("copy data to temp size is %d\n",nLeftSize);
      pTemp->dwTempSize+=nLeftSize;
      Mutex[i].Unlock();
	}
}
//---------------------------------------------------------------------------
// 函数:	RecvTempToDest
// 功能:	把接受中间数据传输到处理
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::RecvTempToDest()
{
	int i;
    CLIENT_DATA*  pTemp;
    
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
      pTemp=&Client_data_array[i];
	  if(!pTemp->Socket) continue;
      if(!pTemp->dwTempSize) continue;
	  Mutex[i].Lock();
	  memcpy(pTemp->RecieveDest,pTemp->RecieveTemp,pTemp->dwTempSize);
//	  g_DebugLog("copy temp to dest size is %d\n",pTemp->dwTempSize);
      pTemp->dwDestSize=pTemp->dwTempSize;
	  pTemp->dwTempSize=0;
      Mutex[i].Unlock();
	}
}


//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KNetServer::Close(void)
{
    // close socket
    if (m_Socket)
	{
        shutdown(m_Socket,2);
        closesocket(m_Socket);
    }
    WSACleanup();
}

//--------------------------------------------------------------------------------
