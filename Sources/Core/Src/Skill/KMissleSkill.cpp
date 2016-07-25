#include "Skill.h"
// KMissleSkills.cpp: implementation of the KMissleSkills class.
//
//////////////////////////////////////////////////////////////////////
#include "KCore.h"
#include "KMissleSkill.h"
#include "kmissle.h"
#include "KMissleSet.h"
#include "KNpc.h"
#include "math.h"
#include "KNpcSet.h"
#include "KSubWorld.h"
#include "KMath.h"
#include "KEngine.h"
#include "KTabFile.h"
#include "KTabFileCtrl.h"
#include "KMissleMagicAttribsData.h"
#ifndef _SERVER
#include "../../Represent/iRepresent/iRepresentshell.h"
#include "scene/KScenePlaceC.h"
#include "../../Represent/iRepresent/KRepresentUnit.h"
#include "imgref.h"
#include "KMagicDesc.h"
#include "skill.h"
#endif

#define	 NPCINDEXOFOBJECT 0 //物件发魔法时所对应的Npc编号

const char * g_MagicID2String(int nAttrib);

/*!*****************************************************************************
// Function		: KMissleSkill::KMissleSkill
// Purpose		: 
// Return		: 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
KMissleSkill::KMissleSkill()
{
	m_nSkillCostType = attrib_mana;
	m_nFlySkillId =  m_nCollideSkillId = m_nVanishedSkillId = -1;
	m_nWaitTime = 0;
	m_nRowIdInSkillTabFile = m_nImmediateAttribsNum = m_nStateAttribsNum = m_nMissleAttribsNum = m_nDamageAttribsNum = 0;
#ifndef _SERVER
	m_szSkillDesc[0] = 0;
	m_szPreCastEffectFile[0] = 0;
#endif
	
	
}
/*!*****************************************************************************
// Function		: KMissleSkill::~KMissleSkill
// Purpose		: 
// Return		: 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
KMissleSkill::~KMissleSkill()
{
	while (m_MissleLevelUpSettingList.GetTail())
	{
		KEffectDataNode * pNode = (KEffectDataNode*) m_MissleLevelUpSettingList.GetTail();
		delete pNode;
		m_MissleLevelUpSettingList.RemoveTail();
	}
}

//		当玩家调用某个技能时发生 [5/28/2002]
//		客户端和服务器端在技能的调用方面有一些不同
//		服务器端一般收到的经过客户端处理完的参数
//		游戏世界以消息命令机制执行每个变化，因此对于发技能应该也是统一数据接口
//		客户端时，如果该消息是来自本机玩家的输入，则必须将其转换为实际的消息
//		来执行。同时还应将转换好的消息传给服务器端
/*
有关传的参数是MapX，还是PointX根据具体的魔法技能而定
比如一般飞行魔法为Map坐标，而定点魔法为Point坐标
*/
/*
注意当调用Cast时，必须已确保当前的nLauncherIndex与Socket相对应的dwId一致，即IsMatch()通过。
*/

/*!*****************************************************************************
// Function		: KMissleSkill::Cast
// Purpose		: 发技能的统一接口
// Return		: 
// Argumant		: int nLauncher 发送者Id
// Argumant		: int nParam1   
// Argumant		: int nParam2
// Argumant		: int nWaitTime 发送的延迟时间
// Argumant		: eSkillLauncherType eLauncherType 发送者类型
// Comments		:
// Author		: RomanDou
*****************************************************************************/

BOOL	KMissleSkill::Cast(TSkillParam * pSkillParam)
{
	BOOL bResult = KFightSkill::Cast(pSkillParam);
	if (!bResult)  return FALSE;

	CastMissles(pSkillParam);
	if (m_bStartEvent)
	{
		TSkillParam StartSkillParam = *pSkillParam;
		StartSkillParam.usSkillID = m_nStartSkillId;
		StartSkillParam.usSkillLevel = m_nEventSkillLevel;
		g_CastSkill(&StartSkillParam);
	}
}

void	KMissleSkill::TrigerMissleEventSkill(int nMissleId, int nSkillId, int nSkillLevel)
{
	CheckAndGenSkillData();
	
	if (nMissleId > MAX_MISSLE || nMissleId < 0) return ;
	
	KMissle * pMissle = &(Missle[nMissleId]);
	
	if (pMissle->m_nMissleId < 0) return;

	if (pMissle->m_SkillParam.dwSkillParams & PARAM_LAUNCHER)
	{
		switch(pMissle->m_SkillParam.tLauncher.tActorInfo.nActorType)
		{
		case Actor_Npc:
			if (!Npc[pMissle->m_SkillParam.tLauncher.tActorInfo.nActorIndex].IsMatch(pMissle->m_SkillParam.tLauncher.tActorInfo.dwActorID)) 
				return;
			break;
		case Actor_Obj:
		//	if (!Object[pMissle->m_SkillParam.tLauncher.tActorInfo.nActorIndex].IsMatch(pMissle->m_SkillParam.tLauncher.tActorInfo.dwActorID)) 
				return;
			break;
		case Actor_Missle:
			if (!Missle[pMissle->m_SkillParam.tLauncher.tActorInfo.nActorIndex].IsMatch(pMissle->m_SkillParam.tLauncher.tActorInfo.dwActorID)) 
				return;
			break;
		default:
			return;
		}
	}

	if (nSkillId > 0)//如果有消亡技能则发消亡技能
	{
		if (nSkillLevel <= 0) return ;
		TSkillParam SkillParam ;
		SkillParam.usSkillID = nSkillId;
		SkillParam.usSkillLevel = nSkillLevel;
		SkillParam.nWaitTime = 0;
		SkillParam.tLauncher = pMissle->m_SkillParam.tLauncher;
		
		if (m_bByMissle)
		{	
			SkillParam.dwSkillParams |= PARAM_REF;
			SkillParam.tRef.tActorInfo.nActorType = Actor_Missle;
			SkillParam.tRef.tActorInfo.dwActorID = pMissle->m_dwID;
			SkillParam.tRef.tActorInfo.nActorIndex = nMissleId;
		}
		else
		{
			SkillParam.dwSkillParams |= (pMissle->m_SkillParam.dwSkillParams & PARAM_LAUNCHER != 0);
			SkillParam.tLauncher.tActorInfo = pMissle->m_SkillParam.tLauncher.tActorInfo;
		}
		
		pMissle->GetMpsPos(&SkillParam.tTarget.tPoint.nX, &SkillParam.tTarget.tPoint.nY);
		SkillParam.tTarget.tPoint.nSubWorldIndex = pMissle->m_nSubWorldId;
		
		g_CastSkill(&SkillParam);
	}


}

