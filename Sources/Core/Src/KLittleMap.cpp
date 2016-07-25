//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KLittleMap.cpp
// Date:	2003.05.08
// Code:	边城浪子
//---------------------------------------------------------------------------
#include "KCore.h"

#ifndef _SERVER

#include	"ImgRef.h"
#include	"KSubWorld.h"
#include	"KRegion.h"
#include	"KLittleMap.h"
#include	"KPlayer.h"
#include    "../../Engine/Src/KSG_StringProcess.h"

//---------------------------------------------------------------------------
//	功能：构造函数
//---------------------------------------------------------------------------
KLittleMap::KLittleMap()
{
	memset(&m_sMapRect, 0, sizeof(this->m_sMapRect));
	m_nMapWidth = 0;
	m_nMapHeight = 0;
	m_pbyLoadFlag =	NULL;
	m_ppbtBarrier = NULL;

	m_nScreenX = 500;
	m_nScreenY = 100;
	m_nShowWidth = LITTLE_MAP_SHOW_REGION_WIDTH;
	m_nShowHeight = LITTLE_MAP_SHOW_REGION_HEIGHT;

	m_sBarrierColor.Color_dw	= 0xffffff00;
	m_sSelfColor.Color_dw		= 0x00ff00ff;
	m_sTeammateColor.Color_dw	= 0x00ff00ff;
	m_sPlayerColor.Color_dw		= 0x00ff00ff;
	m_sFightNpcColor.Color_dw	= 0x00ff00ff;
	m_sNormalNpcColor.Color_dw	= 0x00ff00ff;
	m_bTeammateShowFlag			= TRUE;
	m_bPlayerShowFlag			= TRUE;
	m_bFightNpcShowFlag			= TRUE;
	m_bNormalNpcShowFlag		= TRUE;
	m_bColorLoadFlag = FALSE;
}

//---------------------------------------------------------------------------
//	功能：析构函数
//---------------------------------------------------------------------------
KLittleMap::~KLittleMap()
{
	Release();
	m_nShowFlag = 0;
}

//---------------------------------------------------------------------------
//	功能：清空
//---------------------------------------------------------------------------
void	KLittleMap::Release()
{
	if (m_ppbtBarrier)
	{
		for (int i = 0; i < m_nMapWidth * m_nMapHeight; i++)
		{
			if (!m_ppbtBarrier[i])
				continue;
			delete []m_ppbtBarrier[i];
			m_ppbtBarrier[i] = NULL;
		}
		delete []m_ppbtBarrier;
		m_ppbtBarrier = NULL;
	}

	if (m_pbyLoadFlag)
	{
		delete []m_pbyLoadFlag;
		m_pbyLoadFlag = NULL;
	}

	memset(&m_sMapRect, 0, sizeof(this->m_sMapRect));
	m_nMapWidth = 0;
	m_nMapHeight = 0;

//	m_nShowFlag = 0;
}

