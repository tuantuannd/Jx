//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNpcResList.h
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Obj Class
//---------------------------------------------------------------------------

#pragma once

#ifndef _SERVER

#include "KList.h"
#include "KNpcResNode.h"
#include "KPlayerMenuState.h"

class KNpcResList : public KList
{
public:
	CActionName				m_cActionName;
	CActionName				m_cNpcAction;
	CStateMagicTable		m_cStateTable;
	KPlayerMenuStateGraph	m_cMenuState;

private:

	// 查找同名的 NpcResNode 并返回节点
	KNpcResNode*		FindNpcRes(char *lpszNpcName);

public:
    KNpcResList();
    ~KNpcResList();
	
    // 初始化 ActionName
	BOOL				Init();
	// 添加一个 NpcResNode 并返回节点，如果已存在，直接返回节点
	KNpcResNode*		AddNpcRes(char *lpszNpcName);
};
#ifndef _SERVER
extern KNpcResList	g_NpcResList;
#endif

#endif