/*!*****************************************************************************
// Function		: KMissleSkill::Vanish
// Purpose		: 子弹生命结束时回调
// Return		: 
// Argumant		: KMissle* Missle
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void	KMissleSkill::Vanish(int nMissleId)
{
	TrigerMissleEventSkill(nMissleId, m_nVanishedSkillId, m_nEventSkillLevel);
}
/*!*****************************************************************************
// Function		: KMissleSkill::FlyEvent
// Purpose		: 
// Return		: void 
// Argumant		: int nMissleId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissleSkill::FlyEvent(int nMissleId)
{
	TrigerMissleEventSkill(nMissleId, m_nFlySkillId, m_nEventSkillLevel);
}

/*!*****************************************************************************
// Function		: KMissleSkill::Collidsion
// Purpose		: 子弹被撞时回调
// Return		: 
// Argumant		: KMissle* Missle
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void	KMissleSkill::Collidsion(int nMissleId)
{
	TrigerMissleEventSkill(nMissleId, m_nCollideSkillId, m_nEventSkillLevel);
}
/*!*****************************************************************************
// Function		: KMissleSkill::CastMissles
// Purpose		: 发送子弹技能
// Return		: 
// Argumant		: int nLauncher  发送者id
// Argumant		: int nParam1
// Argumant		: int nParam2
// Argumant		: int nWaitTime  延长时间
// Argumant		: eSkillLauncherType eLauncherType 发送者类型
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL	KMissleSkill::CastMissles(TSkillParam * pSkillParam)
{
	_ASSERT(pSkillParam);
	int nRegionId		=	0;
	int nSrcPX			=	0;//点坐标
	int nSrcPY			=	0;
	int nDesPX			=	0;
	int nDesPY			=	0;

	
	if (pSkillParam->dwSkillParams & PARAM_USEREF)
	{
		if(pSkillParam->dwSkillParams & PARAM_REF)
		{
			pSkillParam->tRef.tActorInfo.GetMps(nSrcPX, nSrcPY);
		}
		else
		{
			nSrcPX = pSkillParam->tRef.tPoint.nX;
			nSrcPY = pSkillParam->tRef.tPoint.nY;

		}
	}
	else if (pSkillParam->dwSkillParams & PARAM_LAUNCHER)
	{
		pSkillParam->tLauncher.tActorInfo.GetMps(nSrcPX, nSrcPY);
	}
	else
	{
		nSrcPX = pSkillParam->tLauncher.tPoint.nX;
		nSrcPY = pSkillParam->tLauncher.tPoint.nY;
	}


	if (pSkillParam->dwSkillParams & PARAM_TARGET)
	{
		pSkillParam->tTarget.tActorInfo.GetMps(nDesPX,nDesPY);
	}
	else
	{
		nDesPX = pSkillParam->tTarget.tPoint.nX;
		nDesPY = pSkillParam->tTarget.tPoint.nY;
	}
	
	CastMissleFunc[m_eMisslesForm](nSrcPX, nSrcPY, nDesPX, nDesPY, pSkillParam);
	return TRUE;
}

/*!*****************************************************************************
// Function		: KMissleSkill::CastZone
// Purpose		: 
// Return		: int 
// Argumant		: int nLauncher
// Argumant		: eSkillLauncherType eLauncherType
// Argumant		: int nDir
// Argumant		: int nRefPX
// Argumant		: int nRefPY
// Argumant		: int nWaitTime
// Argumant		: int nTargetId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
//nValue1 = 0 表示矩形区域  nValue1 = 1 表示圆形区域
//nValue2 = 0 
int KMissleSkill::CastZone(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam)
{
	if (pSkillParam->dwSkillParams & PARAM_ONLYDIR) return 0;
	
	int nCastMissleNum	= 0;
	int nBeginPX ;
	int nBeginPY ;
	if (m_nChildSkillNum == 1)
	{
		nBeginPX = nRefPX;
		nBeginPY = nRefPY;
	}
	else 
	{
		nBeginPX	= nRefPX - m_nChildSkillNum * pSkillParam->GetCellWidth() / 2;
		nBeginPY	= nRefPY - m_nChildSkillNum * pSkillParam->GetCellHeight() / 2;
	}
	
#ifdef _SERVER
	KMissleMagicAttribsData * pNewMagicAttribsData = CreateMissleMagicAttribsData(pSkillParam);
#endif //_SERVER
	
	for (int i = 0; i < m_nChildSkillNum; i ++)
		for (int j = 0; j < m_nChildSkillNum; j ++)
		{
			if (m_bBaseSkill)
			{
				int nMissleIndex ;
				int nSubWorldId = pSkillParam->GetSubWorldIndex(); 
				if (nSubWorldId < 0) goto exit;
				
				if (m_nValue1 == 1)
					if ( ((i - m_nChildSkillNum / 2) * (i - m_nChildSkillNum / 2) + (j - m_nChildSkillNum / 2) * (j - m_nChildSkillNum / 2)) > (m_nChildSkillNum * m_nChildSkillNum / 4))
						continue;

					nMissleIndex = MissleSet.Add(nSubWorldId, nBeginPX + j * SubWorld[nSubWorldId].m_nCellWidth , nBeginPY +  i * SubWorld[nSubWorldId].m_nCellHeight);
					if (nMissleIndex < 0)	continue;
					Missle[nMissleIndex].m_nDir				= g_GetDirIndex(nSrcPX, nSrcPY, nRefPX, nRefPY);
					Missle[nMissleIndex].m_nDirIndex		= g_DirIndex2Dir(Missle[nMissleIndex].m_nDir, MaxMissleDir);
					CreateMissle(pSkillParam, m_nChildSkillId, nMissleIndex);
					Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
					Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
					Missle[nMissleIndex].m_SkillParam		= *pSkillParam;
					Missle[nMissleIndex].m_nStartLifeTime	= pSkillParam->nWaitTime + GetMissleGenerateTime(i * m_nChildSkillNum + j);
					Missle[nMissleIndex].m_nLifeTime		+= Missle[nMissleIndex].m_nStartLifeTime;
					
					if (Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Line)
					{
						Missle[nMissleIndex].m_nXFactor = g_DirCos(Missle[nMissleIndex].m_nDir, MaxMissleDir);
						Missle[nMissleIndex].m_nYFactor = g_DirSin(Missle[nMissleIndex].m_nDir, MaxMissleDir);
					}
#ifdef _SERVER
					Missle[nMissleIndex].SetMagicAttribsData(pNewMagicAttribsData);
#endif //
					
					nCastMissleNum ++;
#ifndef _SERVER				
					Missle[nMissleIndex].m_MissleRes.PlaySound(Missle[nMissleIndex].m_eMissleStatus, 0, 0, 0);
#endif
					
			}
			else
			{
				if (m_nChildSkillLevel <= 0) continue;
					
				TSkillParam ChildSkillParam;
				ChildSkillParam.dwSkillParams = 0;
				
				ChildSkillParam.dwSkillParams &= ((pSkillParam->dwSkillParams & PARAM_LAUNCHER ) != 0);
				ChildSkillParam.tLauncher.tActorInfo = pSkillParam->tLauncher.tActorInfo;

				ChildSkillParam.tTarget.tPoint.nX = nBeginPX + j * pSkillParam->GetCellWidth() ;
				ChildSkillParam.tTarget.tPoint.nY = nBeginPY +  i * pSkillParam->GetCellHeight();
				ChildSkillParam.nWaitTime = pSkillParam->nWaitTime + GetMissleGenerateTime(i * m_nChildSkillNum + j );
				ChildSkillParam.usSkillID = m_nChildSkillId;
				ChildSkillParam.usSkillLevel = m_nChildSkillLevel;
				nCastMissleNum += g_CastSkill(&ChildSkillParam);
			}
			
		}
exit:	
#ifdef _SERVER
		if (!nCastMissleNum) 
			if (pNewMagicAttribsData)
				if (pNewMagicAttribsData->GetRef() == 0)
					delete pNewMagicAttribsData;
#endif
				return nCastMissleNum;
}

/*!*****************************************************************************
// Function		: KMissleSkill::CastLine
// Purpose		: 
// Return		: 
// Argumant		: int nLauncher
// Argumant		: eSkillLauncherType eLauncherType
// Argumant		: int nDir
// Argumant		: int nRefPX
// Argumant		: int nRefPY
// Argumant		: int nWaitTime
// Argumant		: int nTargetId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
// Value1 子弹之间的间距
// Value2 
int		KMissleSkill::CastLine(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam)
{
	_ASSERT(pSkillParam);
	int nDesSubX		= 0;
	int nDesSubY		= 0;
	int nCastMissleNum	= 0;
	int nDirIndex = 0;
	int nDir = 0;
	
	if (pSkillParam->dwSkillParams & PARAM_ONLYDIR)
	{
		nDir = pSkillParam->nDir;
		nDirIndex = g_Dir2DirIndex(nDir, MaxMissleDir);
	}
	else
	{
		nDirIndex = g_GetDirIndex(nSrcPX, nSrcPY, nRefPX, nRefPY);
		nDir = g_DirIndex2Dir(nDirIndex , MaxMissleDir);
	}
	
	//子弹之间的间距
	int nMSDistanceEach = m_nValue1;
	
#ifdef _SERVER
	KMissleMagicAttribsData * pNewMagicAttribsData = CreateMissleMagicAttribsData(pSkillParam);
#endif //_SERVER
	
	//分别生成多少子弹
	for(int i = 0; i < m_nChildSkillNum; i++)
	{
		nDesSubX	= nRefPX + ((nMSDistanceEach * (i + 1) * g_DirCos(nDirIndex, MaxMissleDir) )>>10);
		nDesSubY	= nRefPY + ((nMSDistanceEach * (i + 1) * g_DirSin(nDirIndex, MaxMissleDir) )>>10);
		
		if (nDesSubX < 0 || nDesSubY < 0) 	continue;
		
		if (m_bBaseSkill)
		{
			int nMissleIndex ;
			int nSubWorldId = pSkillParam->GetSubWorldIndex(); 

			if (nSubWorldId < 0)	goto exit;
			
			nMissleIndex = MissleSet.Add(nSubWorldId, nDesSubX, nDesSubY);
			if (nMissleIndex < 0)	continue;
			
			Missle[nMissleIndex].m_nDir				= nDir;
			Missle[nMissleIndex].m_nDirIndex		= nDirIndex;
			CreateMissle(pSkillParam, m_nChildSkillId, nMissleIndex);
			Missle[nMissleIndex].m_SkillParam		= *pSkillParam;
			Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
			Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
			Missle[nMissleIndex].m_nStartLifeTime	= pSkillParam->nWaitTime + GetMissleGenerateTime(i);
			Missle[nMissleIndex].m_nLifeTime		+= Missle[nMissleIndex].m_nStartLifeTime;	
			if (Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Line)
			{
				Missle[nMissleIndex].m_nXFactor = g_DirCos(nDir, MaxMissleDir);
				Missle[nMissleIndex].m_nYFactor = g_DirSin(nDir, MaxMissleDir);
			}
			
			
#ifdef _SERVER
			Missle[nMissleIndex].SetMagicAttribsData(pNewMagicAttribsData);
#endif //_SERVER
			
#ifndef _SERVER				
			Missle[nMissleIndex].m_MissleRes.PlaySound(Missle[nMissleIndex].m_eMissleStatus, 0, 0, 0)		;		
#endif
			nCastMissleNum ++;
		}
		else
		{
				if (m_nChildSkillLevel <= 0) continue;

				TSkillParam ChildSkillParam;
				ChildSkillParam.dwSkillParams = 0;

				ChildSkillParam.dwSkillParams &= ((pSkillParam->dwSkillParams & PARAM_LAUNCHER ) != 0);
				ChildSkillParam.tLauncher.tActorInfo = pSkillParam->tLauncher.tActorInfo;

				ChildSkillParam.tTarget.tPoint.nX = nDesSubX ;
				ChildSkillParam.tTarget.tPoint.nY = nDesSubY;
				ChildSkillParam.nWaitTime = pSkillParam->nWaitTime + GetMissleGenerateTime(i);
				ChildSkillParam.usSkillID = m_nChildSkillId;
				ChildSkillParam.usSkillLevel = m_nChildSkillLevel;
				nCastMissleNum += g_CastSkill(&ChildSkillParam);
		}
		
	}
	
exit:	
#ifdef _SERVER
	if (!nCastMissleNum) 
		if (pNewMagicAttribsData)
			if (pNewMagicAttribsData->GetRef() == 0)
				delete pNewMagicAttribsData;
#endif
			return nCastMissleNum;
}
/*
int		KMissleSkill::CastExtractiveLineMissle(int nSrcX, int nSrcY, int nDesX, int nDesY, int nXOffset, int nYOffset, int nDesX, int nDesY, TSkillParam * pSkillParam)
{
	int nDesSubX		= 0;
	int nDesSubY		= 0;
	int nCastMissleNum	= 0;
	
	//子弹之间的间距

	if (nSrcPX == nDesPX && nSrcPY == nDesPY)		return FALSE ;
	nDistance = g_GetDistance(nSrcPX, nSrcPY, nDesPX, nDesPY);
	if (nDistance == 0 ) return FALSE;
	int		nYLength = nDesPY - nSrcPY;
	int		nXLength = nDesPX - nSrcPX;
	int		nSin = (nYLength << 10) / nDistance;	// 放大1024倍
	int		nCos = (nXLength << 10) / nDistance;
	
#ifdef _SERVER
	KMissleMagicAttribsData * pNewMagicAttribsData = CreateMissleMagicAttribsData(nLauncher);
#endif //_SERVER
	
	//分别生成多少子弹
	{
		
		if (m_bBaseSkill)
		{
			int nMissleIndex ;
			int nSubWorldId = pSkillParam->GetSubWorldIndex(); 
			
			if (nSubWorldId < 0)	goto exit;
			nMissleIndex = MissleSet.Add(nSubWorldId, nSrcPX, nSrcPY);
			
			if (nMissleIndex < 0)	goto exit;
			
			Missle[nMissleIndex].m_nDir				= g_GetDirIndex(nSrcPX, nSrcPY, nDesPX, nDesPY);
			Missle[nMissleIndex].m_nDirIndex		= g_DirIndex2Dir(Missle[nMissleIndex].m_nDir, MaxMissleDir);
			CreateMissle(pSkillParam, m_nChildSkillId, nMissleIndex);

			if (Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Parabola)
			{
				int nLength = g_GetDistance(nSrcPX, nSrcPY, nDesX, nDesY);
				int nTime = nLength / Missle[nMissleIndex].m_nSpeed;
				Missle[nMissleIndex].m_nHeightSpeed	= Missle[nMissleIndex].m_nZAcceleration * (nTime - 1) / 2;
			}

			Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
			Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
			Missle[nMissleIndex].m_SkillParam		= *pSkillParam;
			Missle[nMissleIndex].m_nStartLifeTime	= pSkillParam->nWaitTime + GetMissleGenerateTime(0);
			Missle[nMissleIndex].m_nLifeTime		+= Missle[nMissleIndex].m_nStartLifeTime;	
			if (Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Line || Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Parabola)
			{
				
				Missle[nMissleIndex].m_nXFactor = nCos;
				Missle[nMissleIndex].m_nYFactor = nSin;
			}
			
			
#ifdef _SERVER
			Missle[nMissleIndex].SetMagicAttribsData(pNewMagicAttribsData);
#endif //_SERVER
			
#ifndef _SERVER				
			Missle[nMissleIndex].m_MissleRes.PlaySound(Missle[nMissleIndex].m_eMissleStatus, 0, 0, 0)		;		
#endif
			nCastMissleNum ++;
		}
		else
		{
				if (m_nChildSkillLevel <= 0) continue;

				TSkillParam ChildSkillParam;
				ChildSkillParam.dwSkillParams = 0;
				
				ChildSkillParam.dwSkillParams &= ((pSkillParam->dwSkillParams & PARAM_LAUNCHER ) != 0);
				ChildSkillParam.tLauncher.tActorInfo = pSkillParam.tLauncher.tActorInfo;

				ChildSkillParam.tTarget.tPoint.nX = nSrcPX ;
				ChildSkillParam.tTarget.tPoint.nY = nSrcPY;
				ChildSkillParam.nWaitTime = pSkillParam->nWaitTime + GetMissleGenerateTime(0);
				ChildSkillParam.usSkillID = m_nChildSkillId;
				ChildSkillParam.usSkillLevel = m_nChildSkillLevel;
				nCastMissleNum += g_CastSkill(&ChildSkillParam);

		}
		
	}
	
exit:	
#ifdef _SERVER
	if (!nCastMissleNum) 
		if (pNewMagicAttribsData)
			if (pNewMagicAttribsData->GetRef() == 0)
				delete pNewMagicAttribsData;
#endif
			
			return nCastMissleNum;
			
}
*/
/*!*****************************************************************************
// Function		: KMissleSkill::CastWall
// Purpose		: Wall Magic 
// Return		: int 
// Argumant		: int nLauncher
// Argumant		: eSkillLauncherType eLauncherType
// Argumant		: int nDir
// Argumant		: int nRefPX
// Argumant		: int nRefPY
// Argumant		: int nWaitTime
// Argumant		: int nTargetId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
/*
m_nValue1 表示子弹之间的距离，单位像素点
*/
	/*
	火墙时，第一数字参数表示子弹之间的长度间隔
	X2  = X1 + N * SinA
	Y2  = Y2 - N * CosA
	*/

