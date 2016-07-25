//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMsgNode.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KMemManager.h"
#include "KMsgNode.h"
//---------------------------------------------------------------------------
// 函数:	KMsgNode
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMsgNode::KMsgNode()
{
	m_pBuffer = NULL;
	m_nBufLen = 0;
}
//---------------------------------------------------------------------------
// 函数:	~KMsgNode
// 功能:	析构
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMsgNode::~KMsgNode()
{
	if (m_pBuffer)
		g_MemManager.Free(m_pBuffer);
}
//---------------------------------------------------------------------------
// 函数:	SetName
// 功能:	set node name
// 参数:	char*
// 返回:	void
//---------------------------------------------------------------------------
void KMsgNode::SetMsg(PVOID pBuf, ULONG nLen)
{
	if (m_pBuffer)
		g_MemManager.Free(m_pBuffer);
	m_pBuffer = g_MemManager.Malloc(nLen);
	g_MemCopy(m_pBuffer, pBuf, nLen);
	m_nBufLen = nLen;
}
//--------------------------------------------------------------------------------
