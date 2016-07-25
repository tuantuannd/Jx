/*******************************************************************************
// FileName			:	KMissle.cpp
// FileAuthor		:	RomanDou
// FileCreateDate	:	2002-6-10 15:32:22
// FileDescription	:	
// Revision Count	:	
*******************************************************************************/

#include "KCore.h"
#include "KMissle.h"
#include "KSubWorld.h"
#include "KSubWorldSet.h"
#include "KRegion.h"
#include "KNpc.h"
#include "KNpcSet.h"
#include "KMath.h"
#include <math.h>
#include "KSkillSpecial.h"
//#include "myassert.h"
#ifndef _SERVER
#include "../../Represent/iRepresent/iRepresentshell.h"
#include "Scene\KScenePlaceC.h"
#include "ImgRef.h"
#endif
#include "Scene/ObstacleDef.h"
#include "KPlayer.h"
#include "KMissleSet.h"

#ifdef _STANDALONE
#include "KSG_StringProcess.h"
#else
#include "../../Engine/Src/KSG_StringProcess.h"
#endif

TCollisionMatrix g_CollisionMatrix[64] =
{
	{0,	0, -1, 1, 0, 1, 1, 1}, // 0--------
	{0,	0, -1, 1, 0, 1, 1, 1}, // 1
	{0,	0, -1, 1, 0, 1, 1, 1}, // 2
	{0,	0, -1, 1, 0, 1, 1, 1}, // 3
	
	{0,	0, -1, 0, -1, 1, 0, 1}, // 4
	{0,	0, -1, 0, -1, 1, 0, 1}, // 5
	{0,	0, -1, 0, -1, 1, 0, 1}, // 6
	{0,	0, -1, 0, -1, 1, 0, 1}, // 7
	{0,	0, -1, 0, -1, 1, 0, 1}, // 8--------
	{0,	0, -1, 0, -1, 1, 0, 1}, // 9
	{0,	0, -1, 0, -1, 1, 0, 1}, // 10
	{0,	0, -1, 0, -1, 1, 0, 1}, // 11
	
	{0,	0, -1, -1, -1, 0, -1, 1}, // 12
	{0,	0, -1, -1, -1, 0, -1, 1}, // 13
	{0,	0, -1, -1, -1, 0, -1, 1}, // 14
	{0,	0, -1, -1, -1, 0, -1, 1}, // 15
	{0,	0, -1, -1, -1, 0, -1, 1}, // 16--------
	{0,	0, -1, -1, -1, 0, -1, 1}, // 17
	{0,	0, -1, -1, -1, 0, -1, 1}, // 18
	{0,	0, -1, -1, -1, 0, -1, 1}, // 19
	
	{0,	0, 0, -1, -1, -1, -1, 0}, // 20
	{0,	0, 0, -1, -1, -1, -1, 0}, // 21
	{0,	0, 0, -1, -1, -1, -1, 0}, // 22
	{0,	0, 0, -1, -1, -1, -1, 0}, // 23
	{0,	0, 0, -1, -1, -1, -1, 0}, // 24--------
	{0,	0, 0, -1, -1, -1, -1, 0}, // 25
	{0,	0, 0, -1, -1, -1, -1, 0}, // 26
	{0,	0, 0, -1, -1, -1, -1, 0}, // 27
	
	{0,	0, -1, -1, 0, -1, 1, -1}, // 28
	{0,	0, -1, -1, 0, -1, 1, -1}, // 29
	{0,	0, -1, -1, 0, -1, 1, -1}, // 30
	{0,	0, -1, -1, 0, -1, 1, -1}, // 31
	{0,	0, -1, -1, 0, -1, 1, -1}, // 32--------
	{0,	0, -1, -1, 0, -1, 1, -1}, // 33
	{0,	0, -1, -1, 0, -1, 1, -1}, // 34
	{0,	0, -1, -1, 0, -1, 1, -1}, // 35
	
	{0,	0, 0, -1, 1, -1, 1, 0}, // 36
	{0,	0, 0, -1, 1, -1, 1, 0}, // 37
	{0,	0, 0, -1, 1, -1, 1, 0}, // 38
	{0,	0, 0, -1, 1, -1, 1, 0}, // 39
	{0,	0, 0, -1, 1, -1, 1, 0}, // 40--------
	{0,	0, 0, -1, 1, -1, 1, 0}, // 41
	{0,	0, 0, -1, 1, -1, 1, 0}, // 42
	{0,	0, 0, -1, 1, -1, 1, 0}, // 43
	
	{0,	0, 1, -1, 1, 0, 1, 1}, // 44
	{0,	0, 1, -1, 1, 0, 1, 1}, // 45
	{0,	0, 1, -1, 1, 0, 1, 1}, // 46
	{0,	0, 1, -1, 1, 0, 1, 1}, // 47
	{0,	0, 1, -1, 1, 0, 1, 1}, // 48--------
	{0,	0, 1, -1, 1, 0, 1, 1}, // 49
	{0,	0, 1, -1, 1, 0, 1, 1}, // 50
	{0,	0, 1, -1, 1, 0, 1, 1}, // 51
	
	{0,	0, 1, 0, 1, 1, 0, 1}, // 52
	{0,	0, 1, 0, 1, 1, 0, 1}, // 53
	{0,	0, 1, 0, 1, 1, 0, 1}, // 54
	{0,	0, 1, 0, 1, 1, 0, 1}, // 55
	{0,	0, 1, 0, 1, 1, 0, 1}, // 56---------
	{0,	0, 1, 0, 1, 1, 0, 1}, // 57
	{0,	0, 1, 0, 1, 1, 0, 1}, // 58
	{0,	0, 1, 0, 1, 1, 0, 1}, // 59
	
	{0,	0, -1, 1, 0, 1, 1, 1}, // 60
	{0,	0, -1, 1, 0, 1, 1, 1}, // 61
	{0,	0, -1, 1, 0, 1, 1, 1}, // 62
	{0,	0, -1, 1, 0, 1, 1, 1}, // 63
};

KMissle g_MisslesLib[MAX_MISSLESTYLE];

//每个格子的像素长宽
#define CellWidth		(SubWorld[m_nSubWorldId].m_nCellWidth << 10)
#define CellHeight		(SubWorld[m_nSubWorldId].m_nCellHeight << 10)

