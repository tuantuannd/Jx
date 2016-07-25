//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCache.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Cache class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KMemManager.h"
#include "KCache.h"
//---------------------------------------------------------------------------
// 函数:	KCache
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KCache::KCache()
{
	m_nMaxNode = 256;
	m_nLimitNode = 256;
}


//---------------------------------------------------------------------------
// 函数:	Release
// 功能:    释放所有Cache中的节点,由于在析构中无法调用虚函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KCache::Release()
{
	KCacheNode* lpNode;
	while (lpNode = (KCacheNode*)m_HashList.RemoveHead())
	{
		FreeNode(lpNode);
		delete(lpNode);
	}
}

//---------------------------------------------------------------------------
// 函数:	~KCache
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KCache::~KCache()
{
    /*Release();*/
}
//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	
// 参数:	
// 返回:	void
//---------------------------------------------------------------------------
void KCache::Init(int nMaxNode)
{
	m_nMaxNode = nMaxNode;
	m_nLimitNode = m_nMaxNode;
}
//---------------------------------------------------------------------------
// 函数:	NewNode
// 功能:	
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
KCacheNode* KCache::NewNode(char* lpName)
{
	//寻找相同的节点
	KCacheNode* lpNode = (KCacheNode*)m_HashList.Find(lpName);
	
	//如果不存在
	if (NULL == lpNode)
	{
		if (m_HashList.GetNodeCount() < m_nMaxNode)
		{
			//如果没有到最大节点数，就产生一个新节点
			//并把它放在链表的最前面
			lpNode = new KCacheNode;
			lpNode->SetName(lpName);
			m_HashList.AddHead(lpNode);
			LoadNode(lpNode);
			lpNode->m_Ref = 1;
		}
		else
		{	
			//已经达到最大节点数，查看是否有冗余结点，有，就把最后用的一个
			//节点变为当前节点，并把它移动到最前面，否则就扩大最大节点数
			lpNode = (KCacheNode *)m_HashList.GetTail();
			if (lpNode->m_Ref == 0)
			{
				lpNode = (KCacheNode*)m_HashList.RemoveTail();
				FreeNode(lpNode);
				lpNode->SetName(lpName);
				LoadNode(lpNode);
				m_HashList.AddHead(lpNode);
				lpNode->m_Ref = 1;
			}
			else
			{
				lpNode = new KCacheNode;
				lpNode->SetName(lpName);
				m_HashList.AddHead(lpNode);
				LoadNode(lpNode);
				lpNode->m_Ref = 1;
				m_nMaxNode++;
			}
		}
	}
	return lpNode;
}
//---------------------------------------------------------------------------
// 函数:	GetNode
// 功能:	
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
KCacheNode* KCache::GetNode(char* lpName, KCacheNode* lpNode)
{
	//如果不是第一次取，则比较一下名字，看节点是否改变
	if (lpNode)
	{	//如果节点没有改变，就把它移动到最前面
		if (g_StrCmp(lpName, lpNode->GetName()))
		{
			lpNode->Remove();
			m_HashList.AddHead(lpNode);
			lpNode->m_Ref = 1;
			return lpNode;
		}
	}
	//如果是第一次取对象，或者对象已经改变，就产生新对象
	return NewNode(lpName);
}
//---------------------------------------------------------------------------
// 函数:	Prepare
// 功能:	
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
void KCache::Prepare()
{
	// 释放掉比设定大小多出的且在本次使用中用不上的节点内容
	int	i = m_nMaxNode;
	KCacheNode* lpOldNode = NULL;
	KCacheNode* lpNode = (KCacheNode *)m_HashList.GetTail();

	while(i > m_nLimitNode)
	{
		if (lpNode->m_Ref)
			break;
		FreeNode(lpNode);
		lpNode = (KCacheNode *)lpNode->GetPrev();
		i--;
	}
	// 把本次使用标记清零以备下次循环使用
	lpNode = (KCacheNode *)m_HashList.GetHead();
	while(lpNode)
	{
		lpNode->m_Ref = 0;
		lpNode = (KCacheNode *)lpNode->GetNext();
	}
}

void	KCache::ClearNode(char * lpName)
{
	if (NULL == lpName || NULL == lpName[0]) return;
	//寻找该节点
	KCacheNode* lpNode = (KCacheNode*)m_HashList.Find(lpName);
	
	//如果存在,清除掉!
	if (NULL != lpNode)
	{
		lpNode->Remove();
		FreeNode(lpNode);
		delete lpNode;
	}
}
