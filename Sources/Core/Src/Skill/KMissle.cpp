/*******************************************************************************
// FileName			:	KMissle.cpp
// FileAuthor		:	RomanDou
// FileCreateDate	:	2002-6-10 15:32:22
// FileDescription	:	
// Revision Count	:	
*******************************************************************************/

#include "KCore.h"
#include "Skill.h"
#include "KMissle.h"
#include "KSubWorld.h"
#include "KSubWorldSet.h"
#include "KRegion.h"
#include "KNpc.h"
#include "KNpcSet.h"
#include "KMath.h"
#include <math.h>
#include "KSkillSpecial.h"
#include "myassert.h"
#ifndef _SERVER
#include "../../Represent/iRepresent/iRepresentshell.h"
#include "Scene\KScenePlaceC.h"
#include "ImgRef.h"
#endif
#include "Scene\ObstacleDef.h"


KMissle g_MisslesLib[MAX_MISSLESTYLE];

//每个格子的像素长宽
#define CellWidth		(SubWorld[m_nSubWorldId].m_nCellWidth << 10)
#define CellHeight		(SubWorld[m_nSubWorldId].m_nCellHeight << 10)

//每个region格点长宽
#define RegionWidth		(SubWorld[m_nSubWorldId].m_nRegionWidth)
#define RegionHeight	(SubWorld[m_nSubWorldId].m_nRegionHeight)

#define CurRegion		SubWorld[m_nSubWorldId].m_Region[m_nRegionId]
#define CurSubWorld		SubWorld[m_nSubWorldId]

#define LeftRegion		SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[2]
#define RightRegion		SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[6]
#define UpRegion		SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[4]
#define DownRegion		SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[0]
#define LeftUpRegion	SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[3]
#define LeftDownRegion	SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[1]
#define RightUpRegion	SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[5]
#define RightDownRegion	SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_nConnectRegion[7]

//随机移动魔法的左右偏移表
int g_nRandMissleTab[100] = {0	};

CORE_API KMissle Missle[MAX_MISSLE];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
KMissle::KMissle()
{
	m_nMissleId = -1;
	m_nCollideOrVanishTime = 0;
	
#ifdef _SERVER
	m_pMagicAttribsData = NULL;
#else
	m_bFollowNpcWhenCollid = 1;
	m_bRemoving	= FALSE;
#endif
}

void KMissle::Release()
{
#pragma	message(ATTENTION("子弹消亡时，需更新发送者使用该技能时的当前使用次数，使之减一"))
#ifndef _SERVER	
	g_ScenePlace.RemoveObject(CGOG_MISSLE, m_nMissleId, m_SceneID);
	m_MissleRes.Clear();
	m_nMissleId = -1;
#endif
#ifdef _SERVER
	if (m_pMagicAttribsData)
		if (m_pMagicAttribsData->DelRef() == 0)
			delete m_pMagicAttribsData;
		m_pMagicAttribsData = NULL;
#endif
}