//---------------------------------------------------------------------------
//	功能：初始化，分配内存空间
//---------------------------------------------------------------------------
void	KLittleMap::Init(int nLeft, int nTop, int nRight, int nBottom)
{
	Release();

	if (nRight < nLeft || nBottom < nTop)
		return;

	this->m_sMapRect.left	= nLeft;
	this->m_sMapRect.top	= nTop;
	this->m_sMapRect.right	= nRight;
	this->m_sMapRect.bottom	= nBottom;
	this->m_nMapWidth		= nRight - nLeft + 1;
	this->m_nMapHeight		= nBottom - nTop + 1;

	this->m_pbyLoadFlag = (BYTE*)new BYTE[m_nMapWidth * m_nMapHeight];
	memset(m_pbyLoadFlag, 0, sizeof(BYTE) * m_nMapWidth * m_nMapHeight);

	this->m_ppbtBarrier = (BYTE**)new LPVOID[m_nMapWidth * m_nMapHeight];
	for (int i = 0; i < m_nMapWidth * m_nMapHeight; i++)
	{
		this->m_ppbtBarrier[i] = NULL;
	}

	if (m_bColorLoadFlag == FALSE)
	{
		KIniFile	cColorIni;
		int			nR, nG, nB;
		char		szTemp[32];
        const char *pcszTemp = NULL;

		m_bColorLoadFlag = TRUE;
//		g_SetFilePath("\\");
		if (cColorIni.Load(defLITTLE_MAP_SET_FILE))
		{
			cColorIni.GetString("MapColor", "MapColor", "255,255,255", szTemp, sizeof(szTemp));
            
            pcszTemp = szTemp;
            nR = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nG = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nB = KSG_StringGetInt(&pcszTemp, 255);
			//sscanf(szTemp, "%d,%d,%d", &nR, &nG, &nB);

			this->m_sBarrierColor.Color_b.r = (BYTE)nR;
			this->m_sBarrierColor.Color_b.g = (BYTE)nG;
			this->m_sBarrierColor.Color_b.b = (BYTE)nB;
			this->m_sBarrierColor.Color_b.a = 0;

			cColorIni.GetString("MapColor", "SelfColor", "255,255,255", szTemp, sizeof(szTemp));

            pcszTemp = szTemp;
            nR = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nG = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nB = KSG_StringGetInt(&pcszTemp, 255);
			//sscanf(szTemp, "%d,%d,%d", &nR, &nG, &nB);

			this->m_sSelfColor.Color_b.r = (BYTE)nR;
			this->m_sSelfColor.Color_b.g = (BYTE)nG;
			this->m_sSelfColor.Color_b.b = (BYTE)nB;
			this->m_sSelfColor.Color_b.a = 0;

			cColorIni.GetInteger("MapColor", "TeammateShow", 1, &this->m_bTeammateShowFlag);
			cColorIni.GetString("MapColor", "TeammateColor", "255,255,255", szTemp, sizeof(szTemp));

            pcszTemp = szTemp;
            nR = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nG = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nB = KSG_StringGetInt(&pcszTemp, 255);
			//sscanf(szTemp, "%d,%d,%d", &nR, &nG, &nB);

			this->m_sTeammateColor.Color_b.r = (BYTE)nR;
			this->m_sTeammateColor.Color_b.g = (BYTE)nG;
			this->m_sTeammateColor.Color_b.b = (BYTE)nB;
			this->m_sTeammateColor.Color_b.a = 0;

			cColorIni.GetInteger("MapColor", "PlayerShow", 1, &this->m_bPlayerShowFlag);
			cColorIni.GetString("MapColor", "PlayerColor", "255,255,255", szTemp, sizeof(szTemp));

            pcszTemp = szTemp;
            nR = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nG = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nB = KSG_StringGetInt(&pcszTemp, 255);
			//sscanf(szTemp, "%d,%d,%d", &nR, &nG, &nB);

			this->m_sPlayerColor.Color_b.r = (BYTE)nR;
			this->m_sPlayerColor.Color_b.g = (BYTE)nG;
			this->m_sPlayerColor.Color_b.b = (BYTE)nB;
			this->m_sPlayerColor.Color_b.a = 0;

			cColorIni.GetInteger("MapColor", "FightNpcShow", 1, &this->m_bFightNpcShowFlag);
			cColorIni.GetString("MapColor", "FightNpcColor", "255,255,255", szTemp, sizeof(szTemp));

            pcszTemp = szTemp;
            nR = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nG = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nB = KSG_StringGetInt(&pcszTemp, 255);
			//sscanf(szTemp, "%d,%d,%d", &nR, &nG, &nB);

			this->m_sFightNpcColor.Color_b.r = (BYTE)nR;
			this->m_sFightNpcColor.Color_b.g = (BYTE)nG;
			this->m_sFightNpcColor.Color_b.b = (BYTE)nB;
			this->m_sFightNpcColor.Color_b.a = 0;

			cColorIni.GetInteger("MapColor", "NormalNpcShow", 1, &this->m_bNormalNpcShowFlag);
			cColorIni.GetString("MapColor", "NormalNpcColor", "255,255,255", szTemp, sizeof(szTemp));

            pcszTemp = szTemp;
            nR = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nG = KSG_StringGetInt(&pcszTemp, 255);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            nB = KSG_StringGetInt(&pcszTemp, 255);
			//sscanf(szTemp, "%d,%d,%d", &nR, &nG, &nB);

			this->m_sNormalNpcColor.Color_b.r = (BYTE)nR;
			this->m_sNormalNpcColor.Color_b.g = (BYTE)nG;
			this->m_sNormalNpcColor.Color_b.b = (BYTE)nB;
			this->m_sNormalNpcColor.Color_b.a = 0;
		}
	}
}

