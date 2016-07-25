//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNetServerNode.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Server node class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KStrBase.h"
#include "KMsgNode.h"
#include "KHashtable.h"
#include "KMemManager.h"
#include "KMutex.h"
#include "KNetThread.h"
#include "KNetServer.h"
#include "KNetServerNode.h"
//--------------------------------------------------------------------
//  n_miniserver_tfunc()
//--------------------------------------------------------------------
int THREADPROC n_miniserver_tfunc(KNetThread *t)
{
    // 
    // the following special case commands are defined:
    //
    // ~handshake [portname] -> ~true or ~false
    // Client asks whether this is really the right 
    // port (since the portname is hashed into a 
    // port number, there may be collissions).
    //
    // ~close   -> ~ok
    // Client requests to shut down the connection.
    //  
    bool connected = TRUE;
    t->ThreadStart();
    KNetServerNode *ims = (KNetServerNode *) t->LockUserData();
    t->UnlockUserData();
    do 
	{
        int len = recv(ims->m_AcceptSocket,ims->m_ReceiveBuf,
                  sizeof(ims->m_ReceiveBuf),0);
        if (len < 1)
		{
            // an error, or socket has been closed
            g_DebugLog("KNetServerNode: recv() returned %d\n",len);
            connected = FALSE;
        }
		else
		{            
            // check for special case command
            if (ims->m_ReceiveBuf[0] == '~')
			{
                char *result = NULL;
                char *cmd = strtok(ims->m_ReceiveBuf," \t~");
                if (strcmp(cmd,"handshake")==0)
				{
                    // client checks for right portname
                    char *pname = strtok(NULL," \t~");
                    if (pname)
					{
                        if (strcmp(pname,ims->m_Server->m_PortName)==0)
							result = "~true";
                        else
						{
							// ooops, wrong port, shut down connection
							result	  = "~false";
							connected = FALSE;
						}
                    }
					else 
					{
                        g_DebugLog("KNetServerNode::n_miniserver_tfunc(): ~handshake command broken!");
                    }
                } 
				else if (strcmp(cmd,"close")==0)
				{
                    result    = "~ok";
                    connected = FALSE;
                }
				else
				{
					g_DebugLog("KNetServerNode::n_miniserver_tfunc(): unknown internal command!");
				}

                // return answer to special case command
                if (result)
				{
					ims->m_Mutex.Lock();
                    send(ims->m_AcceptSocket,result,strlen(result)+1,0);
					ims->m_Mutex.Unlock();
                }
            }
			else
			{
                // normal message, add to msg list and emit signal
                KMsgNode *nd = new KMsgNode();
				nd->SetMsg(ims->m_ReceiveBuf,len);
                nd->SetPtr((void *)ims->m_Id);
                ims->m_Server->m_MsgList.Lock();
                ims->m_Server->m_MsgList.AddTail(nd);
                ims->m_Server->m_MsgList.Unlock();
                ims->m_Server->m_MsgList.SignalEvent();
            }
        }
    } 
	while (connected && !(t->ThreadStopRequested()));

    ims->m_Mutex.Lock();
	if (ims->m_AcceptSocket)
	{
        shutdown(ims->m_AcceptSocket,2);
    	closesocket(ims->m_AcceptSocket);
    	ims->m_AcceptSocket = 0;
	}
    ims->m_Mutex.Unlock();
    g_DebugLog("KNetServerNode: client %d shutting down.\n",(int)ims);
    t->ThreadStop();
    return 0;
}

//--------------------------------------------------------------------
//  n_miniserver_wakeup()
//  28-Oct-98   floh    created
//--------------------------------------------------------------------
void n_miniserver_wakeup(KNetThread *t)
{
    KNetServerNode *ims = (KNetServerNode *) t->LockUserData();
    t->UnlockUserData();
    ims->m_Mutex.Lock();
    shutdown(ims->m_AcceptSocket,2);
    closesocket(ims->m_AcceptSocket);
    ims->m_AcceptSocket=0;
    ims->m_Mutex.Unlock();
}        

//---------------------------------------------------------------------------
// 函数:	KNetServerNode
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KNetServerNode::KNetServerNode(KNetServer *server)
{
    m_Server = server;
    m_Id = m_Server->m_nServerNode++;
    m_ServerSocket = server->m_Socket;
    m_AcceptSocket = 0;
    memset(&(m_ClientAddr),0,sizeof(m_ClientAddr));
    m_Server->m_ServerList.Lock();
    m_Server->m_ServerList.AddTail(this);
    m_Server->m_ServerList.Unlock();
}
//---------------------------------------------------------------------------
// 函数:	~KNetServerNode
// 功能:	分构
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KNetServerNode::~KNetServerNode(void)
{
}
//--------------------------------------------------------------------
//  Listen()
//  28-Oct-98   floh    created
//  31-Oct-98   floh    divided into Listen(), Accept() und Ignore()
//--------------------------------------------------------------------
bool KNetServerNode::Listen(void)
{
    int addr_len;
    bool retval = FALSE;
    
    // wait for a client...
    if (listen(m_ServerSocket,5) != SOCKET_ERROR)
	{
        addr_len = sizeof(m_ClientAddr);
        m_AcceptSocket = accept(m_ServerSocket,
			(struct sockaddr *)&(m_ClientAddr),
			&addr_len);
        if (m_AcceptSocket != INVALID_SOCKET)
		{
            g_DebugLog("client %d: connection accepted from %s.\n", (int) this,
                     inet_ntoa(m_ClientAddr.sin_addr));
            retval = TRUE;
        } 
		else 
			g_DebugLog("KNetServerNode::Listen(): accept() failed!");
    } 
	else 
		g_DebugLog("KNetServerNode::Listen(): listen() failed!");
    return retval;
}

//--------------------------------------------------------------------
//  Accept()
//  31-Oct-98   floh    created
//--------------------------------------------------------------------
bool KNetServerNode::Accept(void)
{
    // create reader thread...
    m_ReceiveThread.Init(
		n_miniserver_tfunc,  // thread func
		0,                   // stack size
		n_miniserver_wakeup, // wakeup func
		&(m_Server->m_MsgList),
		(void *) this);
    return TRUE;
}

//--------------------------------------------------------------------
//  Ignore()
//  31-Oct-98   floh    created
//--------------------------------------------------------------------
void KNetServerNode::Ignore(void)
{
    shutdown(m_AcceptSocket,2);
    closesocket(m_AcceptSocket);
    m_AcceptSocket = 0;
}

//--------------------------------------------------------------------
//  Send()
//  28-Oct-98   floh    created
//--------------------------------------------------------------------
bool KNetServerNode::Send(void *buf, long size)
{
    KASSERT(buf);
    KASSERT(size > 0);
	long res = 0;
    m_Mutex.Lock();
    if (m_AcceptSocket)
	{
		res = send(m_AcceptSocket,(char *)buf,size,0);
		if (res == -1)
		{
			g_DebugLog("KNetServerNode::Send() failed!!!\n");
		}
    }
	m_Mutex.Unlock();
    return TRUE;
}
//--------------------------------------------------------------------------------