int KMissleSkill::CastWall(int nSrcPX, int nSrcPY, int nDesPX, int nDesPY, TSkillParam * pSkillParam)
						   
{
	_ASSERT(pSkillParam);
	int nDir = 0;
	int nDirIndex = 0;

	//墙形魔法不可以只传方向
	if (pSkillParam->dwSkillParams & PARAM_ONLYDIR) return FALSE;
			
	nDirIndex		= g_GetDirIndex(nSrcPX, nSrcPY, nDesPX, nDesPY);
	nDir			= g_DirIndex2Dir(nDirIndex, MaxMissleDir);
	nDir = nDir + MaxMissleDir / 4;
	if (nDir >= MaxMissleDir) nDir -= MaxMissleDir;

	int nDesSubX		= 0;
	int nDesSubY		= 0;
	int nCastMissleNum	= 0;
	
	//子弹之间的间距
	int nMSDistanceEach = m_nValue1;
	int nCurMSDistance	= -1 * nMSDistanceEach * m_nChildSkillNum / 2;
	
	
#ifdef _SERVER
	KMissleMagicAttribsData * pNewMagicAttribsData = CreateMissleMagicAttribsData(pSkillParam->tLauncher.tActorInfo);
#endif //_SERVER
	
	
	//分别生成多少子弹
	for(int i = 0; i < m_nChildSkillNum; i++)
	{
		nDesSubX	= nDesPX + ((nCurMSDistance * g_DirCos(nDirIndex,MaxMissleDir)) >>10);
		nDesSubY	= nDesPY + ((nCurMSDistance * g_DirSin(nDirIndex,MaxMissleDir)) >>10);
		
		if (nDesSubX < 0 || nDesSubY < 0) 	continue;
		
		if (m_bBaseSkill)
		{
			int nMissleIndex ;
			int nSubWorldId = pSkillParam->GetSubWorldIndex(); 
			
			if (nSubWorldId < 0)	goto exit;
			nMissleIndex = MissleSet.Add(nSubWorldId, nDesSubX, nDesSubY);
			if (nMissleIndex < 0)	continue;
			
			Missle[nMissleIndex].m_nDir				= nDir;
			Missle[nMissleIndex].m_nDirIndex		= nDirIndex;
			Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
			CreateMissle(pSkillParam, m_nChildSkillId, nMissleIndex);
			
			Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
			Missle[nMissleIndex].m_SkillParam		= *pSkillParam;
			Missle[nMissleIndex].m_nStartLifeTime	= pSkillParam->nWaitTime + GetMissleGenerateTime(i);
			Missle[nMissleIndex].m_nLifeTime		+= Missle[nMissleIndex].m_nStartLifeTime;
			
			if (Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Line)
			{
				Missle[nMissleIndex].m_nXFactor = g_DirCos(nDir, MaxMissleDir);
				Missle[nMissleIndex].m_nYFactor = g_DirSin(nDir, MaxMissleDir);
			}
			
#ifdef _SERVER
			Missle[nMissleIndex].SetMagicAttribsData(pNewMagicAttribsData);
#endif //_SERVER
			
#ifndef _SERVER							
			Missle[nMissleIndex].m_MissleRes.PlaySound(Missle[nMissleIndex].m_eMissleStatus, 0, 0, 0);		
#endif
			nCastMissleNum ++;
		}
		else
		{
				if (m_nChildSkillLevel <= 0) continue;
				TSkillParam ChildSkillParam;
				ChildSkillParam.dwSkillParams = 0;
				
				ChildSkillParam.dwSkillParams &= ((pSkillParam->dwSkillParams & PARAM_LAUNCHER ) != 0);
				ChildSkillParam.tLauncher.tActorInfo = pSkillParam->tLauncher.tActorInfo;

				ChildSkillParam.tTarget.tPoint.nX = nDesSubX ;
				ChildSkillParam.tTarget.tPoint.nY = nDesSubY;
				ChildSkillParam.nWaitTime = pSkillParam->nWaitTime + GetMissleGenerateTime(i);
				ChildSkillParam.usSkillID = m_nChildSkillId;
				ChildSkillParam.usSkillLevel = m_nChildSkillLevel;
				nCastMissleNum += g_CastSkill(&ChildSkillParam);
		}
		
		nCurMSDistance += nMSDistanceEach;
	}
	
exit:	
#ifdef _SERVER
	if (!nCastMissleNum) 
		if (pNewMagicAttribsData)
			if (pNewMagicAttribsData->GetRef() == 0)
				delete pNewMagicAttribsData;
#endif
			
			return nCastMissleNum;
}

