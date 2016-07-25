//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KHashTable.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KHashTable_H
#define KHashTable_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KStrList.h"
//---------------------------------------------------------------------------
#define TABLE_SIZE	256
//---------------------------------------------------------------------------
unsigned long g_Hash(char* pStr);
//---------------------------------------------------------------------------
class ENGINE_API KHashTable
{
private:
	KMemClass	m_Mem;
	KStrList	m_Table[TABLE_SIZE];
public:
	void		Add(KStrNode* pNode);
	void		Delete(char* pStr);
	KStrNode*	Find(char* pStr);
};
//---------------------------------------------------------------------------
#endif
