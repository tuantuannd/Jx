// KOctreeNode.cpp: implementation of the KOctreeNode class.
//
//////////////////////////////////////////////////////////////////////

#include "KWin32.h"
#include "KDebug.h"
#include "KMemManager.h"
#include "KOctreeNode.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KOctreeNode::KOctreeNode()
{
	dwRef	= 0;
	dwRed	= 0;
	dwGreen	= 0;
	dwBlue	= 0;
	dwIndex	= 0;
	for (int i = 0; i < 8; i++)
		tChild[i] = NULL;
}

KOctreeNode::~KOctreeNode()
{
	Free();
}

void KOctreeNode::Free()
{
	for (int i = 0; i < 8; i++)
	{
		if (tChild[i])
		{
			delete tChild[i];
			tChild[i] = NULL;
		}
	}
}
BOOL KOctreeNode::Insert(BYTE r, BYTE g, BYTE b, int layer, int max_layer)
{
	BYTE	bMask = 0x80;
	BYTE	rgbMask = 0x00;

	if (layer >= max_layer)
	{
		dwRed	+= r;
		dwGreen	+= g;
		dwBlue	+= b;
		dwRef++;
		if (dwRef == 1)	// 第一次加进来的叶子，是一种新的颜色
			return TRUE;
		else
			return FALSE;
	}
	for (int i = 0; i < layer; i++)
	{
		bMask = bMask >> 1;
	}
	if (r & bMask)
	{
		rgbMask |= 0x04;
	}
	if (g & bMask)
	{
		rgbMask |= 0x02;
	}
	if (b & bMask)
	{
		rgbMask |= 0x01;
	}
	if (!tChild[rgbMask])
	{
		tChild[rgbMask] = new KOctreeNode;
	}
	return tChild[rgbMask]->Insert(r, g, b, layer+1, max_layer);
	
}

int	KOctreeNode::QueryIndex(BYTE r, BYTE g, BYTE b, int layer)
{
	BYTE	bMask = 0x80;
	BYTE	rgbMask = 0x00;
	
	if (IsLeaf())
	{
		return dwIndex;
	}
	for (int i = 0; i < layer; i++)
	{
		bMask = bMask >> 1;
	}
	if (r & bMask)
	{
		rgbMask |= 0x04;
	}
	if (g & bMask)
	{
		rgbMask |= 0x02;
	}
	if (b & bMask)
	{
		rgbMask |= 0x01;
	}
	return tChild[rgbMask]->QueryIndex(r, g, b, layer+1);
}

int KOctreeNode::Merge()
{
	int	nRetVal = 0;
	for (int i = 0; i < 8; i++)
	{
		if (tChild[i])
		{
			dwRef	+= tChild[i]->dwRef;
			dwRed	+= tChild[i]->dwRed;
			dwGreen	+= tChild[i]->dwGreen;
			dwBlue	+= tChild[i]->dwBlue;
			delete tChild[i];
			tChild[i] = NULL;
			nRetVal++;
		}
	}
	return nRetVal - 1;
}

BOOL KOctreeNode::IsLeaf()
{
	for (int i = 0; i < 8; i++)
	{
		if (tChild[i])
			return FALSE;
	}
	return TRUE;
}

BOOL KOctreeNode::IsFather()
{
	int nLeaf = 0;
	for (int i = 0; i < 8; i++)
	{
		if (!tChild[i])
			continue;
		if (tChild[i]->IsLeaf())
		{
			nLeaf++;
		}
		else
		{
			return FALSE;
		}
	}
	if (nLeaf)
		return TRUE;
	else 
		return FALSE;
}

int	KOctreeNode::RefenceSum()
{
	int nRetVal = 0;
	for (int i = 0; i < 8; i++)
	{
		if (tChild[i])
		{
			nRetVal += tChild[i]->dwRef;
		}
	}
	return nRetVal;
}