#include "KCore.h"
#include <math.h>
#include "KRegion.h"
#include "KMath.h"
#include "KNpc.h"
#include "KPlayer.h"
#include "KNpcSet.h"
#include "KObjSet.h"
#include "KPlayerSet.h"
#include "KMissleSet.h"
#ifndef _STANDALONE
#include "crtdbg.h"
#endif
#include "Scene/ObstacleDef.h"
#ifdef _SERVER
//#include "KNetServer.h"
//#include "../MultiServer/Heaven/Interface/iServer.h"
#endif
#ifndef _SERVER
#include "scene/KScenePlaceC.h"
#endif
#include "KSubWorld.h"

#ifndef TOOLVERSION
KSubWorld	SubWorld[MAX_SUBWORLD];
#else 
CORE_API KSubWorld	SubWorld[MAX_SUBWORLD];
#endif

#define		defLOGIC_CELL_WIDTH			32
#define		defLOGIC_CELL_HEIGHT		32

#ifdef _SERVER
KGlobalMissionArray g_GlobalMissionArray;
#endif

KSubWorld::KSubWorld()
{
	m_Region = NULL;
	m_nRegionWidth	= 512 / 32;
	m_nRegionHeight	= 1024 / 32;
	m_nCellWidth = defLOGIC_CELL_WIDTH;
	m_nCellHeight = defLOGIC_CELL_HEIGHT;
	m_dwCurrentTime = 0;
	m_SubWorldID = -1;
	m_nWeather = WEATHERID_NOTHING;
#ifndef _SERVER
	m_nWorldRegionWidth = 3;
	m_nWorldRegionHeight = 3;
	memset(m_ClientRegionIdx, 0, sizeof(this->m_ClientRegionIdx));
	memset(this->m_szMapPath, 0, sizeof(this->m_szMapPath));
#endif
#ifdef _SERVER
	m_nWorldRegionWidth = 0;
	m_nWorldRegionHeight = 0;
	m_pWeatherMgr = NULL;
	//MissionArry中0虽然无效，但还是加上去，容错.TotalCount 为空间减1，所以不会越界
	for (int i = 0; i <= m_MissionArray.GetTotalCount(); i ++)
	{
		m_MissionArray.m_Data[i].SetOwner(this);
	}
#endif
	m_nTotalRegion = m_nWorldRegionWidth * m_nWorldRegionHeight;
	
}

KSubWorld::~KSubWorld()
{
	if (m_Region)
	{
		delete [] m_Region;
		m_Region = NULL;
	}

#ifdef _SERVER
	if(m_pWeatherMgr)
	{
		delete m_pWeatherMgr;
		m_pWeatherMgr = NULL;
	}
#endif
}

int KSubWorld::FindRegion(int nRegion)
{
	for (int i = 0; i < m_nTotalRegion; i++)
	{
		if (m_Region[i].m_RegionID == nRegion)
			return i;
	}
	return -1;
}

int KSubWorld::FindFreeRegion(int nX, int nY)
{
	if (nX == 0 && nY == 0)
	{
		for (int i = 0; i < m_nTotalRegion; i++)
		{
			if (m_Region[i].m_RegionID == -1)
				return i;
		}
	}
	else
	{
		for (int i = 0; i < m_nTotalRegion; i++)
		{
			if (m_Region[i].m_RegionID == -1)
				return i;
			int nRegoinX = LOWORD(m_Region[i].m_RegionID);
			int nRegoinY = HIWORD(m_Region[i].m_RegionID);

			if ((nX - nRegoinX) * (nX - nRegoinX) + (nY - nRegoinY) * (nY - nRegoinY) > 2)	// 不在附近
				return i;
		}
	}
	return -1;
}

extern int nActiveRegionCount;

void KSubWorld::Activate()
{
	if (m_SubWorldID == -1)
		return;
	m_dwCurrentTime ++;

#ifndef _SERVER
	g_ScenePlace.SetCurrentTime(m_dwCurrentTime);
	NpcSet.ClearActivateFlagOfAllNpc();
#endif

	for (int i = 0; i < m_nTotalRegion; i++)
	{
		if (m_Region[i].IsActive())
		{
//			g_DebugLog("[Region]%d Activating", i);
			m_Region[i].Activate();
			nActiveRegionCount++;
		}
	}

#ifdef _SERVER
	KIndexNode* pNode = (KIndexNode *)m_NoneRegionNpcList.GetHead();
	while(pNode)
	{
		Npc[pNode->m_nIndex].Activate();
		pNode = (KIndexNode *)pNode->GetNext();
	}

	if(m_pWeatherMgr)
	{
		int nWeather = m_pWeatherMgr->Activate();
		if(m_nWeather != nWeather)
		{
			m_nWeather = nWeather;
			SYNC_WEATHER weatherMsg;
			weatherMsg.ProtocolType = s2c_changeweather;
			weatherMsg.WeatherID = (BYTE)m_nWeather;
			BroadCast((const char*)&weatherMsg, sizeof(SYNC_WEATHER));
		}
	}
	
		m_MissionArray.Activate();

#endif
}

int KSubWorld::GetDistance(int nRx1, int nRy1, int nRx2, int nRy2)
{
	return (int)sqrt(double(nRx1 - nRx2) * (nRx1 - nRx2) + (nRy1 - nRy2) * (nRy1 - nRy2));// Fixed By MrChuCong@gmail.com
}

void KSubWorld::Map2Mps(int nR, int nX, int nY, int nDx, int nDy, int *nRx, int *nRy)
{
/*
#ifdef TOOLVERION
	*nRx = nX;
	*nRy = nY;
	return;
#endif
*/
#ifndef _SERVER
//	_ASSERT(nR >= 0 && nR < 9);
#endif
	//_ASSERT(nR >= 0);

	if (nR < 0 || nR >= m_nTotalRegion)
	{
		*nRx = 0;
		*nRy = 0;
		return;
	}

	int x, y;
	
	x = m_Region[nR].m_nRegionX;
	y = m_Region[nR].m_nRegionY;
	
	x += nX * m_nCellWidth;
	y += nY * m_nCellHeight;
	
	x += (nDx >> 10);
	y += (nDy >> 10);
	
	*nRx = x;
	*nRy = y;
}