/*!*****************************************************************************
// Function		: KMissleSkill::CastCircle
// Purpose		: 
// Return		: 
// Argumant		: int nLauncher
// Argumant		: eSkillLauncherType  eLauncherType
// Argumant		: int nDir
// Argumant		: int nRefPX
// Argumant		: int nRefPY
// Argumant		: int nWaitTime
// Argumant		: int nTargetId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
// Value1  == 0 表示发送者为圆心产生圆，否则以目标点为圆心产生圆
int		KMissleSkill::CastCircle(int nSrcPX, int nSrcPY, int nDesPX, int nDesPY, TSkillParam * pSkillParam)
{
	int nDesSubPX	= 0;
	int nDesSubPY	= 0;
	int nFirstStep	= m_nValue2;			//第一步的长度，子弹在刚发出去时离玩家的距离
	int nCurSubDir	= 0;
	int nDirPerNum  = 	MaxMissleDir / m_nChildSkillNum  ;
	int nCastMissleNum = 0;
	
	if (pSkillParam->dwSkillParams & PARAM_ONLYDIR) return 0;
	int nDirIndex = g_GetDirIndex(nSrcPX, nSrcPY, nDesPX, nDesPY);
	int nDir      = g_DirIndex2Dir(nDirIndex, MaxMissleDir);
	
	int nRefPX = 0;
	int nRefPY = 0;

	//以当前点为圆点产生多个围扰的子弹
	//分成两种情况，一种为以原地为原心发出，另一种为以目标点为原心发出
	// 数字参数一表示 是否为原地发出
	if (m_nValue1 == 0)
	{
		nRefPX = nSrcPX;
		nRefPY = nSrcPY;
	}
	else
	{
		nRefPX = nDesPX;
		nRefPY = nDesPY;
	}
	
#ifdef _SERVER
	KMissleMagicAttribsData * pNewMagicAttribsData = CreateMissleMagicAttribsData(pSkillParam);
#endif //_SERVER

	//分别生成多个子弹
	for(int i = 0; i < m_nChildSkillNum; i++)
	{
		int nCurSubDir	= nDir + nDirPerNum * i ;
		
		if (nCurSubDir < 0)
			nCurSubDir = MaxMissleDir + nCurSubDir;
		
		if (nCurSubDir >= MaxMissleDir)
			nCurSubDir -= MaxMissleDir;
		
		int nSinAB	= g_DirSin(nCurSubDir, MaxMissleDir);
		int nCosAB	= g_DirCos(nCurSubDir, MaxMissleDir);
		
		nDesSubPX	= nRefPX + ((nCosAB * nFirstStep) >> 10);
		nDesSubPY	= nRefPY + ((nSinAB * nFirstStep) >> 10);
		
		if (nDesSubPX < 0 || nDesSubPY < 0) 	continue;
		
		if (m_bBaseSkill)
		{
			int nMissleIndex ;
			int nSubWorldId = pSkillParam->GetSubWorldIndex(); 
			
			if (nSubWorldId < 0)	goto exit;
			nMissleIndex = MissleSet.Add(nSubWorldId, nDesSubPX, nDesSubPY);
			
			if (nMissleIndex < 0)	continue;

			Missle[nMissleIndex].m_nDir			= nCurSubDir;
			Missle[nMissleIndex].m_nDirIndex	= g_Dir2DirIndex(nCurSubDir, MaxMissleDir);
			CreateMissle(pSkillParam, m_nChildSkillId, nMissleIndex);

			
			Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
			Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
			Missle[nMissleIndex].m_SkillParam		= *pSkillParam;	
			Missle[nMissleIndex].m_nStartLifeTime	= pSkillParam->nWaitTime + GetMissleGenerateTime(i);
			Missle[nMissleIndex].m_nLifeTime		+= Missle[nMissleIndex].m_nStartLifeTime;
			
			if (Missle[nMissleIndex].m_eMoveKind == MISSLE_MMK_Line)
			{
				Missle[nMissleIndex].m_nXFactor = g_DirCos(nCurSubDir, MaxMissleDir);
				Missle[nMissleIndex].m_nYFactor = g_DirSin(nCurSubDir, MaxMissleDir);
			}
			
			
#ifdef _SERVER
			Missle[nMissleIndex].SetMagicAttribsData(pNewMagicAttribsData);
#endif //_SERVER
			
#ifndef _SERVER							
			Missle[nMissleIndex].m_MissleRes.PlaySound(Missle[nMissleIndex].m_eMissleStatus, 0, 0, 0);
#endif			
			nCastMissleNum ++;
			
		}
		else
		{
				if (m_nChildSkillLevel <= 0) continue;
				TSkillParam ChildSkillParam;
				ChildSkillParam.dwSkillParams = 0;
				
				ChildSkillParam.dwSkillParams &= ((pSkillParam->dwSkillParams & PARAM_LAUNCHER ) != 0);
				ChildSkillParam.tLauncher.tActorInfo = pSkillParam->tLauncher.tActorInfo;

				ChildSkillParam.tTarget.tPoint.nX = nDesSubPX ;
				ChildSkillParam.tTarget.tPoint.nY = nDesSubPX;
				ChildSkillParam.nWaitTime = pSkillParam->nWaitTime + GetMissleGenerateTime(i);
				ChildSkillParam.usSkillID = m_nChildSkillId;
				ChildSkillParam.usSkillLevel = m_nChildSkillLevel;
				nCastMissleNum += g_CastSkill(&ChildSkillParam);
		}
		
	}
	
exit:	
#ifdef _SERVER
	if (!nCastMissleNum) 
		if (pNewMagicAttribsData)
			if (pNewMagicAttribsData->GetRef() == 0)
				delete pNewMagicAttribsData;
#endif
			
			return nCastMissleNum;
}

/*!*****************************************************************************
// Function		: KMissleSkill::CastSpread
// Purpose		: 
// Return		: 
// Argumant		: int nLauncher
// Argumant		: eSkillLauncherType eLauncherType
// Argumant		: int nDir
// Argumant		: int nRefPX
// Argumant		: int nRefPY
// Argumant		: int nWaitTime
// Argumant		: int nTargetId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
/*
Value1 每个子弹相差的角度单位
Value2 每一步的长度，第一步的长度，子弹在刚发出去时离玩家的距离
*/
//  数字参数一表示子弹之间的角度差，以64方向为准
		//  传来的X/Y参数为格子坐标
