#ifndef	KWorldMsgH
#define	KWorldMsgH
#include "KCore.h"
#include "KNode.h"

enum WORLDMSG						//	param1,		param2,		param3
{
	GWM_MAP_BEGIN		= 0,

	GWM_NPC_BEGIN		= 1000,
	GWM_NPC_DEL,					//	npcidx
	GWM_NPC_CHANGE_REGION,			//	srcregion,	desregion,	npcidx
									// zroc 说明：src dest 在 server 上用 idx，在客户端用 ID

	GWM_OBJ_BEGIN		= 2000,
	GWM_OBJ_DEL,
	GWM_OBJ_CHANGE_REGION,			//	srcregion,	desregion,	objidx

	GWM_ITEM_BEGIN		= 3000,
	
	GWM_MISSLE_BEGIN	= 4000,
	GWM_MISSLE_DEL,
	GWM_MISSLE_CHANGE_REGION,		//	srcregion,	desregion,	misidx
	
	
	GWM_PLAYER_BEGIN	= 5000,
	GWM_PLAYER_SKILL,
	GWM_PLAYER_WALKTO,
	GWM_PLAYER_RUNTO,
	GWM_PLAYER_JUMPTO,
	GWM_PLAYER_CHANGE_REGION,
};

enum	WORLDRETURN
{
	GWR_SUCCESS,
	GWR_FALSE,
	GWR_SKILL_NOTENOUGHMANA,
	GWR_SKILL_NOTENOUGHLIFE,
	GWR_SKILL_NOTENOUGHSTAMINA,
	GWR_SKILL_NOTENOUGHMONEY,
	GWR_SKILL_HAVENOSKILL,
	GWR_SKILL_GTMAXTIMES,		//该技能目前发出的数量过多，请等一会
	GWR_SKILL_LTPERCASTTIME,		//无法发该技能需要等待一会时间
};

class KWorldMsgNode : public KNode
{
public:
	DWORD		m_dwMsgType;
	int			m_nParam[3];
};

#ifndef TOOLVERSION
class KWorldMsg
#else
class CORE_API KWorldMsg
#endif
{
private:
	KList	m_LocalMsgQueue;
public:
	BOOL	Send(DWORD	dwMsgType, int nParam1 = 0, int nParam2 = 0, int nParam3 = 0);
	BOOL	Send(KWorldMsgNode *pMsg);		
	BOOL	Peek(KWorldMsgNode *pMsg, DWORD nNext = 0);
	BOOL	Get(KWorldMsgNode *pMsg);
	void	Clear();
};

inline BOOL KWorldMsg::Send(DWORD dwMsgType, int nParam1, int nParam2, int nParam3)
{
	KWorldMsgNode *pNode = NULL;

	pNode = new KWorldMsgNode;
	if (!pNode)
		return FALSE;

	pNode->m_dwMsgType	= dwMsgType;
	pNode->m_nParam[0]	= nParam1;
	pNode->m_nParam[1]	= nParam2;
	pNode->m_nParam[2]	= nParam3;

	m_LocalMsgQueue.AddTail(pNode);
	return TRUE;
}

inline BOOL KWorldMsg::Send(KWorldMsgNode *pMsg)
{
	if (!pMsg)
		return FALSE;

	m_LocalMsgQueue.AddTail(pMsg);
	return TRUE;
}


extern void		g_MessageToChar(int Id, WORLDRETURN rt);
extern void		g_MessageToAll(WORLDRETURN rt);
extern void		g_MessageToCamp(int nCampId, WORLDRETURN rt);
extern void		g_MessageToChar(int nId, char * szMessage);
extern void		g_MessageToAll(char * szMessage);
extern void		g_MessageToCamp(int nCampId, char * szMessage);
extern char*	g_GWR2Message(WORLDRETURN gwrMessage);

//extern KWorldMsg g_WorldMsg;
#endif
