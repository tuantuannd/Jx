// ***************************************************************************************
// 场景中的对象树
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-19
// ---------------------------------------------------------------------------------------
// ***************************************************************************************
#include "..\KCore.h"
#include "Windows.h"
#include "KIpoTree.h"
#include "KIpotLeaf.h"
#include "KIpotBranch.h"
#include "SceneDataDef.h"
#include "..\CoreDrawGameObj.h"
#include "..\CoreShell.h"
#include "..\ImgRef.h"
#include "..\MyAssert.H"
#include "..\KNpc.h"
#include "KEngine.h"
#include <math.h>

#define MAIN_PLAYER_LIGHT_RADIUS    (256 + 64)

KSelfBreathLight::KSelfBreathLight()
{
	nDir = 1;
	dwLastBreathTime = timeGetTime();
}

void KSelfBreathLight::Breath()
{
	DWORD dwCurTime = timeGetTime();
	
	fRadius += (dwCurTime - dwLastBreathTime) / fCycle * (fMaxRange - fMinRange) * nDir * 2;
	if(nDir == 1 && fRadius > fMaxRange)
	{
		nDir = -1;
		fRadius = fMaxRange;
	}
	else if(nDir == -1 && fRadius < fMinRange)
	{
		nDir = 1;
		fRadius = fMinRange;
	}
	m_nRadius = (int)fRadius;
	dwLastBreathTime = dwCurTime;
}

//##ModelId=3DDA92EE0346
KIpoTree::KIpoTree()
{
	m_pMainBranch = NULL;
	m_bProcessBioLights = true;
	m_bDynamicLighting = true;
	m_nCurrentTime = 0;
	m_bIsIndoor = false;
	m_dwAmbient = 0xff000000;
	memset(&m_PermanentLeaf, 0, sizeof(m_PermanentLeaf));

    ptrdiff_t ulAddress;

    m_pbyLColorBase = new unsigned char [(sizeof(KLColor) * LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT) + 255];
    _ASSERT(m_pbyLColorBase);
    ulAddress = (ptrdiff_t)m_pbyLColorBase;
    ulAddress = (ulAddress + 255) / 256 * 256;
    pLColor = (KLColor *)ulAddress;

    m_pby_ptpBase = new unsigned char [(sizeof(KLColor) * LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT) + 255];
    _ASSERT(m_pby_ptpBase);
    ulAddress = (ptrdiff_t)m_pby_ptpBase;
    ulAddress = (ulAddress + 255) / 256 * 256;
    ptp = (KLColor *)ulAddress;
}

//##ModelId=3DDA92EE0350
KIpoTree::~KIpoTree()
{
	Clear();

    if (m_pby_ptpBase)
    {
        delete []m_pby_ptpBase;
        m_pby_ptpBase = NULL;
        ptp = NULL;
    }

    if (m_pbyLColorBase)
    {
        delete []m_pbyLColorBase;
        m_pbyLColorBase = NULL;

        pLColor = NULL;
    }
}

//##ModelId=3DD9ECFD00E6
void KIpoTree::Paint(RECT* pRepresentArea, IPOT_RENDER_LAYER eLayer)
{
	if(eLayer == IPOT_RL_COVER_GROUND && m_bDynamicLighting)
	{
		// 渲染光照图
		RenderLightMap();
		// 设置表现模块的光照信息
		g_pRepresent->SetLightInfo(m_nLeftTopX, m_nLeftTopY, (unsigned int*)pLightingArray);
	}
	
	if (eLayer == IPOT_RL_OBJECT)
	{
		if (m_pMainBranch)
			m_pMainBranch->PaintObjectLayer(pRepresentArea);
		else
			m_DefaultBranch.PaintObjectLayer(pRepresentArea);
	}
	else if (m_pMainBranch)
		m_pMainBranch->PaintNoneObjectLayer(pRepresentArea, eLayer);
	else
		m_DefaultBranch.PaintNoneObjectLayer(pRepresentArea, eLayer);
}

void KIpoTree::Breathe()
{
	list<KLightBase*>::iterator i;
	for (i = m_LightList.begin(); i != m_LightList.end(); ++i)
	{
		if((*i)->m_pParent == NULL)
			(*i)->Breath();
	}
}

//##ModelId=3DDA96360240
void KIpoTree::Clear()
{
	if (m_pMainBranch)
	{
		delete (m_pMainBranch);
		m_pMainBranch = NULL;
	}
	m_DefaultBranch.Clear();
	RemoveRtoGroupWithPermanentLeaf();
	
	//clear all dyna light
	list<KLightBase*>::iterator i;
	for (i = m_LightList.begin(); i != m_LightList.end(); ++i)
	{
		delete (*i);
	}
	m_LightList.clear();
}

void KIpoTree::RemoveRtoGroupWithPermanentLeaf()
{
	KIpotRuntimeObj* pRto;
	while(pRto = (KIpotRuntimeObj*)m_PermanentLeaf.pBrother)
	{
		m_PermanentLeaf.pBrother = pRto->pBrother;
		pRto->pAheadBrother = NULL;
		pRto->pBrother = NULL;
	}
}

//##ModelId=3DDA96780082
/*void KIpoTree::RemoveAllRtoLeafs()
{
	if (m_pMainBranch)
		m_pMainBranch->RemoveAllRtoLeafs(NULL);
	m_DefaultBranch.RemoveAllRtoLeafs(NULL);
	RemoveRtoGroupWithPermanentLeaf();
}*/

//##ModelId=3DDAB2E20352
void KIpoTree::AddBranch(KIpotBuildinObj* pBranchObj)
{
	if (m_pMainBranch)
		m_pMainBranch->AddBranch(pBranchObj);
	else if (pBranchObj)
	{
		m_pMainBranch = new KIpotBranch;
		m_pMainBranch->AddAObject(pBranchObj);
	}
}

//##ModelId=3DE1D38A0173
void KIpoTree::Fell()
{
	if (m_pMainBranch)
	{
		m_pMainBranch->RemoveAllRtoLeafs(&m_PermanentLeaf);
		delete(m_pMainBranch);
		m_pMainBranch = NULL;
	}
	m_DefaultBranch.RemoveAllRtoLeafs(&m_PermanentLeaf);
	m_DefaultBranch.Clear();

	//清除所有内建对象的光源
	list<KLightBase*>::iterator i;
	for (i = m_LightList.begin(); i != m_LightList.end(); )
	{
		if((*i)->m_pParent == NULL)
		{
			delete (*i);
			i = m_LightList.erase(i);
			continue;
		}
		++i;
	}
}

//##ModelId=3DE2A05601A3
/*void KIpoTree::RemoveOutsideRtoLeafs(RECT& rc)
{
	if (m_pMainBranch)
		m_pMainBranch->RemoveOutsideRtoLeafs(rc);
	m_DefaultBranch.RemoveOutsideRtoLeafs(rc);
	//to be replace
//	m_RtoLeafBranch.RemoveOutsideRtoLeafs(rc);
}
*/

