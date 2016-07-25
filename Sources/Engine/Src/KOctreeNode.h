// KOctreeNode.h: interface for the KOctreeNode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	KOctreeNodeH
#define KOctreeNodeH

class ENGINE_API KOctreeNode  
{
public:
	DWORD	dwRef;
	DWORD	dwRed;
	DWORD	dwGreen;
	DWORD	dwBlue;
	DWORD	dwIndex;	// Ë÷ÒýÖµ
	KOctreeNode	*tChild[8];
public:
	KOctreeNode();
	BOOL	Insert(BYTE r, BYTE g, BYTE b, int n, int m);
	int		QueryIndex(BYTE r, BYTE g, BYTE b, int layer);
	int		Merge();
	BOOL	IsLeaf();
	BOOL	IsFather();
	int		RefenceSum();
	void	Free();
	virtual ~KOctreeNode();

};

#endif
