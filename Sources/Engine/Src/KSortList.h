//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSortList.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KSortList_H
#define KSortList_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
//---------------------------------------------------------------------------
typedef struct {
	DWORD	HashCode;
	PVOID	ListItem;
} SORT, *PSORT;
//---------------------------------------------------------------------------
class ENGINE_API KSortList
{
private:
	KMemClass	m_ListMem;
	KMemClass	m_SortMem;
	int			m_nItemNum;
	int			m_nItemSize;
	int 		m_nItemCount;
public:
	KSortList();
	~KSortList();
	BOOL		Init(int nItemSize, int ItemCount);
	BOOL		Insert(DWORD dwHashCode, PVOID pvItem);
	BOOL		Search(DWORD dwHashCode, PVOID pvItem);
	void		Free();
};
//---------------------------------------------------------------------------
#endif
