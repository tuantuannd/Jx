#include "KCore.h"
#include "KWorldMsg.h"

//KWorldMsg g_WorldMsg;


BOOL KWorldMsg::Get(KWorldMsgNode *pMsg)
{
	KWorldMsgNode* pNode = NULL;

	pNode = (KWorldMsgNode *)m_LocalMsgQueue.GetHead();
	if (pNode)
	{
		pMsg->m_dwMsgType = pNode->m_dwMsgType;
		memcpy(pMsg->m_nParam, pNode->m_nParam, sizeof(pNode->m_nParam));
		pNode->Remove();
		if (pNode)	
		{
			delete pNode;
			pNode = NULL;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL KWorldMsg::Peek(KWorldMsgNode *pMsg, DWORD nNext /* = 0 */)
{
	KWorldMsgNode* pNode = NULL;

	pNode = (KWorldMsgNode *)m_LocalMsgQueue.GetHead();
	while(nNext && pNode)
	{
		pNode = (KWorldMsgNode *)pNode->GetNext();
		nNext--;
	}
	if (pNode)
	{
		pMsg->m_dwMsgType = pNode->m_dwMsgType;
		memcpy(pMsg->m_nParam, pNode->m_nParam, sizeof(pNode->m_nParam));
		return TRUE;
	}
	else
		return FALSE;
}

void KWorldMsg::Clear()
{
	KWorldMsgNode *pNode, *pNextNode;

	pNode = (KWorldMsgNode *)m_LocalMsgQueue.GetHead();
	pNextNode = pNode;
	
	while(pNextNode)
	{
		pNextNode = (KWorldMsgNode *)pNode->GetNext();
		m_LocalMsgQueue.RemoveHead();
		if (pNode)
			delete pNode;
		pNode = pNextNode;
	}
}
