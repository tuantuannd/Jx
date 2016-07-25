#ifndef KMISSION_H
#define KMISSION_H

#ifdef _SERVER
#include "KCore.h"
#include "KEngine.h"
#include "KLinkArrayTemplate.h"
#include "KTaskFuns.h"
#include "KPlayerDef.h"
#include "KSubWorld.h"
#include "KSubWorldSet.h"

#include "KMissionArray.h"
#include "KPlayerChat.h"
#include "KSortScript.h"
#include "TaskDef.h"

//Mission
//每个Mission有自已的独立的全局变量
//为每一个Mission记录玩家列表的统一的机制
//包括最大人数，等等都要有
//加个Chanel，有利于消息发送
//有功能可以编列所有的参加者id
extern int g_PlayerTimerCallBackFun(void * pOwner, char * szScriptFileName);
extern int g_MissionTimerCallBackFun(void * pOwner, char * szScriptFileName);
#define MAX_TIMER_PERMISSION 3

typedef struct 
{
	unsigned long m_ulPlayerIndex;
	unsigned long m_ulPlayerID;
	unsigned char m_ucPlayerGroup;
	unsigned long m_ulJoinTime;
	int			  m_nParam1;
	int			  m_nParam2;
}TMissionPlayerInfo;

//管理玩家集合的模板类

template<class T , unsigned long ulSize>
class _KMissionPlayerArray:public KLinkArrayTemplate<T , ulSize>
{
public:
	unsigned long FindSame(T *pT)
	{
		int nIdx = 0;
		while(1)
		{
			nIdx = m_UseIdx.GetNext(nIdx);
			if (!nIdx)
				break;
			
			if (pT->m_ulPlayerIndex == m_Data[nIdx].m_ulPlayerIndex)
				return nIdx;
		}
		return 0;
	};

	int GetParam1(unsigned long ulIndex) const
	{
		if (ulIndex >= ulSize) 
			return 0;
		return m_Data[ulIndex].m_nParam1;
	};

	BOOL SetParam1(unsigned long ulIndex, int nParam1)
	{
		if (ulIndex >= ulSize) 
			return FALSE;
		
		m_Data[ulIndex].m_nParam1 =  nParam1;
		return TRUE;
	};

	int GetParam2(unsigned long ulIndex) const
	{
		if (ulIndex >= ulSize) 
			return 0;
		return m_Data[ulIndex].m_nParam2;
	};

	BOOL SetParam2(unsigned long ulIndex, int nParam2)
	{
		if (ulIndex >= ulSize) 
			return FALSE;
		
		m_Data[ulIndex].m_nParam2 =  nParam2;
		return TRUE;
	};
};

typedef _KMissionPlayerArray<TMissionPlayerInfo, MAX_PLAYER> KMissionPlayerArray;

//管理Mission时间触发器集合的模板类
typedef KTimerFunArray <KTimerTaskFun, MAX_TIMER_PERMISSION, g_MissionTimerCallBackFun> KMissionTimerArray;



#define MAX_MISSION_VALUE_COUNT 100
class KMission
{
	int	m_MissionValue[MAX_MISSION_VALUE_COUNT];
	unsigned long m_ulMissionId;
	void * m_pOwner;
public:
	KMissionTimerArray	m_cTimerTaskSet;
	KMission()
	{
		m_pOwner = NULL;
		for (int i = 1; i < MAX_TIMER_PERMISSION; i ++)
		{
			KTimerTaskFun * pTimer = (KTimerTaskFun*)m_cTimerTaskSet.GetData(i);
			if (pTimer)
			pTimer->SetOwner(this);
		}
		memset(m_MissionValue, 0, sizeof(m_MissionValue));
		m_ulMissionId = 0;
	};
	BOOL	Activate();
	
	void SetOwner(void * pOwner)
	{
		m_pOwner = pOwner;
	};
	
	void * GetOwner()
	{
		return m_pOwner;
	};
	
	void	Init()
	{
		memset(m_MissionValue, 0, sizeof(m_MissionValue));
		m_ulMissionId = 0;
	};

	BOOL SetMissionId(unsigned long ulMissionId)
	{
		m_ulMissionId = ulMissionId;
		return TRUE;
	}

	unsigned long GetMissionId()
	{
		return m_ulMissionId;
	}
	
