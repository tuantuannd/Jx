//////////////////////////////////////////////////////////////////////////////////////
//	文件名			:	KNewProtocolProcess.h
//	创建人			:	王西贝
//	创建时间		:	2003-7-14 13:17:29
//	文件说明		:	针对单个Account的新协议的处理
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __KNEWPROTOCOLPROCESS_H__
#define __KNEWPROTOCOLPROCESS_H__

#ifndef _STANDALONE
#include <map>
#include <list>

#include "../../MultiServer/Common/Library.h"
#include "../../MultiServer/Common/Buffer.h"
#else
#include "zport.h"
#endif

#ifdef _STANDALONE
class IClient;
#else
struct IClient;
#endif
class KNewProtocolProcess
{
protected:
	typedef void (*FUN_PROCESS_NEW_PROTOCOL)(IClient* pTransfer,
										  DWORD dwFromIP, DWORD dwFromRelayID,
										  int nPlayerIndex, BYTE *pData, int nDataSize);
	typedef std::map<WORD, FUN_PROCESS_NEW_PROTOCOL> NEW_PROTOCOL_HASH;

	typedef std::list<OnlineGameLib::Win32::CBuffer *> DATAList;
	DATAList m_theTransList;
	DATAList m_theChatList;
	DATAList m_theTongList;

	OnlineGameLib::Win32::CBuffer::Allocator	m_theAllocator;

public:
	KNewProtocolProcess();
	~KNewProtocolProcess();
//protected:
	NEW_PROTOCOL_HASH m_ProtocolTable;

	static void P_ProcessGMExecute(IClient* pTransfer,
						   DWORD dwFromIP, DWORD dwFromRelayID,
						   int nPlayerIndex, BYTE* pData, int nDataSize);
	static void P_ProcessGMFindPlayer(IClient* pTransfer,
						   DWORD dwFromIP, DWORD dwFromRelayID,
						   int nPlayerIndex, BYTE* pData, int nDataSize);

	static void P_ProcessPlayerCommExtend(IClient* pTransfer,
						   DWORD dwFromIP, DWORD dwFromRelayID,
						   int nPlayerIndex, BYTE* pData, int nDataSize);

	static void P_ProcessChatExtend(IClient* pTransfer,
						   DWORD dwFromIP, DWORD dwFromRelayID,
						   int nPlayerIndex, BYTE* pData, int nDataSize);

public:
	void ProcessNetMsg(IClient* pTransfer, DWORD dwFromIP, DWORD dwFromRelayID,
					   int nPlayerIndex, BYTE* pData, int nDataSize);

	void SendNetMsgToTransfer(IClient* pClient);
	void SendNetMsgToChat(IClient* pClient);
	void SendNetMsgToTong(IClient* pClient);

	void PushMsgInTransfer(const void * const pData, const size_t datalength);
	void PushMsgInChat(const void * const pData, const size_t datalength);
	void PushMsgInTong(const void * const pData, const size_t datalength);
	//给所有服务器的每个人发送某个协议,向上发到Relay的
	void BroadcastGlobal(const void * const pData, const size_t datalength);
	//给本服务器的每个人发送某个协议,向下直接发到客户的
	void BroadcastLocalServer(const void * const pData, const size_t datalength);
	//在本服务器的范围执行某个协议
	void ExecuteLocalServer(const void * const pData, const size_t datalength);
};

extern KNewProtocolProcess g_NewProtocolProcess;
#endif //__KNEWPROTOCOLPROCESS_H__
