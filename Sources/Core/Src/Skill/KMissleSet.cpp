#include "KMissle.h"
#include "KMissleSet.h"
#include "KSubWorld.h"

CORE_API KMissleSet MissleSet;

void KMissleSet::Init()
{
	
	m_FreeIdx.Init(MAX_MISSLE);
	m_UseIdx.Init(MAX_MISSLE);
	
	// 开始时所有的数组元素都为空
	for (int i = MAX_MISSLE - 1; i > 0; i--)
	{
		m_FreeIdx.Insert(i);
	}
}

int KMissleSet::FindFree()
{
	return m_FreeIdx.GetNext(0);
}

int KMissleSet::Add(int nSubWorldId, int nRegionID,int nMapX, int nMapY, int nOffsetX , int nOffsetY)
{
	
	if (nRegionID < 0 ) return -1;
	int nFreeIndex = FindFree();
	if (nFreeIndex <=0 ) return -1;
	
	Missle[nFreeIndex].m_nMissleId		= nFreeIndex;
	Missle[nFreeIndex].m_nRegionId		= nRegionID;
	Missle[nFreeIndex].m_nCurrentMapX	= nMapX;
	Missle[nFreeIndex].m_nCurrentMapY	= nMapY;
	Missle[nFreeIndex].m_nXOffset		= nOffsetX;
	Missle[nFreeIndex].m_nYOffset		= nOffsetY;
	Missle[nFreeIndex].m_nSubWorldId	= nSubWorldId;
	
	m_FreeIdx.Remove(nFreeIndex);
	m_UseIdx.Insert(nFreeIndex);
	
	return nFreeIndex;
}

/*!*****************************************************************************
// Function		: KMissleSet::Add
// Purpose		: 
// Return		: int 
// Argumant		: int nSubWorldId
// Argumant		: int nPX
// Argumant		: int nPY
// Comments		:
// Author		: RomanDou
*****************************************************************************/
int KMissleSet::Add(int nSubWorldId, int nPX, int nPY)
{
	if (nSubWorldId < 0) return -1;
	
	int nFreeIndex = FindFree();
	if (nFreeIndex <= 0) return -1;
	
	SubWorld[nSubWorldId].Mps2Map(nPX, nPY, &Missle[nFreeIndex].m_nRegionId, &Missle[nFreeIndex].m_nCurrentMapX, &Missle[nFreeIndex].m_nCurrentMapY, &Missle[nFreeIndex].m_nXOffset, &Missle[nFreeIndex].m_nYOffset);
	
	if (Missle[nFreeIndex].m_nRegionId < 0) return -1;
	
	Missle[nFreeIndex].m_nMissleId = nFreeIndex;
	Missle[nFreeIndex].m_nCurrentMapZ = Missle[nFreeIndex].m_nHeight;
	Missle[nFreeIndex].m_nSubWorldId = nSubWorldId;
	Missle[nFreeIndex].m_bRemoving = FALSE;
	SubWorld[nSubWorldId].m_Region[Missle[nFreeIndex].m_nRegionId].AddMissle(nFreeIndex);//m_WorldMessage.Send(GWM_MISSLE_ADD, Missle[nFreeIndex].m_nRegionId, nFreeIndex );
	SubWorld[nSubWorldId].m_Region[Missle[nFreeIndex].m_nRegionId].AddRef(Missle[nFreeIndex].m_nCurrentMapX, Missle[nFreeIndex].m_nCurrentMapY, obj_missle);
	m_FreeIdx.Remove(nFreeIndex);
	m_UseIdx.Insert(nFreeIndex);
	g_DebugLog("[Missle]Missle%dAdd", nFreeIndex);
	
	return nFreeIndex;
}

int	KMissleSet::CreateMissile(int nSkillId, int nMissleId, int nLauncher,  int nTargetId ,int nSubWorldId, int nPX, int nPY, int nDir)
{
	int nMissleIndex = -1;
	
	if (nMissleIndex = Add(nSubWorldId, nPX, nPY) <= 0)
		return -1;
	return nMissleIndex;
}

/*!*****************************************************************************
// Function		: KMissleSet::Activate
// Purpose		: 
// Return		: int 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
int KMissleSet::Activate()
{
	for (int i = 1;i <= MAX_MISSLE;i ++)
	{
		Missle[i].Activate();
	}
	return 1;
}

void KMissleSet::Remove(int nIndex)
{
	if (nIndex <= 0) return;
	
	if (Missle[nIndex].m_nMissleId < 0) return;
	Missle[nIndex].Release();
	g_DebugLog("[Missle]Missle%dRemove", nIndex);
	g_DebugLog("[Missle]Count%d", GetCount());
	m_FreeIdx.Insert(nIndex);
	m_UseIdx.Remove(nIndex);
}

void KMissleSet::Draw()
{
	for (int i = 1;i <= MAX_MISSLE ;i ++)
	{
#ifndef _SERVER
		Missle[i].Paint();
#endif
	}
	
}

int KMissleSet::GetCount()
{
	int nCount  = 0;
	for (int i = 0;i < MAX_MISSLE ;i ++)
	{
	if (Missle[i].m_nMissleId > 0)
		nCount ++;

	}
	return nCount;
}

void KMissleSet::ClearMissles()
{
	int nUsedIndex = m_UseIdx.GetNext(0);
	
	while (nUsedIndex != 0)
	{
		
		Remove(nUsedIndex);
		nUsedIndex = m_UseIdx.GetNext(0);
	}
}