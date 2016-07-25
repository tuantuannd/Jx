#include "KWin32.h"
#include "Kengine.h"
#include "KSortBinTree.h"

KSortBinTree::KSortBinTree()
{
	m_pTreeRoot = NULL;
}

//---------------------------------------------------------------------------
// 函数:	BTSearch
// 功能:	
// 参数:	TBinTreeNode * pParentTBinTreeNode
// 参数:	TBinTreeNode * pTBinTreeNode
// 参数:	DWORD nKey
// 参数:	BOOL * pResult
// 返回:	TBinTreeNode * 

//---------------------------------------------------------------------------
TBinTreeNode * KSortBinTree::Search(TBinTreeNode * pParentTBinTreeNode, TBinTreeNode * pTBinTreeNode, TBinTreeNode *pKeyNode, BOOL * pResult)
{
	
	if (pTBinTreeNode == NULL || pParentTBinTreeNode == NULL)
	{
		*pResult = FALSE;
		return pParentTBinTreeNode;
	}
	
	
	if (*pKeyNode == *pTBinTreeNode)
	{
		*pResult = TRUE;
		return pTBinTreeNode;
	}
	else if (*pKeyNode > *pTBinTreeNode)
		return Search(pTBinTreeNode, pTBinTreeNode->pRightChild, pKeyNode, pResult);
	else
		return Search(pTBinTreeNode, pTBinTreeNode->pLeftChild, pKeyNode, pResult);
	
}



//---------------------------------------------------------------------------
// 函数:	BTInsert
// 功能:	以pTScrpt为父，插入以szKey为关键点的结点
// 参数:	TBinTreeNode *pTBinTreeNode 
// 参数:	char * szKey
// 返回:	TBinTreeNode * 返回插入的结点
//---------------------------------------------------------------------------
TBinTreeNode * KSortBinTree::AddNode(TBinTreeNode *pNewTBinTreeNode, TBinTreeNode *pTBinTreeNode)
{

	int nResult = 0;
	if (pNewTBinTreeNode == NULL)
		return NULL;
	
	pNewTBinTreeNode->pLeftChild = NULL;
	pNewTBinTreeNode->pParent = NULL;
	pNewTBinTreeNode->pRightChild = NULL;


	if (*pNewTBinTreeNode == *pTBinTreeNode)
		return NULL;

	//根据大小确定左子还是右子
	if (nResult = *pNewTBinTreeNode > *pTBinTreeNode)
	{
		pTBinTreeNode->pRightChild = pNewTBinTreeNode;
		pNewTBinTreeNode->pParent = pTBinTreeNode;
	}
	else 
	{
		pTBinTreeNode->pLeftChild = pNewTBinTreeNode;
		pNewTBinTreeNode->pParent = pTBinTreeNode;
	}
	return pNewTBinTreeNode;
	
}