	unsigned long Msg2Group(const char * strMsg, unsigned char ucGroup)
	{
		if (!strMsg || !strMsg[0]) 
			return 0;
		int nCount = 0;
		int nIdx = 0;
		while(1)
		{
			nIdx =	m_MissionPlayer.m_UseIdx.GetNext(nIdx);
			if (!nIdx)
				break;
			
			if (m_MissionPlayer.m_Data[nIdx].m_ucPlayerGroup == ucGroup)
			{
				KPlayerChat::SendSystemInfo(1, m_MissionPlayer.m_Data[nIdx].m_ulPlayerIndex, MESSAGE_SYSTEM_ANNOUCE_HEAD, (char *) strMsg, strlen(strMsg));
				nCount ++;
			}
		}
		return nCount;
	}

	unsigned long Msg2All(const char * strMsg)
	{
		if (!strMsg || !strMsg[0]) 
			return 0;
		int nCount = 0;
		int nIdx = 0;
		while(1)
		{
			nIdx =	m_MissionPlayer.m_UseIdx.GetNext(nIdx);
			if (!nIdx)
				break;
			
			KPlayerChat::SendSystemInfo(1, m_MissionPlayer.m_Data[nIdx].m_ulPlayerIndex, MESSAGE_SYSTEM_ANNOUCE_HEAD, (char *) strMsg, strlen(strMsg));
			nCount ++;
		}
		return nCount;
	}
	unsigned long Msg2Player(const char * strMsg, unsigned long ulPlayerIndex)
	{
		if (!strMsg || !strMsg[0]) 
			return 0;
		KPlayerChat::SendSystemInfo(1, ulPlayerIndex, MESSAGE_SYSTEM_ANNOUCE_HEAD, (char *) strMsg, strlen(strMsg));
	};

	unsigned long GetNextPlayer(unsigned long ulIdx, unsigned char ucGroup, unsigned long &ulPlayerIndex)
	{
		ulPlayerIndex = 0;
		while(1)
		{
			ulIdx =	m_MissionPlayer.m_UseIdx.GetNext(ulIdx);
			if (ulIdx == 0) 
				return 0;
			
			if (ucGroup)
			{
				if (m_MissionPlayer.m_Data[ulIdx].m_ucPlayerGroup == ucGroup)
				{
					ulPlayerIndex = m_MissionPlayer.m_Data[ulIdx].m_ulPlayerIndex;
					return ulIdx;
				}
			}
			else
			{
				ulPlayerIndex = m_MissionPlayer.m_Data[ulIdx].m_ulPlayerIndex;
				return ulIdx;
			}
		}
		return 0;
	}
	
	unsigned long GetPlayerCount() const
	{ 
		return m_MissionPlayer.GetUsedCount();
	};
	
	void SetMissionValue(unsigned long ulValueId, int nValue)
	{
		if (ulValueId >= MAX_MISSION_VALUE_COUNT)
			return ;
		m_MissionValue[ulValueId] = nValue;
	};

	unsigned long GetGroupPlayerCount (unsigned char ucGroup ) const 
	{
		if (ucGroup == 0)
			 return GetPlayerCount();
		unsigned long ulGroupPlayerCount = 0;
		int nIdx = 0;
		while(1)
		{
			nIdx = m_MissionPlayer.m_UseIdx.GetNext(nIdx);
			if (!nIdx)
				break;
			
			if (m_MissionPlayer.m_Data[nIdx].m_ucPlayerGroup == ucGroup)
				ulGroupPlayerCount ++;
		}
		return ulGroupPlayerCount;
	};	
	
	int GetMissionValue(unsigned long ulValueId) const
	{
		if (ulValueId >= MAX_MISSION_VALUE_COUNT)
			return 0;
		return m_MissionValue[ulValueId];
	};
	BOOL	ExecuteScript(char * ScriptFileName, char * szFunName, int nParam);
	BOOL	ExecuteScript(DWORD dwScriptId,  char * szFunName, int nParam);
	BOOL StartTimer(unsigned char ucTimerId, unsigned long ulTimerInterval)
	{
		KTimerTaskFun Timer;
		Timer.SetTimer(ulTimerInterval, ucTimerId);
		Timer.SetOwner(this);
		m_cTimerTaskSet.Add(&Timer);
		return TRUE;
	}

