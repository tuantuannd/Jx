#include "kwin32.h"
#include "KEngine.h"
#include "KFindBinTree.h"

void KFindBinTreeNode::Init(int nLevel)
{
	m_nMaxRef = (1 << nLevel);
	if (nLevel)
	{
		pLeftChild = new KFindBinTreeNode;
		pLeftChild->pParent = this;
		pLeftChild->Init(nLevel - 1);
		pRightChild = new KFindBinTreeNode;
		pRightChild->pParent = this;
		pRightChild->Init(nLevel - 1);
	}
}

int KFindBinTreeNode::Insert(int nCount)
{
	m_nRef++;
	if (!pLeftChild)
		return nCount;

	if (!pRightChild)
		return nCount;

	if (pLeftChild->CanInsert())
	{
		return pLeftChild->Insert(nCount << 1);
	}
	return pRightChild->Insert((nCount << 1) + 1);
}

void KFindBinTreeNode::Remove(int nIdx, int nLevel)
{
	m_nRef--;

	if (!nLevel)
		return;

	nLevel--;
	DWORD	dwMask = (1 << nLevel);

	if (nIdx & dwMask)
	{
		pRightChild->Remove(nIdx - dwMask, nLevel);
	}
	else
	{
		pLeftChild->Remove(nIdx, nLevel);
	}

}

void KFindBinTree::Init(int nLevel)
{
	m_nLevel = nLevel;
	pRootNode = new KFindBinTreeNode;
	pRootNode->Init(nLevel);
}

int KFindBinTree::FindFree()
{
	int nRet = 1;
	if (pRootNode->CanInsert())
		return pRootNode->Insert(0);
	else
		return 0;
}

void KFindBinTree::Remove(int nIndex)
{
	pRootNode->Remove(nIndex, m_nLevel);
}