// KOctree.h: interface for the KOctree class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	KOctreeH
#define KOctreeH

#include "KOctreeNode.h"
#include "KPalette.h"
#define	defMaxLayer	5


class ENGINE_API KOctree  
{
public:
	int				m_nColors;	// 总共有几种色了
	KOctreeNode		RootNode;	// 树的根节点
	KPAL24			*m_Palette;
private:
	int				m_nMinRef;	// 最小引用次数
	int				m_nPalOff;	// 调色板偏移
	KOctreeNode*	m_pMinNode;	// 最小引用总数的父节点	
public:
	void			Insert(BYTE r, BYTE g, BYTE b);
	void			ReduceNode();
	void			GetMinSumNode(KOctreeNode *pNode);
	void			InitPalette();
	void			MakePalette(KOctreeNode *pNode);
	int				QueryIndex(BYTE r, BYTE g, BYTE b);
	KOctree();
	virtual ~KOctree();

};

#endif // !defined(AFX_KOCTREE_H__23B11C8F_8C0F_483F_950D_58B1977101FE__INCLUDED_)