int		KMissleSkill::CastSpread(int nSrcPX, int nSrcPY, int nDesPX, int nDesPY, TSkillParam * pSkillParam)
{
	int nDesSubMapX		= 0;
	int nDesSubMapY		= 0;
	int nFirstStep		= m_nValue2;			//第一步的长度，子弹在刚发出去时离玩家的距离
	int nCurMSRadius	= m_nChildSkillNum / 2 ; 
	int nCurSubDir		= 0;
	int	nCastMissleNum  = 0;			//实际发送的Missle的数量
	int nDir = 0;
	int nDirIndex = 0;
	// Sin A+B = SinA*CosB + CosA*SinB
	// Cos A+B = CosA*CosB - SinA*SinB
	// Sin A = nYFactor
	// Cos A = nXFactor
	
#ifdef _SERVER
	KMissleMagicAttribsData * pNewMagicAttribsData = CreateMissleMagicAttribsData(pSkillParam->tLauncher.tActorInfo);
#endif
	
	int nDesSubX = 0;
	int nDesSubY = 0;
	int nXFactor = 0;
	int nYFactor = 0;
	
	if (pSkillParam->dwSkillParams & PARAM_ONLYDIR)
	{
		nDir = pSkillParam->nDir;
		nDirIndex = g_Dir2DirIndex(nDir, MaxMissleDir);
	}
	else
	{
		nDirIndex = g_GetDirIndex(nSrcPX, nSrcPY, nDesPX, nDesPY);
		nDir = g_DirIndex2Dir(nDirIndex , MaxMissleDir);
	}

	
	if (pSkillParam->dwSkillParams & PARAM_TARGET)
	{
		int nDistance = 0;
		nDistance = (int)sqrt((nDesPX - nSrcPX)*(nDesPX - nSrcPX) +	(nDesPY - nSrcPY)*(nDesPY - nSrcPY));
		nXFactor = ((nDesPX - nSrcPX) << 10) / nDistance;
		nYFactor = ((nDesPY - nSrcPY) << 10) / nDistance;
		
		nDesSubX = nSrcPX + ((nXFactor * nFirstStep) >> 10);
		nDesSubY = nSrcPY + ((nYFactor * nFirstStep) >> 10);
		
		if (nDesSubX < 0  || nDesSubY < 0 ) return 0;
	}
	
	
	//分别生成多个子弹
	for(int i = 0; i < m_nChildSkillNum; i++)
	{
		int nDSubDir	= m_nValue1 * nCurMSRadius; 
		nCurSubDir		= nDir - m_nValue1 * nCurMSRadius;
		
		
		if (nCurSubDir < 0)
			nCurSubDir = MaxMissleDir + nCurSubDir;
		
		if (nCurSubDir >= MaxMissleDir)
			nCurSubDir -= MaxMissleDir;
		
		int nSinAB	;
		int nCosAB	;
		
		if (pSkillParam->dwSkillParams & PARAM_TARGET)
		{
			nDSubDir	+= 48;
			if (nDSubDir >= MaxMissleDir)
				nDSubDir -= MaxMissleDir;
			//sin(a - b) = sinacosb - cosa*sinb
			//cos(a - b) = cosacoab + sinasinb
			nSinAB = (nYFactor * g_DirCos(nDSubDir, MaxMissleDir) - nXFactor * g_DirSin(nDSubDir, MaxMissleDir)) >> 10;
			nCosAB = (nXFactor * g_DirCos(nDSubDir, MaxMissleDir) + nYFactor * g_DirSin(nDSubDir , MaxMissleDir)) >> 10;
		}
		else
		{
			nSinAB = g_DirSin(nCurSubDir, MaxMissleDir);
			nCosAB = g_DirCos(nCurSubDir, MaxMissleDir);
		}
		
		nDesSubX	= nDesPX + ((nCosAB * nFirstStep) >> 10);
		nDesSubY	= nDesPY + ((nSinAB * nFirstStep) >> 10);
		
		if (nDesSubX < 0 || nDesSubY < 0) 	continue;
		
		if (m_bBaseSkill)
		{
			
			int nMissleIndex ;
			int nSubWorldId = pSkillParam->GetSubWorldIndex(); 
			if (nSubWorldId < 0)	goto exit;
			
			
			nMissleIndex = MissleSet.Add(nSubWorldId, nDesSubX, nDesSubY);
			
			
			if (nMissleIndex < 0)	continue;

			Missle[nMissleIndex].m_nDir				= nCurSubDir;
			Missle[nMissleIndex].m_nDirIndex		= g_Dir2DirIndex(nCurSubDir, MaxMissleDir);
			CreateMissle(pSkillParam, m_nChildSkillId, nMissleIndex);
			Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
			Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
			Missle[nMissleIndex].m_nSrcMapX			= nDesPX;
			Missle[nMissleIndex].m_nSrcMapY			= nDesPX;
			Missle[nMissleIndex].m_nStartLifeTime	= pSkillParam->nWaitTime + GetMissleGenerateTime(i);
			Missle[nMissleIndex].m_nLifeTime		+= Missle[nMissleIndex].m_nStartLifeTime;
			Missle[nMissleIndex].m_nXFactor			= nCosAB;
			Missle[nMissleIndex].m_nYFactor			= nSinAB;
			
#ifdef _SERVER
			Missle[nMissleIndex].SetMagicAttribsData(pNewMagicAttribsData);
#endif //_SERVER
			
#ifndef _SERVER							
			Missle[nMissleIndex].m_MissleRes.PlaySound(Missle[nMissleIndex].m_eMissleStatus, 0, 0, 0)		;
#endif			
			nCastMissleNum ++;
		}
		else
		{
			if (m_nChildSkillLevel <= 0) continue;
				TSkillParam ChildSkillParam;
				ChildSkillParam.dwSkillParams = 0;
				
				ChildSkillParam.dwSkillParams &= ((pSkillParam->dwSkillParams & PARAM_LAUNCHER ) != 0);
				ChildSkillParam.tLauncher.tActorInfo = pSkillParam->tLauncher.tActorInfo;

				ChildSkillParam.tTarget.tPoint.nX = nDesSubX ;
				ChildSkillParam.tTarget.tPoint.nY = nDesSubY;
				ChildSkillParam.nWaitTime = pSkillParam->nWaitTime + GetMissleGenerateTime(i);
				ChildSkillParam.usSkillID = m_nChildSkillId;
				ChildSkillParam.usSkillLevel = m_nChildSkillLevel;
				nCastMissleNum += g_CastSkill(&ChildSkillParam);
		}
		
		nCurMSRadius -- ;
	}
exit:	
#ifdef _SERVER
	if (!nCastMissleNum) 
		if (pNewMagicAttribsData)
			if (pNewMagicAttribsData->GetRef() == 0)
				delete pNewMagicAttribsData;
#endif
			
			return nCastMissleNum;
}