KMissle::~KMissle()
{
	
}
/*!*****************************************************************************
// Function		: KMissle::GetInfoFromTabFile
// Purpose		: 获得TabFile有关子弹的基本信息
// Return		: BOOL 
// Argumant		: int nMissleId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL KMissle::GetInfoFromTabFile(int nMissleId)
{
	if (nMissleId <= 0 ) return FALSE;
	KITabFile * pITabFile = &g_MisslesSetting;
	return GetInfoFromTabFile(pITabFile, nMissleId);
}

BOOL KMissle::GetInfoFromTabFile(KITabFile * pMisslesSetting, int nMissleId)
{
	if (nMissleId <= 0 ) return FALSE;
	m_nMissleId		= nMissleId;
	int nRow = nMissleId;
	
	pMisslesSetting->GetString(nRow, "MissleName",		   "", m_szMissleName,30, TRUE);
	
	int nHeightOld ;
	pMisslesSetting->GetInteger(nRow, "MissleHeight",		0, &nHeightOld, TRUE);
	m_nHeight = nHeightOld << 10;
	
	pMisslesSetting->GetInteger(nRow, "LifeTime",			0, &m_nLifeTime, TRUE);
	pMisslesSetting->GetInteger(nRow, "Speed",				0, &m_nSpeed, TRUE);
	pMisslesSetting->GetInteger(nRow, "ResponseSkill",		0, &m_nSkillId, TRUE);
	pMisslesSetting->GetInteger(nRow, "CollidRange",		0, &m_nCollideRange, TRUE);
	pMisslesSetting->GetInteger(nRow, "ColVanish",			0, &m_bCollideVanish, TRUE);
	pMisslesSetting->GetInteger(nRow, "CanColFriend",		0, &m_bCollideFriend, TRUE);
	pMisslesSetting->GetInteger(nRow, "CanSlow",			0, &m_bCanSlow, TRUE);
	pMisslesSetting->GetInteger(nRow, "IsRangeDmg",		0, &m_bRangeDamage, TRUE);
	pMisslesSetting->GetInteger(nRow, "DmgRange",			0, &m_nDamageRange, TRUE);
	pMisslesSetting->GetInteger(nRow, "MoveKind",			0, (int*)&m_eMoveKind, TRUE);
	pMisslesSetting->GetInteger(nRow, "FollowKind",		0, (int*)&m_eFollowKind, TRUE);
	pMisslesSetting->GetInteger(nRow, "Zacc",				0,(int*)&m_nZAcceleration, TRUE);
	pMisslesSetting->GetInteger(nRow, "Zspeed",				0,(int*)&m_nHeightSpeed, TRUE);
	pMisslesSetting->GetInteger(nRow, "Param1",			0, &m_nParam1, TRUE);
	pMisslesSetting->GetInteger(nRow, "Param2",			0, &m_nParam2, TRUE);
	pMisslesSetting->GetInteger(nRow, "Param3",			0, &m_nParam3, TRUE);
	pMisslesSetting->GetInteger(nRow, "AutoExplode",	0, (int*)&m_bAutoExplode, TRUE);
	
#ifndef _SERVER	
	char AnimFileCol[64];
	char SndFileCol[64];
	char AnimFileInfoCol[100];
	char szAnimFileInfo[100];
	pMisslesSetting->GetInteger(nRow, "MultiShow",		0, &m_bMultiShow, TRUE);
	for (int i  = 0; i < MAX_MISSLE_STATUS; i++)
	{
		sprintf(AnimFileCol, "AnimFile%d", i + 1);
		sprintf(SndFileCol,  "SndFile%d", i + 1);
		sprintf(AnimFileInfoCol, "AnimFileInfo%d", i + 1);
		
		pMisslesSetting->GetString(nRow, AnimFileCol,			"", m_MissleRes.m_MissleRes[i].AnimFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, SndFileCol,			"", m_MissleRes.m_MissleRes[i].SndFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, AnimFileInfoCol,		"", szAnimFileInfo, 100, TRUE);
		m_MissleRes.m_MissleRes[i].nInterval = 1;
		m_MissleRes.m_MissleRes[i].nDir = 16;
		m_MissleRes.m_MissleRes[i].nTotalFrame = 100;
		sscanf(szAnimFileInfo, "%d,%d,%d", 
			&m_MissleRes.m_MissleRes[i].nTotalFrame,
			&m_MissleRes.m_MissleRes[i].nDir,
			&m_MissleRes.m_MissleRes[i].nInterval);
		
		sprintf(AnimFileCol, "AnimFileB%d", i + 1);
		sprintf(SndFileCol,  "SndFileB%d", i + 1);
		sprintf(AnimFileInfoCol, "AnimFileInfoB%d", i + 1);
		
		pMisslesSetting->GetString(nRow, AnimFileCol,			"", m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].AnimFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, SndFileCol,			"", m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].SndFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, AnimFileInfoCol,		"", szAnimFileInfo, 100, TRUE);
		
		m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nInterval = 1;
		m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nDir = 16;
		m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nTotalFrame = 100;
		
		sscanf(szAnimFileInfo, "%d,%d,%d", 
			&m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nTotalFrame,
			&m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nDir,
			&m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nInterval);
		
	}
	pMisslesSetting->GetInteger(nRow, "LoopPlay",			0, &m_MissleRes.m_bLoopAnim, TRUE);
	pMisslesSetting->GetInteger(nRow, "SubLoop",		0, &m_MissleRes.m_bSubLoop, TRUE);
	pMisslesSetting->GetInteger(nRow, "SubStart",		0, &m_MissleRes.m_nSubStart, TRUE);
	pMisslesSetting->GetInteger(nRow, "SubStop",		0, &m_MissleRes.m_nSubStop, TRUE);
	pMisslesSetting->GetInteger(nRow, "ColFollowTarget",0, (int *)&m_bFollowNpcWhenCollid, TRUE);
#endif
	return TRUE;
}

BOOL KMissle::Init( int nLauncher, int nMissleId, int nXFactor, int nYFactor, int nLevel)
{
#ifndef _SERVER
	m_MissleRes.Init();
#endif
	return	TRUE;
}


/*!*****************************************************************************
// Function		: KMissle::Activate
// Purpose		: 
// Return		: void 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
int KMissle::Activate()
{	
	if (m_nMissleId <= 0)
		return  0 ;
	
	eMissleStatus eLastStatus = m_eMissleStatus;
	if (m_eMissleStatus != MS_DoVanish &&	m_eMissleStatus != MS_DoCollision)
		if (m_nCurrentLife >= m_nLifeTime)
		{
			if (m_bAutoExplode)
			{
				ProcessCollision();//处理碰撞
			}
			DoVanish();
#ifdef _SERVER
			m_nCurrentLife ++;
			return 1;
#endif
		}
		
		if (m_nCurrentLife == m_nStartLifeTime)	
			DoFly();
		
		switch(m_eMissleStatus)
		{
		case MS_DoWait:
			OnWait();
			break;
		case MS_DoFly:
			OnFly();
			if (m_bFlyEvent)
				if ( (m_nCurrentLife - m_nStartLifeTime) % m_nFlyEventTime == 0 )
				{
					_ASSERT(m_nSkillId < MAX_SKILL && m_nLevel < MAX_SKILLLEVEL);
					g_GetSkill(m_nSkillId,m_nLevel)->OnEvent(Event_Fly, m_nMissleId, 0);
					
				}
				break;
		case MS_DoCollision:
			OnCollision();
			break;
		case MS_DoVanish:
			OnVanish();
			break;
		}
		
#ifndef _SERVER
		//子弹未消亡掉
		if (m_nMissleId > 0)
		{
			int nSrcX;
			int nSrcY;
			SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
			g_ScenePlace.MoveObject(CGOG_MISSLE, m_nMissleId, nSrcX, nSrcY, m_nCurrentMapZ, m_SceneID);
		}
		
#endif
		m_nCurrentLife ++;
		return 1;
}

/*!*****************************************************************************
// Function		: KMissle::OnWait
// Purpose		: 
// Return		: void 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissle::OnWait()
{
	return;
}
/*!*****************************************************************************
// Function		: KMissle::OnCollision
// Purpose		: 
// Return		: void 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissle::OnCollision()
{
	return;	
}



BOOL KMissle::CheckCollision(int nBarrierKind)
{
	int nLauncher = 0;
	if (m_SkillParam.dwSkillParams & PARAM_LAUNCHER)
	{
		//now , only imp the launcher is npc,so...
		_ASSERT(m_SkillParam.tLauncher.tActorInfo.nActorType == Actor_Npc);
		nLauncher = m_SkillParam.tLauncher.tActorInfo.nActorIndex;
	}
	else return FALSE;

#ifdef TOOLVERSION
	return FALSE;
#endif
	if (m_nCurrentMapZ < 0) 
	{
		DoVanish(); 
		return TRUE;
	}
	
	if (m_nCurrentMapZ > 20) return FALSE;
	if (m_nRegionId < 0) 
	{
		DoVanish();
		return TRUE;
	}
	KIndexNode *pNode = NULL;
	pNode = (KIndexNode *)SubWorld[m_nSubWorldId].m_Region[m_nRegionId].m_NpcList.GetHead();
	int nAbsX = 0;
	int nAbsY = 0;
	while(pNode)
	{
		if (this->m_nCollideRange == 1)
		{
			if ((nAbsX = abs(Npc[pNode->m_nIndex].m_MapX - m_nCurrentMapX)) <= 1 && (nAbsY = abs(Npc[pNode->m_nIndex].m_MapY - m_nCurrentMapY)) <= 1 \
				&& pNode->m_nIndex != nLauncher)
			{
				BOOL bCollision = FALSE;
				
				if (nAbsX || nAbsY) 
				{
					//(H - dy2 + dy1)**2 + (W - dx1 + dx2)**2 <= H**2 + W**2 ==>>
					//2(dy1-dy2)H + (dy1-dy2)**2 + 2(dx2-dx1)W + (dx2-dx1)**2 >=0
					int nDY = Npc[pNode->m_nIndex].m_OffY - m_nYOffset;
					int	nDX = m_nXOffset - Npc[pNode->m_nIndex].m_OffX;
					int itest = 2 * nDY * CellHeight + nDY * nDY + 2 * nDX * CellWidth + nDX * nDX;
					if (2 * nDY * CellHeight + nDY * nDY + 2 * nDX * CellWidth + nDX * nDX >= 0)	
						bCollision = TRUE;
				}
				else
					bCollision = TRUE;//同一点，则碰撞!
				
				if (bCollision && IsRelationSuitable(NpcSet.GetRelation(pNode->m_nIndex, nLauncher) , m_eRelation))
				{
					if (m_nDamageRange == 1)//
						ProcessCollision(nLauncher, m_nRegionId, Npc[pNode->m_nIndex].m_MapX, Npc[pNode->m_nIndex].m_MapY, m_nDamageRange , m_eRelation);
					else
						ProcessCollision();//处理碰撞
					DoCollision();//子弹作碰撞后的效果
					return TRUE;
				}
			}
		}
		else
		{
			if ((nAbsX = abs(Npc[pNode->m_nIndex].m_MapX - m_nCurrentMapX)) <= m_nCollideRange && (nAbsY = abs(Npc[pNode->m_nIndex].m_MapY - m_nCurrentMapY)) <= m_nCollideRange \
				&& pNode->m_nIndex != nLauncher)
			{
				if (IsRelationSuitable(NpcSet.GetRelation(pNode->m_nIndex, nLauncher) , m_eRelation))
				{
					ProcessCollision();//处理碰撞
					DoCollision();//子弹作碰撞后的效果
					return TRUE;
				}
			}
			
		}
		pNode = (KIndexNode*) pNode->GetNext();
	}
	return FALSE;
}

inline DWORD	KMissle::GetCurrentSubWorldTime()
{
	return SubWorld[m_nSubWorldId].m_dwCurrentTime;
}

void KMissle::OnFly()
{
	switch(this->m_eMoveKind)
	{
	case	MISSLE_MMK_Stand:							//	原地
		{
			//高度小于等于该障碍后，表示已碰撞
			/*
			如果子弹为碰撞后直接消亡的类型，
			则直接调用消失函数，否则调用碰撞魔法
			*/
			
			if (m_nZAcceleration)
			{
				m_nHeight += m_nHeightSpeed;
				if (!m_nHeight) m_nHeight = 0;
				m_nHeightSpeed -= m_nZAcceleration;
				m_nCurrentMapZ = m_nHeight >> 10;
			}
			
			int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
			if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
			{
				DoVanish();
				return;
			}
			CheckCollision(nBarrierKind);
		}
		break;
	case	MISSLE_MMK_Parabola:						//	抛物线
	case	MISSLE_MMK_Line:							//	直线飞行
		{
			int x = m_nXOffset;
			int y = m_nYOffset;
			int dx				= (m_nSpeed * m_nXFactor);
			int dy				= (m_nSpeed * m_nYFactor);
			
			if (m_nZAcceleration)
			{
				m_nHeight += m_nHeightSpeed;
				if (!m_nHeight) m_nHeight = 0;
				m_nHeightSpeed -= m_nZAcceleration;
				m_nCurrentMapZ = m_nHeight >> 10;
			}
			
			int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
			if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
			{
				DoVanish();
				return;
			}
			m_nXOffset			+=  dx;//* m_nCurrentLife;
			m_nYOffset			+=  dy;//* m_nCurrentLife;
			CheckBeyondRegion();
			CheckCollision(nBarrierKind);
			g_DebugLog("r%d x%d, y%d, dx%d, dy%d", m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset);
		}
		break;
		//按照设计方案，随机飞行无法达到客服两端的同步
	case	MISSLE_MMK_Random:							//	随机飞行（暗黑二女巫的Charged Bolt）
		{
			
		}break;
		//参数一表示顺时针还是逆时针转动
		//参数二表示固定原心还是围饶发动者
		//dx = SinA * R
		//dy = Ctg(90-A/2).R = SinA*SinA / (1 + CosA) * R
	case	MISSLE_MMK_Circle:							//	环行飞行（围绕在身边，暗黑二刺客的集气）
		{
			int nPreAngle = m_nAngle - 1;
			if (nPreAngle < 0) nPreAngle = MaxMissleDir -1;
			m_nDir = m_nAngle + (MaxMissleDir / 4);
			if (m_nDir >= MaxMissleDir) m_nDir = m_nDir - MaxMissleDir;
			int dx = (m_nSpeed + 50)  * (g_DirCos(m_nAngle,MaxMissleDir) - g_DirCos(nPreAngle,MaxMissleDir)) ;
			int dy = (m_nSpeed + 50)  * (g_DirSin(m_nAngle,MaxMissleDir) - g_DirSin(nPreAngle, MaxMissleDir)) ; 
			int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
			if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
			{
				DoVanish();
				return;
			}
			if (m_nParam2) //原地转
			{
				m_nXOffset += dx;
				m_nYOffset += dy;
			}
			else			// 围绕着发送者转
			{

				if (!Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].IsMatch(m_SkillParam.tLauncher.tActorInfo.dwActorID))
				{
					DoVanish();
					return ;
				}
				m_nCurrentMapX	= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapX;
				m_nCurrentMapY	= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapY;
				m_nXOffset		= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapX + dx;
				m_nYOffset		= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapY + dy;
			}
			
			if (m_nZAcceleration)
			{
				m_nHeight += m_nHeightSpeed;
				if (!m_nHeight) m_nHeight = 0;
				m_nHeightSpeed -= m_nZAcceleration;
				m_nCurrentMapZ = m_nHeight >> 10;
			}
			
			CheckBeyondRegion();
			CheckCollision(nBarrierKind);
			if (m_nParam1)
			{
				m_nAngle ++;
				if (m_nAngle >= MaxMissleDir)
					m_nAngle = 0;
			}
			else
			{
				m_nAngle --;
				if (m_nAngle < 0 )
					m_nAngle = MaxMissleDir - 1;
			}
		}
		break;
		
		//参数一表示顺时针还是逆时针转动
		//参数二表示固定原心还是围饶发动者
	case	MISSLE_MMK_Helix:							//	阿基米德螺旋线（暗黑二游侠的Bless Hammer）
		{
			
			int nPreAngle = m_nAngle - 1;
			if (nPreAngle < 0) nPreAngle = MaxMissleDir -1;
			m_nDir = m_nAngle + (MaxMissleDir / 4);
			if (m_nDir >= MaxMissleDir) m_nDir = m_nDir - MaxMissleDir;
			
			int dx = (m_nSpeed + m_nCurrentLife + 50)  * (g_DirCos(m_nAngle,MaxMissleDir) - g_DirCos(nPreAngle, MaxMissleDir)) ;
			int dy = (m_nSpeed + m_nCurrentLife + 50)  * (g_DirSin(m_nAngle,MaxMissleDir) - g_DirSin(nPreAngle,MaxMissleDir)) ; 
			
			int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
			
			if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
			{
				DoVanish();
				return;
			}
			if (m_nParam2) //原地转
			{
				m_nXOffset += dx;
				m_nYOffset += dy;
			}
			else			// 围绕着发送者转
			{
				if (!Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].IsMatch(m_SkillParam.tLauncher.tActorInfo.dwActorID))
				{
					DoVanish();
					return ;
				}
				m_nCurrentMapX	= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapX;
				m_nCurrentMapY	= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapY;
				m_nXOffset		= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapX + dx;
				m_nYOffset		= Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].m_MapY + dy;
			}
			
			if (m_nZAcceleration)
			{
				m_nHeight += m_nHeightSpeed;
				if (!m_nHeight) m_nHeight = 0;
				m_nHeightSpeed -= m_nZAcceleration;
				m_nCurrentMapZ = m_nHeight >> 10;
			}
			
			CheckBeyondRegion();
			CheckCollision(nBarrierKind);
			if (m_nParam1)
			{
				m_nAngle ++;
				if (m_nAngle >= MaxMissleDir)
					m_nAngle = 0;
			}
			else
			{
				m_nAngle --;
				if (m_nAngle < 0 )
					m_nAngle = MaxMissleDir - 1;
			}
		}
		break; 
	case	MISSLE_MMK_Follow:							//	跟踪目标飞行
		{
			/*int nDistance = 0;
			int nSrcMpsX = 0;
			int nSrcMpsY = 0;
			int nDesMpsX = 0;
			int nDesMpsY = 0;
			
			SubWorld[m_nSubWorldId].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, &nSrcMpsX, &nSrcMpsY);
			SubWorld[m_nSubWorldId].Map2Mps(Npc[m_nFollowNpcIdx].m_RegionIndex, Npc[m_nFollowNpcIdx].m_MapX, Npc[m_nFollowNpcIdx].m_MapY, Npc[m_nFollowNpcIdx].m_OffX, Npc[m_nFollowNpcIdx].m_OffY, &nDesMpsX, &nDesMpsY);
			nDistance = SubWorld[m_nSubWorldId].GetDistance(nSrcMpsX, nSrcMpsY, nDesMpsX, nDesMpsY);
			int nXFactor = ((nDesMpsX - nSrcMpsY ) <<10) / nDistance;
			int nYFactor = ((nDesMpsY - nSrcMpsY ) <<10) / nDistance;
			int dx				= nXFactor * m_nSpeed;
			int dy				= nYFactor * m_nSpeed;	
			if (m_nZAcceleration)
			{
				m_nHeight += m_nHeightSpeed;
				if (!m_nHeight) m_nHeight = 0;
				m_nHeightSpeed -= m_nZAcceleration;
				m_nCurrentMapZ = m_nHeight >> 10;
			}
			int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
			if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
			{
				DoVanish();
				return;
			}
			m_nXOffset			+= dx;
			m_nYOffset			+= dy;
			CheckBeyondRegion();
			CheckCollision(nBarrierKind);*/
		}break;
		
	case	MISSLE_MMK_Motion:							//	玩家动作类
		{
			
		}break;
		
		//nParam1 = 起始时Z轴上升的速度
		//nParam2 = 向下的加速度
		/*case	MISSLE_MMK_Parabola:						//	抛物线
		{
		int x	= m_nXOffset;
		int y	= m_nYOffset;
		int dx	= (m_nSpeed * g_DirCos(m_nDirIndex,MaxMissleDir))  ;//* m_nCurrentLife;
		int dy	= (m_nSpeed * g_DirSin(m_nDirIndex,MaxMissleDir)) ;//* m_nCurrentLife;
		//	m_nCurrentMapZ		+= m_nHeightSpeed / 1000;
		m_nHeightSpeed		-= 1000;
		//dy -= m_nHeight / 1000;
		//m_nHeight			+= m_nHeightSpeed;
		int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
		if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
		{
		DoVanish();
		return;
		}
		m_nXOffset			+= dx;
		m_nYOffset			+= dy;
		int nYShowOffset	= m_nYOffset  - m_nCurrentMapZ / 1000;
		int nShowDirIndex	= g_GetDirIndex(x,y, m_nXOffset, m_nYOffset);
		
		  if (nShowDirIndex != -1)
		  m_nDir = g_DirIndex2Dir(nShowDirIndex, MaxMissleDir);
		  
			CheckBeyondRegion();
			CheckCollision(nBarrierKind);
			
			  }break;
		*/	
	case MISSLE_MMK_SingleLine:						//	必中的单一直线飞行魔法
		{
			//单一必中类子弹，类式于传奇以及其它的同类网络游戏中的基本直线魔法			
#ifdef _SERVER
			
#else
			int x = m_nXOffset;
			int y = m_nYOffset;
			int dx = (m_nSpeed * m_nXFactor);
			int dy = (m_nSpeed * m_nYFactor);
			
			int nBarrierKind = SubWorld[m_nSubWorldId].TestBarrier(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, 0, 0);
			if (nBarrierKind == Obstacle_Normal || nBarrierKind == Obstacle_Jump)
			{
				DoVanish();
				return;
			}
			m_nXOffset			+=  dx;//* m_nCurrentLife;
			m_nYOffset			+=  dy;//* m_nCurrentLife;
			CheckBeyondRegion();
			CheckCollision(nBarrierKind);
#endif
		}
	}
}
/*!*****************************************************************************
// Function		: KMissle::OnVanish
// Purpose		: 
// Return		: void 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissle::OnVanish()
{
	
}

#ifndef _SERVER
void KMissle::Paint()
{
	if (m_nMissleId <= 0 ) return;
	int nSrcX;
	int nSrcY;
	SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
	
	if (!m_nZAcceleration)
		m_MissleRes.Draw(m_eMissleStatus, nSrcX, nSrcY, m_nCurrentMapZ, m_nDir,m_nLifeTime - m_nStartLifeTime,  m_nCurrentLife - m_nStartLifeTime );
	else
	{
		
		//int nLen = (int)sqrt(m_nXFactor* m_nXFactor + m_nYFactor * m_nYFactor);
		int nDirIndex = g_GetDirIndex(0,0,m_nXFactor, m_nYFactor);
		int nDir = g_DirIndex2Dir(nDirIndex, 64);
		
		m_MissleRes.Draw(m_eMissleStatus, nSrcX, nSrcY, m_nCurrentMapZ, nDir,m_nLifeTime - m_nStartLifeTime,  m_nCurrentLife - m_nStartLifeTime );
	}
	
	g_DebugLog("Missle[%d],%d,%d, %d",m_nMissleId, nSrcX, nSrcY, m_nRegionId);
	//对于客户端，直到子弹及其产生的效果全部播放完才终止并删除掉!
	if (m_MissleRes.m_bHaveEnd && (!m_MissleRes.m_SkillSpecialList.GetHead()))
		SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_DEL, m_nMissleId);
}
#endif

BOOL	KMissle::Save()
{
	return TRUE;
}

BOOL	KMissle::CheckBeyondRegion()
{
	int nOldRegion		= m_nRegionId;
	int nOldMapX		= m_nCurrentMapX;
	int nOldMapY		= m_nCurrentMapY;
	int nOldOffX		= m_nXOffset;
	int nOldOffY		= m_nYOffset;
	
	//	处理NPC的坐标变幻
	//	CELLWIDTH、CELLHEIGHT、OffX、OffY均是放大了1024倍
	CurRegion.DecRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
	if (m_nXOffset < 0)
	{
		m_nCurrentMapX--;
		m_nXOffset += CellWidth;
	}
	else if (m_nXOffset > CellWidth)
	{
		m_nCurrentMapX++;
		m_nXOffset -= CellWidth;
	}
	
	if (m_nYOffset < 0)
	{
		m_nCurrentMapY--;
		m_nYOffset += CellHeight;
	}
	else if (m_nYOffset > CellHeight)
	{
		m_nCurrentMapY++;
		m_nYOffset -= CellHeight;
	}
	
	if (m_nCurrentMapX < 0)
	{
		m_nRegionId = LeftRegion;
		m_nCurrentMapX += RegionWidth;
	}
	else if (m_nCurrentMapX >= RegionWidth)
	{
		m_nRegionId = RightRegion;
		m_nCurrentMapX -= RegionWidth;
	}
	if (m_nCurrentMapY < 0)
	{
		m_nRegionId = UpRegion;
		m_nCurrentMapY += RegionHeight;
	}
	else if (m_nCurrentMapY >= RegionHeight)
	{
		m_nRegionId = DownRegion;
		m_nCurrentMapY -= RegionHeight;
	}
	if (m_nRegionId < 0) 
	{
		m_nRegionId = nOldRegion;
		m_nCurrentMapX = nOldMapX;
		m_nCurrentMapY = nOldMapY;
		m_nXOffset = nOldOffX;
		m_nYOffset = nOldOffY;
		DoVanish();
		return TRUE; 
	}
	
	CurRegion.AddRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
	
	if (nOldRegion != m_nRegionId)
	{
		SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_CHANGE_REGION, nOldRegion, m_nRegionId, m_nMissleId);
	}
	return TRUE;
}

KMissle&	KMissle::operator=(KMissle& Missle)
{
	Missle.m_bCanSlow		=	m_bCanSlow;
	Missle.m_bCollideEvent	=	m_bCollideEvent;
	Missle.m_bCollideFriend =	m_bCollideFriend;
	Missle.m_bCollideVanish	=	m_bCollideVanish;
	Missle.m_bRangeDamage	=	m_bRangeDamage;
	Missle.m_eFollowKind	=	m_eFollowKind;
	Missle.m_eMoveKind		=	m_eMoveKind;
	Missle.m_nAction		=	m_nAction;
	Missle.m_nAngle			=	m_nAngle;
	Missle.m_nCollideRange	=	m_nCollideRange;
	Missle.m_nCurrentLife	=	0;
	Missle.m_nDamageRange	=	m_nDamageRange;
	Missle.m_nHeight		=	m_nHeight;
	Missle.m_nLifeTime		=	m_nLifeTime;
	Missle.m_nSpeed			=   m_nSpeed;
	Missle.m_nParam1		=	m_nParam1;
	Missle.m_nParam2		=	m_nParam2;
	Missle.m_nParam3		=	m_nParam3;
	Missle.m_nCurrentMapZ	=   m_nHeight >> 10;
	Missle.m_bFlyEvent		=	m_bFlyEvent;
	Missle.m_nFlyEventTime  =	m_nFlyEventTime;
	Missle.m_nZAcceleration =	m_nZAcceleration;
	Missle.m_nHeightSpeed	=	m_nHeightSpeed;
	
	strcpy(Missle.m_szMissleName	,	m_szMissleName);
	//	if (m_eMoveKind == MISSLE_MMK_Parabola)
	//	{
	//		Missle.m_nHeight = 0;
	//		Missle.m_nHeightSpeed	= m_nZAcceleration * m_nLifeTime / 2;
	//100000 / m_nLifeTime + m_nLifeTime / 2;
	//	}
#ifndef  _SERVER
	Missle.m_bMultiShow		=  m_bMultiShow;
	Missle.m_MissleRes.m_bLoopAnim = m_MissleRes.m_bLoopAnim;
	Missle.m_MissleRes.m_bHaveEnd = FALSE;
	int nOffset = 0;
	
	//如果是相同的子弹可以以不同方式显示时，则随机产生
	if (m_bMultiShow)		
	{
		if (g_Random(2) == 0)
		{
			nOffset = 0;
		}
		else
			nOffset = MAX_MISSLE_STATUS;
	}
	
	for (int t = 0; t < MAX_MISSLE_STATUS ; t++)
	{
		strcpy(Missle.m_MissleRes.m_MissleRes[t].AnimFileName,m_MissleRes.m_MissleRes[t + nOffset].AnimFileName);
		
		Missle.m_MissleRes.m_MissleRes[t].nTotalFrame = m_MissleRes.m_MissleRes[t + nOffset].nTotalFrame;
		Missle.m_MissleRes.m_MissleRes[t].nDir = m_MissleRes.m_MissleRes[t + nOffset].nDir;
		Missle.m_MissleRes.m_MissleRes[t].nInterval = m_MissleRes.m_MissleRes[t + nOffset].nInterval;
		
		strcpy(Missle.m_MissleRes.m_MissleRes[t].SndFileName,m_MissleRes.m_MissleRes[t + nOffset].SndFileName);
	}
	Missle.m_MissleRes.m_bSubLoop = m_MissleRes.m_bSubLoop;
	Missle.m_MissleRes.m_nSubStart = m_MissleRes.m_nSubStart;
	Missle.m_MissleRes.m_nSubStop = m_MissleRes.m_nSubStop;
#endif	
	
	return (Missle);
}

/*!*****************************************************************************
// Function		: KMissle::ProcessDamage
// Purpose		: 
// Return		: BOOL 
// Argumant		: int nNpcId
// Comments		:
// Author		: RomanDou
*****************************************************************************/
BOOL KMissle::ProcessDamage(int nNpcId)
{
#ifdef _SERVER
	if (Npc[m_SkillParam.tLauncher.tActorInfo.nActorIndex].IsMatch(m_SkillParam.tLauncher.tActorInfo.dwActorID))	
		if (m_pMagicAttribsData) 
		{
			//if (m_pMagicAttribsData->m_nDamageMagicAttribsNum > 0)
			Npc[nNpcId].ReceiveDamage(m_SkillParam.tLauncher.tActorInfo.nActorIndex, m_bIsMelee, m_pMagicAttribsData->m_pDamageMagicAttribs);
			
			if (m_pMagicAttribsData->m_nStateMagicAttribsNum > 0)
				Npc[nNpcId].SetStateSkillEffect(m_SkillParam.tLauncher.tActorInfo.nActorIndex, m_nSkillId, m_nLevel, m_pMagicAttribsData->m_pStateMagicAttribs, m_pMagicAttribsData->m_nStateMagicAttribsNum, m_pMagicAttribsData->m_pStateMagicAttribs[0].nValue[1]);
			
			if (m_pMagicAttribsData->m_nImmediateMagicAttribsNum > 0)
				Npc[nNpcId].SetImmediatelySkillEffect(m_SkillParam.tLauncher.tActorInfo.nActorIndex, m_pMagicAttribsData->m_pImmediateAttribs, m_pMagicAttribsData->m_nImmediateMagicAttribsNum);
			return TRUE;
		}
#endif //_SERVER
		return FALSE;
}