//##ModelId=3DE3D4AB036F
void KIpoTree::AddLeafLine(KIpotBuildinObj* pLeaf)
{
	if (m_pMainBranch)
		m_pMainBranch->AddLeafLine(pLeaf);
	else
		m_DefaultBranch.AddLeafLine(pLeaf);
}

//##ModelId=3DE3D4AB0371
void KIpoTree::AddLeafPoint(KIpotLeaf* pLeaf)
{
	if (m_pMainBranch)
		m_pMainBranch->AddLeafPoint(pLeaf);
	else
		m_DefaultBranch.AddLeafPoint(pLeaf);
	// 
	if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
	{
		KIpotRuntimeObj* pRtoLeaf = (KIpotRuntimeObj*)pLeaf;
		if (pRtoLeaf->eLayerParam & IPOT_RL_LIGHT_PROP)
		{
			// 增加一个光源
			KLightBase *pLight = new KLightBase;
			if(pLight)
			{
				KLightInfo lightInfo;
				CoreGetGameObjLightInfo(pRtoLeaf->uGenre, pRtoLeaf->nId, &lightInfo);
				pLight->m_dwColor = lightInfo.dwColor;
				pLight->m_oPosition = lightInfo.oPosition;
				pLight->m_nRadius = lightInfo.nRadius;
				pLight->m_pParent = pLeaf;
				
				if (pRtoLeaf->uGenre == CGOG_NPC && Npc[pRtoLeaf->nId].IsPlayer())
				{
					pLight->m_dwColor = 0xffffffff;
					if(m_bIsIndoor)
					{
						if(m_dwAmbient == 0xff404040)
							pLight->m_nRadius = 0;
						else
							pLight->m_nRadius = MAIN_PLAYER_LIGHT_RADIUS;
					}
					else
					{
						// 是主角，按时间调整光源亮度
						int n = 0;
						if(m_nCurrentTime >= 480 && m_nCurrentTime <= 960)
							pLight->m_nRadius = 0;
						else if(m_nCurrentTime < 480)
						{
							n = 480 - m_nCurrentTime;
							pLight->m_nRadius = MAIN_PLAYER_LIGHT_RADIUS;
						}
						else
						{
							n = m_nCurrentTime - 960;
							pLight->m_nRadius = MAIN_PLAYER_LIGHT_RADIUS;
						}

						if(n > 360)
							n = 360;
						float f = n / 360.0f;
						f = f * f;
						KLColor color;
						color.SetColor(pLight->m_dwColor);
						color.Scale(f);
						pLight->m_dwColor = color.GetColor();
					}
				}

				pLight->m_nRadius = MAIN_PLAYER_LIGHT_RADIUS;
				pLight->m_dwColor = 0xff808080;

				if(pLight->m_nRadius)
                {
					m_LightList.push_back(pLight);
                }
                else
                {
                    delete pLight;
                    pLight = NULL;
                }
			}
		}
	}
}

void KIpoTree::PluckRto(KIpotRuntimeObj* pLeaf)
{
	pLeaf->Pluck();
	list<KLightBase*>::iterator i;
	for (i = m_LightList.begin(); i != m_LightList.end(); ++i)
	{
		if((*i)->m_pParent == pLeaf)
		{
			delete (*i);
			m_LightList.erase(i);
			break;
		}
	}
}

void KIpoTree::SetPermanentBranchPos(int nLeftX, int nRightX, int y)
{
	POINT	p1, p2;
	if (nLeftX < nRightX)
	{
		p1.x = nLeftX;
		p2.x = nRightX;
	}
	else if (nLeftX > nRightX)
	{
		p2.x = nLeftX;
		p1.x = nRightX;
	}
	else
	{
		p1.x = nLeftX;
		p2.x = nRightX + 2048;
	}
	p2.y = p1.y = y;
	m_DefaultBranch.SetLine(&p1, &p2);
}

void KIpoTree::StrewRtoLeafs(RECT& KeepRtoArea)
{
	KIpotBranch* pBranch;
	if (m_pMainBranch)
		pBranch = m_pMainBranch;
	else
		pBranch = &m_DefaultBranch;

	KIpotRuntimeObj* pRto;
	while(pRto = (KIpotRuntimeObj*)m_PermanentLeaf.pBrother)
	{
		m_PermanentLeaf.pBrother = pRto->pBrother;
		pRto->pAheadBrother = NULL;
		pRto->pBrother = NULL;

		if (pRto->oPosition.x >= KeepRtoArea.left  ||
			pRto->oPosition.x <  KeepRtoArea.right ||
			pRto->oPosition.y >= KeepRtoArea.top   ||
			pRto->oPosition.y <  KeepRtoArea.bottom)
		{
			pBranch->AddLeafPoint(pRto);
		}
		else
		{
			PluckRto(pRto);				
		}
	}

	if(m_bDynamicLighting)
	{
		// 清空遮挡信息
		for(int j=0; j<LIGHTING_GRID_WIDTH*LIGHTING_GRID_HEIGHT; j++)
		{
			pObstacle[j].nObstacle = 0;
		}

		// 遍历树，计算遮挡信息
		if (m_pMainBranch)
			m_pMainBranch->EnumerateObjects(this, ObjectsCallback);
		else
			m_DefaultBranch.EnumerateObjects(this, ObjectsCallback);
	}
}