/*!*****************************************************************************
// Function		: KMissleSkill::CreateMissle
// Purpose		: 设置子弹的基本数据，以及该技能该等级下的对子弹信息的变动数据
//					设置用于数值计算的指针
// Return		: 
// Argumant		: int nChildSkillId
// Argumant		: int nMissleIndex
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void	KMissleSkill::CreateMissle(TSkillParam * pSkillParam, int nChildSkillId, int nMissleIndex)
{
	_ASSERT(pSkillParam);
	KMissle * pMissle = &Missle[nMissleIndex];
	g_MisslesLib[nChildSkillId] = *pMissle;//复制拷贝对象
	
	pMissle->m_nLevel			= m_nLevel;
	pMissle->m_bCollideEvent	= m_bCollideEvent;
	pMissle->m_bVanishedEvent   = m_bVanishedEvent;
	pMissle->m_bStartEvent		= m_bStartEvent;
	pMissle->m_bFlyEvent		= m_bFlyingEvent;
	pMissle->m_nFlyEventTime	= m_nFlyEventTime;
	pMissle->m_nMissleId		= nMissleIndex;
	pMissle->m_bClientSend      = m_bClientSend;
	pMissle->m_bMustBeHit		= m_bMustBeHit;
	pMissle->m_bIsMelee			= m_bIsMelee;
	pMissle->m_bByMissle		= m_bByMissle;
	
	//设置子弹的目标关系
	if (m_bTargetEnemy)
	{
		if (m_bTargetAlly)
			pMissle->m_eRelation = MissleRelation_All;
		else
			pMissle->m_eRelation = MissleRelation_EnemyOnly;
	}
	else
	{
		if (m_bTargetAlly)
		{
			pMissle->m_eRelation = MissleRelation_AllyOnly;
		}
		else
			pMissle->m_eRelation = MissleRelation_None;
	}
	
#ifndef _SERVER
	pMissle->m_MissleRes.m_bNeedShadow   = m_bNeedShadow;
	pMissle->m_MissleRes.m_nMaxShadowNum = m_nMaxShadowNum;
	pMissle->m_MissleRes.m_nMissleId	 = nMissleIndex;
	if (!pMissle->m_MissleRes.Init()) g_DebugLog("创建子弹贴图失败！！！%s", __FILE__) ;
	pMissle->DoWait();
#endif
	
	for (int i = 0  ; i < m_nMissleAttribsNum; i ++)
	{
		switch (m_MissleAttribs[i].nAttribType)
		{
		case magic_missle_movekind_v:
			{
				pMissle->m_eMoveKind	= (eMissleMoveKind) m_MissleAttribs[i].nValue[0];
			}break;
			
		case magic_missle_speed_v:	
			{
				pMissle->m_nSpeed		= m_MissleAttribs[i].nValue[0];
			}break;
			
		case magic_missle_lifetime_v:
			{
				pMissle->m_nLifeTime	= m_MissleAttribs[i].nValue[0];
			}break;
			
		case magic_missle_height_v:	
			{
				pMissle->m_nHeight		= m_MissleAttribs[i].nValue[0];
			}break;
			
		case magic_missle_damagerange_v:
			{
				pMissle->m_nDamageRange = m_MissleAttribs[i].nValue[0];
			}break;
			
		case magic_missle_radius_v:	
			{
			}break;
		}
	}
	
	if (!(pSkillParam->dwSkillParams & PARAM_LAUNCHER) && pSkillParam->tLauncher.tActorInfo.nActorType == Actor_Npc) 
	{
		if (m_bIsMelee)
			pMissle->m_nLifeTime = Npc[pSkillParam->tLauncher.tActorInfo.nActorIndex].ModifyMissleLifeTime(pMissle->m_nLifeTime);
		else
		{
			pMissle->m_nSpeed = Npc[pSkillParam->tLauncher.tActorInfo.nActorIndex].ModifyMissleSpeed(pMissle->m_nSpeed);
			pMissle->m_bCollideVanish = Npc[pSkillParam->tLauncher.tActorInfo.nActorIndex].ModifyMissleCollsion(pMissle->m_bCollideVanish);
		}
	}
	
}




/*!*****************************************************************************
// Function		: KMissleSkill::SetMissleGenerateTime
// Purpose		: 获得当前的子弹的实际产生时间
// Return		: void 
// Argumant		: Missle * pMissle
// Argumant		: int nNo
// Comments		:
// Author		: RomanDou
*****************************************************************************/
unsigned int KMissleSkill::GetMissleGenerateTime(int nNo)
{
	CheckAndGenSkillData();
	switch(m_eMisslesGenerateStyle)
	{
	case SKILL_MGS_NULL:
		{
			return m_nWaitTime;
		}break;
		
	case SKILL_MGS_SAMETIME:
		{
			return  m_nWaitTime + m_nMisslesGenerateData;
		}break;
		
	case SKILL_MGS_ORDER:		
		{
			return  m_nWaitTime + nNo * m_nMisslesGenerateData;
		}break;
		
	case SKILL_MGS_RANDONORDER:	
		{
			if (g_Random(2) == 1) 
				return m_nWaitTime + nNo * m_nMisslesGenerateData + g_Random(m_nMisslesGenerateData);
			else 
				return m_nWaitTime + nNo * m_nMisslesGenerateData  - g_Random(m_nMisslesGenerateData / 2);
		}break;
		
	case SKILL_MGS_RANDONSAME:	
		{
			return  m_nWaitTime + g_Random(m_nMisslesGenerateData);
		}break;
		
	case SKILL_MGS_CENTEREXTENDLINE:
		{
			if (m_nChildSkillNum <= 1) return m_nWaitTime;
			int nCenter = m_nChildSkillNum / 2	;
			return m_nWaitTime + abs(nNo - nCenter) * m_nMisslesGenerateData ;
		}
	}
	return m_nWaitTime;
}