void KMissle::DoVanish()
{
	if (m_eMissleStatus == MS_DoVanish) return ;
#ifndef _SERVER
	m_MissleRes.m_bHaveEnd = TRUE;
	m_nCollideOrVanishTime = m_nCurrentLife;
#endif
	if (m_bVanishedEvent)	
	{
		_ASSERT(m_nSkillId < MAX_SKILL && m_nLevel < MAX_SKILLLEVEL);
		if (g_GenSkillData(m_nSkillId, m_nLevel))
			g_pSkill[m_nSkillId][m_nLevel]->OnEvent(Event_Vanish, m_nMissleId, 0);
	}
#ifdef _SERVER	//服务器端时子弹一旦进入消亡期则直接删除掉
	SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_DEL, m_nMissleId);
	return ;
#endif
	m_eMissleStatus = MS_DoVanish;
#ifndef _SERVER 
	if (m_nRegionId < 0)
	{
		m_bRemoving = TRUE;
		return ;
	}
	int nSrcX = 0 ;
	int nSrcY = 0 ;
	SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
	CreateSpecialEffect(MS_DoVanish, nSrcX, nSrcY, m_nCurrentMapZ);
#endif
}

void KMissle::DoCollision()
{
	if (m_eMissleStatus == MS_DoCollision) return;
	if (m_bCollideEvent)	
	{
		_ASSERT(m_nSkillId < MAX_SKILL && m_nLevel < MAX_SKILLLEVEL);
		if (g_GenSkillData(m_nSkillId, m_nLevel))
			g_pSkill[m_nSkillId][m_nLevel]->OnEvent(Event_Collision, m_nMissleId, 0);
	}
	if (m_bCollideVanish)
	{
#ifndef _SERVER
		m_MissleRes.m_bHaveEnd = TRUE;
#endif
		DoVanish();
	}
	else 
	{
#ifndef _SERVER		
		//增加撞后的效果	
		int nSrcX = 0 ;
		int nSrcY = 0 ;
		SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
		if (!m_MissleRes.m_SkillSpecialList.GetHead())
			CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ);
#endif
		m_eMissleStatus = MS_DoFly;
	}
}

