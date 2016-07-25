//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KList.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KList_H
#define	KList_H
//---------------------------------------------------------------------------
#include "KDebug.h"
#include "KNode.h"
//---------------------------------------------------------------------------

class KList
{
public:
	KNode m_ListHead; // 头节点（不是第一个节点）
	KNode m_ListTail; // 尾节点（不是最后的节点）
public:
	KList(void);
	KNode* GetHead(void); // 取得第一个节点
	KNode* GetTail(void); // 取得最后一个节点
	void AddHead(KNode *pNode); // 在最前面增加一个节点
	void AddTail(KNode *pNode); // 在最后面增加一个节点
	KNode* RemoveHead(void); // 删除第一个节点
	KNode* RemoveTail(void); // 删除最后一个节点
	BOOL IsEmpty(void); // 是否是个空的链表
	LONG GetNodeCount(void);
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// 函数:	KList
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
inline KList::KList(void)
{
	m_ListHead.m_pNext = &m_ListTail;
	m_ListTail.m_pPrev = &m_ListHead;
}
//-------------------------------------------------------------------------
// 函数:	IsEmpty
// 功能:	是否为空
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
inline BOOL KList::IsEmpty(void)
{
	return (m_ListHead.GetNext() == NULL);
}
//-------------------------------------------------------------------------
// 函数:	GetHead
// 功能:	取得链表的头
// 参数:	void
// 返回:	KNode*
//---------------------------------------------------------------------------
inline KNode* KList::GetHead(void)
{
	return m_ListHead.GetNext();
}
//-------------------------------------------------------------------------
// 函数:	GetTail
// 功能:	取得链表的尾
// 参数:	void
// 返回:	KNode*
//---------------------------------------------------------------------------
inline KNode* KList::GetTail(void)
{
	return m_ListTail.GetPrev();
}
//-------------------------------------------------------------------------
// 函数:	AddHead
// 功能:	在头部增加一个节点
// 参数:	KNode*
// 返回:	BOOL
//---------------------------------------------------------------------------
inline void KList::AddHead(KNode *pNode)
{
	m_ListHead.InsertAfter(pNode);
}
//-------------------------------------------------------------------------
// 函数:	AddTail
// 功能:	在末尾增加一个节点
// 参数:	KNode*
// 返回:	void
//---------------------------------------------------------------------------
inline void KList::AddTail(KNode *pNode)
{
	m_ListTail.InsertBefore(pNode);
}
//-------------------------------------------------------------------------
// 函数:	RemoveHead
// 功能:	删除第一个节点
// 参数:	void
// 返回:	KNode*
//---------------------------------------------------------------------------
inline KNode* KList::RemoveHead(void)
{
	KNode* pNode = m_ListHead.GetNext();
	if (pNode)
		pNode->Remove();
	return pNode;
}
//-------------------------------------------------------------------------
// 函数:	RemoveTail
// 功能:	删除最后一个节点
// 参数:	void
// 返回:	KNode*
//---------------------------------------------------------------------------
inline KNode* KList::RemoveTail(void)
{
	KNode* pNode = m_ListTail.GetPrev();
	if (pNode)
		pNode->Remove();
	return pNode;
}
//-------------------------------------------------------------------------
// 函数:	GetNodeCount
// 功能:	取得节点个数
// 参数:	void
// 返回:	LONG
//---------------------------------------------------------------------------
inline LONG KList::GetNodeCount(void)
{
	long nNode = 0;
	KNode* pNode = GetHead();
	while (pNode)
	{
		pNode = pNode->GetNext();
		nNode++;
	}
	return nNode;
}
//-------------------------------------------------------------------------

#endif
