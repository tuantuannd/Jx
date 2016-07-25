//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNetServerNode.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KNetServerNode_H
#define	KNetServerNode_H
//---------------------------------------------------------------------------
#include <winsock.h>
//---------------------------------------------------------------------------
class ENGINE_API KNode;
class ENGINE_API KMutex;
class ENGINE_API KNetThread;
class ENGINE_API KNetServer;
class ENGINE_API KNetServerNode : public KNode
{
public:
    KNetThread m_ReceiveThread;// receiver thread
    KNetServer* m_Server;// my mother server
    long m_Id;
    SOCKET m_ServerSocket;
    SOCKET m_AcceptSocket;
    KMutex m_Mutex;
    struct sockaddr_in m_ClientAddr;
    char m_ReceiveBuf[32 * 1024];
    
    KNetServerNode(KNetServer *_server);
    ~KNetServerNode(void);
    bool Listen(void);
    bool Accept(void);
    void Ignore(void);
    bool Send(void *buf, long size);
};
//---------------------------------------------------------------------------
#endif
