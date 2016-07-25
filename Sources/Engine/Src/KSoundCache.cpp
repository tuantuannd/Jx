//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSoundCache.cpp
// Date:	2000.08.08
// Code:	Huyi(Spe)
// Desc:	Cache class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KWavSound.h"
#include "KSoundCache.h"


KSoundCache::KSoundCache()
{

}

KSoundCache::~KSoundCache()
{
    Release();
}

//---------------------------------------------------------------------------
// 函数:	LoadNode
// 功能:	
// 参数:	
// 返回:	void
//---------------------------------------------------------------------------
BOOL KSoundCache::LoadNode(KCacheNode* lpNode)
{
	if (!lpNode)
		return FALSE;

	FreeNode(lpNode);
	char* pszMsg = lpNode->GetName();
	if (pszMsg && pszMsg[0])
	{
		KWavSound* pSound = new KWavSound;
		if (pSound)
		{
			if (pSound->Load(pszMsg))
				lpNode->m_lpData = pSound;
			else
				delete pSound;
		}
	}
	return (lpNode->m_lpData != NULL);
}
//---------------------------------------------------------------------------
// 函数:	FreeNode
// 功能:	
// 参数:	
// 返回:	void
//---------------------------------------------------------------------------
void KSoundCache::FreeNode(KCacheNode* lpNode)
{
	KWavSound* pSound = (KWavSound*)lpNode->m_lpData;
	if (pSound)
	{
		delete pSound;
	}
	lpNode->m_lpData = NULL;
}
//---------------------------------------------------------------------------