BOOL	KMissleSkill::GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow)
{
	if (!pSkillsSettingFile || nRow < 0) return FALSE;
	pSkillsSettingFile->GetInteger(nRow, "ChildSkillNum",	0, &m_nChildSkillNum,TRUE);
	pSkillsSettingFile->GetInteger(nRow, "MisslesForm",		0, (int *)&m_eMisslesForm, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "CharAnimId",		0, (int *)&m_nCharActionId, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "IsPhysical",		0, &m_bIsPhysical, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "IsAura",			0, &m_bIsAura, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "IsUseAR",			0, &m_bUseAttackRate, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "BaseSkill",		0, &m_bBaseSkill, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "ByMissle",		0, &m_bByMissle, TRUE);
	
	pSkillsSettingFile->GetInteger(nRow, "FlyEvent",		0, &m_bFlyingEvent, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "StartEvent",		0, &m_bStartEvent, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "CollideEvent",	0, &m_bCollideEvent, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "VanishedEvent",	0, &m_bVanishedEvent, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "FlySkillId",		0, &m_nFlySkillId, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "StartSkillId",	0, &m_nStartSkillId, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "VanishedSkillId",	0, &m_nVanishedSkillId, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "CollidSkillId",	0, &m_nCollideSkillId, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "Param1",			0, &m_nValue1, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "Param2",			0, &m_nValue2, TRUE);
	
	pSkillsSettingFile->GetInteger(nRow, "EventSkillLevel", 0, &m_nEventSkillLevel, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "IsMelee",			0, &m_bIsMelee, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "FlyEventTime",	0, &m_nFlyEventTime, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "ClientSend",		0, &m_bClientSend, TRUE);

	pSkillsSettingFile->GetInteger(nRow, "MslsGenerate",	0, (int *)&m_eMisslesGenerateStyle, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "MslsGenerateData",0, &m_nMisslesGenerateData, TRUE);
	pSkillsSettingFile->GetInteger(nRow, "MaxShadowNum",	0, &m_nMaxShadowNum, TRUE);
#ifndef _SERVER
	pSkillsSettingFile->GetInteger(nRow, "NeedShadow",		0, &m_bNeedShadow, TRUE);
#endif
}


BOOL	KMissleSkill::ParseString2MagicAttrib(char * szMagicAttribName, char * szValue)
{
	int nValue1 = 0;
	int nValue2 = 0;
	int nValue3 = 0;
	if ((!szMagicAttribName) || (!szMagicAttribName[0])) return FALSE;
	
	//nValue2 当值为-1时为永久性状态，0为非状态，其它值为有时效性状态魔法效果
	//需要将状态数据与非状态数据分离出来，放入相应的数组内，并记录总数量
	for (int i  = 0 ; i <= magic_normal_end; i ++)
	{
		if (!strcmp(szMagicAttribName, MagicAttrib2String(i)))
		{
			sscanf(szValue, "%d,%d,%d", &nValue1, &nValue2, &nValue3);
			if (KFightSkill::ParseString2MagicAttrib((MAGIC_ATTRIB)i, nValue1, nValue2, nValue3)) continue;
			if (!ParseString2MagicAttrib((MAGIC_ATTRIB)i, nValue1, nValue2, nValue3))
				g_DebugLog("[Skill]Can Not Find Magic Attrib %d",i);
		}
	}
			
	return FALSE;
}


BOOL	KMissleSkill::ParseString2MagicAttrib(MAGIC_ATTRIB MagicAttrib, int nValue1, int nValue2, int nValue3)
{
	if (MagicAttrib > magic_missle_begin && MagicAttrib < magic_missle_end)
	{
		m_MissleAttribs[m_nMissleAttribsNum].nAttribType = MagicAttrib;
		m_MissleAttribs[m_nMissleAttribsNum].nValue[0] = nValue1;
		m_MissleAttribs[m_nMissleAttribsNum].nValue[1] = nValue2;
		m_MissleAttribs[m_nMissleAttribsNum].nValue[2] = nValue3;
		m_nMissleAttribsNum ++;
		return TRUE;
	}
	
	if (MagicAttrib > magic_skill_begin && MagicAttrib < magic_skill_end)
	{
		switch(MagicAttrib)
		{
		case magic_skill_misslenum_v:
			{
				m_nChildSkillNum = nValue1;
			}break;
			
		case magic_skill_misslesform_v:
			{
				m_eMisslesForm = (eMisslesForm) nValue1;
			}break;
		case magic_skill_param1_v:
			{
				m_nValue1 = nValue1;
			}
			break;
		case magic_skill_param2_v:	
			{
				m_nValue2 = nValue2;
			}break;
		case magic_skill_eventskilllevel:
			{
				m_nEventSkillLevel = nValue1;
			}
		}
		return FALSE;
	}
}