//---------------------------------------------------------------------------
//	功能：获得某个region的障碍数据的存储地址
//---------------------------------------------------------------------------
BYTE*	KLittleMap::GetBarrierBuf(int nX, int nY)
{
	if (nX < m_sMapRect.left ||
		nX > m_sMapRect.right ||
		nY < m_sMapRect.top ||
		nY > m_sMapRect.bottom)
		return NULL;
	if (!m_pbyLoadFlag || !m_ppbtBarrier)
		return NULL;
	int	nRegion = (nY - m_sMapRect.top) * m_nMapWidth + nX - m_sMapRect.left;
	if (m_pbyLoadFlag[nRegion])
		return NULL;
	if (!m_ppbtBarrier[nRegion])
	{
		m_ppbtBarrier[nRegion] = (BYTE*)new BYTE[REGION_GRID_WIDTH * REGION_GRID_HEIGHT];
		memset(m_ppbtBarrier[nRegion], 0, sizeof(BYTE) * REGION_GRID_WIDTH * REGION_GRID_HEIGHT);
	}
	return m_ppbtBarrier[nRegion];
}

//---------------------------------------------------------------------------
//	功能：设定已载入某个region的障碍数据
//---------------------------------------------------------------------------
void	KLittleMap::SetHaveLoad(int nX, int nY)
{
	if (nX < m_sMapRect.left ||
		nX > m_sMapRect.right ||
		nY < m_sMapRect.top ||
		nY > m_sMapRect.bottom)
		return;
	if (m_pbyLoadFlag)
		m_pbyLoadFlag[(nY - m_sMapRect.top) * m_nMapWidth + nX - m_sMapRect.left] = 1;
}

