//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSpriteCache.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KSpriteCache_H
#define KSpriteCache_H
//---------------------------------------------------------------------------
#include "KCache.h"
//---------------------------------------------------------------------------
class ENGINE_API KSpriteCache : public KCache
{
public:
    KSpriteCache();
    ~KSpriteCache();

	virtual BOOL LoadNode(KCacheNode* lpNode);
	virtual void FreeNode(KCacheNode* lpNode);
};
//---------------------------------------------------------------------------
#endif