//每个region格点长宽
#define RegionWidth		(SubWorld[m_nSubWorldId].m_nRegionWidth)
#define RegionHeight	(SubWorld[m_nSubWorldId].m_nRegionHeight)

#define CurRegion		SubWorld[m_nSubWorldId].m_Region[m_nRegionId]
#define CurSubWorld		SubWorld[m_nSubWorldId]

#define LeftRegion(nRegionId)	SubWorld[m_nSubWorldId].m_Region[nRegionId].m_nConnectRegion[2]
#define RightRegion(nRegionId)		SubWorld[m_nSubWorldId].m_Region[nRegionId].m_nConnectRegion[6]
#define UpRegion(nRegionId)		SubWorld[m_nSubWorldId].m_Region[nRegionId].m_nConnectRegion[4]
#define DownRegion(nRegionId)		SubWorld[m_nSubWorldId].m_Region[nRegionId].m_nConnectRegion[0]

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
	m_ulDamageInterval = 0;
	m_nTempParam1 = 0;
	m_nTempParam2 = 0;
	m_nFirstReclaimTime = 0;
	m_nEndReclaimTime = 0;
	
#ifdef _SERVER
	m_pMagicAttribsData = NULL;
	m_ulNextCalDamageTime = 0;
#else
	m_bFollowNpcWhenCollid = 1;
	m_bRemoving	= FALSE;
	m_btRedLum = m_btGreenLum = m_btBlueLum = 0xff;
	m_usLightRadius = 50;
#endif
}

void KMissle::Release()
{
///#pragma	message(ATTENTION("子弹消亡时，需更新发送者使用该技能时的当前使用次数，使之减一"))
#ifndef _SERVER	
	g_ScenePlace.RemoveObject(CGOG_MISSLE, m_nMissleId, m_SceneID);
	m_MissleRes.Clear();
	m_nMissleId = -1;
	m_nFollowNpcIdx = 0;
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
	
	BOOL bAutoExplode = 0;
	pMisslesSetting->GetInteger(nRow, "AutoExplode",	0, (int*)&bAutoExplode, TRUE);
	m_bAutoExplode = bAutoExplode;
	
	pMisslesSetting->GetInteger(nRow, "DmgInterval",	0, (int*)&m_ulDamageInterval, TRUE);
	
#ifndef _SERVER	
	char AnimFileCol[64];
	char SndFileCol[64];
	char AnimFileInfoCol[100];
	char szAnimFileInfo[100];

    const char *pcszTemp = NULL;
	
	pMisslesSetting->GetInteger(nRow, "RedLum",	    255, (int*)&m_btRedLum, TRUE);
	pMisslesSetting->GetInteger(nRow, "GreenLum",	255, (int*)&m_btGreenLum, TRUE);
	pMisslesSetting->GetInteger(nRow, "BlueLum",	255, (int*)&m_btBlueLum, TRUE);
	
	int nLightRadius = 0;
	pMisslesSetting->GetInteger(nRow, "LightRadius", 50, (int*)&nLightRadius, TRUE);
	m_usLightRadius = nLightRadius;
	
	pMisslesSetting->GetInteger(nRow, "MultiShow",		0, &m_bMultiShow, TRUE);
	for (int i  = 0; i < MAX_MISSLE_STATUS; i++)
	{
		sprintf(AnimFileCol, "AnimFile%d", i + 1);
		sprintf(SndFileCol,  "SndFile%d", i + 1);
		sprintf(AnimFileInfoCol, "AnimFileInfo%d", i + 1);
		
		pMisslesSetting->GetString(nRow, AnimFileCol,			"", m_MissleRes.m_MissleRes[i].AnimFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, SndFileCol,			"", m_MissleRes.m_MissleRes[i].SndFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, AnimFileInfoCol,		"", szAnimFileInfo, 100, TRUE);
		
		//m_MissleRes.m_MissleRes[i].nInterval = 1;
		//m_MissleRes.m_MissleRes[i].nDir = 16;
		//m_MissleRes.m_MissleRes[i].nTotalFrame = 100;

        pcszTemp = szAnimFileInfo;
        m_MissleRes.m_MissleRes[i].nTotalFrame = KSG_StringGetInt(&pcszTemp, 100);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        m_MissleRes.m_MissleRes[i].nDir = KSG_StringGetInt(&pcszTemp, 16);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        m_MissleRes.m_MissleRes[i].nInterval = KSG_StringGetInt(&pcszTemp, 1);
		//sscanf(szAnimFileInfo, "%d,%d,%d", 
		//	&m_MissleRes.m_MissleRes[i].nTotalFrame,
		//	&m_MissleRes.m_MissleRes[i].nDir,
		//	&m_MissleRes.m_MissleRes[i].nInterval
        //);

		
		sprintf(AnimFileCol, "AnimFileB%d", i + 1);
		sprintf(SndFileCol,  "SndFileB%d", i + 1);
		sprintf(AnimFileInfoCol, "AnimFileInfoB%d", i + 1);
		
		pMisslesSetting->GetString(nRow, AnimFileCol,			"", m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].AnimFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, SndFileCol,			"", m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].SndFileName, 64, TRUE);
		pMisslesSetting->GetString(nRow, AnimFileInfoCol,		"", szAnimFileInfo, 100, TRUE);
		
		//m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nInterval = 1;
		//m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nDir = 16;
		//m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nTotalFrame = 100;
		
        pcszTemp = szAnimFileInfo;
        m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nTotalFrame = KSG_StringGetInt(&pcszTemp, 100);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nDir = KSG_StringGetInt(&pcszTemp, 16);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nInterval = KSG_StringGetInt(&pcszTemp, 1);

		//sscanf(szAnimFileInfo, "%d,%d,%d", 
		//	&m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nTotalFrame,
		//	&m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nDir,
		//	&m_MissleRes.m_MissleRes[i + MAX_MISSLE_STATUS].nInterval
        //);
		
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
	if (m_nMissleId <= 0 || m_nRegionId < 0)
	{
		return  0 ;
	}
	
	_ASSERT(m_nLauncher > 0);
	if (m_nLauncher <= 0)
		return 0;
	
	//子弹的主人已经离开，So 子弹消亡
	if (!Npc[m_nLauncher].IsMatch(m_dwLauncherId) || Npc[m_nLauncher].m_SubWorldIndex != m_nSubWorldId || Npc[m_nLauncher].m_RegionIndex < 0)
	{
		DoVanish();
		return 0;	
	}
	
	//跟踪的目标人物已经不在该地图上时，自动清空
	if (m_nFollowNpcIdx > 0)
	{
		if (!Npc[m_nFollowNpcIdx].IsMatch(m_dwFollowNpcID) || Npc[m_nFollowNpcIdx].m_SubWorldIndex != m_nSubWorldId)
		{
			m_nFollowNpcIdx = 0;
		}
	}
	
	eMissleStatus eLastStatus = m_eMissleStatus;
	
	//如果当前状态是子弹生命正常结束正准备消亡状态时，而不是消亡中或者已碰撞中
	if (
		m_nCurrentLife >= m_nLifeTime 
		&& m_eMissleStatus != MS_DoVanish 
		&& m_eMissleStatus != MS_DoCollision
		)
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
	
	if (m_nCurrentLife == m_nStartLifeTime && m_eMissleStatus != MS_DoVanish)	
	{
		if (PrePareFly())
		{
#ifndef _SERVER
			int nSrcX2 = 0 ;
			int nSrcY2 = 0 ;
			SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX2, &nSrcY2);
			m_MissleRes.PlaySound(MS_DoFly, nSrcX2, nSrcY2, 0);
			//CreateSpecialEffect(MS_DoFly, nSrcX2, nSrcY2, m_nCurrentMapZ);
#endif
			
			DoFly();
		}
		else
			DoVanish();
	}
	
	switch(m_eMissleStatus)
	{
	case MS_DoWait:
		{
			OnWait();
		}
		break;
	case MS_DoFly:
		{
			OnFly();
			if (m_bFlyEvent)
			{
				if ( (m_nCurrentLife - m_nStartLifeTime) % m_nFlyEventTime == 0 )
				{
					_ASSERT(m_nSkillId < MAX_SKILL && m_nLevel < MAX_SKILLLEVEL);
					if (m_nLevel  <= 0 ) return 0;
					KSkill * pOrdinSkill = (KSkill *) g_SkillManager.GetSkill(m_nSkillId , m_nLevel);
					if (pOrdinSkill)
					{
						pOrdinSkill->FlyEvent(this);
					}
				}
			}
		}
		break;
	case MS_DoCollision:
		{
			OnCollision();
		}
		break;
	case MS_DoVanish:
		{
			OnVanish();
		}
		break;
	}
	
