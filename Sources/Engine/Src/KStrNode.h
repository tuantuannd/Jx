//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KStrNode.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	KStrNode_H
#define	KStrNode_H
//---------------------------------------------------------------------------
#include "KNode.h"
#include "KStrBase.h"
//---------------------------------------------------------------------------
#define MAX_STRLEN		80
//---------------------------------------------------------------------------
class ENGINE_API KStrNode : public KNode
{
public:
	char m_Name[MAX_STRLEN];
public:
	KStrNode(void);
	virtual char* GetName(){return m_Name;};
	virtual void  SetName(char* str){g_StrCpyLen(m_Name, str, MAX_STRLEN);};
};
//---------------------------------------------------------------------------
#endif
