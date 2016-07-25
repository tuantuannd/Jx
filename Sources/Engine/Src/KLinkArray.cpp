#include "KWin32.h"
#include "KEngine.h"
#include "KLinkArray.h"

KLinkArray::~KLinkArray()
{
	 if (pNode) 
	 {
		 delete [] pNode;
		 pNode = NULL;
	 }
}

void KLinkArray::Init(int nSize)
{
	if (pNode)
	{
		delete [] pNode;
		pNode = NULL;
		nCount = 0;
	}

	pNode = new KLinkNode[nSize];
}

void KLinkArray::Remove(int nIdx)
{
	if (!nIdx)
		return;
	
	if ((pNode[nIdx].nNext == 0 && pNode[0].nPrev != nIdx) || (pNode[nIdx].nPrev == 0 && pNode[0].nNext != nIdx))
	{
		g_DebugLog("[error]Node:%d Remove twice", nIdx);
		return;
	}

	pNode[pNode[nIdx].nPrev].nNext = pNode[nIdx].nNext;
	pNode[pNode[nIdx].nNext].nPrev = pNode[nIdx].nPrev;
	pNode[nIdx].nNext = 0;
	pNode[nIdx].nPrev = 0;
	nCount--;
}

void KLinkArray::Insert(int nIdx)
{
	if (!nIdx)
		return;

	if (pNode[nIdx].nNext != 0 || pNode[nIdx].nPrev != 0)
	{
		g_DebugLog("[error]Node:%d Insert twice", nIdx);
		return;
	}

	pNode[nIdx].nNext = pNode[0].nNext;
	pNode[pNode[0].nNext].nPrev = nIdx;
	pNode[0].nNext = nIdx;
	pNode[nIdx].nPrev = 0;
	nCount++;
}