void KMissle::DoFly()
{
	if (m_eMissleStatus == MS_DoFly) return ;
	//初始化贴图
	m_eMissleStatus = MS_DoFly;
}


/*!*****************************************************************************
// Function		: KMissle::ProcessCollision
// Purpose		: 
// Return		: int 
// Argumant		: int nLauncherIdx
// Argumant		: int nRegionId
// Argumant		: int nMapX
// Argumant		: int nMapY
// Argumant		: int nRange
// Argumant		: MISSLE_RELATION eRelation
// Comments		:
// Author		: RomanDou
*****************************************************************************/
int KMissle::ProcessCollision(int nLauncherIdx, int nRegionId, int nMapX, int nMapY, int nRange , MISSLE_RELATION eRelation)
{
#ifdef TOOLVERSION 
	return 0;
#endif
	if (nLauncherIdx <= 0 ) return 0;
	int nRangeX = nRange;
	int	nRangeY = nRange;
	int	nSubWorld = Npc[nLauncherIdx].m_SubWorldIndex;
	int	nRegion = nRegionId;
	int	nRet = 0;
	int	nRMx, nRMy, nSearchRegion;
	// 检查范围内的格子里的NPC
	for (int i = -nRangeX; i < nRangeX; i++)
	{
		for (int j = -nRangeY; j < nRangeY; j++)
		{
			// 去掉边角几个格子，保证视野是椭圆形
			if ((i * i + j * j ) > nRangeX * nRangeX)
				continue;
			
			// 确定目标格子实际的REGION和坐标确定
			nRMx = nMapX + i;
			nRMy = nMapY + j;
			nSearchRegion = nRegion;
			if (nRMx < 0)
			{
				nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[2];
				nRMx += SubWorld[nSubWorld].m_nRegionWidth;
			}
			else if (nRMx >= SubWorld[nSubWorld].m_nRegionWidth)
			{
				nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[6];
				nRMx -= SubWorld[nSubWorld].m_nRegionWidth;
			}
			if (nSearchRegion < 0) continue;
			if (nRMy < 0)
			{
				nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[4];
				nRMy += SubWorld[nSubWorld].m_nRegionHeight;
			}
			else if (nRMy >= SubWorld[nSubWorld].m_nRegionHeight)
			{
				nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[0];
				nRMy -= SubWorld[nSubWorld].m_nRegionHeight;
			}	
			if (nSearchRegion < 0) continue;
			// 从REGION的NPC列表中查找满足条件的NPC		
			
			int nNpcIdx = SubWorld[nSubWorld].m_Region[nSearchRegion].FindNpc(nRMx, nRMy, nLauncherIdx, relation_all);
			if (nNpcIdx > 0)	
			{
				nRet++;
				switch(eRelation)
				{
				case MissleRelation_None:
					break;
					
				case MissleRelation_EnemyOnly:
					if (NpcSet.GetRelation(nLauncherIdx, nNpcIdx) == relation_enemy)
					{
#ifndef _SERVER
						int nSrcX = 0;
						int nSrcY = 0;
						SubWorld[0].Map2Mps(nSearchRegion, Npc[nNpcIdx].m_MapX,Npc[nNpcIdx].m_MapY, Npc[nNpcIdx].m_OffX, Npc[nNpcIdx].m_OffY,  &nSrcX, &nSrcY);
						
						if (m_bFollowNpcWhenCollid)
							CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ,nNpcIdx);
						else 
							CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ);
						
#else
						ProcessDamage(nNpcIdx);						
#endif
					}
					break;
					
				case MissleRelation_AllyOnly:
					if (NpcSet.GetRelation(nLauncherIdx, nNpcIdx) == relation_ally)
					{
#ifndef _SERVER
						int nSrcX = 0;
						int nSrcY = 0;
						SubWorld[0].Map2Mps(nSearchRegion, Npc[nNpcIdx].m_MapX,Npc[nNpcIdx].m_MapY, Npc[nNpcIdx].m_OffX, Npc[nNpcIdx].m_OffY,  &nSrcX, &nSrcY);
						
						if (m_bFollowNpcWhenCollid)
							CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ, nNpcIdx);
						else 
							CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ);
#else
						ProcessDamage(nNpcIdx);						
#endif
					}
					break;
					
				case MissleRelation_All:
					if (NpcSet.GetRelation(nLauncherIdx, nNpcIdx) == relation_enemy || NpcSet.GetRelation(nLauncherIdx, nNpcIdx) == relation_ally)
					{
#ifndef _SERVER
						int nSrcX = 0;
						int nSrcY = 0;
						SubWorld[0].Map2Mps(nSearchRegion, Npc[nNpcIdx].m_MapX,Npc[nNpcIdx].m_MapY, Npc[nNpcIdx].m_OffX, Npc[nNpcIdx].m_OffY,  &nSrcX, &nSrcY);
						if (m_bFollowNpcWhenCollid)
							CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ, nNpcIdx);
						else 
							CreateSpecialEffect(MS_DoCollision, nSrcX, nSrcY, m_nCurrentMapZ);
						
#else
						ProcessDamage(nNpcIdx);						
#endif
					}
					break;
				}
			}
		}
	}
	return nRet;
}