void KSubWorld::Map2Mps(int nRx, int nRy, int nX, int nY, int nDx, int nDy, int *pnX, int *pnY)
{
	*pnX = (nRx * REGION_GRID_WIDTH + nX) * defLOGIC_CELL_WIDTH + (nDx >> 10);
	*pnY = (nRy * REGION_GRID_HEIGHT + nY) * defLOGIC_CELL_HEIGHT + (nDy >> 10);
}

void KSubWorld::Mps2Map(int Rx, int Ry, int * nR, int * nX, int * nY, int *nDx, int * nDy)
{
	if (m_nCellWidth == 0 || m_nCellHeight == 0 || m_nRegionWidth == 0 || m_nRegionHeight == 0)
		return;
	int x = Rx / (m_nRegionWidth * m_nCellWidth);
	int	y = Ry / (m_nRegionHeight * m_nCellHeight);

	*nX = 0;
	*nY = 0;
	*nDx = 0;
	*nDy = 0;
#ifdef _SERVER
	// 非法的坐标
	if (x >= m_nWorldRegionWidth + m_nRegionBeginX || y >= m_nWorldRegionHeight + m_nRegionBeginY || x < m_nRegionBeginX || y < m_nRegionBeginY)
	{
		*nR = -1;
		return;
	}
#endif
	// 非法的坐标
#ifdef _SERVER
	*nR = GetRegionIndex(MAKELONG(x, y));
#else
	int nRegionID = MAKELONG(x, y);
	*nR = FindRegion(nRegionID);
	if (*nR == -1)
		return;
#endif
	if (*nR >= m_nTotalRegion)
	{
		*nR = -1;
		return;
	}

	x = Rx - m_Region[*nR].m_nRegionX;
	y = Ry - m_Region[*nR].m_nRegionY;

	*nX = x / m_nCellWidth;
	*nY = y / m_nCellHeight;

	*nDx = (x - *nX * m_nCellWidth) << 10;
	*nDy = (y - *nY * m_nCellHeight) << 10;
}

BYTE	KSubWorld::TestBarrier(int nMpsX, int nMpsY)
{
	if (m_nCellWidth == 0 || m_nCellHeight == 0 || m_nRegionWidth == 0 || m_nRegionHeight == 0)
		return 0xff;

	int x = nMpsX / (m_nRegionWidth * m_nCellWidth);
	int	y = nMpsY / (m_nRegionHeight * m_nCellHeight);
#ifdef _SERVER
	// 非法的坐标
	if (x >= m_nWorldRegionWidth + m_nRegionBeginX || y >= m_nWorldRegionHeight + m_nRegionBeginY || x < m_nRegionBeginX || y < m_nRegionBeginY)
		return 0xff;
	int nRegion = GetRegionIndex(MAKELONG(x, y));
#else
	int nRegion = FindRegion(MAKELONG(x, y));
	if (nRegion == -1)
		return 0xff;
#endif
	if (nRegion >= m_nTotalRegion)
		return 0xff;

	x = nMpsX - m_Region[nRegion].m_nRegionX;
	y = nMpsY - m_Region[nRegion].m_nRegionY;

	int nCellX = x / m_nCellWidth;
	int nCellY = y / m_nCellHeight;

	int nOffX = x - nCellX * m_nCellWidth;
	int nOffY = y - nCellY * m_nCellHeight;

#ifndef _SERVER
	BYTE bRet = (BYTE)g_ScenePlace.GetObstacleInfo(nMpsX, nMpsY);
	if (bRet != Obstacle_NULL)
		return bRet;
	return m_Region[nRegion].GetBarrier(nCellX, nCellY, nOffX, nOffY);
#endif
#ifdef _SERVER
	return m_Region[nRegion].GetBarrier(nCellX, nCellY, nOffX, nOffY);
#endif
}

BYTE KSubWorld::TestBarrier(int nRegion, int nMapX, int nMapY, int nDx, int nDy, int nChangeX, int nChangeY)
{
	int nOldMapX = nMapX;
	int nOldMapY = nMapY;
	int nOldRegion = nRegion;

	nDx += nChangeX;
	nDy += nChangeY;

	if (nDx < 0)
	{
		nDx += (m_nCellWidth << 10);
		nMapX--;
	}
	else if (nDx >= (m_nCellWidth << 10))
	{
		nDx -= (m_nCellWidth << 10);
		nMapX++;
	}
	
	if (nDy < 0)
	{
		nDy += (m_nCellHeight << 10);
		nMapY--;
	}
	else if (nDy >= (m_nCellHeight << 10))
	{
		nDy -= (m_nCellHeight << 10);
		nMapY++;
	}
	
	if (nMapX < 0)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_LEFT] == -1)
			return 0xff;

		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_LEFT];
		nMapX += m_nRegionWidth;
	}
	else if (nMapX >= m_nRegionWidth)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_RIGHT] == -1)
			return 0xff;
		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_RIGHT];
		nMapX -= m_nRegionWidth;
	}
	
	if (nMapY < 0)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_UP] == -1)
			return 0xff;
		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_UP];;
		nMapY += m_nRegionHeight;
	}
	else if (nMapY >= m_nRegionHeight)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_DOWN] == -1)
			return 0xff;
		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_DOWN];
		nMapY -= m_nRegionHeight;
	}

	int nXf, nYf;
	nXf = (nDx >> 10);
	nYf = (nDy >> 10);

#ifdef TOOLVERSION
	return Obstacle_NULL;
#endif
#ifndef _SERVER

	int nMpsX, nMpsY;
	Map2Mps(nRegion, nMapX, nMapY, nDx, nDy, &nMpsX, &nMpsY);
	BYTE bRet = (BYTE)g_ScenePlace.GetObstacleInfo(nMpsX, nMpsY);
	if (bRet != Obstacle_NULL)
		return bRet;
//	if (nMapX == nOldMapX && nMapY == nOldMapY && nRegion == nOldRegion)
//		return Obstacle_NULL;

	return m_Region[nRegion].GetBarrier(nMapX, nMapY, nXf, nYf);
#else
//	if (nMapX == nOldMapX && nMapY == nOldMapY && nRegion == nOldRegion)
//		return 0;
	return m_Region[nRegion].GetBarrier(nMapX, nMapY, nXf, nYf);