#ifndef _SERVER
	//子弹未消亡掉
	if (m_nMissleId > 0)
	{
		int nSrcX;
		int nSrcY;
		
		SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
		if (m_usLightRadius && m_eMissleStatus != MS_DoWait)
			g_ScenePlace.MoveObject(CGOG_MISSLE, m_nMissleId, nSrcX, nSrcY, m_nCurrentMapZ, m_SceneID, IPOT_RL_OBJECT | IPOT_RL_LIGHT_PROP );
		else
			g_ScenePlace.MoveObject(CGOG_MISSLE, m_nMissleId, nSrcX, nSrcY, m_nCurrentMapZ, m_SceneID, IPOT_RL_OBJECT);
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

// 1表示正常碰撞到物体，0表示未碰撞到任何物体, -1表示落地
int KMissle::CheckCollision()
{
#ifdef TOOLVERSION
	return FALSE;
#endif
	
	if (m_nCurrentMapZ <= MISSLE_MIN_COLLISION_ZHEIGHT) 
	{
		return -1;
	}
	
	//子弹在高于一定高度时，不处理越界碰撞问题
	if (m_nCurrentMapZ > MISSLE_MAX_COLLISION_ZHEIGHT) return 0;
	
	if (m_nRegionId < 0) 
	{
		return -1;
	}

	int nAbsX = 0;
	int nAbsY = 0;
	int nCellWidth = CellWidth;
	int nCellHeight = CellHeight;
	_ASSERT(nCellWidth > 0 && nCellHeight > 0);
	int nRMx = 0;
	int nRMy = 0;
	int nSearchRegion = 0;
	int nNpcIdx = 0;
	int nDX = 0;
	int nDY = 0;
	int nNpcOffsetX = 0;
	int nNpcOffsetY = 0;
	BOOL bCollision = FALSE;
	
	int nColRegion = m_nRegionId;
	int nColMapX = m_nCurrentMapX;
	int nColMapY = m_nCurrentMapY;
		
	if (m_nCollideRange == 1)
	{
		if (m_bNeedReclaim && m_nCurrentLife >= m_nFirstReclaimTime && m_nCurrentLife <= m_nEndReclaimTime)
		{
			if (m_nCurrentLife == m_nEndReclaimTime) 
				m_bNeedReclaim = FALSE;
			nNpcIdx = 	CheckNearestCollision();
		}
		else
		{
			nNpcIdx = SubWorld[m_nSubWorldId].m_Region[nColRegion].FindNpc(nColMapX, nColMapY, m_nLauncher, m_eRelation);
		}

		if (nNpcIdx > 0)
		{ 
			if (m_nDamageRange == 1)//在目标Npc处碰撞
				ProcessCollision(m_nLauncher, Npc[nNpcIdx].m_RegionIndex , Npc[nNpcIdx].m_MapX, Npc[nNpcIdx].m_MapY, m_nDamageRange , m_eRelation);
			else
				ProcessCollision();//在子弹位置处理碰撞
			DoCollision();//子弹作碰撞后的效果
			return 1;
		}
	}
	else
	{
		for (int i = -m_nCollideRange; i <= m_nCollideRange; i ++)
			for (int j = -m_nCollideRange; j <= m_nCollideRange; j ++)
			{
				if (!GetOffsetAxis(m_nSubWorldId, m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, i , j , nSearchRegion, nRMx, nRMy))
					continue;
				
				_ASSERT(nSearchRegion >= 0);
				nNpcIdx = SubWorld[m_nSubWorldId].m_Region[nSearchRegion].FindNpc(nRMx, nRMy, m_nLauncher, m_eRelation);
				if (nNpcIdx > 0)
				{
					ProcessCollision();//处理碰撞
					DoCollision();//子弹作碰撞后的效果
					return 1;
				}
			}
	}
	
	return 0;
}

inline DWORD	KMissle::GetCurrentSubWorldTime()
{
	return SubWorld[m_nSubWorldId].m_dwCurrentTime;
}

void KMissle::OnFly()
{
	if (m_nInteruptTypeWhenMove)
	{
		//当发送者位置移动了，不仅正从do_wait状态到do_fly状态的新子弹被消失掉
		//而且已进入dofly状态的旧的所属子弹也要强制消失掉
		if (m_nInteruptTypeWhenMove == Interupt_EndOldMissleLifeWhenMove)
		{
			int nPX, nPY;
			Npc[m_nLauncher].GetMpsPos(&nPX, &nPY);
			if (nPX != m_nLauncherSrcPX || nPY != m_nLauncherSrcPY)
			{
				
#ifndef _SERVER 
				int nSrcX2 = 0 ;
				int nSrcY2 = 0 ;
				SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX2, &nSrcY2);
				CreateSpecialEffect(MS_DoVanish, nSrcX2, nSrcY2, m_nCurrentMapZ);
#endif
				
				DoVanish();
				return ;
			}
		}
	}
	
	//检测当前位置是否有障碍
	if (TestBarrier()) 
	{
#ifndef _SERVER 
		int nSrcX3 = 0 ;
		int nSrcY3 = 0 ;
		SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX3, &nSrcY3);
		CreateSpecialEffect(MS_DoVanish, nSrcX3, nSrcY3, m_nCurrentMapZ);
#endif
		DoVanish();
		return;
	}
	
	int nDOffsetX = 0;
	int nDOffsetY = 0;
	
	ZAxisMove();			
	switch(this->m_eMoveKind)
	{
	case	MISSLE_MMK_Stand:							//	原地
		{
			
		}
		break;
	case	MISSLE_MMK_Parabola:						//	抛物线
	case	MISSLE_MMK_Line:							//	直线飞行
		{
			nDOffsetX    = (m_nSpeed * m_nXFactor);
			nDOffsetY	 = (m_nSpeed * m_nYFactor);
		}
		break;
	case MISSLE_MMK_RollBack:
		{
			if (!m_nTempParam1)	
			{
				if (m_nTempParam2 <= m_nCurrentLife)
				{
					m_nXFactor = -m_nXFactor;
					m_nYFactor = -m_nYFactor;
					m_nTempParam1 = 1;
					m_nDir = m_nDir - MaxMissleDir / 2;
					if (m_nDir < 0) m_nDir += MaxMissleDir;
				}
			}

			nDOffsetX = (m_nSpeed * m_nXFactor);
			nDOffsetY = (m_nSpeed * m_nYFactor);
		}break;
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
			if (nPreAngle < 0) nPreAngle = MaxMissleDir - 1;
			m_nDir = m_nAngle + (MaxMissleDir / 4);
			if (m_nDir >= MaxMissleDir) m_nDir = m_nDir - MaxMissleDir;
			int dx = (m_nSpeed + 50)  * (g_DirCos(m_nAngle,MaxMissleDir) - g_DirCos(nPreAngle,MaxMissleDir)) ;
			int dy = (m_nSpeed + 50)  * (g_DirSin(m_nAngle,MaxMissleDir) - g_DirSin(nPreAngle, MaxMissleDir)) ; 
			
			if (m_nParam2) //原地转
			{
				nDOffsetX = dx;
				nDOffsetY = dy;
			}
			else			// 围绕着发送者转
			{
				int nOldRegion = m_nRegionId;
				CurRegion.DecRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
				m_nRegionId		= Npc[m_nLauncher].m_RegionIndex;
				m_nCurrentMapX	= Npc[m_nLauncher].m_MapX;
				m_nCurrentMapY	= Npc[m_nLauncher].m_MapY;
				m_nXOffset		= Npc[m_nLauncher].m_OffX;
				m_nYOffset		= Npc[m_nLauncher].m_OffY;
				CurRegion.AddRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
				
				if (nOldRegion != m_nRegionId)
				{
					SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_CHANGE_REGION, nOldRegion, m_nRegionId, m_nMissleId);
				}  
				nDOffsetX = dx;
				nDOffsetY = dy;
			}
			
			//顺时针还是逆时针
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
			if (nPreAngle < 0) 
			{
				nPreAngle = MaxMissleDir -1;
			}
			m_nDir = m_nAngle + (MaxMissleDir / 4);
			if (m_nDir >= MaxMissleDir) m_nDir = m_nDir - MaxMissleDir;
			
			int dx = (m_nSpeed + m_nCurrentLife + 50)  * (g_DirCos(m_nAngle,MaxMissleDir) - g_DirCos(nPreAngle, MaxMissleDir)) ;
			int dy = (m_nSpeed + m_nCurrentLife + 50)  * (g_DirSin(m_nAngle,MaxMissleDir) - g_DirSin(nPreAngle,MaxMissleDir)) ; 
			
			if (m_nParam2) //原地转
			{
				nDOffsetX = dx;
				nDOffsetY = dy;
			}
			else			// 围绕着发送者转
			{
				int nOldRegion = m_nRegionId;
				CurRegion.DecRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
				m_nRegionId		= Npc[m_nLauncher].m_RegionIndex;
				m_nCurrentMapX	= Npc[m_nLauncher].m_MapX;
				m_nCurrentMapY	= Npc[m_nLauncher].m_MapY;
				m_nXOffset		= Npc[m_nLauncher].m_OffX;
				m_nYOffset		= Npc[m_nLauncher].m_OffY;
				CurRegion.AddRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
				
				if (nOldRegion != m_nRegionId)
				{
					SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_CHANGE_REGION, nOldRegion, m_nRegionId, m_nMissleId);
				}  
				nDOffsetX = dx;
				nDOffsetY = dy;
			}
			
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
			int nDistance = 0;
			int nSrcMpsX = 0;
			int nSrcMpsY = 0;
			int nDesMpsX = 0;
			int nDesMpsY = 0;
			
			SubWorld[m_nSubWorldId].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, &nSrcMpsX, &nSrcMpsY);
			SubWorld[m_nSubWorldId].Map2Mps(Npc[m_nFollowNpcIdx].m_RegionIndex, Npc[m_nFollowNpcIdx].m_MapX, Npc[m_nFollowNpcIdx].m_MapY, Npc[m_nFollowNpcIdx].m_OffX, Npc[m_nFollowNpcIdx].m_OffY, &nDesMpsX, &nDesMpsY);
			nDistance = SubWorld[m_nSubWorldId].GetDistance(nSrcMpsX, nSrcMpsY, nDesMpsX, nDesMpsY);
			
			if (nDistance != 0)
			{
				int nXFactor = ((nDesMpsX - nSrcMpsY ) << 10) / nDistance;
				int nYFactor = ((nDesMpsY - nSrcMpsY ) << 10) / nDistance;
				int dx		 = nXFactor * m_nSpeed;
				int dy		 = nYFactor * m_nSpeed;	
				nDOffsetX	 = dx;
				nDOffsetY	 = dy;
			}
			
		}break;
		
	case	MISSLE_MMK_Motion:							//	玩家动作类
		{
			
		}break;
		
	case MISSLE_MMK_SingleLine:						//	必中的单一直线飞行魔法
		{
			//单一必中类子弹，类式于传奇以及其它的同类网络游戏中的基本直线魔法			
#ifdef _SERVER
			
#else
			int x = m_nXOffset;
			int y = m_nYOffset;
			int dx = (m_nSpeed * m_nXFactor);
			int dy = (m_nSpeed * m_nYFactor);
			nDOffsetX	=  dx;//* m_nCurrentLife;
			nDOffsetY	=  dy;//* m_nCurrentLife;
			
#endif
		}
		break;
	default:
		_ASSERT(0);
		
	}
	
	//
	if (CheckBeyondRegion(nDOffsetX, nDOffsetY))
	{
		if (CheckCollision() == -1) 
		{
			if (m_bAutoExplode)
			{
				ProcessCollision();//处理碰撞
			}
#ifndef _SERVER 
			int nSrcX4 = 0 ;
			int nSrcY4 = 0 ;
			SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX4, &nSrcY4);
			CreateSpecialEffect(MS_DoVanish, nSrcX4, nSrcY4, m_nCurrentMapZ);
#endif
			DoVanish();
			return;
		}
	}
	else//如果子弹飞行过程中进入了一个无效的Region则子弹自动消亡
	{
		DoVanish();
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
	{
		m_MissleRes.Draw(m_eMissleStatus, nSrcX, nSrcY, m_nCurrentMapZ, m_nDir,m_nLifeTime - m_nStartLifeTime,  m_nCurrentLife - m_nStartLifeTime );
	}
	else
	{
		int nDirIndex = g_GetDirIndex(0,0,m_nXFactor, m_nYFactor);
		int nDir = g_DirIndex2Dir(nDirIndex, 64);
		m_MissleRes.Draw(m_eMissleStatus, nSrcX, nSrcY, m_nCurrentMapZ, nDir,m_nLifeTime - m_nStartLifeTime,  m_nCurrentLife - m_nStartLifeTime );
	}
	
	//对于客户端，直到子弹及其产生的效果全部播放完才终止并删除掉!
	if (m_MissleRes.m_bHaveEnd && (m_MissleRes.SpecialMovieIsAllEnd()))
		SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_DEL, m_nMissleId);
}
#endif