int KMissle::ProcessCollision()
{
#ifdef TOOLVERSION
	return 0;
#endif
	if (m_bClientSend) return 0;
	int nLauncher = 0;
	if (m_SkillParam.dwSkillParams & PARAM_LAUNCHER)
	{
		//now , only imp the launcher is npc,so...
		_ASSERT(m_SkillParam.tLauncher.tActorInfo.nActorType == Actor_Npc);
		nLauncher = m_SkillParam.tLauncher.tActorInfo.nActorIndex;
	}
	else return FALSE;

	return ProcessCollision(nLauncher, m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nDamageRange , m_eRelation);
}

#ifndef _SERVER 
//生成某个特效结点
#define MISSLE_Y_OFFSET 1
BOOL KMissle::CreateSpecialEffect(eMissleStatus eStatus, int nPX, int nPY, int nPZ, int nNpcIndex)
{
	if (!m_MissleRes.m_MissleRes[eStatus].AnimFileName[0]) return FALSE; 
	KSkillSpecialNode * pNode = NULL;
	//同一颗子碟不能有几个爆炸效果在一个Npc身上
	if (nNpcIndex > 0)
	{
		pNode = (KSkillSpecialNode*)m_MissleRes.m_SkillSpecialList.GetHead();
		while(pNode)
		{
			if (pNode->m_pSkillSpecial->m_dwMatchID == Npc[nNpcIndex].m_dwID) return FALSE;
			pNode = (KSkillSpecialNode*)pNode->GetNext();
		}
	}
	
	pNode = new KSkillSpecialNode;
	KSkillSpecial * pSkillSpecial = new KSkillSpecial;
	pNode->m_pSkillSpecial = pSkillSpecial;
	
	int nSrcX = nPX;
	int nSrcY = nPY;
	
	pSkillSpecial->m_nPX = nSrcX;
	pSkillSpecial->m_nPY = nSrcY + MISSLE_Y_OFFSET;
	pSkillSpecial->m_nPZ = nPZ;
	pSkillSpecial->m_nNpcIndex = nNpcIndex;
	pSkillSpecial->m_dwMatchID = Npc[nNpcIndex].m_dwID;
	pSkillSpecial->m_pMissleRes = &m_MissleRes.m_MissleRes[eStatus];
	pSkillSpecial->m_nBeginTime = g_SubWorldSet.GetGameTime();
	pSkillSpecial->m_nEndTime = g_SubWorldSet.GetGameTime() + (pSkillSpecial->m_pMissleRes->nInterval * pSkillSpecial->m_pMissleRes->nTotalFrame / pSkillSpecial->m_pMissleRes->nDir);
	pSkillSpecial->m_nCurDir = g_DirIndex2Dir(m_nDirIndex, m_MissleRes.m_MissleRes[eStatus].nDir);
	pSkillSpecial->Init();
	m_MissleRes.m_SkillSpecialList.AddTail(pNode);
	
	return TRUE;
}
#endif

