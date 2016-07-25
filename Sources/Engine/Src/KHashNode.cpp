//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KHashNode.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Simple double linked node class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KStrBase.h"
#include "KHashNode.h"
//---------------------------------------------------------------------------
// 函数:	KHashNode
// 功能:	构造
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KHashNode::KHashNode(void)
{
	m_pHashTable = NULL;
}
//---------------------------------------------------------------------------
// 函数:	SetName
// 功能:	set node name
// 参数:	char*
// 返回:	void
//---------------------------------------------------------------------------
void KHashNode::SetName(char* pStr)
{
	if (KStrNode::IsLinked())
	{
		m_pHashTable->Delete(m_Name);
		KStrNode::SetName(pStr);
		m_pHashTable->Add(this);
	}
	else
	{
		KStrNode::SetName(pStr);
	}
}
//--------------------------------------------------------------------------------