BOOL	KMissle::CheckBeyondRegion(int nDOffsetX, int nDOffsetY)
{
	if (m_nRegionId < 0) 
		return FALSE;
	//未动
	if (nDOffsetX == 0 && nDOffsetY == 0) return TRUE;

	if (abs(nDOffsetX) > CellWidth) 
	{
		return FALSE;
	}

	if (abs(nDOffsetY) > CellHeight) 
	{
		return FALSE;
	}

	int nOldRegion		= m_nRegionId;
	int nNewXOffset		= m_nXOffset + nDOffsetX;
	int nNewYOffset		= m_nYOffset + nDOffsetY;
	int nNewMapX		= m_nCurrentMapX;
	int nNewMapY		= m_nCurrentMapY;
	int nNewRegion		= m_nRegionId;
	
	DWORD nRegionWidth = RegionWidth;
	DWORD nRegionHeight = RegionHeight;
	
	_ASSERT(abs(nNewXOffset) <= CellWidth * 2);
	_ASSERT(abs(nNewYOffset) <= CellHeight * 2);
	
	//	处理NPC的坐标变幻
	//	CELLWIDTH、CELLHEIGHT、OffX、OffY均是放大了1024倍
	
	if (nNewXOffset < 0)
	{
		nNewMapX--;
		nNewXOffset += CellWidth;
	}
	else if (nNewXOffset > CellWidth)
	{
		nNewMapX++;
		nNewXOffset -= CellWidth;
	}
	
	if (nNewYOffset < 0)
	{
		nNewMapY--;
		nNewYOffset += CellHeight;
	}
	else if (nNewYOffset > CellHeight)
	{
		nNewMapY++;
		nNewYOffset -= CellHeight;
	}
	
	if (nNewMapX < 0)
	{
		nNewRegion = LeftRegion(m_nRegionId);
		nNewMapX += nRegionWidth;
	}
	else if ((DWORD)nNewMapX >= nRegionWidth)
	{
		nNewRegion = RightRegion(m_nRegionId);
		nNewMapX -= nRegionWidth;
	}

	if (nNewRegion < 0) 
	{
		return FALSE; 
	}
	
	if (nNewMapY < 0)
	{
		nNewRegion = UpRegion(nNewRegion);
		nNewMapY += nRegionHeight;
	}
	else if (nNewMapY >= RegionHeight)
	{
		nNewRegion = DownRegion(nNewRegion);
		nNewMapY -= nRegionHeight;
	}
	
	//下一个位置为不合法位置，则消亡
	if (nNewRegion < 0) 
	{
		return FALSE; 
	}
	else
	{
		CurRegion.DecRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
		_ASSERT(m_nCurrentMapX >= 0  &&  m_nCurrentMapY >= 0);
		
		m_nRegionId	   = nNewRegion;
		m_nCurrentMapX = nNewMapX;
		m_nCurrentMapY = nNewMapY;
		m_nXOffset	   = nNewXOffset;
		m_nYOffset	   = nNewYOffset;
		CurRegion.AddRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
		
		if (nOldRegion != m_nRegionId)
		{
			SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_CHANGE_REGION, nOldRegion, m_nRegionId, m_nMissleId);
		}
	}
	return TRUE;
}