#endif
}

BYTE KSubWorld::TestBarrierMin(int nRegion, int nMapX, int nMapY, int nDx, int nDy, int nChangeX, int nChangeY)
{
	int nOldMapX = nMapX;
	int nOldMapY = nMapY;
	int nOldRegion = nRegion;

	nDx += nChangeX;
	nDy += nChangeY;

	if (nDx < 0)
	{
		nDx += (m_nCellWidth << 10);
		nMapX--;
	}
	else if (nDx >= (m_nCellWidth << 10))
	{
		nDx -= (m_nCellWidth << 10);
		nMapX++;
	}
	
	if (nDy < 0)
	{
		nDy += (m_nCellHeight << 10);
		nMapY--;
	}
	else if (nDy >= (m_nCellHeight << 10))
	{
		nDy -= (m_nCellHeight << 10);
		nMapY++;
	}
	
	if (nMapX < 0)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_LEFT] == -1)
			return 0xff;

		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_LEFT];
		nMapX += m_nRegionWidth;
	}
	else if (nMapX >= m_nRegionWidth)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_RIGHT] == -1)
			return 0xff;
		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_RIGHT];
		nMapX -= m_nRegionWidth;
	}
	
	if (nMapY < 0)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_UP] == -1)
			return 0xff;
		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_UP];;
		nMapY += m_nRegionHeight;
	}
	else if (nMapY >= m_nRegionHeight)
	{
		if (m_Region[nRegion].m_nConnectRegion[DIR_DOWN] == -1)
			return 0xff;
		nRegion = m_Region[nRegion].m_nConnectRegion[DIR_DOWN];
		nMapY -= m_nRegionHeight;
	}

#ifndef _SERVER

	int nMpsX, nMpsY;
	Map2Mps(nRegion, nMapX, nMapY, nDx, nDy, &nMpsX, &nMpsY);
	BYTE bRet = (BYTE)g_ScenePlace.GetObstacleInfoMin(nMpsX, nMpsY, nDx & 0x000003ff, nDy & 0x000003ff);
	if (bRet != Obstacle_NULL)
		return bRet;

	if (nMapX == nOldMapX && nMapY == nOldMapY && nRegion == nOldRegion)
		return Obstacle_NULL;
	return m_Region[nRegion].GetBarrierMin(nMapX, nMapY, nDx, nDy, TRUE);

#else

	if (nMapX == nOldMapX && nMapY == nOldMapY && nRegion == nOldRegion)
	{
		return m_Region[nRegion].GetBarrierMin(nMapX, nMapY, nDx, nDy, FALSE);
	}

	return m_Region[nRegion].GetBarrierMin(nMapX, nMapY, nDx, nDy, TRUE);
#endif
}

DWORD KSubWorld::GetTrap(int nMpsX, int nMpsY)
{
	int nRegion, nMapX, nMapY, nOffX, nOffY;
	Mps2Map(nMpsX, nMpsY, &nRegion, &nMapX, &nMapY, &nOffX, &nOffY);
	if (nRegion == -1)
		return 0;
	return m_Region[nRegion].GetTrap(nMapX, nMapY);
}

BYTE KSubWorld::GetBarrier(int nMpsX, int nMpsY)
{
#ifdef _SERVER
	int nRegion, nMapX, nMapY, nOffX, nOffY;
	Mps2Map(nMpsX, nMpsY, &nRegion, &nMapX, &nMapY, &nOffX, &nOffY);
	
	if (nRegion == -1)
		return 0xff;
	return m_Region[nRegion].GetBarrier(nMapX, nMapY, (nOffX >> 10), (nOffY) >> 10);
#else
	return (BYTE)g_ScenePlace.GetObstacleInfo(nMpsX, nMpsY);
#endif
}

#ifdef _SERVER
BOOL KSubWorld::LoadMap(int nId)
{
	KIniFile	IniFile;
	char		szPathName[FILE_NAME_LENGTH];
	char		szFileName[FILE_NAME_LENGTH];
	char		szKeyName[32];

	m_SubWorldID = nId;
	g_SetFilePath("\\settings");
	IniFile.Load("MapList.ini");
	sprintf(szKeyName, "%d", nId);
	IniFile.GetString("List", szKeyName, "", szPathName, sizeof(szPathName));

	g_SetFilePath("\\maps");
	sprintf(szFileName, "%s.wor", szPathName);
	if (!IniFile.Load(szFileName))
		return FALSE;
	
	int nIsInDoor;
	IniFile.GetInteger("MAIN", "IsInDoor", 0, &nIsInDoor);
	if(!nIsInDoor)
	{
		m_pWeatherMgr = new KWeatherMgr;
		m_pWeatherMgr->InitFromIni(IniFile);
	}

	m_nRegionWidth = 512 / 32;
	m_nRegionHeight = 1024 / 32;

	RECT	sRect;
	IniFile.GetRect("MAIN", "rect", &sRect);
	m_nRegionBeginX = sRect.left;
	m_nRegionBeginY = sRect.top;
	m_nWorldRegionWidth = sRect.right - sRect.left + 1;
	m_nWorldRegionHeight = sRect.bottom - sRect.top + 1;
	m_nTotalRegion = m_nWorldRegionWidth * m_nWorldRegionHeight;
	
	m_nCellWidth = defLOGIC_CELL_WIDTH;
	m_nCellHeight = defLOGIC_CELL_HEIGHT;

	if (m_nTotalRegion <= 0 || m_nWorldRegionWidth <= 0 || m_nWorldRegionHeight <= 0)
		return FALSE;

	m_Region = new KRegion[m_nTotalRegion];

	char	szPath[MAX_PATH];
	sprintf(szPath, "\\maps\\%s", szPathName);
	int		nX, nY, nIdx;

	// 加载地图障碍，并连接各Region
	for (nY = 0; nY < m_nWorldRegionHeight; nY++)
	{
		for (nX = 0; nX < m_nWorldRegionWidth; nX++)
		{
			char	szRegionPath[128];

			g_SetFilePath(szPath);
			nIdx = nY * m_nWorldRegionWidth + nX;
			if (m_Region[nIdx].Load(nX + m_nRegionBeginX, nY + m_nRegionBeginY))
			{
				m_Region[nIdx].Init(m_nRegionWidth, m_nRegionHeight);
				m_Region[nIdx].m_nIndex = nIdx;
			}
			for (int i = 0; i < 8; i++)
			{
				short nTmpX, nTmpY;
				nTmpX = (short)LOWORD(m_Region[nIdx].m_nConRegionID[i]) - m_nRegionBeginX;
				nTmpY = (short)HIWORD(m_Region[nIdx].m_nConRegionID[i]) - m_nRegionBeginY;
				if (nTmpX < 0 || nTmpY < 0 || nTmpX >= m_nWorldRegionWidth || nTmpY >= m_nWorldRegionHeight)
				{
					m_Region[nIdx].m_nConnectRegion[i] = -1;
					continue;
				}
				int nConIdx = nTmpY * m_nWorldRegionWidth + nTmpX;
				m_Region[nIdx].m_nConnectRegion[i] = nConIdx;
			}
		}
	}

	sprintf(szPath, "\\maps\\%s", szPathName);
	for (nY = 0; nY < m_nWorldRegionHeight; nY++)
	{
		for (nX = 0; nX < m_nWorldRegionWidth; nX++)
		{
			g_SetFilePath(szPath);
			m_Region[nY * m_nWorldRegionWidth + nX].LoadObject(m_nIndex,nX + m_nRegionBeginX, nY + m_nRegionBeginY);
		}
	}

	return TRUE;
}
#endif

