//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSpriteCache.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KSoundCache_H
#define KSoundCache_H
//---------------------------------------------------------------------------
#include "KCache.h"
//---------------------------------------------------------------------------
class ENGINE_API KSoundCache : public KCache
{
public:
    KSoundCache();
    ~KSoundCache();

	virtual BOOL LoadNode(KCacheNode* lpNode);
	virtual void FreeNode(KCacheNode* lpNode);
};
//---------------------------------------------------------------------------
#endif