void KMissle::DoWait()
{
	//	if (m_eMissleStatus == MS_DoWait) return;
	m_eMissleStatus = MS_DoWait;
	
#ifndef _SERVER 
	int nSrcX = 0 ;
	int nSrcY = 0 ;
	SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
	CreateSpecialEffect(MS_DoWait, nSrcX, nSrcY, m_nCurrentMapZ);
	
	/*
	if (!m_MissleRes.m_MissleRes[MS_DoWait].AnimFileName[0])  return ; 
	int nBeginTime  = SubWorld[0].m_dwCurrentTime + 	m_nStartLifeTime - Skill[m_nSkillId][m_nLevel].m_nWaitTime;
	int nEndTime = nBeginTime + m_MissleRes.m_MissleRes[MS_DoWait].nInterval * m_MissleRes.m_MissleRes[MS_DoWait].nTotalFrame / m_MissleRes.m_MissleRes[MS_DoWait].nDir ;
	if (nEndTime <= nBeginTime) return ;
	
	  SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
	  KSkillSpecialNode * pNode = new KSkillSpecialNode;
	  KSkillSpecial * pSkillSpecial = new KSkillSpecial;
	  pNode->m_pSkillSpecial = pSkillSpecial;
	  pSkillSpecial->m_nPX = nSrcX;
	  pSkillSpecial->m_nPY = nSrcY;
	  pSkillSpecial->m_pMissleRes = &m_MissleRes.m_MissleRes[MS_DoWait];
	  pSkillSpecial->m_nBeginTime = nBeginTime;
	  pSkillSpecial->m_nEndTime	= nEndTime;
	  pSkillSpecial->Init();
	  m_MissleRes.m_SkillSpecialList.AddTail(pNode);
	*/
#endif
	
}

void	KMissle::PrePareFly()
{
	if (m_bByMissle)
		DoFly();
}

void KMissle::GetMpsPos(int *pPosX, int *pPosY)
{
	SubWorld[m_nSubWorldId].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, this->m_nXOffset, m_nYOffset, pPosX, pPosY);
};