KMissle&	KMissle::operator=(KMissle& Missle)
{
	Missle.m_nTempParam1	=	0;
	Missle.m_nTempParam2	=	0;
	Missle.m_nDesMapX			=	0;
	Missle.m_nDesMapY			=	0;
	Missle.m_nDesRegion		=	0;
	Missle.m_bNeedReclaim	=	FALSE;
	Missle.m_nFirstReclaimTime = 0;
	Missle.m_nEndReclaimTime = 0;
	memset(Missle.m_NeedReclaimPos, 0, sizeof(m_NeedReclaimPos));

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
	Missle.m_bAutoExplode	=	m_bAutoExplode;
	Missle.m_ulDamageInterval = m_ulDamageInterval;
	strcpy(Missle.m_szMissleName	,	m_szMissleName);
	
#ifndef  _SERVER
	Missle.m_bMultiShow		=  m_bMultiShow;
	Missle.m_MissleRes.m_bLoopAnim = m_MissleRes.m_bLoopAnim;
	Missle.m_MissleRes.m_bHaveEnd = FALSE;
	Missle.m_btRedLum		= m_btRedLum;
	Missle.m_btGreenLum		= m_btGreenLum;
	Missle.m_btBlueLum		= m_btBlueLum;
	Missle.m_usLightRadius	= m_usLightRadius;
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
	bool bCalDamage = false;

	_ASSERT (Npc[m_nLauncher].IsMatch(m_dwLauncherId));
	
	if (m_pMagicAttribsData) 
	{
		if (Npc[nNpcId].ReceiveDamage(m_nLauncher, m_bIsMelee, m_pMagicAttribsData->m_pDamageMagicAttribs, m_bUseAttackRating, m_bDoHurt))
		{
			if (m_pMagicAttribsData->m_nStateMagicAttribsNum > 0)
				Npc[nNpcId].SetStateSkillEffect(m_nLauncher, m_nSkillId, m_nLevel, m_pMagicAttribsData->m_pStateMagicAttribs, m_pMagicAttribsData->m_nStateMagicAttribsNum, m_pMagicAttribsData->m_pStateMagicAttribs[0].nValue[1]);
			
			if (m_pMagicAttribsData->m_nImmediateMagicAttribsNum > 0)
				Npc[nNpcId].SetImmediatelySkillEffect(m_nLauncher, m_pMagicAttribsData->m_pImmediateAttribs, m_pMagicAttribsData->m_nImmediateMagicAttribsNum);
		}
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
		KSkill * pOrdinSkill = (KSkill *) g_SkillManager.GetSkill(m_nSkillId,m_nLevel);
		if (pOrdinSkill)
        {
			pOrdinSkill->Vanish(this);
        }
	}
#ifdef _SERVER	//服务器端时子弹一旦进入消亡期则直接删除掉
	SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_DEL, m_nMissleId);
	m_eMissleStatus = MS_DoVanish;
	return ;
