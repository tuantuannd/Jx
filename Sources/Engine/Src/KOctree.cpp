// KOctree.cpp: implementation of the KOctree class.
//
//////////////////////////////////////////////////////////////////////

#include "KWin32.h"
#include "KDebug.h"
#include "KMemManager.h"
#include "KOctree.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KOctree::KOctree()
{
	m_nColors = 0;
	m_Palette = NULL;
	m_nPalOff = 0;
	m_nMinRef = 0x7fffffff;
}

KOctree::~KOctree()
{
	if (m_Palette)
	{
		g_MemManager.Free(m_Palette);
		m_Palette = NULL;
	}
}

void KOctree::Insert(BYTE r, BYTE g, BYTE b)
{
	KASSERT(defMaxLayer <= 8);
	if (RootNode.Insert(r, g, b, 0, defMaxLayer))	
	{
		m_nColors++;
	}
}

void KOctree::ReduceNode()
{
	while(m_nColors > 256)
	{
		GetMinSumNode(&RootNode);
		m_nColors -= m_pMinNode->Merge();
		m_nMinRef = 0x7fffffff;
	}
}

void KOctree::GetMinSumNode(KOctreeNode *pNode)
{
	int i;
	if (pNode->IsLeaf())
		return;

	if (pNode->IsFather())
	{
		if (pNode->RefenceSum() < m_nMinRef)
		{
			m_pMinNode = pNode;
			m_nMinRef = pNode->RefenceSum();
			return;
		}
	}
	else
	{
		for (i = 0; i < 8; i++)
		{
			if (pNode->tChild[i])
				GetMinSumNode(pNode->tChild[i]);
		}
	}
}

void KOctree::InitPalette()
{
	m_nPalOff = 0;
//	m_nColors = 0;
//	RootNode.Free();
	if (m_Palette == NULL)
	{
		m_Palette = (KPAL24*)g_MemManager.Calloc(256 * sizeof(KPAL24));
	}
}

void KOctree::MakePalette(KOctreeNode *pNode)
{
	KASSERT(m_nColors <= 256);
	if (pNode->IsLeaf())
	{
		m_Palette[m_nPalOff].Red	= (BYTE)(pNode->dwRed / pNode->dwRef);
		m_Palette[m_nPalOff].Green	= (BYTE)(pNode->dwGreen / pNode->dwRef);
		m_Palette[m_nPalOff].Blue	= (BYTE)(pNode->dwBlue / pNode->dwRef);
		pNode->dwIndex = m_nPalOff;
		m_nPalOff++;
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			if (pNode->tChild[i])
				MakePalette(pNode->tChild[i]);
		}
	}
}

int	KOctree::QueryIndex(BYTE r, BYTE g, BYTE b)
{
	return RootNode.QueryIndex(r, g, b, 0);
}