#ifndef _SERVER
BOOL KSubWorld::LoadMap(int nId, int nRegion)
{
	static int	nXOff[8] = {0, -1, -1, -1, 0,  1, 1, 1};
	static int	nYOff[8] = {1, 1,  0,  -1, -1, -1, 0, 1};
	KIniFile	IniFile;

	if (!m_Region)
	{
		m_Region = new KRegion[MAX_REGION];
	}

	if (nId != m_SubWorldID)
	{
		SubWorld[0].Close();
		g_ScenePlace.ClosePlace();
		// NpcSet.RemoveAll(Player[CLIENT_PLAYER_INDEX].m_nIndex); -- later finish it. spe

		char	szKeyName[32], szPathName[FILE_NAME_LENGTH];

		g_SetFilePath("\\settings");
		IniFile.Load("MapList.ini");
		sprintf(szKeyName, "%d", nId);
		IniFile.GetString("List", szKeyName, "", szPathName, sizeof(szPathName));

		sprintf(m_szMapPath, "\\maps\\%s", szPathName);

		g_ScenePlace.OpenPlace(nId);
		m_SubWorldID = nId;
		m_nRegionWidth = KScenePlaceRegionC::RWPP_AREGION_WIDTH / 32;
		m_nRegionHeight = KScenePlaceRegionC::RWPP_AREGION_HEIGHT / 32;
		m_nCellWidth = 32;
		m_nCellHeight = 32;
		Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_RegionIndex = -1;
	}
	int nX = LOWORD(nRegion);
	int nY = HIWORD(nRegion);

	int nIdx = FindRegion(nRegion);

	if (nIdx < 0)
	{
		nIdx = m_ClientRegionIdx[0];

		if (m_Region[nIdx].Load(nX, nY))
		{
			m_Region[nIdx].m_nIndex = nIdx;
			m_Region[nIdx].Init(m_nRegionWidth, m_nRegionHeight);
			m_Region[nIdx].LoadObject(0, nX, nY, m_szMapPath);
		}
	}
	
	g_ScenePlace.SetFocusPosition(m_Region[nIdx].m_nRegionX, m_Region[nIdx].m_nRegionY, 0);
	m_ClientRegionIdx[0] = nIdx;

	for (int i = 0; i < 8; i++)
	{
		int nConIdx;
		nConIdx = FindRegion(m_Region[nIdx].m_nConRegionID[i]);

		if (nConIdx < 0)
		{
			nConIdx = FindFreeRegion(nX, nY);
			_ASSERT(nConIdx >= 0);

			if (m_Region[nConIdx].Load(nX + nXOff[i], nY + nYOff[i]))
			{
				m_Region[nConIdx].m_nIndex = nConIdx;
				m_Region[nConIdx].Init(m_nRegionWidth, m_nRegionHeight);
				m_Region[nConIdx].LoadObject(0, nX + nXOff[i], nY + nYOff[i], m_szMapPath);
			}
			else
			{
				m_Region[nConIdx].m_nIndex = -1;
				m_Region[nConIdx].m_RegionID = -1;
				nConIdx = -1;
			}
		}
		m_ClientRegionIdx[i + 1] = nConIdx;
		m_Region[nIdx].m_nConnectRegion[i] = nConIdx;
	}

	if (m_Region[nIdx].m_nConnectRegion[0] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[0] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[1] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[7] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[4] = nIdx;
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[2] = m_Region[nIdx].m_nConnectRegion[1];
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[3] = m_Region[nIdx].m_nConnectRegion[2];
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[5] = m_Region[nIdx].m_nConnectRegion[6];
		m_Region[m_Region[nIdx].m_nConnectRegion[0]].m_nConnectRegion[6] = m_Region[nIdx].m_nConnectRegion[7];
	}

	if (m_Region[nIdx].m_nConnectRegion[1] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[0] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[1] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[7] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[4] = m_Region[nIdx].m_nConnectRegion[2];
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[2] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[3] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[5] = nIdx;
		m_Region[m_Region[nIdx].m_nConnectRegion[1]].m_nConnectRegion[6] = m_Region[nIdx].m_nConnectRegion[0];
	}

	if (m_Region[nIdx].m_nConnectRegion[2] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[0] = m_Region[nIdx].m_nConnectRegion[1];
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[1] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[7] = m_Region[nIdx].m_nConnectRegion[0];
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[4] = m_Region[nIdx].m_nConnectRegion[3];
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[2] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[3] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[5] = m_Region[nIdx].m_nConnectRegion[4];
		m_Region[m_Region[nIdx].m_nConnectRegion[2]].m_nConnectRegion[6] = nIdx;
	}

	if (m_Region[nIdx].m_nConnectRegion[3] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[0] = m_Region[nIdx].m_nConnectRegion[2];
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[1] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[2] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[3] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[4] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[5] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[6] = m_Region[nIdx].m_nConnectRegion[4];
		m_Region[m_Region[nIdx].m_nConnectRegion[3]].m_nConnectRegion[7] = nIdx;
	}

	if (m_Region[nIdx].m_nConnectRegion[4] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[0] = nIdx;
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[1] = m_Region[nIdx].m_nConnectRegion[2];
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[2] = m_Region[nIdx].m_nConnectRegion[3];
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[3] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[4] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[5] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[6] = m_Region[nIdx].m_nConnectRegion[5];
		m_Region[m_Region[nIdx].m_nConnectRegion[4]].m_nConnectRegion[7] = m_Region[nIdx].m_nConnectRegion[6];
	}

	if (m_Region[nIdx].m_nConnectRegion[5] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[0] = m_Region[nIdx].m_nConnectRegion[6];
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[1] = nIdx;
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[2] = m_Region[nIdx].m_nConnectRegion[4];
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[3] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[4] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[5] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[6] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[5]].m_nConnectRegion[7] = -1;
	}

	if (m_Region[nIdx].m_nConnectRegion[6] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[0] = m_Region[nIdx].m_nConnectRegion[7];
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[1] = m_Region[nIdx].m_nConnectRegion[0];
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[2] = nIdx;
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[3] = m_Region[nIdx].m_nConnectRegion[4];
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[4] = m_Region[nIdx].m_nConnectRegion[5];
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[5] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[6] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[6]].m_nConnectRegion[7] = -1;
	}
	
	if (m_Region[nIdx].m_nConnectRegion[7] >= 0)
	{
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[0] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[1] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[2] = m_Region[nIdx].m_nConnectRegion[0];
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[3] = nIdx;
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[4] = m_Region[nIdx].m_nConnectRegion[6];
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[5] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[6] = -1;
		m_Region[m_Region[nIdx].m_nConnectRegion[7]].m_nConnectRegion[7] = -1;
	}

	return TRUE;
}
#endif

