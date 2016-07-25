//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNpcResList.cpp
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Obj Class
//---------------------------------------------------------------------------
#include "KCore.h"

#ifndef _SERVER

#include	"CoreUseNameDef.h"
#include	"KNpcResList.h"

#ifndef _SERVER
KNpcResList	g_NpcResList;
#endif

KNpcResList::KNpcResList()
{
}

KNpcResList::~KNpcResList()
{
	KNpcResNode		*pTempNode;
	KNpcResNode		*pTempNextNode;

    // 释放所有链表中的节点
   	pTempNode = (KNpcResNode *)this->GetHead();
	while (pTempNode)
    {
		pTempNextNode = (KNpcResNode *)pTempNode->GetNext();

        delete pTempNode;

        pTempNode = pTempNextNode;
	}
}

//-------------------------------------------------------------------------
//	功能：	初始化
//-------------------------------------------------------------------------
BOOL	KNpcResList::Init()
{
	if ( !m_cActionName.Init(ACTION_FILE_NAME) )
		return FALSE;
	if ( !m_cNpcAction.Init(NPC_ACTION_NAME) )
		return FALSE;
	if ( !m_cStateTable.Init() )
		return FALSE;
	if ( !m_cMenuState.Init() )
		return FALSE;
	return TRUE;
}

//-------------------------------------------------------------------------
//	功能：	查找同名的 NpcResNode 并返回节点
//-------------------------------------------------------------------------
KNpcResNode*	KNpcResList::FindNpcRes(char *lpszNpcName)
{
	if ( !lpszNpcName || !lpszNpcName[0])
		return NULL;

	KNpcResNode		*pTempNode;

	pTempNode = (KNpcResNode *)this->GetHead();
	if (pTempNode == NULL)
		return NULL;
	if (strcmp(pTempNode->m_szNpcName, lpszNpcName) == 0)
	{
		return pTempNode;
	}

	while (1)
	{
		pTempNode = (KNpcResNode *)pTempNode->GetNext();
		if (pTempNode == NULL)
			return NULL;
		if (strcmp(pTempNode->m_szNpcName, lpszNpcName) == 0)
		{
			return pTempNode;
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------
//	功能：	添加一个 NpcResNode 并返回节点，如果已存在，直接返回节点
//-------------------------------------------------------------------------
KNpcResNode*	KNpcResList::AddNpcRes(char *lpszNpcName)
{
	if ( !lpszNpcName || !lpszNpcName[0])
		return FALSE;

	KNpcResNode		*pTempNode;
	pTempNode = FindNpcRes(lpszNpcName);
	if ( pTempNode != NULL )
		return pTempNode;

	pTempNode = new KNpcResNode;
	if ( !pTempNode->Init(lpszNpcName, &m_cActionName, &m_cNpcAction) )
	{
		delete pTempNode;
		return NULL;
	}

	this->AddTail(pTempNode);

	return pTempNode;
}

#endif