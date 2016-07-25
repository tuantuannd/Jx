//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KHashTable.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Memory Opration Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KHashTable.h"
//---------------------------------------------------------------------------
// 函数:	Hash
// 功能:	get hash code
// 参数:	char*, int
// 返回:	DWORD
//---------------------------------------------------------------------------
unsigned long g_Hash(char* pStr)
{
    unsigned long i = 0;
    unsigned long j = 1;
    while (*pStr)
	{
		i += ((unsigned char)*pStr) * j++;
		pStr++;
	}
    return (i % TABLE_SIZE);
}
//---------------------------------------------------------------------------
// 函数:	Add
// 功能:	add string node to table
// 参数:	KStrNode*
// 返回:	void
//---------------------------------------------------------------------------
void KHashTable::Add(KStrNode* pNode)
{
	unsigned long i = g_Hash(pNode->GetName());
	KStrNode* pStrNode = new KStrNode;
	pStrNode->SetName(pNode->GetName());
	m_Table[i].AddHead(pStrNode);
}
//---------------------------------------------------------------------------
// 函数:	Delete
// 功能:	delete string in hash table
// 参数:	char*
// 返回:	void
//---------------------------------------------------------------------------
void KHashTable::Delete(char* pStr)
{
	KStrNode* pNode = Find(pStr);
	if (pNode)
	{
		pNode->Remove();
		delete pNode;
	}
}
//---------------------------------------------------------------------------
// 函数:	Find
// 功能:	find string in hash table
// 参数:	char*
// 返回:	KStrNode*
//---------------------------------------------------------------------------
KStrNode* KHashTable::Find(char* pStr)
{
	unsigned long i = g_Hash(pStr);
	KStrNode* pNode = m_Table[i].Find(pStr);
	return NULL;
}
//---------------------------------------------------------------------------
