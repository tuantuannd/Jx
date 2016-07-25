//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCache.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KCache_H
#define KCache_H
//---------------------------------------------------------------------------
#include "KStrNode.h"
#include "KStrList.h"
//---------------------------------------------------------------------------
class KCacheNode : public KStrNode
{
public:
	PVOID	m_lpData;			//对象指针
	LONG	m_Ref;				//本帧是否被引用
public:
	KCacheNode() {m_Ref = 0; m_lpData = NULL;};
};
//---------------------------------------------------------------------------
class ENGINE_API KCache
{
public:
	LONG m_nMaxNode;			//最大对象节点数
	LONG m_nLimitNode;			//设定的对象节点数
	KStrList m_HashList;		//对象哈希链表
public:
	KCache();
	~KCache();
	virtual void Init(int nMaxNode=256);
	virtual KCacheNode* NewNode(char* lpName);
	virtual KCacheNode* GetNode(char* lpName, KCacheNode* lpNode);
	virtual BOOL LoadNode(KCacheNode* lpNode){return FALSE;};
	virtual void FreeNode(KCacheNode* lpNode){};
	void	ClearNode(char * lpName);//清除某个结点
	void	Prepare();
    virtual void Release();
};
//---------------------------------------------------------------------------
#endif
