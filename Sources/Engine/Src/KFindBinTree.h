#ifndef KFindBinTree_H
#define	KFindBinTree_H

class ENGINE_API KFindBinTreeNode
{
public:
	KFindBinTreeNode	*pParent;
	KFindBinTreeNode	*pLeftChild;
	KFindBinTreeNode	*pRightChild;
private:
	int					m_nRef;
	int					m_nMaxRef;
public:
	KFindBinTreeNode() { pParent = pLeftChild = pRightChild = NULL; m_nRef = m_nMaxRef = 0;}
	~KFindBinTreeNode() { if (pLeftChild) delete pLeftChild; if (pRightChild) delete pRightChild;}
	void				Init(int nLevel);
	BOOL				CanInsert() { if (m_nRef >= m_nMaxRef) return FALSE;return TRUE; }
	int					Insert(int nCount);
	void				Remove(int nIdx, int nLevel);
	int					GetRef() const { return m_nRef; }
};


class ENGINE_API KFindBinTree
{
public:
private:
	int					m_nLevel;
	KFindBinTreeNode	*pRootNode;
public:
	KFindBinTree() { pRootNode = NULL; }
	~KFindBinTree() { if (pRootNode) delete pRootNode; }
	void				Init(int nLevel);
	int					FindFree();
	int					GetCount() { return pRootNode->GetRef(); }
	void				Remove(int nIndex);
};

#endif