void KSubWorld::LoadTrap()
{
}

void KSubWorld::MessageLoop()
{
	KWorldMsgNode	Msg;
	while(m_WorldMessage.Get(&Msg))
	{
		ProcessMsg(&Msg);
	}
}

void KSubWorld::ProcessMsg(KWorldMsgNode *pMsg)
{
	switch(pMsg->m_dwMsgType)
	{
	case GWM_NPC_DEL:
#ifndef _SERVER
		if (Npc[pMsg->m_nParam[0]].m_RegionIndex >= 0)
		{
			int nIdx = pMsg->m_nParam[0];
			int nSubWorld = Npc[nIdx].m_SubWorldIndex;
			int nRegion = Npc[nIdx].m_RegionIndex;
			SubWorld[nSubWorld].m_Region[nRegion].RemoveNpc(nIdx);
			SubWorld[nSubWorld].m_Region[nRegion].DecRef(Npc[nIdx].m_MapX, Npc[nIdx].m_MapY, obj_npc);
		}
		NpcSet.Remove(pMsg->m_nParam[0]);
#else
		{
			int nIdx = pMsg->m_nParam[0];
			int nSubWorld = Npc[nIdx].m_SubWorldIndex;
			int nRegion = Npc[nIdx].m_RegionIndex;
			
			SubWorld[nSubWorld].m_Region[nRegion].RemoveNpc(nIdx);
			SubWorld[nSubWorld].m_Region[nRegion].DecRef(Npc[nIdx].m_MapX, Npc[nIdx].m_MapY, obj_npc);
			NpcSet.Remove(nIdx);
		}
#endif
		break;
/*	case GWM_NPC_CHANGE_REGION:
		NpcChangeRegion(pMsg->m_nParam[0], pMsg->m_nParam[1], pMsg->m_nParam[2]);
		break;*/
	case GWM_OBJ_DEL:
#ifndef _SERVER
		if (Object[pMsg->m_nParam[0]].m_nRegionIdx >= 0)
			SubWorld[Object[pMsg->m_nParam[0]].m_nSubWorldID].m_Region[Object[pMsg->m_nParam[0]].m_nRegionIdx].RemoveObj(pMsg->m_nParam[0]);
		ObjSet.Remove(pMsg->m_nParam[0]);
#else
		SubWorld[Object[pMsg->m_nParam[0]].m_nSubWorldID].m_Region[Object[pMsg->m_nParam[0]].m_nRegionIdx].RemoveObj(pMsg->m_nParam[0]);
		ObjSet.Remove(pMsg->m_nParam[0]);
#endif
		break;
	case GWM_OBJ_CHANGE_REGION:
		ObjChangeRegion(pMsg->m_nParam[0], pMsg->m_nParam[1], pMsg->m_nParam[2]);
		break;
	case GWM_MISSLE_CHANGE_REGION:
		MissleChangeRegion(pMsg->m_nParam[0], pMsg->m_nParam[1], pMsg->m_nParam[2]);
		break;
	case GWM_MISSLE_DEL:
#ifndef _SERVER
		if (Missle[pMsg->m_nParam[0]].m_nRegionId >= 0)
		{
			SubWorld[Missle[pMsg->m_nParam[0]].m_nSubWorldId].m_Region[Missle[pMsg->m_nParam[0]].m_nRegionId].RemoveMissle(pMsg->m_nParam[0]);
	//		g_DebugLog("[Missle]Missle%dDEL", pMsg->m_nParam[0]);
			MissleSet.Remove(pMsg->m_nParam[0]);
		}
#else
		SubWorld[Missle[pMsg->m_nParam[0]].m_nSubWorldId].m_Region[Missle[pMsg->m_nParam[0]].m_nRegionId].RemoveMissle(pMsg->m_nParam[0]);
		MissleSet.Remove(pMsg->m_nParam[0]);
#endif
		break;
/*	case GWM_PLAYER_CHANGE_REGION:
		PlayerChangeRegion(pMsg->m_nParam[0], pMsg->m_nParam[1], pMsg->m_nParam[2]);
		break;*/
	default:
		break;
	}	
}

