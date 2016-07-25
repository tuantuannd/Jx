//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMsgNode.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KMsgNode_H
#define	KMsgNode_H
//---------------------------------------------------------------------------
#include "KNode.h"
//---------------------------------------------------------------------------
class ENGINE_API KMsgNode : public KNode
{
public:
	PVOID	m_pCustom;
	PVOID	m_pBuffer;
	ULONG	m_nBufLen;
public:
	KMsgNode();
	~KMsgNode();
	void	SetMsg(PVOID pBuf, ULONG nLen);
	PVOID	GetMsgBuf(){return m_pBuffer;};
	ULONG	GetMsgLen(){return m_nBufLen;};
	void	SetPtr(PVOID pPtr){m_pCustom = pPtr;};
	PVOID	GetPtr(){return m_pCustom;};
};
//---------------------------------------------------------------------------
#endif