	BOOL StopTimer(unsigned char ucTimerId)
	{
		KTimerTaskFun Timer;
		Timer.SetTimer(1, ucTimerId);
		unsigned long ulIndex = 0;
		KTimerTaskFun * pTimer = m_cTimerTaskSet.GetData(&Timer);
		if (pTimer)
			pTimer->CloseTimer();
		
	}

	DWORD GetTimerRestTimer(unsigned char ucTimerId)
	{
		KTimerTaskFun Timer;
		Timer.SetTimer(1, ucTimerId);
		unsigned long ulIndex = 0;
		KTimerTaskFun * pTimer = m_cTimerTaskSet.GetData(&Timer);
		if (pTimer)
			return pTimer->GetRestTime();
		return 0;
	}

	BOOL StopMission()
	{
		m_ulMissionId = 0;
		Init();
		m_cTimerTaskSet.Clear();
		m_MissionPlayer.Clear();
		return TRUE;
	}
	
	KMissionPlayerArray m_MissionPlayer;

/*	unsigned long FindSame(unsigned long ulPlayerIndex )
	{
		if (ulPlayerIndex >= MAX_PLAYER)
		{
			return 0;
		}
		int nIdx = 0;

		while(1)
		{
			nIdx = m_UseIdx.GetNext(nIdx);
			if (!nIdx)
				break;
			
			if (m_MissionPlayer[nIdx].ulPlayerIndex == ulPlayerIndex)
				return nIdx;
		}
		return 0;
	};
*/

	unsigned long	AddPlayer(unsigned long ulPlayerIndex, unsigned long ulPlayerID, unsigned char ucPlayerGroup, int ulJoinTime = 0, int nParam1 = 0, int nParam2 = 0)
	{
		if (ulPlayerIndex >= MAX_PLAYER|| ulPlayerID == 0)
			return 0;
		
		unsigned long i = 0;
		TMissionPlayerInfo * pInfo = m_MissionPlayer.FindFree(i);
		
		if (!i || !pInfo)
			return 0;
		
		pInfo->m_ulPlayerIndex = ulPlayerIndex;
		pInfo->m_ulPlayerID = ulPlayerID;
		pInfo->m_nParam1 = nParam1;
		pInfo->m_nParam2 = nParam2;
		pInfo->m_ucPlayerGroup = ucPlayerGroup;
		
		if (!ulJoinTime)
			pInfo->m_ulJoinTime = g_SubWorldSet.GetGameTime();
		else
			pInfo->m_ulJoinTime = ulJoinTime;
		m_MissionPlayer.m_FreeIdx.Remove(i);
		m_MissionPlayer.m_UseIdx.Insert(i);
		m_MissionPlayer.m_ulFreeSize --;
		
		return i;
	};
	
	BOOL	RemovePlayer(unsigned long ulPlayerIndex, unsigned long ulPlayerID = 0)
	{
		if (ulPlayerIndex >= MAX_PLAYER)
			return 0;
		TMissionPlayerInfo Info;
		Info.m_ulPlayerIndex = ulPlayerIndex;
		Info.m_ulPlayerID = ulPlayerID;
		if (m_MissionPlayer.Remove(&Info))
		{
			if (m_ulMissionId)
			{
				char szScript[MAX_PATH];
				g_MissionTabFile.GetString(m_ulMissionId + 1, 2, "", szScript, MAX_PATH);
				if (szScript[0])
				{
					ExecuteScript(szScript, "OnLeave", ulPlayerIndex);
				}
			}
		}
		return TRUE;
	};

	unsigned long GetMissionPlayer_DataIndex(unsigned long ulPlayerIndex)//
	{
		if (ulPlayerIndex >= MAX_PLAYER)
			return 0;
		TMissionPlayerInfo Info;
		Info.m_ulPlayerIndex = ulPlayerIndex;
		return m_MissionPlayer.FindSame(&Info);
	}

	unsigned long GetMissionPlayer_PlayerIndex(unsigned long ulDataIndex)
	{
		if (ulDataIndex > m_MissionPlayer.GetTotalCount())
			return 0;
		return m_MissionPlayer.m_Data[ulDataIndex].m_ulPlayerIndex;		
	}
	
	unsigned long GetMissionPlayer_GroupId(unsigned long ulDataIndex)
	{
		if (ulDataIndex > m_MissionPlayer.GetTotalCount())
			return 0;
		return m_MissionPlayer.m_Data[ulDataIndex].m_ucPlayerGroup;
	}
};
#endif
#endif