#ifndef _SERVER
void	KMissleSkill::GetDesc(char * pszMsg, int nOwnerIndex)
{
	if (!pszMsg) return;
	if (nOwnerIndex <= 0 )	return ;
	strcpy(pszMsg,"");
	char szTemp[100];
	strcat(pszMsg, m_szName);
	strcat(pszMsg, "\n");
	switch(m_eSkillStyle)
	{
	case SKILL_SS_Missles:
		{
			strcat(pszMsg, "子弹技\n");
			szTemp[0] = 0;
			switch(m_eMisslesForm)
			{
			case SKILL_MF_Wall:
				{
					sprintf(szTemp, "墙形 数量%d\n", m_nChildSkillNum);
				}break;			//墙形	多个子弹呈垂直方向排列，类式火墙状
				
			case SKILL_MF_Line:
				{
					sprintf(szTemp, "线形 数量%d\n", m_nChildSkillNum);
				}break;					//线形	多个子弹呈平行于玩家方向排列
			case SKILL_MF_Spread:
				{
					sprintf(szTemp, "散形 数量%d\n", m_nChildSkillNum);
				}break;				//散形	多个子弹呈一定的角度的发散状	
			case SKILL_MF_Circle:
				{
					sprintf(szTemp, "圆形发散 数量%d\n", m_nChildSkillNum);
				}break;				//圆形	多个子弹围成一个圈
			case SKILL_MF_Random:{}break;				//随机	多个子弹随机排放
			case SKILL_MF_Zone:
				{
					if (m_nValue1)
						sprintf(szTemp, "圆形区域 数量%d\n", m_nChildSkillNum);
					else 
						sprintf(szTemp, "方形区域 数量%d\n", m_nChildSkillNum);
					
				}break;					//区域	多个子弹放至在某个范围内
			case SKILL_MF_AtTarget:
				{
					if (m_nValue1)
						sprintf(szTemp, "定点圆形区域 数量%d\n", m_nChildSkillNum);
					else 
						sprintf(szTemp, "定点方形区域 数量%d\n", m_nChildSkillNum);
				}break;				//定点	多个子弹根据
			case SKILL_MF_AtFirer:
				{
					if (m_nValue1)
						sprintf(szTemp, "定点圆形区域 数量%d\n", m_nChildSkillNum);
					else 
						sprintf(szTemp, "定点方形区域 数量%d\n", m_nChildSkillNum);
					
				}break;				//本身	多个子弹停在玩家当前位置
			}
			strcat(pszMsg, szTemp);

		}break;			//	子弹类		本技能用于发送子弹类
		
	case SKILL_SS_Melee:
		{
			strcat(pszMsg, "格斗技\n");
		}break;
	case SKILL_SS_InitiativeNpcState:
		{
			strcat(pszMsg, "主动辅助武功\n");			
			if (m_StateAttribs[0].nValue[1] > 0)
			{
				sprintf (szTemp, "状态持续时间:%d\n" ,m_StateAttribs[0].nValue[1]);
				strcat(pszMsg,szTemp);
			}
		}break;	//	主动类		本技能用于改变当前Npc的主动状态
	case SKILL_SS_PassivityNpcState:
		{
			strcat(pszMsg, "被动辅助武功\n");
		}break;		//	被动类		本技能用于改变Npc的被动状态

		
	case SKILL_SS_CreateNpc:{}break;				//	产生Npc类	本技能用于生成一个新的Npc
	case SKILL_SS_BuildPoison:{}break;			//	炼毒类		本技能用于炼毒
	case SKILL_SS_AddPoison:{}break;				//	加毒类		本技能用于给武器加毒性
	case SKILL_SS_GetObjDirectly:{}break;		//	取物类		本技能用于隔空取物
	case SKILL_SS_StrideObstacle :{}break;		//	跨越类		本技能用于跨越障碍
	case SKILL_SS_BodyToObject:{}break;			//	变物类		本技能用于将尸体变成宝箱
	case SKILL_SS_Mining:{}break;				//	采矿类		本技能用于采矿随机生成矿石
	case SKILL_SS_RepairWeapon:{}break;			//	修复类		本技能用于修复装备
	case SKILL_SS_Capture:{}break;				//	捕捉类		本技能用于捕捉动物Npc
	}
	if (g_MisslesLib[m_nChildSkillId].m_bRangeDamage) strcat(pszMsg, "区域伤害 ");
	
	switch(g_MisslesLib[m_nChildSkillId].m_eMoveKind)
	{
	case MISSLE_MMK_Stand:
		{
		strcat(pszMsg, "原地 ");
		}break;							//	原地
	case MISSLE_MMK_Line:
		{
		strcat(pszMsg, "直线飞行 ");
		}break;							//	直线飞行
	case MISSLE_MMK_Random:{}break;							//	随机飞行（暗黑二女巫的Charged Bolt）
	case MISSLE_MMK_Circle:
		{
			strcat(pszMsg, "环形飞行 ");
		}break;							//	环行飞行（围绕在身边，暗黑二刺客的集气）
	case MISSLE_MMK_Helix:
		{
			strcat(pszMsg, "环形飞行 ");
		}break;							//	阿基米德螺旋线（暗黑二游侠的Bless Hammer）
	case MISSLE_MMK_Follow:{}break;							//	跟踪目标飞行
	case MISSLE_MMK_Motion:{}break;							//	玩家动作类
	case MISSLE_MMK_Parabola:
		{
			strcat(pszMsg, "抛物飞行 ");
		}break;						//	抛物线
	case MISSLE_MMK_SingleLine:{}break;						//	必中的单一直线飞行魔法
	}
	
	if (!g_MisslesLib[m_nChildSkillId].m_bCollideVanish)
		strcat(pszMsg, "穿透 ");
	if (g_MisslesLib[m_nChildSkillId].m_nDamageRange > 1) 
	{
		sprintf(szTemp, "伤害范围:%d ", g_MisslesLib[m_nChildSkillId].m_nDamageRange);
		strcat(pszMsg, szTemp);
	}
	if (g_MisslesLib[m_nChildSkillId].m_nKnockBack)
	{
		sprintf(szTemp, "震退距离:%d ", g_MisslesLib[m_nChildSkillId].m_nKnockBack);
		strcat(pszMsg, szTemp);
	}
	if (g_MisslesLib[m_nChildSkillId].m_bAutoExplode)
	{
		strcat(pszMsg, "消亡自爆 ");
	}

	if (m_bIsAura) strcat(pszMsg, "光环 ");
	if (this->m_bIsPhysical) strcat(pszMsg, "物理 ");
	if (this->m_bIsMelee) strcat(pszMsg, "近身 ");
	if (this->m_bTargetOnly) strcat(pszMsg, "必中 ");
	if (this->m_bTargetAlly) strcat(pszMsg, "对友 ");
	if (this->m_bTargetEnemy) strcat(pszMsg, "对敌 ");
	if (this->m_bTargetObj)	  strcat(pszMsg, "对物 ");
	if (this->m_bTargetSelf) strcat(pszMsg, "对已 ");
	if (this->m_bUseAttackRate) strcat(pszMsg, "考虑命中率 ");
	
	strcat (pszMsg, "\n");
	
	strcat(pszMsg, m_szSkillDesc);
	strcat(pszMsg, "\n");
	sprintf(szTemp, "当前等级:%d", m_nLevel);
	strcat(pszMsg, szTemp);
	strcat(pszMsg, "\n");
	int i = 0;

	switch(m_nSkillCostType)
	{
	case attrib_mana:
		sprintf(szTemp, "内力消耗:%d\n", GetSkillCost());
		strcat(pszMsg,szTemp);
		break;
	case attrib_stamina:
		sprintf(szTemp, "体力消耗:%d\n", GetSkillCost());
		strcat(pszMsg,szTemp);
		break;
	case attrib_life:
		sprintf(szTemp, "生命消耗:%d\n", GetSkillCost());
		strcat(pszMsg,szTemp);
		break;
	}
	
	sprintf(szTemp,"有效距离:%d\n", GetAttackRadius());
	strcat(pszMsg,szTemp);

	
	//不随等级变化的立即伤害
	for (i  = 0; i < m_nImmediateAttribsNum; i ++)
	{
		if (!m_ImmediateAttribs[i].nAttribType) continue;
		char * pszInfo = (char *)g_MagicDesc.GetDesc(&m_ImmediateAttribs[i]);
		if (!pszInfo) continue;
		strcat(pszMsg, pszInfo);
		strcat(pszMsg, "\n");
	}
	//子弹随玩家属性计算而成的伤害
	//KMagicAttrib *DamageAttribs[MAX_MISSLE_DAMAGEATTRIB];
	KMagicAttrib *DamageAttribs = m_DamageAttribs;
	//根据玩家的基本属性，确定子弹的伤害
	//Npc[nOwnerIndex].AppendSkillEffect(m_DamageAttribs, DamageAttribs);
	for (i  = 0; i < MAX_MISSLE_DAMAGEATTRIB; i ++)
	{
		if (!(DamageAttribs + i)->nAttribType) continue;
		char * pszInfo = (char *)g_MagicDesc.GetDesc((DamageAttribs + i));
		if (!pszInfo) continue;
		strcat(pszMsg, pszInfo);
		strcat(pszMsg, "\n");
	}
	//状态技能效果
	for (i  = 0; i < m_nStateAttribsNum; i ++)
	{
		if (!m_StateAttribs[i].nAttribType) continue;
		char * pszInfo = (char *)g_MagicDesc.GetDesc(&m_StateAttribs[i]);
		if (!pszInfo) continue;
		strcat(pszMsg, pszInfo);
		strcat(pszMsg, "\n");
	}

}
#endif