#ifdef _SERVER
void KSubWorld::NpcChangeRegion(int nSrcRnidx, int nDesRnIdx, int nIdx)
{
	if (nIdx <= 0 || nIdx >= MAX_NPC)
		return;

	Npc[nIdx].SetActiveFlag(TRUE);
	if (nSrcRnidx == -1)
	{
		_ASSERT(nDesRnIdx >= 0);
		if (nDesRnIdx >= 0)
		{
			m_Region[nDesRnIdx].AddNpc(nIdx);
			Npc[nIdx].m_RegionIndex = nDesRnIdx;
		}
		return;
	}
	else if (nSrcRnidx == VOID_REGION)
	{
		Npc[nIdx].m_Node.Remove();
		Npc[nIdx].m_Node.Release();
	}
	else if (nSrcRnidx >= 0)
	{
		SubWorld[Npc[nIdx].m_SubWorldIndex].m_Region[nSrcRnidx].RemoveNpc(nIdx);
		if (nDesRnIdx != VOID_REGION)	// 不是加入到死亡重生链表
			Npc[nIdx].m_RegionIndex = -1;
	}

	if (nDesRnIdx >= 0)
	{
		m_Region[nDesRnIdx].AddNpc(nIdx);
		Npc[nIdx].m_RegionIndex = nDesRnIdx;
	}
	else if (nDesRnIdx == VOID_REGION)
	{
		m_NoneRegionNpcList.AddTail(&Npc[nIdx].m_Node);
		Npc[nIdx].m_Node.AddRef();
	}
}
#endif

#ifndef _SERVER
void KSubWorld::NpcChangeRegion(int nSrcRnidx, int nDesRnIdx, int nIdx)
{
	int		nSrc, nDest;

	if (nSrcRnidx == -1)
	{
		nDest = SubWorld[0].FindRegion(nDesRnIdx);
		if (nDest < 0)
			return;
		m_Region[nDest].AddNpc(nIdx);
		Npc[nIdx].m_dwRegionID = m_Region[nDest].m_RegionID;
		Npc[nIdx].m_RegionIndex = nDest;
		return;
	}

	nSrc = SubWorld[0].FindRegion(nSrcRnidx);
	if (nSrc >= 0)
		SubWorld[0].m_Region[nSrc].RemoveNpc(nIdx);

	nDest = SubWorld[0].FindRegion(nDesRnIdx);

	KIndexNode *pNode = &Npc[nIdx].m_Node;
	if (nDest >= 0)
	{
		m_Region[nDest].AddNpc(nIdx);
		
		if (Player[CLIENT_PLAYER_INDEX].m_nIndex == nIdx)
			LoadMap(m_SubWorldID, m_Region[nDest].m_RegionID);
		Npc[nIdx].m_dwRegionID = m_Region[nDest].m_RegionID;
		Npc[nIdx].m_RegionIndex = nDest;
	}
	else if (Player[CLIENT_PLAYER_INDEX].m_nIndex == nIdx)
	{
		LoadMap(m_SubWorldID, nDesRnIdx);
		nDest = SubWorld[0].FindRegion(nDesRnIdx);
		if (nDest >= 0)
		{
			m_Region[nDest].AddNpc(nIdx);
			Npc[nIdx].m_dwRegionID = m_Region[nDest].m_RegionID;
			Npc[nIdx].m_RegionIndex = nDest;
		}
	}
}
#endif

void KSubWorld::ObjChangeRegion(int nSrcRnidx, int nDesRnIdx, int nIdx)
{
#ifdef _SERVER
	_ASSERT(nSrcRnidx >= 0);
#endif
	if (nDesRnIdx == -1)
	{
		SubWorld[Object[nIdx].m_nSubWorldID].m_Region[nSrcRnidx].RemoveObj(nIdx);
//		ObjSet.Remove(nIdx);
		return;
	}
	if (nSrcRnidx == -1)
	{
		m_Region[nDesRnIdx].AddObj(nIdx);
		return;
	}
	KIndexNode* pNode = (KIndexNode *)m_Region[nSrcRnidx].GetObjNode(nIdx);
	if (pNode)
	{
		pNode->Remove();
		m_Region[nDesRnIdx].m_ObjList.AddTail(pNode);
	}
}

void KSubWorld::MissleChangeRegion(int nSrcRnidx, int nDesRnIdx, int nIdx)
{
	if (nIdx <= 0 || nIdx >= MAX_MISSLE)
		return;

	if (nSrcRnidx == -1)
	{
		m_Region[nDesRnIdx].AddMissle(nIdx);
		return;
	}

	SubWorld[Missle[nIdx].m_nSubWorldId].m_Region[nSrcRnidx].RemoveMissle(nIdx);

	if (nDesRnIdx == -1)
	{
		MissleSet.Remove(nIdx);
		return;
	}
	
	m_Region[nDesRnIdx].AddMissle(nIdx);
}

#ifdef _SERVER
void KSubWorld::PlayerChangeRegion(int nSrcRnidx, int nDesRnIdx, int nIdx)
{
	if (nIdx <= 0 || nIdx >= MAX_PLAYER)
		return;

	if (nSrcRnidx < 0)
		return;

	RemovePlayer(nSrcRnidx, nIdx);
	
	if (nDesRnIdx == -1)
	{
		//PlayerSet.PrepareRemove(Player[nIdx].m_nNetConnectIdx);
		printf("Player change region to not exist..., kill it\n");
		g_pServer->ShutdownClient(Player[nIdx].m_nNetConnectIdx);
		return;
	}

	if (nDesRnIdx >= 0)
	{
		AddPlayer(nDesRnIdx, nIdx);
	}
/* 因为RemovePlayer和AddPlayer已经做了从链表中清除节点，修改周边九个REGION的引用计数的操作了
	KIndexNode* pNode = &Player[nIdx].m_Node;
	if (pNode->m_Ref > 0)
	{
		pNode->Remove();
		pNode->Release();
	}
	m_Region[nSrcRnidx].m_nActive--;
	if (m_Region[nSrcRnidx].m_nActive < 0)
		m_Region[nSrcRnidx].m_nActive = 0;
	
	for (int i = 0; i < 8; i++)
	{
		if (m_Region[nSrcRnidx].m_nConnectRegion[i] < 0)
			continue;
		m_Region[m_Region[nSrcRnidx].m_nConnectRegion[i]].m_nActive--;
		if (m_Region[m_Region[nSrcRnidx].m_nConnectRegion[i]].m_nActive < 0)
			m_Region[m_Region[nSrcRnidx].m_nConnectRegion[i]].m_nActive = 0;
	}

	if (pNode->m_Ref == 0)
	{
		m_Region[nDesRnIdx].m_PlayerList.AddTail(pNode);
		pNode->AddRef();
	}

	m_Region[nDesRnIdx].m_nActive++;
	for (i = 0; i < 8; i++)
	{
		if (m_Region[nDesRnIdx].m_nConnectRegion[i] < 0)
			continue;
		m_Region[m_Region[nDesRnIdx].m_nConnectRegion[i]].m_nActive++;
		if (m_Region[m_Region[nDesRnIdx].m_nConnectRegion[i]].m_nActive < 0)
			m_Region[m_Region[nDesRnIdx].m_nConnectRegion[i]].m_nActive = 0;
	}
*/
}
#endif

