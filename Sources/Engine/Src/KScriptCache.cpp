/*******************************************************************************
// FileName			:	KScriptCache.cpp
// FileAuthor		:	RomanDou
// FileCreateDate	:	2002-7-23 10:25:56
// FileDescription	:	
// Revision Count	:	
// Questions		:   脚本的Cache，网络版时是否采用Cache机制了？Cache通过检索字符串操作效率会降低一些，有优化的余地
*******************************************************************************/

#include "KWin32.h"
#include "KScriptCache.h"

KScriptCache::KScriptCache()
{

}

KScriptCache::~KScriptCache()
{
    Release();      // 调用基类中的函数,释放所有节点
}

BOOL KScriptCache::LoadNode(KCacheNode* lpNode)
{
	KLuaScript* pScript = new KLuaScript;
	pScript->Init();
	if (pScript->Load(lpNode->GetName()))
		lpNode->m_lpData = pScript;
	else
		lpNode->m_lpData = NULL;
	return lpNode->m_lpData != NULL;
}
//---------------------------------------------------------------------------
// 函数:	FreeNode
// 功能:	
// 参数:	
// 返回:	void
//---------------------------------------------------------------------------
void KScriptCache::FreeNode(KCacheNode* lpNode)
{
	KLuaScript* pLuaScript = (KLuaScript*)lpNode->m_lpData;
	if (pLuaScript)
	{
		pLuaScript->Exit();
		delete pLuaScript;
	}
	lpNode->m_lpData = NULL;
}