// 遍历树中对象时对每个对象的回调函数
void KIpoTree::ObjectsCallback(void* p, KIpotLeaf* pLeaf)
{
	_ASSERT(p);
	_ASSERT(pLeaf);
	static int nObstacleID = 1;
	nObstacleID++;

	if(pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		return;

	KIpoTree *pTree = (KIpoTree*)p;
	KIpotBuildinObj* pObj = (KIpotBuildinObj*)pLeaf;
	KBuildinObj* pBio = ((KIpotBuildinObj*)pLeaf)->pBio;
	if((pBio->Props & SPBIO_P_BLOCK_LIGHT_MASK) == SPBIO_P_BLOCK_LIGHT_NONE)
		return;

	if((pBio->Props & SPBIO_P_BLOCK_LIGHT_MASK) == SPBIO_P_BLOCK_LIGHT_BOTTOM)
	{
		// 按线段处理遮挡
		pTree->FillLineObstacle(pObj->oPosition.x - pTree->m_nLeftTopX, pObj->oPosition.y - pTree->m_nLeftTopY, 
								pObj->oEndPos.x - pTree->m_nLeftTopX, pObj->oEndPos.y - pTree->m_nLeftTopY, nObstacleID);
	}
	else if((pBio->Props & SPBIO_P_BLOCK_LIGHT_MASK) == SPBIO_P_BLOCK_LIGHT_CIRCLE)
	{
		// 按圆处理光线遮挡，忽略半径，都按一个格子处理
		int nX = pObj->oPosition.x;
		int nY = pObj->oPosition.y;
		int nIdx;
		if(nX < pTree->m_nLeftTopX || nY < pTree->m_nLeftTopY ||
				nX >= pTree->m_nLeftTopX + AREGION_WIDTH * 3 || nY >= pTree->m_nLeftTopY + AREGION_HEIGHT * 3)
			return;
		else
		{
			nX = (nX - pTree->m_nLeftTopX) / LIGHTING_GRID_SIZEX;
			nY = (nY - pTree->m_nLeftTopY) / LIGHTING_GRID_SIZEY;
			nIdx = nY * LIGHTING_GRID_WIDTH + nX;
			pTree->pObstacle[nIdx].nObstacle = nObstacleID;
			// 将方向的x分量设为0，表示不考虑方向
			pTree->pObstacle[nIdx].vDir.fX = 0.0f;
		}
	}
}

void KIpoTree::FillLineObstacle(int x1, int y1, int x2, int y2, int nObstacleID)
{
	if(x1 >= x2)
		return;

	float fx1, fy1, fx2, fy2, fdx, fdy, fStepX, fStepY;
	int gStepX, gStepY, gx, gy, nIdx, gx1, gy1, gx2, gy2;
	fx1 = (float)x1, fy1 = (float)y1, fx2 = (float)x2, fy2 = (float)y2;
	fdx = (float)fabs(fx2 - fx1);
	fdy = (float)fabs(fy2 - fy1);
	KVector2 vDir;
	vDir.fX = fdx;
	vDir.fY = fdy;
	float fFlope1, fFlope2;
	gx1 = x1 / LIGHTING_GRID_SIZEX;
	gy1 = y1 / LIGHTING_GRID_SIZEX;
	gx2 = x2 / LIGHTING_GRID_SIZEX;
	gy2 = y2 / LIGHTING_GRID_SIZEX;

	fFlope1 = fdy / fdx;

	// 斜率小于45度的情况
	if(fdx >= fdy)
	{
		if(gx1 == gx2)
			return;

		if(x2 > x1)
		{
			gStepX = 1;
			fStepX = LIGHTING_GRID_SIZEX;
		}
		else
		{
			gStepX = -1;
			fStepX = -LIGHTING_GRID_SIZEX;
		}
		if(y2 > y1)
		{
			gStepY = 1;
			fStepY = fdy / fdx * LIGHTING_GRID_SIZEX;
		}
		else
		{
			gStepY = -1;
			fStepY = -fdy / fdx * LIGHTING_GRID_SIZEX;
		}
		while(1)
		{
			int nx = (int)fx1;
			int ny = (int)fy1;

			gx = nx / LIGHTING_GRID_SIZEX;
			gy = ny / LIGHTING_GRID_SIZEX;

			if(gx >0 && gx < LIGHTING_GRID_WIDTH - 1 && gy >0 && gy < LIGHTING_GRID_HEIGHT - 1)
			{
				nIdx = gy * LIGHTING_GRID_WIDTH + gx;
				pObstacle[nIdx].nObstacle = nObstacleID;
				pObstacle[nIdx].vDir = vDir;

				fFlope2 = (float)(fabs((gy + gStepY) * 32.0f - fy1) / fabs((gx + gStepX) * 32.0f - fx1));
				if(fFlope2 > fFlope1)
				{
					nIdx = (gy + gStepY) * LIGHTING_GRID_WIDTH + gx;
					pObstacle[nIdx].nObstacle = nObstacleID;
					pObstacle[nIdx].vDir = vDir;
				}
				else
				{
					nIdx = gy * LIGHTING_GRID_WIDTH + gx + gStepX;
					pObstacle[nIdx].nObstacle = nObstacleID;
					pObstacle[nIdx].vDir = vDir;
				}
			}

			fx1 += fStepX;
			fy1 += fStepY;
			gx1 += gStepX;
			if(gx1 == gx2)
			{
				gx = ((int)fx1) / LIGHTING_GRID_SIZEX;
				gy = ((int)fy1) / LIGHTING_GRID_SIZEX;
				if(gx >0 && gx < LIGHTING_GRID_WIDTH - 1 && gy >0 && gy < LIGHTING_GRID_HEIGHT - 1)
				{
					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pObstacle[nIdx].nObstacle = nObstacleID;
					pObstacle[nIdx].vDir = vDir;
				}
				return;
			}
		}
	}
	// 斜率大于45度的情况
	else
	{
		if(gy1 == gy2)
			return;

		if(y2 > y1)
		{
			gStepY = 1;
			fStepY = LIGHTING_GRID_SIZEX;
		}
		else
		{
			gStepY = -1;
			fStepY = -LIGHTING_GRID_SIZEX;
		}
		if(x2 > x1)
		{
			gStepX = 1;
			fStepX = fdx / fdy * LIGHTING_GRID_SIZEX;
		}
		else
		{
			gStepX = -1;
			fStepX = -fdx / fdy * LIGHTING_GRID_SIZEX;
		}
		while(1)
		{
			int nx = (int)fx1;
			int ny = (int)fy1;

			gx = nx / LIGHTING_GRID_SIZEX;
			gy = ny / LIGHTING_GRID_SIZEX;

			if(gx >0 && gx < LIGHTING_GRID_WIDTH - 1 && gy >0 && gy < LIGHTING_GRID_HEIGHT - 1)
			{
				nIdx = gy * LIGHTING_GRID_WIDTH + gx;
				pObstacle[nIdx].nObstacle = nObstacleID;
				pObstacle[nIdx].vDir = vDir;

				fFlope2 = (float)(fabs((gy + gStepY) * 32.0f - fy1) / fabs((gx + gStepX) * 32.0f - fx1));
				if(fFlope2 > fFlope1)
				{
					nIdx = (gy + gStepY) * LIGHTING_GRID_WIDTH + gx;
					pObstacle[nIdx].nObstacle = nObstacleID;
					pObstacle[nIdx].vDir = vDir;
				}
				else
				{
					nIdx = gy * LIGHTING_GRID_WIDTH + gx + gStepX;
					pObstacle[nIdx].nObstacle = nObstacleID;
					pObstacle[nIdx].vDir = vDir;
				}
			}

			fx1 += fStepX;
			fy1 += fStepY;
			gy1 += gStepY;
			if(gy1 == gy2)
			{
				gx = ((int)fx1) / LIGHTING_GRID_SIZEX;
				gy = ((int)fy1) / LIGHTING_GRID_SIZEX;
				if(gx >0 && gx < LIGHTING_GRID_WIDTH - 1 && gy >0 && gy < LIGHTING_GRID_HEIGHT - 1)
				{
					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pObstacle[nIdx].nObstacle = nObstacleID;
					pObstacle[nIdx].vDir = vDir;
				}
				return;
			}
		}
	}
}

/*
void KIpoTree::RenderLightMap()
{
	int j, nIdx;

	for(j=0; j<LIGHTING_GRID_WIDTH*LIGHTING_GRID_HEIGHT; j++)
	{
			pLColor[j].r = 0x10;
			pLColor[j].g = 0x10;
			pLColor[j].b = 0x10;
	}

	list<KLightBase*>::iterator i;
	for (i = m_LightList.begin(); i != m_LightList.end(); ++i)
	{
		KLightBase *pLight = *i;
		// 光源相对于九区域左上角的坐标
		int nLightPosX = pLight->m_oPosition.nX - m_nLeftTopX;
		int nLightPosY = pLight->m_oPosition.nY - m_nLeftTopY;
		// 光源的格子坐标
		int nLightGridX = nLightPosX / LIGHTING_GRID_SIZEX;
		int nLightGridY = nLightPosY / LIGHTING_GRID_SIZEY;
		// 光源所在格子中心的坐标
		int nLightGridCenterX = nLightGridX * LIGHTING_GRID_SIZEX + LIGHTING_GRID_SIZEX / 2;
		int nLightGridCenterY = nLightGridY * LIGHTING_GRID_SIZEY + LIGHTING_GRID_SIZEY / 2;

		unsigned int nr, ng, nb;
		pLight->m_dwColor = 0xff404040;
		nr = (pLight->m_dwColor & 0x00ff0000) >> 16;
		ng = (pLight->m_dwColor & 0x0000ff00) >> 8;
		nb = pLight->m_dwColor & 0x000000ff;

		if(pLight->m_nRadius == 500)
		{
			int j, x, y, gx, gy;
			int nGridRadius = pLight->m_nRadius / LIGHTING_GRID_SIZEX;
			for(int r=0; r<nGridRadius; r++)
			{
				x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY - r * LIGHTING_GRID_SIZEY;
				gx = nLightGridX - r;
				gy = nLightGridY - r;
				for(j=0; j<(r * 2 + 1); j++)
				{
					float f = sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
								(y - nLightPosY)) / pLight->m_nRadius;
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;
					f = f*f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);

					x += LIGHTING_GRID_SIZEX;
					gx += 1;
				}

				if(r == 0)
					continue;

				x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY + r * LIGHTING_GRID_SIZEY;
				gx = nLightGridX - r;
				gy = nLightGridY + r;
				for(j=0; j<(r * 2 + 1); j++)
				{
					float f = sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
								(y - nLightPosY)) / pLight->m_nRadius;
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;
					f = f*f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);

					x += LIGHTING_GRID_SIZEX;
					gx += 1;
				}

				x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY - (r - 1) * LIGHTING_GRID_SIZEY;
				gx = nLightGridX - r;
				gy = nLightGridY - (r - 1);
				for(j=0; j<(r * 2 - 1); j++)
				{
					float f = sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
								(y - nLightPosY)) / pLight->m_nRadius;
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;
					f = f*f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);

					y += LIGHTING_GRID_SIZEX;
					gy += 1;
				}

				x = nLightGridCenterX + r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY - (r - 1) * LIGHTING_GRID_SIZEY;
				gx = nLightGridX + r;
				gy = nLightGridY - (r - 1);
				for(j=0; j<(r * 2 - 1); j++)
				{
					float f = sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
								(y - nLightPosY)) / pLight->m_nRadius;
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;
					f = f*f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);

					y += LIGHTING_GRID_SIZEX;
					gy += 1;
				}
			}
		}
	}

	for(j=0; j<LIGHTING_GRID_WIDTH*LIGHTING_GRID_HEIGHT; j++)
	{
		if(pLColor[j].r > 0xff)
			pLColor[j].r = 0xff;
		if(pLColor[j].g > 0xff)
			pLColor[j].g = 0xff;
		if(pLColor[j].b > 0xff)
			pLColor[j].b = 0xff;
		pLightingArray[j] = 0xff000000 | (pLColor[j].r<<16) | (pLColor[j].g<<8) | pLColor[j].b;
	}
}*/

/*
void KIpoTree::RenderLightMap()
{
	int nn = m_LightList.size();
	int j, nIdx, nLightID=1;
	KLColor ptp[LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT];

	// 按环境光清空光照图
	DWORD dwR, dwG, dwB;
	dwR = (m_dwAmbient & 0x00ff0000) >>16;
	dwG = (m_dwAmbient & 0x0000ff00) >>8;
	dwB = m_dwAmbient & 0x000000ff;
	for(j=0; j<LIGHTING_GRID_WIDTH*LIGHTING_GRID_HEIGHT; j++)
	{
			ptp[j].r = pLColor[j].r = dwR;
			ptp[j].g = pLColor[j].g = dwG;
			ptp[j].b = pLColor[j].b = dwB;
	}

	list<KLightBase*>::iterator i;
	// 计算每一个光源的光照结果
	for (i = m_LightList.begin(); i != m_LightList.end(); ++i)
	{
		nLightID++;
		KLightBase *pLight = *i;
		// 光源相对于九区域左上角的坐标
		int nLightPosX = pLight->m_oPosition.nX - m_nLeftTopX;
		int nLightPosY = pLight->m_oPosition.nY - m_nLeftTopY;
		// 光源的格子坐标
		int nLightGridX = nLightPosX / LIGHTING_GRID_SIZEX;
		int nLightGridY = nLightPosY / LIGHTING_GRID_SIZEY;
		if(nLightGridX < 0 || nLightGridX >= LIGHTING_GRID_WIDTH || 
			nLightGridY < 0 || nLightGridY >= LIGHTING_GRID_HEIGHT)
			continue;
		// 光源所在格子中心的坐标
		int nLightGridCenterX = nLightGridX * LIGHTING_GRID_SIZEX + LIGHTING_GRID_SIZEX / 2;
		int nLightGridCenterY = nLightGridY * LIGHTING_GRID_SIZEY + LIGHTING_GRID_SIZEY / 2;

		// 获取光源的r，g，b分量
		unsigned int nr, ng, nb;

		nr = (pLight->m_dwColor & 0x00ff0000) >> 18;
		ng = (pLight->m_dwColor & 0x0000ff00) >> 10;
		nb = (pLight->m_dwColor & 0x000000ff) >> 2;

		float fr = (float)pLight->m_nRadius * (float)pLight->m_nRadius;

		int j, x, y, gx, gy;
		int nGridRadius = pLight->m_nRadius / LIGHTING_GRID_SIZEX;

		gy = nLightGridY - (nGridRadius - 1);
		// 从中心点向外扩展计算光照强度
		for(int r=0; r<nGridRadius; r++)
		{
			x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
			y = nLightGridCenterY - r * LIGHTING_GRID_SIZEY;
			gx = nLightGridX - r;
			gy = nLightGridY - r;
			for(j=0; j<(r * 2 + 1); j++)
			{
				if(CanLighting(x, y, nLightPosX, nLightPosY))
				{
					float f = (float)(sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
							(y - nLightPosY)) / pLight->m_nRadius);
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);
				}

				x += LIGHTING_GRID_SIZEX;
				gx += 1;
			}

			if(r == 0)
				continue;

			x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
			y = nLightGridCenterY + r * LIGHTING_GRID_SIZEY;
			gx = nLightGridX - r;
			gy = nLightGridY + r;
			for(j=0; j<(r * 2 + 1); j++)
			{
				if(CanLighting(x, y, nLightPosX, nLightPosY))
				{
					float f = (float)(sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
							(y - nLightPosY)) / pLight->m_nRadius);
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);
				}

				x += LIGHTING_GRID_SIZEX;
				gx += 1;
			}

			x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
			y = nLightGridCenterY - (r - 1) * LIGHTING_GRID_SIZEY;
			gx = nLightGridX - r;
			gy = nLightGridY - (r - 1);
			for(j=0; j<(r * 2 - 1); j++)
			{
				if(CanLighting(x, y, nLightPosX, nLightPosY))
				{
					float f = (float)(sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
							(y - nLightPosY)) / pLight->m_nRadius);
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);
				}

				y += LIGHTING_GRID_SIZEX;
				gy += 1;
			}

			x = nLightGridCenterX + r * LIGHTING_GRID_SIZEX;
			y = nLightGridCenterY - (r - 1) * LIGHTING_GRID_SIZEY;
			gx = nLightGridX + r;
			gy = nLightGridY - (r - 1);
			for(j=0; j<(r * 2 - 1); j++)
			{
				if(CanLighting(x, y, nLightPosX, nLightPosY))
				{
					float f = (float)(sqrt((x - nLightPosX) * (x - nLightPosX) + (y - nLightPosY) * 
							(y - nLightPosY)) / pLight->m_nRadius);
					if(f > 1.0f)
						f = 1.0f;
					f = 1.0f - f;

					nIdx = gy * LIGHTING_GRID_WIDTH + gx;
					pLColor[nIdx].r += (unsigned int)(nr * f);
					pLColor[nIdx].g += (unsigned int)(ng * f);
					pLColor[nIdx].b += (unsigned int)(nb * f);
				}

				y += LIGHTING_GRID_SIZEX;
				gy += 1;
			}
		}
	}

	// 对光照图进行过滤，使阴影边沿平滑
	KLColor *pO, *pL, *pR, *pU, *pD, *pDest;
	for(int m=1; m<LIGHTING_GRID_HEIGHT-1;m++)
	{
		int nIdx = m * LIGHTING_GRID_WIDTH + 1;
		pDest = ptp + nIdx;
		pO = pLColor + nIdx;
		pL = pO - 1;
		pR = pO + 1;
		pU = pO - LIGHTING_GRID_WIDTH;
		pD = pO + LIGHTING_GRID_WIDTH;
		for(int n=1; n<LIGHTING_GRID_WIDTH-1;n++)
		{
			pDest->r = (pO->r + pR->r + pL->r + pU->r + pD->r) / 5;
			pDest->g = (pO->g + pR->g + pL->g + pU->g + pD->g) / 5;
			pDest->b = (pO->b + pR->b + pL->b + pU->b + pD->b) / 5;
			pDest++, pO++, pL++, pR++, pU++, pD++;
		}
	}

	// 将r，g，b的值限制在0xff之内，防止色彩错误
	for(j=0; j<LIGHTING_GRID_WIDTH*LIGHTING_GRID_HEIGHT; j++)
	{
		if(ptp[j].r > 0xff)
			ptp[j].r = 0xff;
		if(ptp[j].g > 0xff)
			ptp[j].g = 0xff;
		if(ptp[j].b > 0xff)
			ptp[j].b = 0xff;
		pLightingArray[j] = 0xff000000 | (ptp[j].r<<16) | (ptp[j].g<<8) | ptp[j].b;
	}
}
*/


void KIpoTree::RenderLightMap()
{
	int nn = m_LightList.size();
	int j, nLightID=1;

	m_dwAmbient = 0xff101010;

	// 按环境光清空光照图
	DWORD dwR, dwG, dwB;
	dwR = (m_dwAmbient & 0x00ff0000) >>16;
	dwG = (m_dwAmbient & 0x0000ff00) >>8;
	dwB = m_dwAmbient & 0x000000ff;
    KLColor *pTempColor = pLColor;
    
    // 如果每一个颜色分量是2个字节,可以考虑多一个项,然后用一个MMX的寄存器存放
    // 正好8个字节
	for(j=0; j < LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT / 4; j++)
	{
			pTempColor[0].r = dwR;
			pTempColor[1].r = dwR;
			pTempColor[2].r = dwR;
			pTempColor[3].r = dwR;

			pTempColor[0].g = dwG;
			pTempColor[1].g = dwG;
			pTempColor[2].g = dwG;
			pTempColor[3].g = dwG;


			pTempColor[0].b = dwB;
			pTempColor[1].b = dwB;
			pTempColor[2].b = dwB;
			pTempColor[3].b = dwB;
            
            pTempColor += 4;
	}

	list<KLightBase*>::iterator i;
	// 计算每一个光源的光照结果
	for (i = m_LightList.begin(); i != m_LightList.end(); ++i)
	{
		nLightID++;
		KLightBase *pLight = *i;
		// 光源相对于九区域左上角的坐标
		int nLightPosX = pLight->m_oPosition.nX - m_nLeftTopX;
		int nLightPosY = pLight->m_oPosition.nY - m_nLeftTopY;
		// 光源的格子坐标
		int nLightGridX = nLightPosX / LIGHTING_GRID_SIZEX;
		int nLightGridY = nLightPosY / LIGHTING_GRID_SIZEY;
		if(nLightGridX < 0 || nLightGridX >= LIGHTING_GRID_WIDTH || 
			nLightGridY < 0 || nLightGridY >= LIGHTING_GRID_HEIGHT)
			continue;

		// 光源所在格子中心的坐标
		int nLightGridCenterX = nLightGridX * LIGHTING_GRID_SIZEX + LIGHTING_GRID_SIZEX / 2;
		int nLightGridCenterY = nLightGridY * LIGHTING_GRID_SIZEY + LIGHTING_GRID_SIZEY / 2;

		// 获取光源的r，g，b分量
		unsigned int ur, ug, ub;

		ur = (pLight->m_dwColor & 0x00ff0000) >> 18;
		ug = (pLight->m_dwColor & 0x0000ff00) >> 10;
		ub = (pLight->m_dwColor & 0x000000ff) >> 2;

		//float fr = (float)pLight->m_nRadius * (float)pLight->m_nRadius;

		int j, x, y, gx, gy;
		int nGridRadius = pLight->m_nRadius / LIGHTING_GRID_SIZEX;
        float fLightRadiusDenom =  1.0 / ((float)pLight->m_nRadius);
        //fLightRadiusDenom *= fLightRadiusDenom;
        unsigned int uDistance65536 = 0;
        KLColor *pCurPos = NULL;

		
		gy = nLightGridY - (nGridRadius - 1);
		// 从中心点向外扩展计算光照强度
		for(int r = 0; r < nGridRadius; r++)
		{
			gx = nLightGridX - r;
			gy = nLightGridY - r;
			if (!
				((gx < 0) || (gy < 0))
			)
			{
				x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY - r * LIGHTING_GRID_SIZEY;
				pCurPos = &pLColor[gy * LIGHTING_GRID_WIDTH + gx];
				for(j=0; j<(r * 2 + 1); j++)
				{
					if(CanLighting(x, y, nLightPosX, nLightPosY))
					{
						//float f = 65536.0 - ((float)(
						//    (
						//        ((float)(x - nLightPosX)) * ((float)(x - nLightPosX)) + 
						//        ((float)(y - nLightPosY)) * ((float)(y - nLightPosY))
						//    ) * 
						//    fLightRadiusDenom *
						//    65536.0
						//));
						float f = 65536.0 - ((float)(
							sqrt(double // Fixed By MrChuCong@gmail.com
								(x - nLightPosX) * (x - nLightPosX) + 
								(y - nLightPosY) * (y - nLightPosY)
							) * 
							fLightRadiusDenom * 
							65536.0
						));

						uDistance65536 = (int)(f);

						if (((int)uDistance65536) > 0)
						{
							pCurPos->r += (ur * uDistance65536 / 65536);
							pCurPos->g += (ug * uDistance65536 / 65536);
							pCurPos->b += (ub * uDistance65536 / 65536);
						}
					}

					gx++;
                    if (gx >= LIGHTING_GRID_WIDTH)
                        break;

					x += LIGHTING_GRID_SIZEX;
					pCurPos++;
				}
			}

			if(r == 0)
				continue;

			gx = nLightGridX - r;
			gy = nLightGridY + r;
			if (!
					((gx < 0) || (gy >= LIGHTING_GRID_HEIGHT))
			)
			{
				x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY + r * LIGHTING_GRID_SIZEY;
				pCurPos = &pLColor[gy * LIGHTING_GRID_WIDTH + gx];
				for(j=0; j<(r * 2 + 1); j++)
				{
					if(CanLighting(x, y, nLightPosX, nLightPosY))
					{
						//float f = 65536.0 - ((float)(
						//    (
						//        ((float)(x - nLightPosX)) * ((float)(x - nLightPosX)) + 
						//        ((float)(y - nLightPosY)) * ((float)(y - nLightPosY))
						//    ) * 
						//    fLightRadiusDenom *
						//    65536.0
						//));
						float f = 65536.0 - ((float)(
							sqrt(double // Fixed By MrChuCong@gmail.com
								(x - nLightPosX) * (x - nLightPosX) + 
								(y - nLightPosY) * (y - nLightPosY)
							) * 
							fLightRadiusDenom * 
							65536.0
						));

						uDistance65536 = (int)(f);

						if (((int)uDistance65536) > 0)
						{
							pCurPos->r += (ur * uDistance65536 / 65536);
							pCurPos->g += (ug * uDistance65536 / 65536);
							pCurPos->b += (ub * uDistance65536 / 65536);
						}
					}

					gx++;
                    if (gx >= LIGHTING_GRID_WIDTH)
                        break;

					x += LIGHTING_GRID_SIZEX;
					pCurPos++;
				}
			}

			gx = nLightGridX - r;
			gy = nLightGridY - (r - 1);
			if (!
				((gx < 0) || (gy < 0))
			)
			{
				x = nLightGridCenterX - r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY - (r - 1) * LIGHTING_GRID_SIZEY;
				pCurPos = &pLColor[gy * LIGHTING_GRID_WIDTH + gx];
				for(j=0; j<(r * 2 - 1); j++)
				{
					if(CanLighting(x, y, nLightPosX, nLightPosY))
					{
						//float f = 65536.0 - ((float)(
						//    (
						//        ((float)(x - nLightPosX)) * ((float)(x - nLightPosX)) + 
						//        ((float)(y - nLightPosY)) * ((float)(y - nLightPosY))
						//    ) * 
						//    fLightRadiusDenom *
						//    65536.0
						//));
						float f = 65536.0 - ((float)(
							sqrt(double // Fixed By MrChuCong@gmail.com
								(x - nLightPosX) * (x - nLightPosX) + 
								(y - nLightPosY) * (y - nLightPosY)
							) * 
							fLightRadiusDenom * 
							65536.0
						));

						uDistance65536 = (int)(f);

						if (((int)uDistance65536) > 0)
						{
							pCurPos->r += (ur * uDistance65536 / 65536);
							pCurPos->g += (ug * uDistance65536 / 65536);
							pCurPos->b += (ub * uDistance65536 / 65536);
						}
					}

					gy++;
                    if (gy >= LIGHTING_GRID_HEIGHT)
                        break;

					y += LIGHTING_GRID_SIZEX;
					pCurPos += LIGHTING_GRID_WIDTH;
				}
			}

			
			gx = nLightGridX + r;
			gy = nLightGridY - (r - 1);
			if (!
				((gx >= LIGHTING_GRID_WIDTH ) || (gy < 0))
			)
			{
				x = nLightGridCenterX + r * LIGHTING_GRID_SIZEX;
				y = nLightGridCenterY - (r - 1) * LIGHTING_GRID_SIZEY;
				pCurPos = &pLColor[gy * LIGHTING_GRID_WIDTH + gx];
				for(j=0; j<(r * 2 - 1); j++)
				{
					if(CanLighting(x, y, nLightPosX, nLightPosY))
					{
						//float f = 65536.0 - ((float)(
						//    (
						//        ((float)(x - nLightPosX)) * ((float)(x - nLightPosX)) + 
						//        ((float)(y - nLightPosY)) * ((float)(y - nLightPosY))
						//    ) * 
						//    fLightRadiusDenom *
						//    65536.0
						//));
						float f = 65536.0 - ((float)(
							sqrt(double // Fixed By MrChuCong@gmail.com
								(x - nLightPosX) * (x - nLightPosX) + 
								(y - nLightPosY) * (y - nLightPosY)
							) * 
							fLightRadiusDenom * 
							65536.0
						));

						uDistance65536 = (int)(f);

						if (((int)uDistance65536) > 0)
						{
							pCurPos->r += (ur * uDistance65536 / 65536);
							pCurPos->g += (ug * uDistance65536 / 65536);
							pCurPos->b += (ub * uDistance65536 / 65536);
						}
					}

					gy++;
                    if (gy >= LIGHTING_GRID_HEIGHT)
                        break;

					y += LIGHTING_GRID_SIZEX;
					pCurPos += LIGHTING_GRID_WIDTH;
				}
			}
		}
	}

    //memcpy(ptp, pLColor, sizeof(ptp));

    
	// 对光照图进行过滤，使阴影边沿平滑
    

	int m;
    int n;
    DWORD   *pdwLight;
    KLColor *pO, *pL, *pR, *pU, *pD, *pDest;
    int nIdx = 1 * LIGHTING_GRID_WIDTH + 1;
	for(m=1; m<LIGHTING_GRID_HEIGHT-1;m++)
	{
		pDest = ptp + nIdx;
        pdwLight = pLightingArray + nIdx;
		pO = pLColor + nIdx;
		pL = pO - 1;
		pR = pO + 1;
		pU = pO - LIGHTING_GRID_WIDTH;
		pD = pO + LIGHTING_GRID_WIDTH;
		for(n=1; n<LIGHTING_GRID_WIDTH-1;n++)
		{
			//pDest->r = (pO->r + pR->r + pL->r + pU->r + pD->r) / 5;
			//pDest->g = (pO->g + pR->g + pL->g + pU->g + pD->g) / 5;
			//pDest->b = (pO->b + pR->b + pL->b + pU->b + pD->b) / 5;
			//pDest->r = (pO->r + pR->r + pL->r + (pU->r + pD->r) / 2) / 4;
			//pDest->g = (pO->g + pR->g + pL->g + (pU->g + pD->g) / 2) / 4;
			//pDest->b = (pO->b + pR->b + pL->b + (pU->b + pD->b) / 2) / 4;
			//pDest->r = (pO->r + (pR->r + pL->r) / 2 + pU->r + pD->r) / 4;
			//pDest->g = (pO->g + (pR->g + pL->g) / 2 + pU->g + pD->g) / 4;
			//pDest->b = (pO->b + (pR->b + pL->b) / 2 + pU->b + pD->b) / 4;
			
            pDest->r = (pO->r + pL->r + pU->r + pD->r) / 4;
		    pDest->g = (pO->g + pL->g + pU->g + pD->g) / 4;
			pDest->b = (pO->b + pL->b + pU->b + pD->b) / 4;

            *pdwLight = 0xff000000 |
                (((pDest->r > 0xff) ? 0xff : pDest->r) << 16) |
                (((pDest->g > 0xff) ? 0xff : pDest->g) << 8) |
                (((pDest->b > 0xff) ? 0xff : pDest->b));


            //pDest->r = pO->r;
		    //pDest->g = pO->g;
			//pDest->b = pO->b;

			pDest++, pO++, pL++, pR++, pU++, pD++, pdwLight++;
		}
		nIdx += LIGHTING_GRID_WIDTH;
	}
    

	// 将r，g，b的值限制在0xff之内，防止色彩错误
//	for(j = 0; j < LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT; j++)
//	{
//        //pLightingArray[j] = 0xff000000 |
//        //    ((((-(ptp[j].r > 0xff)) | ptp[j].r) & 0xff) << 16) |
//        //    ((((-(ptp[j].g > 0xff)) | ptp[j].g) & 0xff) << 8) |
//        //    ((((-(ptp[j].b > 0xff)) | ptp[j].b) & 0xff));
//
//		//if(ptp[j].r > 0xff)
//		//	ptp[j].r = 0xff;
//        
//		//if(ptp[j].g > 0xff)
//		//	ptp[j].g = 0xff;
//		//if(ptp[j].b > 0xff)
//		//	ptp[j].b = 0xff;
//		//pLightingArray[j] = 0xff000000 | (ptp[j].r<<16) | (ptp[j].g<<8) | ptp[j].b;
//
//        pLightingArray[j] = 0xff000000 |
//            (((ptp[j].r > 0xff) ? 0xff : ptp[j].r) << 16) |
//            (((ptp[j].g > 0xff) ? 0xff : ptp[j].g) << 8) |
//            (((ptp[j].b > 0xff) ? 0xff : ptp[j].b));
//
//	}
    for (m = 0; m < LIGHTING_GRID_HEIGHT; m++)
    {
        pLightingArray[m * LIGHTING_GRID_WIDTH + 0] = m_dwAmbient | 0xff000000;
        pLightingArray[m * LIGHTING_GRID_WIDTH + LIGHTING_GRID_WIDTH - 1] = m_dwAmbient | 0xff000000;
    }

    for (n = 0; n < LIGHTING_GRID_WIDTH; n++)
    {
        pLightingArray[n] = m_dwAmbient | 0xff000000;
        pLightingArray[(LIGHTING_GRID_HEIGHT - 1) * LIGHTING_GRID_WIDTH + n] = m_dwAmbient | 0xff000000;
    }
    //pLightingArray[0] = m_dwAmbient;    // 设置环境光
}


bool KIpoTree::CanLighting(int x, int y, int nLightPosX, int nLightPosY)
{
	int nGridX, nGridY, nGridLightPosX, nGridLightPosY, nGridStep, nCurrentGridX, nCurrentGridY, nIdx;
	nGridX = x / LIGHTING_GRID_SIZEX;
	nGridY = y / LIGHTING_GRID_SIZEX;
	nGridLightPosX = nLightPosX / LIGHTING_GRID_SIZEX;
	nGridLightPosY = nLightPosY / LIGHTING_GRID_SIZEX;
	if(nGridX == nGridLightPosX && nGridY == nGridLightPosY)
		return true;

	int nObstacle = pObstacle[nGridY * LIGHTING_GRID_WIDTH + nGridX].nObstacle;
	KVector2 vDir = pObstacle[nGridY * LIGHTING_GRID_WIDTH + nGridX].vDir;
	KVector2 vLight;
	vLight.fX = (float)(x - nLightPosX);
	vLight.fY = (float)(y - nLightPosY);

	float fx, fy, fLightPosX, fLightPosY, fdx, fdy, fStepX, fStepY;
	fx = (float)x, fy =(float) y, fLightPosX = (float)nLightPosX, fLightPosY = (float)nLightPosY;
	fdx = (float)fabs(fLightPosX - fx);
	fdy = (float)fabs(fLightPosY - fy);

	// 斜率小于45度的情况
	if(fdx >= fdy)
	{
		// 计算线段前进累加值
		if(nLightPosX > x)
		{
			nGridStep = 1;
			fStepX = LIGHTING_GRID_SIZEX;
		}
		else
		{
			nGridStep = -1;
			fStepX = -LIGHTING_GRID_SIZEX;
		}
		if(nLightPosY > y)
			fStepY = fdy / fdx * LIGHTING_GRID_SIZEX;
		else
			fStepY = -fdy / fdx * LIGHTING_GRID_SIZEX;

		while(1)
		{
			fx += fStepX;
			fy += fStepY;
			nGridX += nGridStep;
			if(nGridX == nGridLightPosX)
			{
				// 没有碰到障碍，根据这点本身是否障碍和障碍方向判断是否受光
				if(nObstacle && vDir.fX != 0.0f)
				{
					if(vDir.fX * vLight.fY - vDir.fY * vLight.fX < 0.0f)
						return true;
					else
						return false;
				}
				else
					return true;
			}

			nCurrentGridX = ((int)fx) / LIGHTING_GRID_SIZEX;
			nCurrentGridY = ((int)fy) / LIGHTING_GRID_SIZEX;
			nIdx = nCurrentGridY * LIGHTING_GRID_WIDTH + nCurrentGridX;
			// 检查是否碰到挡光障碍
			if(pObstacle[nIdx].nObstacle != 0 && pObstacle[nIdx].nObstacle != nObstacle)
				return false;
		}
	}
	// 斜率大于45度的情况
	else
	{
		if(nLightPosY > y)
		{
			nGridStep = 1;
			fStepY = LIGHTING_GRID_SIZEX;
		}
		else
		{
			nGridStep = -1;
			fStepY = -LIGHTING_GRID_SIZEX;
		}
		if(nLightPosX > x)
			fStepX = fdx / fdy * LIGHTING_GRID_SIZEX;
		else
			fStepX = -fdx / fdy * LIGHTING_GRID_SIZEX;
		while(1)
		{
			fx += fStepX;
			fy += fStepY;
			nGridY += nGridStep;
			if(nGridY == nGridLightPosY)
			{
				// 没有碰到障碍，根据这点本身是否障碍和障碍方向判断是否受光
				if(nObstacle && vDir.fX != 0.0f)
				{
					if(vDir.fX * vLight.fY - vDir.fY * vLight.fX < 0.0f)
						return true;
					else
						return false;
				}
				else
					return true;
			}

			nCurrentGridX = ((int)fx) / LIGHTING_GRID_SIZEX;
			nCurrentGridY = ((int)fy) / LIGHTING_GRID_SIZEX;
			nIdx = nCurrentGridY * LIGHTING_GRID_WIDTH + nCurrentGridX;
			// 检查是否碰到挡光障碍
			if(pObstacle[nIdx].nObstacle != 0 && pObstacle[nIdx].nObstacle != nObstacle)
				return false;
		}
	}
}

/*
bool KIpoTree::CanLighting(int x1, int y1, int x2, int y2)
{
	int gx1, gy1, gx2, gy2, gStep, gx, gy, nIdx;
	gx1 = x1 / LIGHTING_GRID_SIZEX;
	gy1 = y1 / LIGHTING_GRID_SIZEX;
	gx2 = x2 / LIGHTING_GRID_SIZEX;
	gy2 = y2 / LIGHTING_GRID_SIZEX;
	if(gx1 == gx2 && gy1 == gy2)
		return true;

	int nObstacle = pObstacle[gy1 * LIGHTING_GRID_WIDTH + gx1].nObstacle;
	KVector2 vDir = pObstacle[gy1 * LIGHTING_GRID_WIDTH + gx1].vDir;
	KVector2 vLight;
	vLight.fX = (float)(x1 - x2);
	vLight.fY = (float)(y1 - y2);

	float fx1, fy1, fx2, fy2, fdx, fdy, fStepX, fStepY;
	fx1 = (float)x1, fy1 =(float) y1, fx2 = (float)x2, fy2 = (float)y2;
	fdx = (float)fabs(fx2 - fx1);
	fdy = (float)fabs(fy2 - fy1);

	// 斜率小于45度的情况
	if(fdx >= fdy)
	{
		// 计算线段前进累加值
		if(x2 > x1)
		{
			gStep = 1;
			fStepX = LIGHTING_GRID_SIZEX;
		}
		else
		{
			gStep = -1;
			fStepX = -LIGHTING_GRID_SIZEX;
		}
		if(y2 > y1)
			fStepY = fdy / fdx * LIGHTING_GRID_SIZEX;
		else
			fStepY = -fdy / fdx * LIGHTING_GRID_SIZEX;

		while(1)
		{
			fx1 += fStepX;
			fy1 += fStepY;
			gx1 += gStep;
			if(gx1 == gx2)
			{
				// 没有碰到障碍，根据这点本身是否障碍和障碍方向判断是否受光
				if(nObstacle && vDir.fX != 0.0f)
				{
					if(vDir.fX * vLight.fY - vDir.fY * vLight.fX < 0.0f)
						return true;
					else
						return false;
				}
				else
					return true;
			}

			gx = ((int)fx1) / LIGHTING_GRID_SIZEX;
			gy = ((int)fy1) / LIGHTING_GRID_SIZEX;
			nIdx = gy * LIGHTING_GRID_WIDTH + gx;
			// 检查是否碰到挡光障碍
			if(pObstacle[nIdx].nObstacle != 0 && pObstacle[nIdx].nObstacle != nObstacle)
				return false;
		}
	}
	// 斜率大于45度的情况
	else
	{
		if(y2 > y1)
		{
			gStep = 1;
			fStepY = LIGHTING_GRID_SIZEX;
		}
		else
		{
			gStep = -1;
			fStepY = -LIGHTING_GRID_SIZEX;
		}
		if(x2 > x1)
			fStepX = fdx / fdy * LIGHTING_GRID_SIZEX;
		else
			fStepX = -fdx / fdy * LIGHTING_GRID_SIZEX;
		while(1)
		{
			fx1 += fStepX;
			fy1 += fStepY;
			gy1 += gStep;
			if(gy1 == gy2)
			{
				// 没有碰到障碍，根据这点本身是否障碍和障碍方向判断是否受光
				if(nObstacle && vDir.fX != 0.0f)
				{
					if(vDir.fX * vLight.fY - vDir.fY * vLight.fX < 0.0f)
						return true;
					else
						return false;
				}
				else
					return true;
			}

			gx = ((int)fx1) / LIGHTING_GRID_SIZEX;
			gy = ((int)fy1) / LIGHTING_GRID_SIZEX;
			nIdx = gy * LIGHTING_GRID_WIDTH + gx;
			// 检查是否碰到挡光障碍
			if(pObstacle[nIdx].nObstacle != 0 && pObstacle[nIdx].nObstacle != nObstacle)
				return false;
		}
	}
}
*/
void KIpoTree::AddBuildinLight(KBuildInLightInfo* pLights, int nCount)
{
	if(!m_bProcessBioLights)
		return;

	int nMinRange, nMaxRange;

	for(int i=0; i<nCount; i++)
	{
		// 增加一个光源
		KSelfBreathLight *pLight = new KSelfBreathLight;
		if(pLight)
		{
			// 保证nMaxRange大于nMinRange
			if(pLights[i].nMaxRange >= pLights[i].nMinRange)
				nMaxRange = pLights[i].nMaxRange, nMinRange = pLights[i].nMinRange;
			else
				nMaxRange = pLights[i].nMinRange, nMinRange = pLights[i].nMaxRange;
			// 半径为负值，忽略这个光源
			if(nMinRange < 0)
			{
				_ASSERT(0);
				continue;
			}

			nMinRange = nMaxRange - 8;

			pLight->m_dwColor = pLights[i].dwColor;
			pLight->m_oPosition.nX = pLights[i].oPos.x;
			pLight->m_oPosition.nY = pLights[i].oPos.y;
			pLight->m_oPosition.nZ = pLights[i].oPos.z;
			pLight->m_nRadius = nMinRange + g_Random(nMaxRange - nMinRange);
			pLight->fRadius = (float)(pLight->m_nRadius);
			pLight->m_pParent = NULL;
			pLight->fMaxRange = (float)nMaxRange;
			pLight->fMinRange = (float)nMinRange;
			pLight->fCycle = 100 * 0.875f + g_Random(60 / 4);
			m_LightList.push_back(pLight);
		}
	}
}

void KIpoTree::EnableBioLights(bool bEnable)
{
	m_bProcessBioLights = bEnable;

	if(!bEnable)
	{
		//清除所有内建对象的光源
		list<KLightBase*>::iterator i;
		for (i = m_LightList.begin(); i != m_LightList.end(); )
		{
			if((*i)->m_pParent == NULL)
			{
				delete (*i);
				i = m_LightList.erase(i);
				continue;
			}
			++i;
		}
	}
}

void KIpoTree::EnableDynamicLights(bool bEnable)
{
	m_bDynamicLighting = bEnable;
}













