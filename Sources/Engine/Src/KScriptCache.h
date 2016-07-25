#ifndef __KSCRIPTCACHE_H__
#define __KSCRIPTCACHE_H__
#include "KEngine.h"
#include "KLuaScript.h"
#include "KCache.h"

class ENGINE_API KScriptCache : public KCache
{
public:
    KScriptCache();
    ~KScriptCache();

	virtual BOOL LoadNode(KCacheNode* lpNode);
	virtual void FreeNode(KCacheNode* lpNode);
};

#endif //__KSCRIPTCACHE_H__