//---------------------------------------------------------------------------
// 函数:	 BTDelete
// 功能:	删除结点
// 参数:	TBinTreeNode * pTBinTreeNode
// 参数:	TBinTreeNode ** ppRootTBinTreeNode
// 返回:	TBinTreeNode * 
//---------------------------------------------------------------------------
TBinTreeNode *  KSortBinTree::RemoveNode(TBinTreeNode * pTBinTreeNode, TBinTreeNode ** ppRootTBinTreeNode)
{
	TBinTreeNode * pFindTBinTreeNode;
	if (pTBinTreeNode == NULL)
		return NULL;
	
	if (pTBinTreeNode->pLeftChild == NULL && pTBinTreeNode->pRightChild == NULL)//该结点没有左右子
	{
		if (*ppRootTBinTreeNode == pTBinTreeNode)
		{
			*ppRootTBinTreeNode = NULL;
		}
		else
		{
			int nResult ;
			
			if (*pTBinTreeNode < *pTBinTreeNode->pParent)
				nResult = -1;
			else 
				nResult = 1;
			
			if (nResult < 0)//小于,处在父结点左方
				pTBinTreeNode->pParent->pLeftChild = NULL;
			else
				pTBinTreeNode->pParent->pRightChild = NULL;
			
		}
		
		//	delete pTBinTreeNode;
		pTBinTreeNode = NULL;
		return NULL;
	}
	
	else if (!(pTBinTreeNode->pLeftChild && pTBinTreeNode->pRightChild))//只有单子时
	{
		if (pTBinTreeNode == *ppRootTBinTreeNode)
		{
			if (pTBinTreeNode->pLeftChild)
			{
				*ppRootTBinTreeNode = pTBinTreeNode->pLeftChild;
				pTBinTreeNode->pLeftChild->pParent = NULL;
			}
			else
			{
				*ppRootTBinTreeNode = pTBinTreeNode->pRightChild;
				pTBinTreeNode->pRightChild->pParent = NULL;
			}
			//	delete pTBinTreeNode;
			pTBinTreeNode = NULL;
			return	NULL;
		}
		
		
		int nResult ;

		if (*pTBinTreeNode < *pTBinTreeNode->pParent)
				nResult = -1;
		else 
				nResult = 1;
	
		if (nResult < 0)//在父的左边
		{
			
			if (pTBinTreeNode->pLeftChild)//只有左子
			{
				pTBinTreeNode->pParent->pLeftChild = pTBinTreeNode->pLeftChild;
				pTBinTreeNode->pLeftChild->pParent = pTBinTreeNode->pParent;
			}
			else
			{
				pTBinTreeNode->pParent->pLeftChild = pTBinTreeNode->pRightChild;
				pTBinTreeNode->pRightChild->pParent = pTBinTreeNode->pParent;
			}
			
			//delete pTBinTreeNode;
			pTBinTreeNode = NULL;
			
		}
		else 
		{
			if (pTBinTreeNode->pLeftChild)//只有左子
			{
				pTBinTreeNode->pParent->pRightChild = pTBinTreeNode->pLeftChild;
				pTBinTreeNode->pLeftChild->pParent = pTBinTreeNode->pParent;
			}
			else
			{
				pTBinTreeNode->pParent->pRightChild = pTBinTreeNode->pRightChild;
				pTBinTreeNode->pRightChild->pParent = pTBinTreeNode->pParent;
			}
			//delete pTBinTreeNode;
			pTBinTreeNode = NULL;
			
		} 
		
		
		return NULL;
		
	}  
	else//有全子 
	{
		
		
		{
			
			pFindTBinTreeNode = FindLess(pTBinTreeNode->pLeftChild);
			
			if (pFindTBinTreeNode)
			{
				
				//第一部分：处理该结点有左子时，对左子进行指向的改变
				//该结点仍有左子
				if (pFindTBinTreeNode->pLeftChild)
				{
					//当发现所将要替代的结点正是它的左子时，将原来的关系不将变化；否则按正常思路改变
					if (pFindTBinTreeNode != pTBinTreeNode->pLeftChild)
					{
						pFindTBinTreeNode->pParent->pRightChild = pFindTBinTreeNode->pLeftChild;
						pFindTBinTreeNode->pLeftChild->pParent = pFindTBinTreeNode->pParent;
					}
				}
				else
				{
					if (pFindTBinTreeNode != pTBinTreeNode->pLeftChild)
					{
						pFindTBinTreeNode->pParent->pRightChild = NULL;
					}
					
				}
				
				//第二部分：改变替换结点链结，实现与原结点相同。
				//处理当该替换的结点为删除结点的左子的特殊情况
				if (pFindTBinTreeNode == pTBinTreeNode->pLeftChild)
				{
					//其左子不用交代，保持原状
					pTBinTreeNode->pRightChild->pParent = pFindTBinTreeNode;
					
					//其左子不用交代,保持原状	
					pFindTBinTreeNode->pRightChild = pTBinTreeNode->pRightChild;
					pFindTBinTreeNode->pParent = pTBinTreeNode->pParent;
					
				}
				else
				{
					
					pTBinTreeNode->pLeftChild->pParent = pFindTBinTreeNode;
					pTBinTreeNode->pRightChild->pParent = pFindTBinTreeNode;
					
					
					pFindTBinTreeNode->pLeftChild = pTBinTreeNode->pLeftChild ; 
					pFindTBinTreeNode->pRightChild = pTBinTreeNode->pRightChild;
					pFindTBinTreeNode->pParent = pTBinTreeNode->pParent;
					
				}
				
				
				//第三部分   删除结点之父结点链结
				
				if (*ppRootTBinTreeNode == pTBinTreeNode)
				{
					*ppRootTBinTreeNode = pFindTBinTreeNode;
					pFindTBinTreeNode->pParent = NULL;
				}
				else 
				{
					
					int nResult ;
					
				if (*pTBinTreeNode < *pTBinTreeNode->pParent)
					nResult = -1;
					else 
					nResult = 1;
					
					
					if (nResult < 0)//在父的左面
					{
						pTBinTreeNode->pParent->pLeftChild = pFindTBinTreeNode;
					}
					else
					{
						pTBinTreeNode->pParent->pRightChild = pFindTBinTreeNode;
					}
				}
				
				
				//delete pTBinTreeNode;
				pTBinTreeNode = NULL;
				return pFindTBinTreeNode;
			}
			else
				return NULL;
		}
		
	}	
	
	
}