void KSubWorld::GetMps(int *nX, int *nY, int nSpeed, int nDir, int nMaxDir /* = 64 */)
{
	*nX += (g_DirCos(nDir, nMaxDir) * nSpeed) >> 10;
	*nY += (g_DirSin(nDir, nMaxDir) * nSpeed) >> 10;
}
#ifdef _SERVER
BOOL KSubWorld::SendSyncData(int nIdx, int nClient)
{
	WORLD_SYNC	WorldSync;
	WorldSync.ProtocolType = (BYTE)s2c_syncworld;
	WorldSync.Region = m_Region[Npc[nIdx].m_RegionIndex].m_RegionID;
	WorldSync.Weather = m_nWeather;
	WorldSync.Frame = m_dwCurrentTime;
	WorldSync.SubWorld = m_SubWorldID;

	if (SUCCEEDED(g_pServer->PackDataToClient(nClient, (BYTE*)&WorldSync, sizeof(WORLD_SYNC))))
	{
		return TRUE;
	}
	else
	{
		printf("player Packing world sync data failed...\n");
		return FALSE;
	}
}
#endif

#ifndef _SERVER
void KSubWorld::LoadCell()
{
}
#endif

#ifdef TOOLVERSION
int CORE_API g_ScreenX  = 0;
int CORE_API g_ScreenY  = 0;
#endif

#ifndef _SERVER
void KSubWorld::Paint()
{
	int nIdx = Player[CLIENT_PLAYER_INDEX].m_nIndex;

	int nX, nY;

	Map2Mps(Npc[nIdx].m_RegionIndex, Npc[nIdx].m_MapX, Npc[nIdx].m_MapY, Npc[nIdx].m_OffX, Npc[nIdx].m_OffY, &nX, &nY);
	
//	m_nScreenX = nX - 400;
//	m_nScreenY = nY - 300;

#ifdef TOOLVERSION
	g_ScreenX = nX;
	g_ScreenY = nY;
#endif

	for (int i = 0; i < m_nTotalRegion; i++)
	{
		m_Region[i].Paint();
	}
}

void KSubWorld::Mps2Screen(int *Rx, int *Ry)
{
}

void KSubWorld::Screen2Mps(int *Rx, int *Ry)
{
}
#endif

#ifdef _SERVER
int KSubWorld::GetRegionIndex(int nRegionID)
{
	int nRet;
	short nX = (short)LOWORD(nRegionID) - m_nRegionBeginX;
	short nY = (short)HIWORD(nRegionID) - m_nRegionBeginY;

	if (nX < 0 || nY < 0 || nX >= m_nWorldRegionWidth || nY >= m_nWorldRegionHeight)
		return -1;

	nRet = nY * m_nWorldRegionWidth + nX;
	return nRet;
}
#endif

void KSubWorld::Close()
{
	if (m_SubWorldID == -1)
		return;

	for (int i = 0; i < m_nTotalRegion; i++)
	{
		m_Region[i].Close();
	}
	m_WorldMessage.Clear();
	m_nIndex = -1;
	m_SubWorldID = -1;
}

#ifdef _SERVER
void KSubWorld::LoadObject(char* szPath, char* szFileName)
{
	if (!szFileName || !szFileName[0])
		return;

	KIniFile IniFile;
	// 加载世界中的动态物件（Npc、Object、Trap）
	g_SetFilePath(szPath);
	int nObjNumber = 0;
	if (IniFile.Load(szFileName))
	{
		IniFile.GetInteger("MAIN", "elementnum", 0, &nObjNumber);
	}

	int nLength = strlen(szFileName);
	szFileName[nLength - 4] = 0;
	strcat(szPath, "\\");
	strcat(szPath, szFileName);

	char	szSection[32];
	char	szDataFile[32];

	for (int i = 0; i < nObjNumber; i++)
	{
		float	fPos[3];
		int		nPos[3];

		g_SetFilePath(szPath);
		sprintf(szSection, "%d", i);
		IniFile.GetFloat3(szSection, "groundoffset", fPos);
		nPos[0] = (int)(fPos[0] * 32);
		nPos[1] = (int)(fPos[1] * 32);
		nPos[2] = (int)fPos[2];

		szDataFile[0] = 0;
		IniFile.GetString(szSection, "event", "", szDataFile, sizeof(szDataFile));
		strcat(szDataFile, "_data.ini");
		
		KIniFile IniChange;
		int nType = 0, nLevel = 0;
		char	szName[32];
		
		if (!IniChange.Load(szDataFile))
			continue;
		
		IniChange.GetInteger("MAIN", "Type", 0, &nType);
		IniChange.GetInteger("MAIN", "Level", 4, &nLevel);
		IniChange.GetString("MAIN", "mapedit_templatesection", "", szName, sizeof(szName));
		
		switch(nType)
		{
		case kind_normal:		// 普通战斗npc
			{
				int nFindNo = g_NpcSetting.FindRow(szName);
				if (nFindNo == -1)
					continue;
				else
					nFindNo = nFindNo - 2;
				
				int nSettingInfo = MAKELONG(nLevel, nFindNo);
				NpcSet.Add(nSettingInfo, m_nIndex, nPos[0], nPos[1]);
			}
			break;
		case kind_player:
			break;
		case kind_partner:
			break;
		case kind_dialoger:		// 普通非战斗npc
			break;
		case kind_bird:			// 客户端only
			break;
		case kind_mouse:		// 客户端only
			break;
		default:
			break;
		}
	}
}
#endif

