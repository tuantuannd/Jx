//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KHashNode.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KHashNode_H
#define	KHashNode_H
//---------------------------------------------------------------------------
#include "KStrNode.h"
#include "KHashTable.h"
//---------------------------------------------------------------------------
class ENGINE_API KHashNode : public KStrNode
{
private:
	KHashTable* m_pHashTable;
public:
	KHashNode();
	void SetName(char* pStr);
	void SetHashTable(KHashTable* pTable){
		m_pHashTable = pTable;
	}
};
//---------------------------------------------------------------------------
#endif
