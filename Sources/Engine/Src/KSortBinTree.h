#ifndef KSORTBINTREE_H
#define KSORTBINTREE_H

#include "Kengine.h"

class ENGINE_API TBinTreeNode
{
	TBinTreeNode(){ pParent = pLeftChild = pRightChild =  NULL;	}
public:
	virtual BOOL			operator>(TBinTreeNode& p) = 0;
	virtual BOOL			operator==(TBinTreeNode& p) = 0;
 	virtual BOOL			operator<(TBinTreeNode& p) = 0;
	TBinTreeNode			* pParent;				//父
	TBinTreeNode			* pLeftChild;			//左子
	TBinTreeNode			* pRightChild;			//右子
};		//按照文件名或其它属性作为脚本标识的排序二叉树


class ENGINE_API KSortBinTree
{
public:
	KSortBinTree();

	//查找关键字与pKeyNode一致的结点，成功*pResult为True并返回结点指针.
	TBinTreeNode * Search(TBinTreeNode* pKeyNode, BOOL * pResult);

	//插入结点，成功返回新插的结点指针，否则为空
	TBinTreeNode * Insert(TBinTreeNode *pNewNode);
	
	
	//删除本结点
	BOOL		   RemoveThisNode(TBinTreeNode * pNode);
	
	//删除二叉树中与pKeyNode一致的结点
	BOOL		   RemoveKeyNode(TBinTreeNode * pNode);
		
	DWORD		   GetCount()
	{ int OldCount = m_TempCount;
	return InOrder(m_pTreeRoot) - OldCount;
	};
	TBinTreeNode * GetRoot(){return m_pTreeRoot;	};
		

private:
	TBinTreeNode * m_pTreeRoot;		//二叉树根支点
	int				m_TempCount;
	TBinTreeNode * AddNode(TBinTreeNode *pNewNode, TBinTreeNode *pTBinTreeNode);
	TBinTreeNode * RemoveNode(TBinTreeNode * pTBinTreeNode, TBinTreeNode ** ppRootTBinTreeNode);
	TBinTreeNode * Search(TBinTreeNode * pParentTBinTreeNode, TBinTreeNode * pTBinTreeNode, TBinTreeNode * pKeyNode, BOOL * pResult);
	TBinTreeNode * FindLess(TBinTreeNode * pTBinTreeNode);
	DWORD		   InOrder(TBinTreeNode * pTBinTreeNode);//中序遍历
};



#endif	//KSortBinTree_H