//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2002 by Kingsoft
//
// File:	KNetServer.h
// Date:	2002.07.10
// Code:	QiuXin
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KNetServer_H
#define	KNetServer_H
//---------------------------------------------------------------------------
#include <winsock.h>

#define  MAX_CLIENT_NUMBER   1024
#define  SEND_BUFFER_SIZE      1024
#define  RECIEVE_BUFFER_SIZE 256 

#define  MAX_BACK_NUM        32 
#define  MAX_ISOLATE_NUM     128 
#define  MIN_COMPRESS_SIZE   1024

enum LOGSTATUS
{
	LogStatus_UnConnected,
	LogStatus_Connected,
	LogStatus_DBLoading,	// Loading PlayerInfo From DataBase
	LogStatus_Sync,			// Send(Sync) PlayerInfo To Client
	LogStatus_Login,	
	LogStatus_ConnectFail,
};

typedef struct {
                char   id[4];//"PACK"
				int    CountNum;//服务器循环数
				WORD   isCompressed;//0 不压缩 1：压缩
				WORD   DataNum;//数据块数目
				WORD   dwSize;//整个可用数据包大小（不包含Data以前的数据头）
                WORD   dwCompSize;//压缩后可用数据包大小，不压缩时此值与dwSize相同 
				WORD   CipherVerify;//密钥校验和
				WORD   wResered;
				BYTE   Data[SEND_BUFFER_SIZE];//数据区
        		}SEND_DATA;

typedef struct {
                SOCKET          Socket;
				struct sockaddr_in m_ClientAddr;
				int             m_nEnterLoopRate;//进入时的系统计数    
                int             LogStatus;//0 未连接 1连接 2申请加入 3已加入 4异常断线
	            int             IsolateNum;    //发送失败次数
				BYTE            RecieveBuffer[RECIEVE_BUFFER_SIZE];
				int             LastRecvSize;
				BYTE            RecieveTemp[8*RECIEVE_BUFFER_SIZE];
				DWORD           dwTempSize;
                BYTE            RecieveDest[8*RECIEVE_BUFFER_SIZE];
				DWORD           dwDestSize;
				int             nShakeHandNum;
                SEND_DATA       SendBuffer; 
                SEND_DATA       SendTemp; 
                SEND_DATA       SendDest;   
                WORD            CurOffset;//当前数据块偏移
				BYTE            Cipher[16];
				BYTE            NextCipher[16];
				WORD            CipherVerify;//密钥校验和
         		}CLIENT_DATA;
//---------------------------------------------------------------------------

class ENGINE_API KNetServer
{
private:
	struct sockaddr_in m_HostAddr;
	int                n_ClientNum;   //已经连接的客户端数目
    timeval            tval;          //检测间隔 (为1毫秒)
    void*	           pCodec;

//
	short GetPortNumFromName(char *pname); 
	void  Encipher(int nClientIndex);
public:
    SOCKET m_Socket;
    CLIENT_DATA Client_data_array[MAX_CLIENT_NUMBER];
//for debug only    
	DWORD  n_SendSize;   //总共发送的数据量
    WORD   n_SendRecord[16];//最近16次发送的数据量
	int    n_RecordIndex;//最新位置指针
//

    KNetServer(void);
    virtual ~KNetServer(void);
	virtual BOOL Create(char* portname);
	virtual void Close();
	virtual void CheckConnect();
	virtual int  RecieveData(int nClientIndex,BYTE *pBuffer,DWORD dwSize);
	virtual int  RecieveMessage(int nClientIndex,DWORD dwSize);
	virtual int  MessagePreProcess(int nClientIndex);
	virtual int  SendMessage(int nClientIndex);
	virtual int  SendDest(int nClientIndex);
    void CopyDataToTemp(void);
	void CopyTempToDest(void);
    void RecvDataToTemp(void);
	void RecvTempToDest(void);
	void MakeNewCipher(int nClientIndex);   
    void ClearRecvInfo(int nClientIndex); 
	void SetCountNum(int nClientIndex,int nCountNum);  
	int  GetClientNum() {return n_ClientNum; };
	void DelOneClient(int nClientIndex);
};
//---------------------------------------------------------------------------
extern ENGINE_API KNetServer* g_pNetServer;

#endif