#endif
	m_eMissleStatus = MS_DoVanish;
#ifndef _SERVER 
	if (m_nRegionId < 0)
	{
		_ASSERT(0);
		m_bRemoving = TRUE;
		return ;
	}
#endif
}

void KMissle::DoCollision()
{
	if (m_eMissleStatus == MS_DoCollision) return;
	
#ifndef _SERVER
	int nSrcX = 0 ;
	int nSrcY = 0 ;
	SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX, &nSrcY);
#endif
	
	if (m_bCollideEvent)	
	{
		_ASSERT(m_nSkillId < MAX_SKILL && m_nLevel < MAX_SKILLLEVEL);
		KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(m_nSkillId, m_nLevel);
		if (pOrdinSkill)
        {
			pOrdinSkill->Collidsion(this);
        }
	}
	
	if (m_bCollideVanish)
	{
#ifndef _SERVER
		m_MissleRes.m_bHaveEnd = TRUE;
#endif
		
#ifndef _SERVER 
		int nSrcX5 = 0 ;
		int nSrcY5 = 0 ;
		SubWorld[0].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY,m_nXOffset, m_nYOffset, &nSrcX5, &nSrcY5);
		CreateSpecialEffect(MS_DoVanish, nSrcX5, nSrcY5, m_nCurrentMapZ);