void KSubWorld::AddPlayer(int nRegion, int nIdx)
{
	if (nRegion < 0 || nRegion >= m_nTotalRegion)
	{
		printf("Region:(%d) Player(%d)\n", nRegion, nIdx);
		return;
	}

	if (m_Region[nRegion].AddPlayer(nIdx))
	{
		m_Region[nRegion].m_nActive++;

		for (int i = 0; i < 8; i++)
		{
			int nConRegion = m_Region[nRegion].m_nConnectRegion[i];
			if (nConRegion == -1)
				continue;
			
			m_Region[nConRegion].m_nActive++;
		}
	}
	else
	{
		printf("Region:(%d) Player(%d)\n", nRegion, nIdx);
	}
}

void KSubWorld::RemovePlayer(int nRegion, int nIdx)
{
	if (nRegion < 0 || nRegion >= m_nTotalRegion)
		return;

	if (m_Region[nRegion].RemovePlayer(nIdx))
	{	
		m_Region[nRegion].m_nActive--;		

		if (m_Region[nRegion].m_nActive < 0)
		{
			_ASSERT(0);
			m_Region[nRegion].m_nActive = 0;
		}
		
		for (int i = 0; i < 8; i++)
		{
			int nConRegion = m_Region[nRegion].m_nConnectRegion[i];
			if (nConRegion == -1)
				continue;
			
			m_Region[nConRegion].m_nActive--;

			if (m_Region[nConRegion].m_nActive < 0)
			{
				_ASSERT(0);
				m_Region[nConRegion].m_nActive = 0;
			}
		}
	}
}

void KSubWorld::GetFreeObjPos(POINT& pos)
{
	POINT	posLocal = pos;
	POINT	posTemp;
	int nLayer = 1;

	if (CanPutObj(posLocal))
		return;

	while(1)
	{
		for (int i = 0; i <= nLayer; i++)
		{
			posTemp.y = posLocal.y + i * 32;
			posTemp.x = posLocal.x + (nLayer - i) * 32;
			if (CanPutObj(posTemp))
			{
				pos = posTemp;
				return;
			}

			posTemp.y = posLocal.y + i * 32;
			posTemp.x = posLocal.x - (nLayer - i) * 32;
			if (CanPutObj(posTemp))
			{
				pos = posTemp;
				return;
			}

			posTemp.y = posLocal.y - i * 32;
			posTemp.x = posLocal.x + (nLayer - i) * 32;
			if (CanPutObj(posTemp))
			{
				pos = posTemp;
				return;
			}

			posTemp.y = posLocal.y - i * 32;
			posTemp.x = posLocal.x - (nLayer - i) * 32;
			if (CanPutObj(posTemp))
			{
				pos = posTemp;
				return;
			}
		}
		nLayer++;
		if (nLayer >= 10)
			break;
	}
	return;
}

BOOL KSubWorld::CanPutObj(POINT pos)
{
	int nRegion, nMapX, nMapY, nOffX, nOffY;
	Mps2Map(pos.x, pos.y, &nRegion, &nMapX, &nMapY, &nOffX, &nOffY);

	if (nRegion >= 0 
		&& !m_Region[nRegion].GetBarrier(nMapX, nMapY, nOffX, nOffY) 
		&& !m_Region[nRegion].GetRef(nMapX, nMapY, obj_object))
		return TRUE;
	return FALSE;
}

#ifdef _SERVER
void KSubWorld::BroadCast(const char* pBuffer, size_t uSize)
{
	int nIdx = PlayerSet.GetFirstPlayer();
	while(nIdx)
	{
		if (Player[nIdx].m_nNetConnectIdx >= 0 
			&& Player[nIdx].m_nIndex > 0 
			&& Npc[Player[nIdx].m_nIndex].m_SubWorldIndex == m_nIndex)
		{
			g_pServer->PackDataToClient(Player[nIdx].m_nNetConnectIdx, pBuffer, uSize);
		}
		nIdx = PlayerSet.GetNextPlayer();
	}
}

int	KSubWorld::RevivalAllNpc()
{
	KIndexNode * pNode		= NULL;
	size_t	ulCount = 0;
	for (int i = 0; i < m_nTotalRegion; i++)
	{
		KRegion * pCurRegion = &m_Region[i];
		pNode = (KIndexNode *)pCurRegion->m_NpcList.GetHead();
		
		while(pNode)
		{
			int nNpcIdx = pNode->m_nIndex;
			if (!Npc[nNpcIdx].IsPlayer())
			{
				Npc[nNpcIdx].ExecuteRevive();
			}
			pNode = (KIndexNode *)pNode->GetNext();	
		}
	}
	
	pNode = (KIndexNode*)m_NoneRegionNpcList.GetHead();
	
	while(pNode)
	{
		int nNpcIdx = pNode->m_nIndex;
		if (!Npc[nNpcIdx].IsPlayer())
		{
			Npc[nNpcIdx].m_Frames.nTotalFrame = 1;
			Npc[nNpcIdx].m_Frames.nCurrentFrame = 0;
			ulCount ++;
		}
		pNode = (KIndexNode *)pNode->GetNext();	
	}
	
	return ulCount;
}

int KSubWorld::FindNpcFromName(const char * szName)
{
	if (!szName || !szName[0])
		return 0;
	
	KIndexNode * pNode		= NULL;

	int nResult = 0;
	
	for (int i = 0; i < m_nTotalRegion; i++)
	{
		KRegion * pCurRegion = &m_Region[i];
		pNode = (KIndexNode *)pCurRegion->m_NpcList.GetHead();
		
		while(pNode)
		{
			int nNpcIdx = pNode->m_nIndex;
			if (!strcmp(Npc[nNpcIdx].Name, szName))
			{
				nResult = nNpcIdx;
				return nResult;
			}
			pNode = (KIndexNode *)pNode->GetNext();	
		}
	}	
	
	pNode = (KIndexNode*)m_NoneRegionNpcList.GetHead();
	
	while(pNode)
	{
		int nNpcIdx = pNode->m_nIndex;
		if (!strcmp(Npc[nNpcIdx].Name, szName))
		{
			return nNpcIdx;
		}
		pNode = (KIndexNode *)pNode->GetNext();	
	}
	
	return nResult;
}
#endif