//---------------------------------------------------------------------------
//	功能：绘制
//---------------------------------------------------------------------------
void	KLittleMap::Draw(int nX, int nY)
{
	if (!m_nShowFlag)
		return;
	if (!this->m_pbyLoadFlag || !this->m_ppbtBarrier)
		return;

	int		nRx, nRy, nOx, nOy, nSx, nSy, nTx, nTy;

	nSx = nX - REGION_GRID_WIDTH * m_nShowWidth / 2;
	nSy = nY - REGION_GRID_HEIGHT * m_nShowHeight / 2;

	// 显示障碍
	m_nPointPos = 0;
	for (int i = 0; i < m_nShowWidth * REGION_GRID_WIDTH; i++)
	{
		for (int j = 0; j < m_nShowHeight * REGION_GRID_HEIGHT; j++)
		{
			nRx = (nSx + i) / REGION_GRID_WIDTH;
			nRy = (nSy + j) / REGION_GRID_HEIGHT;
			if (nRx < m_sMapRect.left ||
				nRx > m_sMapRect.right ||
				nRy < m_sMapRect.top ||
				nRy > m_sMapRect.bottom)
				continue;
			nRx -= m_sMapRect.left;
			nRy -= m_sMapRect.top;
			if (!m_pbyLoadFlag[nRy * m_nMapWidth + nRx] || !m_ppbtBarrier[nRy * m_nMapWidth + nRx])
				continue;
			nOx = (nSx + i) % REGION_GRID_WIDTH;
			nOy = (nSy + j) % REGION_GRID_HEIGHT;
			if (m_ppbtBarrier[nRy * m_nMapWidth + nRx][nOy * REGION_GRID_WIDTH + nOx])
			{
				m_sPoint[m_nPointPos].Color.Color_dw = m_sBarrierColor.Color_dw;
				m_sPoint[m_nPointPos].oPosition.nZ = 0;
				m_sPoint[m_nPointPos].oPosition.nX = m_nScreenX + i * 2;
				m_sPoint[m_nPointPos].oPosition.nY = m_nScreenY + j;
				m_nPointPos++;
				if (m_nPointPos == LITTLE_MAP_POINT_NUM)
				{
					g_pRepresent->DrawPrimitives(m_nPointPos, m_sPoint, RU_T_POINT, true);
					m_nPointPos = 0;
				}
			}
		}
	}
	if (m_nPointPos > 0 && m_nPointPos < LITTLE_MAP_POINT_NUM)
	{
		g_pRepresent->DrawPrimitives(m_nPointPos, m_sPoint, RU_T_POINT, true);
		m_nPointPos = 0;
	}

	int		nNpcIdx;

	// 显示其他玩家和普通npc
	nNpcIdx = 0;
	while (1)
	{
		nNpcIdx = NpcSet.GetNextIdx(nNpcIdx);
		if (nNpcIdx == 0)
			break;
		if (Npc[nNpcIdx].m_RegionIndex == -1)
			continue;
		if (Npc[nNpcIdx].m_Kind == kind_normal)
		{
			if (!m_bFightNpcShowFlag)
				continue;
			nRx = LOWORD(Npc[nNpcIdx].m_dwRegionID);
			nRy = HIWORD(Npc[nNpcIdx].m_dwRegionID);
			if (nRx < m_sMapRect.left || nRx > m_sMapRect.right || nRy < m_sMapRect.top || nRy > m_sMapRect.bottom)
				continue;
			nOx = Npc[nNpcIdx].m_MapX;
			nOy = Npc[nNpcIdx].m_MapY;
			nTx = nRx * REGION_GRID_WIDTH + nOx;
			nTy = nRy * REGION_GRID_HEIGHT + nOy;

			m_sBigPoint.Color.Color_dw = m_sFightNpcColor.Color_dw;
			m_sBigPoint.oPosition.nX = m_nScreenX + (nTx - nSx) * 2 - 1;
			m_sBigPoint.oPosition.nY = m_nScreenY + nTy - nSy - 1;
			m_sBigPoint.oEndPos.nX = m_sBigPoint.oPosition.nX + 3;
			m_sBigPoint.oEndPos.nY = m_sBigPoint.oPosition.nY + 3;
			g_pRepresent->DrawPrimitives(1, &m_sBigPoint, RU_T_SHADOW, true);
		}
		else if (Npc[nNpcIdx].m_Kind == kind_dialoger)
		{
			if (!m_bNormalNpcShowFlag)
				continue;
			nRx = LOWORD(Npc[nNpcIdx].m_dwRegionID);
			nRy = HIWORD(Npc[nNpcIdx].m_dwRegionID);
			if (nRx < m_sMapRect.left || nRx > m_sMapRect.right || nRy < m_sMapRect.top || nRy > m_sMapRect.bottom)
				continue;
			nOx = Npc[nNpcIdx].m_MapX;
			nOy = Npc[nNpcIdx].m_MapY;
			nTx = nRx * REGION_GRID_WIDTH + nOx;
			nTy = nRy * REGION_GRID_HEIGHT + nOy;

			m_sBigPoint.Color.Color_dw = m_sNormalNpcColor.Color_dw;
			m_sBigPoint.oPosition.nX = m_nScreenX + (nTx - nSx) * 2 - 1;
			m_sBigPoint.oPosition.nY = m_nScreenY + nTy - nSy - 1;
			m_sBigPoint.oEndPos.nX = m_sBigPoint.oPosition.nX + 3;
			m_sBigPoint.oEndPos.nY = m_sBigPoint.oPosition.nY + 3;
			g_pRepresent->DrawPrimitives(1, &m_sBigPoint, RU_T_SHADOW, true);
		}
		else if (Npc[nNpcIdx].m_Kind == kind_player)
		{
			if (!m_bPlayerShowFlag)
				continue;
			if (nNpcIdx == Player[CLIENT_PLAYER_INDEX].m_nIndex)
				continue;
			if (m_bTeammateShowFlag &&
				Player[CLIENT_PLAYER_INDEX].m_cTeam.m_nFlag &&
				((DWORD)g_Team[0].m_nCaptain == Npc[nNpcIdx].m_dwID || g_Team[0].FindMemberID(Npc[nNpcIdx].m_dwID) >= 0))
				continue;
			nRx = LOWORD(Npc[nNpcIdx].m_dwRegionID);
			nRy = HIWORD(Npc[nNpcIdx].m_dwRegionID);
			if (nRx < m_sMapRect.left || nRx > m_sMapRect.right || nRy < m_sMapRect.top || nRy > m_sMapRect.bottom)
				continue;
			nOx = Npc[nNpcIdx].m_MapX;
			nOy = Npc[nNpcIdx].m_MapY;
			nTx = nRx * REGION_GRID_WIDTH + nOx;
			nTy = nRy * REGION_GRID_HEIGHT + nOy;

			m_sBigPoint.Color.Color_dw = m_sPlayerColor.Color_dw;
			m_sBigPoint.oPosition.nX = m_nScreenX + (nTx - nSx) * 2 - 1;
			m_sBigPoint.oPosition.nY = m_nScreenY + nTy - nSy - 1;
			m_sBigPoint.oEndPos.nX = m_sBigPoint.oPosition.nX + 3;
			m_sBigPoint.oEndPos.nY = m_sBigPoint.oPosition.nY + 3;
			g_pRepresent->DrawPrimitives(1, &m_sBigPoint, RU_T_SHADOW, true);
		}
	}

	// 显示队友
	if (Player[CLIENT_PLAYER_INDEX].m_cTeam.m_nFlag && m_bTeammateShowFlag)
	{
		// 队长
		if ((DWORD)g_Team[0].m_nCaptain != Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwID)
		{
			nNpcIdx = NpcSet.SearchID(g_Team[0].m_nCaptain);
			if (nNpcIdx > 0 && Npc[nNpcIdx].m_RegionIndex != -1)
			{
				nRx = LOWORD(Npc[nNpcIdx].m_dwRegionID);
				nRy = HIWORD(Npc[nNpcIdx].m_dwRegionID);
				if (nRx >= m_sMapRect.left && nRx <= m_sMapRect.right && nRy >= m_sMapRect.top && nRy <= m_sMapRect.bottom)
				{
					nOx = Npc[nNpcIdx].m_MapX;
					nOy = Npc[nNpcIdx].m_MapY;
					nTx = nRx * REGION_GRID_WIDTH + nOx;
					nTy = nRy * REGION_GRID_HEIGHT + nOy;

					m_sBigPoint.Color.Color_dw = m_sTeammateColor.Color_dw;
					m_sBigPoint.oPosition.nX = m_nScreenX + (nTx - nSx) * 2 - 1;
					m_sBigPoint.oPosition.nY = m_nScreenY + nTy - nSy - 1;
					m_sBigPoint.oEndPos.nX = m_sBigPoint.oPosition.nX + 3;
					m_sBigPoint.oEndPos.nY = m_sBigPoint.oPosition.nY + 3;
					g_pRepresent->DrawPrimitives(1, &m_sBigPoint, RU_T_SHADOW, true);
				}
			}
		}
		// 队友
		for (i = 0; i < MAX_TEAM_MEMBER; i++)
		{
			if (g_Team[0].m_nMember[i] <= 0 || (DWORD)g_Team[0].m_nMember[i] == Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwID)
				continue;
			nNpcIdx = NpcSet.SearchID(g_Team[0].m_nMember[i]);
			if (nNpcIdx <= 0 || Npc[nNpcIdx].m_RegionIndex == -1)
				continue;
			nRx = LOWORD(Npc[nNpcIdx].m_dwRegionID);
			nRy = HIWORD(Npc[nNpcIdx].m_dwRegionID);
			if (nRx < m_sMapRect.left || nRx > m_sMapRect.right || nRy < m_sMapRect.top || nRy > m_sMapRect.bottom)
				continue;
			nOx = Npc[nNpcIdx].m_MapX;
			nOy = Npc[nNpcIdx].m_MapY;
			nTx = nRx * REGION_GRID_WIDTH + nOx;
			nTy = nRy * REGION_GRID_HEIGHT + nOy;

			m_sBigPoint.Color.Color_dw = m_sTeammateColor.Color_dw;
			m_sBigPoint.oPosition.nX = m_nScreenX + (nTx - nSx) * 2 - 1;
			m_sBigPoint.oPosition.nY = m_nScreenY + nTy - nSy - 1;
			m_sBigPoint.oEndPos.nX = m_sBigPoint.oPosition.nX + 3;
			m_sBigPoint.oEndPos.nY = m_sBigPoint.oPosition.nY + 3;
			g_pRepresent->DrawPrimitives(1, &m_sBigPoint, RU_T_SHADOW, true);
		}
	}

	// 显示主角位置
	nRx = nX / REGION_GRID_WIDTH;
	nRy = nY / REGION_GRID_HEIGHT;
	if (nRx < m_sMapRect.left || nRx > m_sMapRect.right || nRy < m_sMapRect.top || nRy > m_sMapRect.bottom)
		return;
	nRx -= m_sMapRect.left;
	nRy -= m_sMapRect.top;
	nOx = nX % REGION_GRID_WIDTH;
	nOy = nY % REGION_GRID_HEIGHT;

	m_sBigPoint.Color.Color_dw = m_sSelfColor.Color_dw;
	m_sBigPoint.oPosition.nX = m_nScreenX + (nX - nSx) * 2 - 1;
	m_sBigPoint.oPosition.nY = m_nScreenY + nY - nSy - 1;
	m_sBigPoint.oEndPos.nX = m_sBigPoint.oPosition.nX + 3;
	m_sBigPoint.oEndPos.nY = m_sBigPoint.oPosition.nY + 3;
	g_pRepresent->DrawPrimitives(1, &m_sBigPoint, RU_T_SHADOW, true);
}