#endif
		
		DoVanish();
	}
	else 
	{
#ifndef _SERVER		
		//增加撞后的效果	
		if (m_MissleRes.SpecialMovieIsAllEnd())
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

BOOL KMissle::GetOffsetAxis(int nSubWorld, int nSrcRegionId, int nSrcMapX, int nSrcMapY,
							int nOffsetMapX, int nOffsetMapY, 
							int &nDesRegionId, int &nDesMapX, int &nDesMapY)
{
	nDesRegionId = -1;
	// 确定目标格子实际的REGION和坐标确定
	nDesMapX = nSrcMapX + nOffsetMapX;
	nDesMapY = nSrcMapY + nOffsetMapY;
	
	if (nSrcRegionId < 0) 
		return FALSE;

	int nSearchRegion = nSrcRegionId;
	if (nDesMapX < 0)
	{
		nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[2];
		nDesMapX += SubWorld[nSubWorld].m_nRegionWidth;
	}
	else if (nDesMapX >= SubWorld[nSubWorld].m_nRegionWidth)
	{
		nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[6];
		nDesMapX -= SubWorld[nSubWorld].m_nRegionWidth;
	}
	if (nSearchRegion < 0) 
		return FALSE;
	
	if (nDesMapY < 0)
	{
		nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[4];
		nDesMapY += SubWorld[nSubWorld].m_nRegionHeight;
	}
	else if (nDesMapY >= SubWorld[nSubWorld].m_nRegionHeight)
	{
		nSearchRegion = SubWorld[nSubWorld].m_Region[nSearchRegion].m_nConnectRegion[0];
		nDesMapY -= SubWorld[nSubWorld].m_nRegionHeight;
	}	

	if (nSearchRegion < 0) 
		return FALSE;
	nDesRegionId = nSearchRegion;
	return TRUE;
	// 从REGION的NPC列表中查找满足条件的NPC		
	//int nNpcIdx = SubWorld[nSubWorld].m_Region[nSearchRegion].FindNpc(nDesMapX, nDesMapY, nLauncherIdx, relation_all);
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
int KMissle::ProcessCollision(int nLauncherIdx, int nRegionId, int nMapX, int nMapY, int nRange , int eRelation)
{
#ifdef TOOLVERSION 
	return 0;
#endif
#ifdef _SERVER
	if (m_ulDamageInterval)
	{
		if (m_ulNextCalDamageTime > g_SubWorldSet.GetGameTime())
		{
			return FALSE;
		}
		else
		{
			// 6.29 romandou missledamage interval 
			m_ulNextCalDamageTime = g_SubWorldSet.GetGameTime() + m_ulDamageInterval;
		}
	}
#endif
	if (nLauncherIdx <= 0 ) return 0;
	if (nRange <= 0) return 0;
	
	int nRangeX = nRange / 2;
	int	nRangeY = nRangeX;
	int	nSubWorld = Npc[nLauncherIdx].m_SubWorldIndex;
	
	_ASSERT(Npc[nLauncherIdx].m_SubWorldIndex >= 0);
	_ASSERT(nRegionId >= 0);
	
	int	nRegion = nRegionId;
	int	nRet = 0;
	int	nRMx, nRMy, nSearchRegion;

	// 检查范围内的格子里的NPC
	for (int i = -nRangeX; i <= nRangeX; i++)
	{
		for (int j = -nRangeY; j <= nRangeY; j++)
		{
			// 去掉边角几个格子，保证视野是椭圆形
			//if ((i * i + j * j ) > nRangeX * nRangeX)
			//continue;

			if (!GetOffsetAxis(nSubWorld, nRegionId, nMapX, nMapY, i , j , nSearchRegion, nRMx, nRMy))
				continue;

			_ASSERT(nSearchRegion >= 0);

			// 从REGION的NPC列表中查找满足条件的NPC		
			int nNpcIdx = SubWorld[nSubWorld].m_Region[nSearchRegion].FindNpc(nRMx, nRMy, nLauncherIdx, eRelation);
			if (nNpcIdx > 0)	
			{
				nRet++;
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
	return ProcessCollision(m_nLauncher, m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nDamageRange , m_eRelation);
}

#ifndef _SERVER 
//生成某个特效结点
#define MISSLE_Y_OFFSET 1
BOOL KMissle::CreateSpecialEffect(eMissleStatus eStatus, int nPX, int nPY, int nPZ, int nNpcIndex)
{
	
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
	m_MissleRes.PlaySound(eStatus, nPX, nPY, 0);
	if (!m_MissleRes.m_MissleRes[eStatus].AnimFileName[0]) return FALSE; 
	pNode = new KSkillSpecialNode;
	KSkillSpecial * pSkillSpecial = new KSkillSpecial;
	pNode->m_pSkillSpecial = pSkillSpecial;
	
	int nSrcX = nPX;
	int nSrcY = nPY;
	
	pSkillSpecial->m_nPX = nSrcX;
	pSkillSpecial->m_nPY = nSrcY - 5;// MISSLE_Y_OFFSET;
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

BOOL	KMissle::CreateMissleForShow(char * szMovie, char * szFormat, char * szSound, TMissleForShow * pShowParam)
{
	if (!pShowParam || !szMovie || !szMovie[0])
		return FALSE;
	int nPX = 0;
	int nPY = 0;
	int nPZ = 0;
	
	if (pShowParam->nNpcIndex > 0)
	{
		Npc[pShowParam->nNpcIndex].GetMpsPos(&nPX, &nPY);
	}
	else
	{
		nPX = pShowParam->nPX;
		nPY = pShowParam->nPY;
	}

	int nSubWorldId = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SubWorldIndex;
	int nMissleIndex = MissleSet.Add(nSubWorldId , nPX , nPY);
	if (nMissleIndex < 0)	
		return FALSE;
	
	Missle[nMissleIndex].m_nDir				= Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_Dir;
	Missle[nMissleIndex].m_nDirIndex		= g_Dir2DirIndex(Missle[nMissleIndex].m_nDir, MaxMissleDir);
	Missle[nMissleIndex].m_nFollowNpcIdx	= 0;
	Missle[nMissleIndex].m_dwBornTime		= SubWorld[nSubWorldId].m_dwCurrentTime;
	Missle[nMissleIndex].m_nSubWorldId		= nSubWorldId;
	Missle[nMissleIndex].m_nLauncher		= pShowParam->nLauncherIndex;
	Missle[nMissleIndex].m_dwLauncherId		= Npc[pShowParam->nLauncherIndex].m_dwID;
	
	Missle[nMissleIndex].m_nParentMissleIndex = 0;
	
	Missle[nMissleIndex].m_nSkillId			= 0;
	Missle[nMissleIndex].m_nStartLifeTime	= 0;
	Missle[nMissleIndex].m_nLifeTime		= 1;
	Missle[nMissleIndex].m_nRefPX			= 0;
	Missle[nMissleIndex].m_nRefPY			= 0;
	Missle[nMissleIndex].m_MissleRes.Clear();

	Missle[nMissleIndex].m_MissleRes.LoadResource(MS_DoWait, szMovie, szSound);
	char * pcszTemp = szFormat;
	Missle[nMissleIndex].m_MissleRes.m_MissleRes[MS_DoWait].nTotalFrame = KSG_StringGetInt(&pcszTemp, 100);
	KSG_StringSkipSymbol(&pcszTemp, ',');
	Missle[nMissleIndex].m_MissleRes.m_MissleRes[MS_DoWait].nDir = KSG_StringGetInt(&pcszTemp, 16);
	KSG_StringSkipSymbol(&pcszTemp, ',');
    Missle[nMissleIndex].m_MissleRes.m_MissleRes[MS_DoWait].nInterval = KSG_StringGetInt(&pcszTemp, 1);

	Missle[nMissleIndex].CreateSpecialEffect(MS_DoWait, nPX, nPY, nPZ, pShowParam->nNpcIndex);
	return TRUE;
}

void	KMissle::GetLightInfo(KLightInfo * pLightInfo)
{
	if (!pLightInfo) 
	{
		return ;
	}
	
	int nPX, nPY, nPZ;
	GetMpsPos(&nPX, &nPY);
	nPZ = m_nCurrentMapZ;
	
	pLightInfo->oPosition.nX = nPX;
	pLightInfo->oPosition.nY = nPY;
	pLightInfo->oPosition.nZ = nPZ;
	pLightInfo->dwColor = 0xff000000 | m_btRedLum << 16 | m_btGreenLum << 8 | m_btBlueLum;
	pLightInfo->nRadius = m_usLightRadius;
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
#endif
	
}
//当子碟进入fly状态时，需要根据情况变动
BOOL	KMissle::PrePareFly()
{
	if (m_eMoveKind == MISSLE_MMK_RollBack)
		m_nTempParam2 =  m_nStartLifeTime + (m_nLifeTime - m_nStartLifeTime ) / 2;

	//是否会随发送者的移动而中断，类式魔兽3中大型法术
	if (m_nInteruptTypeWhenMove)
	{
		int nPX, nPY;
		Npc[m_nLauncher].GetMpsPos(&nPX, &nPY);
		if (nPX != m_nLauncherSrcPX || nPY != m_nLauncherSrcPY)
		{
			return false;
		}
	}
	
	//子碟位置需要更正为到适当的位置（子弹的出现总是以某个可能位置在不断变化的物体为参照物）
	if (m_bHeelAtParent)
	{
		int nNewPX = 0;
		int nNewPY = 0;
		
		if (m_nParentMissleIndex) // 参考点为母子弹
		{
			if (Missle[m_nParentMissleIndex].m_dwLauncherId != m_dwLauncherId)
			{
				return false;
			}
			else
			{
				int nParentPX, nParentPY;
				int nSrcPX, nSrcPY;
				Missle[m_nParentMissleIndex].GetMpsPos(&nParentPX, &nParentPY);
				GetMpsPos(&nSrcPX, &nSrcPY);
				nNewPX = nSrcPX + (nParentPX - m_nRefPX);
				nNewPY = nSrcPY + (nParentPY - m_nRefPY);
			}
		}
		else
			//参考点为发送者
		{
			_ASSERT(m_nLauncher > 0);
			int nParentPX, nParentPY;
			int nSrcPX, nSrcPY;
			
			Npc[m_nLauncher].GetMpsPos(&nParentPX, &nParentPY);
			GetMpsPos(&nSrcPX, &nSrcPY);
			
			nNewPX = nSrcPX + (nParentPX - m_nRefPX);
			nNewPY = nSrcPY + (nParentPY - m_nRefPY);
		}
		
		int nOldRegion = m_nRegionId;
		CurRegion.DecRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
		SubWorld[m_nSubWorldId].Mps2Map(nNewPX, nNewPY, &m_nRegionId, &m_nCurrentMapX, &m_nCurrentMapY, &m_nXOffset, &m_nYOffset);
		CurRegion.AddRef(m_nCurrentMapX, m_nCurrentMapY, obj_missle);
		
		if (nOldRegion != m_nRegionId)
		{
			SubWorld[m_nSubWorldId].m_WorldMessage.Send(GWM_MISSLE_CHANGE_REGION, nOldRegion, m_nRegionId, m_nMissleId);
		} 
		
	}
	
	return true;
	
}

int KMissle::CheckNearestCollision()
{
	int nSearchRegion = 0;
	int nRMx = 0;
	int nRMy = 0;
	BOOL bCollision = TRUE;
	int nNpcIdx = 0;
	int nDX = 0;
	int nDY = 0;
	int nNpcOffsetX = 0;
	int nNpcOffsetY = 0;
	int nAbsX = 0;
	int nAbsY = 0;
	int nCellWidth = CellWidth;
	int nCellHeight = CellHeight;
	_ASSERT(nCellWidth > 0 && nCellHeight > 0);
	
	for (int i = -1; i <= 1; i ++)
		for (int j = -1; j <= 1; j ++)
		{
			if (!KMissle::GetOffsetAxis(
				m_nSubWorldId,
				m_nRegionId, 
				m_nCurrentMapX, 
				m_nCurrentMapY, 
				i , 
				j , 
				nSearchRegion, 
				nRMx, 
				nRMy
				))
				continue;
			
			_ASSERT(nSearchRegion >= 0);
			
			nNpcIdx = SubWorld[m_nSubWorldId].m_Region[nSearchRegion].FindNpc(nRMx, nRMy, m_nLauncher, m_eRelation);
			
			if (nNpcIdx > 0)
			{
				bCollision = TRUE;
				nDX = m_nCurrentMapX - Npc[nNpcIdx].m_MapX;
				nDY = m_nCurrentMapY - Npc[nNpcIdx].m_MapY;
				nNpcOffsetX = Npc[nNpcIdx].m_OffX;
				nNpcOffsetY = Npc[nNpcIdx].m_OffY;
				nAbsX = abs(nDX);
				nAbsY = abs(nDY);
				
				if (nAbsX)
				{
					if (nDX < 0)
					{
						if (nCellWidth - m_nXOffset + nNpcOffsetX > nCellWidth)
						{
							bCollision = FALSE;
							goto CheckCollision;
						}
					}
					else if (nDX > 0)
					{
						if (nCellWidth - nNpcOffsetX + m_nXOffset > nCellWidth)
						{
							bCollision = FALSE;
							goto CheckCollision;
						}
					}
				}
				
				if (nAbsY)
				{
					if (nDY <0)
					{
						if (nCellHeight - m_nYOffset + nNpcOffsetY > nCellHeight)
						{
							bCollision = FALSE;
							goto CheckCollision;
						}
					}
					else if (nDY >0)
					{
						if (nCellHeight - nNpcOffsetY + m_nYOffset > nCellHeight)
						{
							bCollision = FALSE;
							goto CheckCollision;
						}
					}
				}
				
				
CheckCollision:
				if (bCollision)
					return nNpcIdx;
			}
		}
		
		return 0;
}

void	KMissle::GetMpsPos(int *pPosX, int *pPosY)
{
	SubWorld[m_nSubWorldId].Map2Mps(m_nRegionId, m_nCurrentMapX, m_nCurrentMapY, m_nXOffset, m_nYOffset, pPosX, pPosY);
};