//---------------------------------------------------------------------------
// 函数:	BTFindLess
// 功能:	
// 参数:	TBinTreeNode * pTBinTreeNode
// 返回:	TBinTreeNode * 
//---------------------------------------------------------------------------
TBinTreeNode * KSortBinTree::FindLess(TBinTreeNode * pTBinTreeNode)
{
	if (pTBinTreeNode == NULL)
		return NULL;
	
	if (pTBinTreeNode->pRightChild == NULL )
		return pTBinTreeNode;
	else
		return FindLess(pTBinTreeNode->pRightChild);
	
	
}

//---------------------------------------------------------------------------
// 函数:	BTPreorder
// 功能:	
// 参数:	TBinTreeNode * pTBinTreeNode
// 返回:	DWORD  
//---------------------------------------------------------------------------
DWORD  KSortBinTree::InOrder(TBinTreeNode * pTBinTreeNode)//中序遍历
{
	static int Count = 0	;
	if (pTBinTreeNode != NULL)
	{


		InOrder(pTBinTreeNode->pLeftChild);
		//		printf("%d\n", pTBinTreeNode->nKey);
		InOrder(pTBinTreeNode->pRightChild);
		Count ++;
	}	
	m_TempCount = Count;
	return Count;
	
	
}

TBinTreeNode*  KSortBinTree::Insert(TBinTreeNode *pNewNode)
{
	BOOL nResult = 0;
	TBinTreeNode * pNode = NULL;
	
	if (m_pTreeRoot == NULL) 
	{
		TBinTreeNode * pTBinTreeNode = pNewNode;
		m_pTreeRoot = pTBinTreeNode;
		pTBinTreeNode->pLeftChild = NULL;
		pTBinTreeNode->pRightChild = NULL;
		pTBinTreeNode->pParent = NULL;
		return pTBinTreeNode;
	}
	
	pNode = Search(m_pTreeRoot, m_pTreeRoot, pNewNode, &nResult);
	
	if (nResult == 1)
	{
		return NULL;
		
	}
	else if (nResult == 0)
		return AddNode(pNewNode, pNode);
	else return NULL;
}




BOOL   KSortBinTree::RemoveKeyNode(TBinTreeNode * pNode)
{
	BOOL nResult = FALSE;//查找的结果
	
	TBinTreeNode * pTBinTreeNode = NULL;//查找返回的指针
	
	if (m_pTreeRoot == NULL)
		return FALSE;
	
	pTBinTreeNode = Search(m_pTreeRoot, m_pTreeRoot, pNode, &nResult);
	
	if (nResult && pTBinTreeNode) //在链表中找到了该关键字
	{
		
		return RemoveThisNode(pTBinTreeNode);
	}
	else
		return FALSE;
	return TRUE;
	
}

BOOL   KSortBinTree::RemoveThisNode(TBinTreeNode * pNode)
{
	if (pNode == NULL)
		return FALSE;
	if (m_pTreeRoot == NULL)
		return FALSE;
	TBinTreeNode * pDelNode;
	//该结点并不属于表中的结点，则返回FALSE
	if (pNode->pLeftChild == NULL&&pNode->pRightChild == NULL && pNode->pParent == NULL && pNode != m_pTreeRoot)
		return FALSE;
	pDelNode = RemoveNode(pNode, &m_pTreeRoot);
	return TRUE;
}

TBinTreeNode * KSortBinTree::Search(TBinTreeNode * pKeyNode, BOOL * pResult)
{
	TBinTreeNode * pNode;
	if (m_pTreeRoot == NULL)
		return NULL;
	
	pNode = Search(m_pTreeRoot, m_pTreeRoot, pKeyNode, pResult);
	return pNode;
}
