//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2002 by Kingsoft
//
// File:	KNetClient.h
// Date:	2002.07.10
// Code:	Qiuxin
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KNetClient_H
#define	KNetClient_H
//---------------------------------------------------------------------------
#include <winsock.h>

//---------------------------------------------------------------------------
#define  RECV_DATA_SIZE      1024 
#define  SEND_DATA_SIZE      128 

#define  WM_SOCKET_READ      WM_USER+103

typedef struct {
	DWORD			dwSize;
	BYTE			Data[64*1024];
}RECV_BUFFER;

typedef struct {
	DWORD			dwNum;
	DWORD			dwCurrentOffset;//当前读取数据位置
	BYTE			Data[64*1024];
} COMMAND;


typedef struct {
	char			id[4];//"PACK"
	int				CountNum;//服务器循环数
	WORD			isCompressed;//0 不压缩 1：压缩
	WORD			DataNum;//数据块数目
	WORD			dwSize;//整个可用数据包大小（不包含Data以前的数据头）
	WORD			dwCompSize;//压缩后可用数据包大小，不压缩时此值与dwSize相同 
	WORD			CipherVerify;//密钥校验和	
	WORD            wResered;
	BYTE			Data[RECV_DATA_SIZE];//数据区
} RECV_DATA;


class ENGINE_API KNetClient
{
private:
	SOCKET		m_Socket;   
	sockaddr_in m_ServerAddr;
	short		m_ServerPort;
	char		m_ServerHostName[64];
	char		m_ServerPortName[64];
	int         RecvFlag;	
    void*		pCodec;
	DWORD       Ping;

	WORD        DataNum;//数据块数目
	WORD        dwDataSize; //数据包大小
    BYTE        SendBuffer[SEND_DATA_SIZE];

    RECV_BUFFER RecvBuffer;
	COMMAND     CoreCommand;
	COMMAND     SysCommand;
	
    RECV_DATA   RecvData;
    RECV_DATA   BackRecvData;//后备消息包
    DWORD       BackOffset;//后备消息包当前位置 
	
	BYTE        Cipher[16];//密钥
	BYTE        LastCipher[16];
	WORD        CipherVerify;//密钥校验和
	
	
private:
	bool		FillServerAddr(char* name);
	short		GetPortNumFromName(char* name);
	void        Decipher(void);
	void        SortProtocol(void);
	BOOL        DataProcess(void);
	char *      FindStringInMem(char *pMem,DWORD dwSize,char *pString);
	BOOL        DeCompress(RECV_DATA*  pData);
public:
    DWORD     RecvDataSize;//for debug only
   
	KNetClient();
	virtual ~KNetClient();
	virtual bool	Open();
	virtual void	Close();
	virtual bool	Connect(char *name);
    virtual void	DisConnect(void); 
	virtual BOOL	SendMsg(void*buf, long size);
	        int     RecieveData(BYTE *pBuffer,DWORD dwSize);
	        BOOL	SendPackage(void);
	virtual BOOL	RecvMsg();
	virtual char	*GetHostName(void);
	virtual char	*GetPortName(void);
    virtual int     MessagePreProcess(void);
	int             SendPing(void); 
	int				GetRecvStatus(void) {return RecvFlag;}; 
	int             GetPing(void) {return Ping;};
    COMMAND *       GetCommand(void){return &CoreCommand;};
	void			ClearRecvFlag();

};
extern ENGINE_API KNetClient* g_pNetClient;
//---------------------------------------------------------------------------
#endif