//---------------------------------------------------------------------------
//	功能：队友显示开关
//---------------------------------------------------------------------------
void	KLittleMap::SetTeammateShow(BOOL bFlag)
{
	this->m_bTeammateShowFlag = bFlag;
}

//---------------------------------------------------------------------------
//	功能：其它玩家显示开关
//---------------------------------------------------------------------------
void	KLittleMap::SetPlayerShow(BOOL bFlag)
{
	this->m_bPlayerShowFlag = bFlag;
}

//---------------------------------------------------------------------------
//	功能：战斗npc显示开关
//---------------------------------------------------------------------------
void	KLittleMap::SetFightNpcShow(BOOL bFlag)
{
	this->m_bFightNpcShowFlag = bFlag;
}

//---------------------------------------------------------------------------
//	功能：普通npc显示开关
//---------------------------------------------------------------------------
void	KLittleMap::SetNormalNpcShow(BOOL bFlag)
{
	this->m_bNormalNpcShowFlag = bFlag;
}

//---------------------------------------------------------------------------
//	功能：设定小地图在屏幕上的显示位置
//---------------------------------------------------------------------------
void	KLittleMap::SetScreenPos(int nX, int nY)
{
	m_nScreenX = nX;
	m_nScreenY = nY;
}

//---------------------------------------------------------------------------
//	功能：小地图显示开关
//---------------------------------------------------------------------------
void	KLittleMap::Show(int nFlag)
{
	this->m_nShowFlag = nFlag;
}

//---------------------------------------------------------------------------
//	功能：设定显示内容的大小
//---------------------------------------------------------------------------
void	KLittleMap::SetShowSize(int nWidth, int nHeight)
{
	nWidth /= REGION_GRID_WIDTH * 2;
	nHeight /= REGION_GRID_HEIGHT;
	if (nWidth < 3)
		nWidth = 3;
	if (nHeight < 3)
		nHeight = 3;
	if (nWidth > 20)
		nWidth = 20;
	if (nHeight > 15)
		nHeight = 15;
	this->m_nShowWidth = nWidth;
	this->m_nShowHeight = nHeight;

	// for test
//	int		i, j;
//	int		x, y;
//	x = LOWORD(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwRegionID) - m_nShowWidth / 2;
//	y = HIWORD(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwRegionID) - m_nShowHeight / 2;
//	for (i = x; i < x + m_nShowWidth; i++)
//	{
//		for (j = y; j < y + m_nShowHeight; j++)
//		{
//			KRegion::LoadLittleMapData(i, j, SubWorld[0].m_szMapPath, GetBarrierBuf(i, j));
//			SetHaveLoad(i, j);
//		}
//	}
}

#endif
