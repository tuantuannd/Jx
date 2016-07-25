// ***************************************************************************************
// 场景地图的区域对象的类定义实现
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-11
// ***************************************************************************************
#include "KCore.h"
#include "KEngine.h"
#include "KScenePlaceRegionC.h"
#include "crtdbg.h"
#include "../ImgRef.h"
#include "../../Represent/iRepresent/iRepresentShell.h"
#include "KIpotLeaf.h"
#include "SceneDataDef.h"
#include "SceneMath.h"
#include "ObstacleDef.h"
#include "math.h"
#ifdef SWORDONLINE_SHOW_DBUG_INFO
#include "../GameDataDef.h"
#include "../KSubWorld.h"
#endif

#define	OBJ_IS_COPY_FROM_NEST_REGION	0xff
#define OBJ_IS_SELF_OWNED				4

#define OBJ_IMAGE_ID					oPos2.x
#define	OBJ_IMAGE_ISPOSITION			oPos2.y

#define	LOCAL_MAX_IMG_NUM	80

//#define	OUTPUT_PROCESS_TIME

//##ModelId=3DD7EA9200B2
KScenePlaceRegionC::KScenePlaceRegionC()
{
	m_Flag = 0;
	m_Status = REGION_S_IDLE;
	m_LeftTopCornerScenePos.x = m_LeftTopCornerScenePos.y = 0;
	m_RegionIndex.x = m_RegionIndex.y = 0;
	memset(&m_GroundLayerData, 0, sizeof(KGroundLayerData));
	memset(&m_BiosData, 0, sizeof(KBiosData));
	m_pPrerenderGroundImg = NULL;

	memset(m_TrapInfo, 0, sizeof(m_TrapInfo));
}

//##ModelId=3DD7EA9200F8
KScenePlaceRegionC::~KScenePlaceRegionC()
{
	Clear();
}

//##ModelId=3DD3DD2C0141
void KScenePlaceRegionC::GetRegionIndex(int& nX, int& nY) const
{
	nX = m_RegionIndex.x;
	nY = m_RegionIndex.y;
}

//##ModelId=3DBDAC140299
bool KScenePlaceRegionC::ToLoad(int nIndexX, int nIndexY)
{
	_ASSERT (m_Status != REGION_S_LOADING);
	Clear();
	m_RegionIndex.x = nIndexX;
	m_RegionIndex.y = nIndexY;
	m_Status = REGION_S_TO_LOAD;
	return true;
}

//##ModelId=3DB901F101CD
bool KScenePlaceRegionC::Load(const char* pszBaseFolderName)
{
	if (m_Status != REGION_S_TO_LOAD || pszBaseFolderName == NULL || pszBaseFolderName[0] == 0)
		return false;

	m_Status = REGION_S_LOADING;
	
	char	RegionPathPrefix[180];
	sprintf(RegionPathPrefix, "%s\\v_%03d\\%03d_", pszBaseFolderName, m_RegionIndex.y, m_RegionIndex.x);
	m_LeftTopCornerScenePos.x = m_RegionIndex.x * RWPP_AREGION_WIDTH;
	m_LeftTopCornerScenePos.y = m_RegionIndex.y * RWPP_AREGION_HEIGHT;

	char	File[256];
	sprintf(File, "%s"REGION_COMBIN_FILE_NAME_CLIENT, RegionPathPrefix);
	KPakFile	Data;
	if (Data.Open(File))
	{
		unsigned int uMaxElemFile = 0;
		Data.Read(&uMaxElemFile, sizeof(unsigned int));
		KCombinFileSection	ElemFile[REGION_ELEM_FILE_COUNT] = { 0 };
		if (uMaxElemFile > REGION_ELEM_FILE_COUNT)
		{
			Data.Read(&ElemFile[0], sizeof(KCombinFileSection) * REGION_ELEM_FILE_COUNT);
			Data.Seek(sizeof(KCombinFileSection) * (uMaxElemFile - REGION_ELEM_FILE_COUNT), FILE_CURRENT);
		}
		else
		{
			Data.Read(&ElemFile[0], sizeof(KCombinFileSection) * uMaxElemFile);
		}
		
		unsigned int uOffsetAhead = sizeof(unsigned int) + sizeof(KCombinFileSection) * uMaxElemFile;

		//--地表层--
		if (ElemFile[REGION_GROUND_LAYER_FILE_INDEX].uLength)
		{
			Data.Seek(uOffsetAhead + ElemFile[REGION_GROUND_LAYER_FILE_INDEX].uOffset, FILE_BEGIN);
			LoadGroundLayer(&Data, ElemFile[REGION_GROUND_LAYER_FILE_INDEX].uLength);
		}
		//--内建对象--
		if (ElemFile[REGION_BUILDIN_OBJ_FILE_INDEX].uLength)
		{
			Data.Seek(uOffsetAhead + ElemFile[REGION_BUILDIN_OBJ_FILE_INDEX].uOffset, FILE_BEGIN);
			LoadAboveGroundObjects(&Data, ElemFile[REGION_BUILDIN_OBJ_FILE_INDEX].uLength);
		}
		//--障碍--
		if (ElemFile[REGION_OBSTACLE_FILE_INDEX].uLength)
		{
			Data.Seek(uOffsetAhead + ElemFile[REGION_OBSTACLE_FILE_INDEX].uOffset, FILE_BEGIN);
			LoadObstacle(&Data, ElemFile[REGION_OBSTACLE_FILE_INDEX].uLength);
		}
		else
		{
			LoadObstacle(NULL, 0);
		}
		//--陷阱--
		if (ElemFile[REGION_TRAP_FILE_INDEX].uLength)
		{
			Data.Seek(uOffsetAhead + ElemFile[REGION_TRAP_FILE_INDEX].uOffset, FILE_BEGIN);
			LoadTrap(&Data, ElemFile[REGION_TRAP_FILE_INDEX].uLength);
		}
		Data.Close();
	}
	else
	{
		unsigned int uSize;
		//--地表层--
		sprintf(File, "%s"REGION_GROUND_LAYER_FILE, RegionPathPrefix);
		if (Data.Open(File))
		{
			uSize = Data.Size();
			LoadGroundLayer(&Data, uSize);
		}		
		//--内建对象--
		sprintf(File, "%s"REGION_BUILDIN_OBJ_FILE, RegionPathPrefix);
		if (Data.Open(File))
		{
			uSize = Data.Size();
			LoadAboveGroundObjects(&Data, uSize);
		}
		//--障碍--
		sprintf(File, "%s"REGION_OBSTACLE_FILE, RegionPathPrefix);
		if (Data.Open(File))
		{
			uSize = Data.Size();
			LoadObstacle(&Data, uSize);
		}
		else
			LoadObstacle(NULL, 0);
		//--陷阱--
		sprintf(File, "%s"REGION_TRAP_FILE, RegionPathPrefix);
		if (Data.Open(File))
		{
			uSize = Data.Size();
			LoadTrap(&Data, uSize);
		}
	}
	m_Status = REGION_S_STANDBY;
	
	return true;
}

//##ModelId=3DBF876400B2
void KScenePlaceRegionC::Clear()
{
	if (m_Status != REGION_S_STANDBY)
		return;
	m_Flag = 0;
	if (m_pPrerenderGroundImg)
	{
		//用KRUImage::GROUND_IMG_OCCUPY_FLAG(bMatchReferenceSpot)来表示KRUImage对象是否被占用
		//用KRUImage::GROUND_IMG_OK_FLAG(bFrameDraw)来表示KRUImage对象是否已经预渲染好了
		m_pPrerenderGroundImg->GROUND_IMG_OCCUPY_FLAG = false;
		m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = false;
		m_pPrerenderGroundImg = NULL;
	}

	m_GroundLayerData.uNumGrunode = 0;
	m_GroundLayerData.uNumObject = 0;
	if (m_GroundLayerData.pGrunodes)
	{
		free(m_GroundLayerData.pGrunodes);
		m_GroundLayerData.pGrunodes = NULL;
	}
	if (m_GroundLayerData.pObjects)
	{
		free(m_GroundLayerData.pObjects);
		m_GroundLayerData.pObjects = NULL;
	}
	if (m_BiosData.pLights)
		free(m_BiosData.pLights);
	if (m_BiosData.pBios)
		free (m_BiosData.pBios);
	if (m_BiosData.pLeafs)
		free (m_BiosData.pLeafs);
	memset(&m_BiosData, 0, sizeof(KBiosData));
	m_Status = REGION_S_STANDBY;
}

//##ModelId=3DBF9582039A
bool KScenePlaceRegionC::PrerenderGround(bool bForce)
{
	if (g_pRepresent == NULL || m_pPrerenderGroundImg == NULL ||
		(bForce == false && m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG))
		return true;

	m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = true;

	g_pRepresent->ClearImageData(m_pPrerenderGroundImg->szImage,
		m_pPrerenderGroundImg->uImage, m_pPrerenderGroundImg->nISPosition);

	KRUImage	ImgList[LOCAL_MAX_IMG_NUM];
	KRUImage	*pGi;
	unsigned int nIndex;
	int			CellWidth  = RWPP_AREGION_WIDTH / RWP_NUM_GROUND_CELL_H;
	int			CellHeight = RWPP_AREGION_HEIGHT / 2 / RWP_NUM_GROUND_CELL_V;

	memset(&ImgList, 0, sizeof(ImgList));
	int			nNum = 0;
	pGi = &ImgList[0];

	//----地最底表面:草地、水等----
	KSPRCrunode* pGrunode = m_GroundLayerData.pGrunodes;
	for (nIndex = 0; nIndex < m_GroundLayerData.uNumGrunode; nIndex++)
	{
		pGi->bRenderStyle = IMAGE_RENDER_STYLE_OPACITY;
		pGi->nType = ISI_T_SPR;
		pGi->oPosition.nX = pGrunode->Param.h * CellWidth;
		pGi->oPosition.nY = pGrunode->Param.v * CellHeight;
		memcpy(pGi->szImage, pGrunode->szImgName, pGrunode->Param.nFileNameLen);
		pGi->nFrame = pGrunode->Param.nFrame;
		pGi->uImage = 0;
		pGi->nISPosition = IMAGE_IS_POSITION_INIT;
		pGrunode = (KSPRCrunode*)(((char*)pGrunode) +
			sizeof(KSPRCrunode::KSPRCrunodeParam) + pGrunode->Param.nFileNameLen);
		nNum++;
		if (nNum < LOCAL_MAX_IMG_NUM)
		{
			pGi++;
		}
		else
		{
			g_pRepresent->DrawPrimitivesOnImage(LOCAL_MAX_IMG_NUM, &ImgList[0], RU_T_IMAGE,
				m_pPrerenderGroundImg->szImage, m_pPrerenderGroundImg->uImage,
				m_pPrerenderGroundImg->nISPosition);
			nNum = 0;
			pGi = &ImgList[0];
		}
	}

	//----紧贴地表面的对象:路面等----
	KSPRCoverGroundObj* pObj = m_GroundLayerData.pObjects;
	for (nIndex = 0; nIndex < m_GroundLayerData.uNumObject; nIndex++, pObj++)
	{
		pGi->bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
		pGi->nType = ISI_T_SPR;
		pGi->bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
		pGi->Color.Color_b.a = 255;
		pGi->oPosition.nX = pObj->nPositionX - m_LeftTopCornerScenePos.x;
		pGi->oPosition.nY = (pObj->nPositionY - m_LeftTopCornerScenePos.y) / 2;
		strcpy(pGi->szImage, pObj->szImage);
		pGi->nFrame = pObj->nFrame;
		pGi->uImage = 0;
		pGi->nISPosition = IMAGE_IS_POSITION_INIT;
		nNum++;
		if (nNum < LOCAL_MAX_IMG_NUM)
		{
			pGi++;
		}
		else
		{
			g_pRepresent->DrawPrimitivesOnImage(LOCAL_MAX_IMG_NUM, &ImgList[0], RU_T_IMAGE,
				m_pPrerenderGroundImg->szImage, m_pPrerenderGroundImg->uImage,
				m_pPrerenderGroundImg->nISPosition);
			nNum = 0;
			pGi = &ImgList[0];
		}
	}

	if (nNum)
	{
		g_pRepresent->DrawPrimitivesOnImage(nNum, &ImgList[0], RU_T_IMAGE,
			m_pPrerenderGroundImg->szImage, m_pPrerenderGroundImg->uImage,
			m_pPrerenderGroundImg->nISPosition);
	}
	return true;
}

/*
//##ModelId=3DBF9582039A
bool KScenePlaceRegionC::PrerenderGround()
{
	if (g_pRepresent == NULL || m_pPrerenderGroundImg == NULL ||
		m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG)
		return true;

	m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = true;

	g_pRepresent->ClearImageData(m_pPrerenderGroundImg->szImage,
		m_pPrerenderGroundImg->uImage, m_pPrerenderGroundImg->nImagePosition);

	KRUImage	gi;
	unsigned int nIndex;
	int			CellWidth  = RWPP_AREGION_WIDTH / RWP_NUM_GROUND_CELL_H;
	int			CellHeight = RWPP_AREGION_HEIGHT / 2 / RWP_NUM_GROUND_CELL_V;

	memset(&gi, 0, sizeof(gi));

	//----地最底表面:草地、水等----
	KSPRCrunode* pGrunode = m_GroundLayerData.pGrunodes;
	gi.bRenderStyle = IMAGE_RENDER_STYLE_OPACITY;
	gi.nType = ISI_T_SPR;
	for (nIndex = 0; nIndex < m_GroundLayerData.uNumGrunode; nIndex++)
	{
		gi.nX = pGrunode->Param.h * CellWidth;
		gi.nY = pGrunode->Param.v * CellHeight;
		memcpy(gi.szImage, pGrunode->szImgName, pGrunode->Param.nFileNameLen);
		gi.szImage[pGrunode->Param.nFileNameLen] = 0;
		gi.nFrame = pGrunode->Param.nFrame;
		g_pRepresent->DrawPrimitivesOnImage(1, &gi, RU_T_IMAGE,
			m_pPrerenderGroundImg->szImage, m_pPrerenderGroundImg->uImage,
			m_pPrerenderGroundImg->nImagePosition);
		gi.uImage = 0;
		gi.szImage[0] = 0;
		gi.nImagePosition = IMAGE_IS_POSITION_INIT;
		pGrunode = (KSPRCrunode*)(((char*)pGrunode) +
			sizeof(KSPRCrunode::KSPRCrunodeParam) + pGrunode->Param.nFileNameLen);
	}

	//----紧贴地表面的对象:路面等----
	
	gi.bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
	gi.bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
	gi.Color.Color_b.a = 255;
	KSPRCoverGroundObj* pObj = m_GroundLayerData.pObjects; 
	for (nIndex = 0; nIndex < m_GroundLayerData.uNumObject; nIndex++, pObj++)
	{
		gi.nX = pObj->nPositionX - m_LeftTopCornerScenePos.x;
		gi.nY = (pObj->nPositionY - m_LeftTopCornerScenePos.y) / 2;
		strcpy(gi.szImage, pObj->szImage);
		gi.nFrame = pObj->nFrame;
		g_pRepresent->DrawPrimitivesOnImage(1, &gi, RU_T_IMAGE,
				m_pPrerenderGroundImg->szImage, m_pPrerenderGroundImg->uImage,
				m_pPrerenderGroundImg->nImagePosition);
		gi.uImage = 0;
		gi.szImage[0] = 0;
		gi.nImagePosition = IMAGE_IS_POSITION_INIT;
	}
	
	return true;
}*/

void KScenePlaceRegionC::SetNestRegion(KScenePlaceRegionC* pNest)
{
	if (pNest == NULL || pNest == this)
		return;

	int		nDestX = pNest->m_RegionIndex.x - m_RegionIndex.x;
	int		nDestY = pNest->m_RegionIndex.y - m_RegionIndex.y;

	if (nDestX * nDestX > 1 || nDestY * nDestY > 1)
		return;	//非相邻区域

	unsigned char cRelate = (nDestX * 3 + nDestY + 4);
	unsigned int  i = RWP_NEST_REGION_0 << cRelate;
	if (m_Flag & i)
		return;
	m_Flag |= i;

	RECT	rcthis;
	rcthis.left  = m_LeftTopCornerScenePos.x;
	rcthis.right = m_LeftTopCornerScenePos.x + RWPP_AREGION_WIDTH;
	rcthis.top   = m_LeftTopCornerScenePos.y;
	rcthis.bottom = m_LeftTopCornerScenePos.y +RWPP_AREGION_HEIGHT;
	
	KSPRCoverGroundObj*	pObj = pNest->m_GroundLayerData.pObjects;
	int		nCount = 0;
	for (i = 0; i < pNest->m_GroundLayerData.uNumObject; i++, pObj++)
	{
		if (pObj->bRelateRegion == OBJ_IS_SELF_OWNED &&
			pObj->nPositionX < rcthis.right && pObj->nPositionX + pObj->nWidth > rcthis.left &&
			pObj->nPositionY < rcthis.bottom &&
			pObj->nPositionY + pObj->nHeight + pObj->nHeight > rcthis.top)
		{
			pObj->bRelateRegion = OBJ_IS_COPY_FROM_NEST_REGION;
			nCount++;
		}
	}
	if (nCount == 0)
		return;

	pObj = (KSPRCoverGroundObj*)realloc(m_GroundLayerData.pObjects,
		sizeof(KSPRCoverGroundObj) * (m_GroundLayerData.uNumObject + nCount));
	if (pObj == NULL)
		return;
	m_GroundLayerData.pObjects = pObj;
	pObj = pNest->m_GroundLayerData.pObjects;
	for (i = 0; i < pNest->m_GroundLayerData.uNumObject; i++, pObj++)
	{
		if (pObj->bRelateRegion == OBJ_IS_COPY_FROM_NEST_REGION)
		{
			pObj->bRelateRegion = cRelate;
			AddGrundCoverObj(pObj);
			pObj->bRelateRegion = OBJ_IS_SELF_OWNED;
		}
	}
	//标记区域地表图为未经渲染，既要重新渲染
	if (m_pPrerenderGroundImg)
		m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = false;
}


//##ModelId=3DB9001D00BD
bool KScenePlaceRegionC::LoadGroundLayer(KPakFile* pDataFile, unsigned int uSize)
{
	//_ASSERT(pDataFile);
	KGroundFileHead	Head = { 0 };
	unsigned int uStartPos = pDataFile->Tell();
	pDataFile->Read(&Head, sizeof(KGroundFileHead));
	if (Head.uNumGrunode)
	{
		int nSize = Head.uObjectDataOffset - sizeof(KGroundFileHead);
		m_GroundLayerData.pGrunodes = (KSPRCrunode*)malloc(nSize);
		if (m_GroundLayerData.pGrunodes)
		{
			pDataFile->Read(m_GroundLayerData.pGrunodes, nSize);
			m_GroundLayerData.uNumGrunode = Head.uNumGrunode;
		}
		else
			return false;
	}
	if (Head.uNumObject)
	{
		uSize -= Head.uObjectDataOffset;
		m_GroundLayerData.pObjects = (KSPRCoverGroundObj*)malloc(uSize);
		if (m_GroundLayerData.pObjects)
		{
			pDataFile->Seek(Head.uObjectDataOffset + uStartPos, FILE_BEGIN);
			pDataFile->Read(m_GroundLayerData.pObjects, uSize);
			m_GroundLayerData.uNumObject = Head.uNumObject;
			for (unsigned int i = 0; i < Head.uNumObject; i++)
				m_GroundLayerData.pObjects[i].bRelateRegion = OBJ_IS_SELF_OWNED;
		}
		else
			return false;
	}
	return true;
}

void KScenePlaceRegionC::AddGrundCoverObj(KSPRCoverGroundObj* pObj)
{
//	_ASSERT(pObj);
	int nDest = m_GroundLayerData.uNumObject;
	while(nDest > 0)
	{
		KSPRCoverGroundObj* pSelfObj = &m_GroundLayerData.pObjects[nDest - 1];
		if (pSelfObj->bOrder >  pObj->bOrder)
			nDest --;
		else if (pSelfObj->bOrder == pObj->bOrder)
		{
			if(pSelfObj->nLayer >  pObj->nLayer)
				nDest --;
			else if (pSelfObj->nLayer <  pObj->nLayer)
				break;
			else
			{
				while(nDest > 0)
				{
					pSelfObj = &m_GroundLayerData.pObjects[nDest - 1];
					if (pSelfObj->bOrder == pObj->bOrder &&
						pSelfObj->nLayer == pObj->nLayer &&
						(pSelfObj->nPositionY > pObj->nPositionY ||
							(pSelfObj->nPositionY == pObj->nPositionY &&
							 pSelfObj->bRelateRegion > pObj->bRelateRegion)))
					{
						nDest--;
					}
					else
						break;					
				}
				break;
			}
		}
		else
			break;		
	};
	for (int i = m_GroundLayerData.uNumObject; i > nDest; i--)
		m_GroundLayerData.pObjects[i] = m_GroundLayerData.pObjects[i - 1];
	m_GroundLayerData.pObjects[nDest] = *pObj;
	m_GroundLayerData.uNumObject++;
}

//##ModelId=3DB90015018D
void KScenePlaceRegionC::LoadObstacle(KPakFile* pDataFile, unsigned int uSize)
{
	if (pDataFile && uSize >= sizeof(m_ObstacleInfo))
		pDataFile->Read((LPVOID)m_ObstacleInfo, sizeof(m_ObstacleInfo));
	else
		ZeroMemory(m_ObstacleInfo, sizeof(m_ObstacleInfo));
}

void KScenePlaceRegionC::LoadTrap(KPakFile* pDataFile, unsigned int uSize)
{
	//_ASSERT(pDataFile);
	KTrapFileHead	sTrapFileHead = { 0 };
	KSPTrap			sTrapCell;

	pDataFile->Read(&sTrapFileHead, sizeof(KTrapFileHead));
	for (DWORD i = 0; i < sTrapFileHead.uNumTrap; i++)
	{
		pDataFile->Read(&sTrapCell, sizeof(KSPTrap));
		if (sTrapCell.cY >= RWP_NUM_GROUND_CELL_V * 2 || sTrapCell.cX + sTrapCell.cNumCell - 1 >= RWP_NUM_GROUND_CELL_H)
			continue;
		for (int j = 0; j < sTrapCell.cNumCell; j++)
		{
			m_TrapInfo[sTrapCell.cX + j][sTrapCell.cY] = sTrapCell.uTrapId;
		}
	}
}

//##ModelId=3DBDADE00001
bool KScenePlaceRegionC::LoadAboveGroundObjects(KPakFile* pDataFile, unsigned int uSize)
{
	_ASSERT(pDataFile);
	pDataFile->Read(&m_BiosData.Numbers, sizeof(KBuildinObjFileHead));
	if (uSize < m_BiosData.Numbers.nNumBios * sizeof(KBuildinObj) +
		m_BiosData.Numbers.nNumBulidinLight * sizeof(KBuildInLightInfo) +
		sizeof(KBuildinObjFileHead))
	{
		memset(&m_BiosData.Numbers, 0, sizeof(KBuildinObjFileHead));
		return false;
	}

	int nSize, nRead;
	if (m_BiosData.Numbers.nNumBios)
	{
		nSize = sizeof(KBuildinObj) * m_BiosData.Numbers.nNumBios;
		m_BiosData.pBios = (KBuildinObj*)malloc(nSize);
		if (m_BiosData.pBios == NULL)
		{
			memset(&m_BiosData.Numbers, 0, sizeof(KBuildinObjFileHead));
			return false;
		}

		nRead = pDataFile->Read(m_BiosData.pBios, nSize);
		if (nRead < nSize)
			memset(((char*)m_BiosData.pBios) + nRead, 0, nSize - nRead);

		for (unsigned int i = m_BiosData.Numbers.nNumBios - m_BiosData.Numbers.nNumBiosAbove;
				i < m_BiosData.Numbers.nNumBios; i++)
		{
			m_BiosData.pBios[i].OBJ_IMAGE_ID = 0;
			m_BiosData.pBios[i].OBJ_IMAGE_ISPOSITION = -1;
			if (m_BiosData.pBios[i].nAniSpeed)
				m_BiosData.pBios[i].nAniSpeed = 1;
//			if(!isLoading()) {
//				getSPR(m_BiosData.pBios[i].szImage, m_BiosData.pBios[i].nImgNumFrames, false);
//				releaseSPR();
//			}
		}
	}

	if (m_BiosData.Numbers.nNumBulidinLight)
	{
		nSize = sizeof(KBuildInLightInfo) * m_BiosData.Numbers.nNumBulidinLight;
		m_BiosData.pLights = (KBuildInLightInfo*)malloc(nSize);
		if (m_BiosData.pLights == NULL)
		{
			m_BiosData.Numbers.nNumBulidinLight = 0;
			return false;
		}

		nRead = pDataFile->Read(m_BiosData.pLights, nSize);
		if (nRead < nSize)
			memset(((char*)m_BiosData.pLights) + nRead, 0, nSize - nRead);
	}
	return true;
}

void KScenePlaceRegionC::PaintGroundDirect()
{
	if (g_pRepresent == NULL)
		return;

	KRUImage	ImgList[LOCAL_MAX_IMG_NUM];
	KRUImage	*pGi;
	unsigned int nIndex;
	int			CellWidth  = RWPP_AREGION_WIDTH / RWP_NUM_GROUND_CELL_H;
	int			CellHeight = RWPP_AREGION_HEIGHT / 2 / RWP_NUM_GROUND_CELL_V;

	memset(&ImgList, 0, sizeof(ImgList));
	int			nNum = 0;
	pGi = &ImgList[0];

	//----地最底表面:草地、水等----
	KSPRCrunode* pGrunode = m_GroundLayerData.pGrunodes;
	for (nIndex = 0; nIndex < m_GroundLayerData.uNumGrunode; nIndex++)
	{
		pGi->bRenderStyle = IMAGE_RENDER_STYLE_OPACITY;
		pGi->nType = ISI_T_SPR;
		pGi->oPosition.nX = pGrunode->Param.h * CellWidth + m_LeftTopCornerScenePos.x;
		pGi->oPosition.nY = (pGrunode->Param.v * CellHeight) * 2 + m_LeftTopCornerScenePos.y;
		memcpy(pGi->szImage, pGrunode->szImgName, pGrunode->Param.nFileNameLen);
		pGi->nFrame = pGrunode->Param.nFrame;
		pGi->uImage = 0;
		pGi->nISPosition = IMAGE_IS_POSITION_INIT;
		pGrunode = (KSPRCrunode*)(((char*)pGrunode) +
			sizeof(KSPRCrunode::KSPRCrunodeParam) + pGrunode->Param.nFileNameLen);
		nNum++;
		if (nNum < LOCAL_MAX_IMG_NUM)
		{
			pGi++;
		}
		else
		{
			g_pRepresent->DrawPrimitives(LOCAL_MAX_IMG_NUM, &ImgList[0], RU_T_IMAGE, false);
			nNum = 0;
			pGi = &ImgList[0];
		}
	}

	//----紧贴地表面的对象:路面等----
	KRUImagePart	ImgPart;
	ImgPart.Color.Color_dw = 0;
	ImgPart.bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
	ImgPart.bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
	ImgPart.nType = ISI_T_SPR;
	ImgPart.oPosition.nZ = 0;
	ImgPart.oEndPos.nZ = 0;
	ImgPart.szImage[0] = 0;
	POINT	RegionRBPos;
	RegionRBPos.x = m_LeftTopCornerScenePos.x + RWPP_AREGION_WIDTH;
	RegionRBPos.y = m_LeftTopCornerScenePos.y + RWPP_AREGION_HEIGHT;

	KSPRCoverGroundObj* pObj = m_GroundLayerData.pObjects;
	for (nIndex = 0; nIndex < m_GroundLayerData.uNumObject; nIndex++, pObj++)
	{
		
		if (pObj->bRelateRegion == OBJ_IS_SELF_OWNED &&
			pObj->nPositionX >= m_LeftTopCornerScenePos.x &&
			pObj->nPositionY >= m_LeftTopCornerScenePos.y &&
			pObj->nPositionX + pObj->nWidth < RegionRBPos.x&&
			pObj->nPositionY + pObj->nHeight * 2 < RegionRBPos.y)
		{
			pGi->bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
			pGi->nType = ISI_T_SPR;
			pGi->bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
			pGi->Color.Color_b.a = 255;
			pGi->oPosition.nX = pObj->nPositionX;
			pGi->oPosition.nY = pObj->nPositionY;
			strcpy(pGi->szImage, pObj->szImage);
			pGi->nFrame = pObj->nFrame;
			pGi->uImage = 0;
			pGi->nISPosition = IMAGE_IS_POSITION_INIT;
			nNum++;
			pGi++;
		}
		else
		{
			strcpy(ImgPart.szImage, pObj->szImage);
			ImgPart.nFrame = pObj->nFrame;
			ImgPart.uImage = 0;
			ImgPart.nISPosition = -1;
		
			if (pObj->nPositionX < m_LeftTopCornerScenePos.x)
				ImgPart.oImgLTPos.nX = m_LeftTopCornerScenePos.x - pObj->nPositionX;
			else
				ImgPart.oImgLTPos.nX = 0;
			if (pObj->nPositionX + pObj->nWidth >= RegionRBPos.x)
				ImgPart.oImgRBPos.nX = RegionRBPos.x - pObj->nPositionX;
			else
				ImgPart.oImgRBPos.nX = pObj->nWidth;

			if (pObj->nPositionY < m_LeftTopCornerScenePos.y)
				ImgPart.oImgLTPos.nY = (m_LeftTopCornerScenePos.y - pObj->nPositionY) / 2;
			else
				ImgPart.oImgLTPos.nY = 0;
			if (pObj->nPositionY + pObj->nHeight * 2 >= RegionRBPos.y)
				ImgPart.oImgRBPos.nY = (RegionRBPos.y - pObj->nPositionY) / 2;
			else
				ImgPart.oImgRBPos.nY = pObj->nHeight;

			ImgPart.oPosition.nX = pObj->nPositionX + ImgPart.oImgLTPos.nX;
			ImgPart.oPosition.nY = pObj->nPositionY + ImgPart.oImgLTPos.nY * 2;

			ImgPart.oEndPos.nX = pObj->nPositionX + ImgPart.oImgRBPos.nX;
			ImgPart.oEndPos.nY = pObj->nPositionY + ImgPart.oImgRBPos.nY * 2;
		}
		
		if (ImgPart.szImage[0] || nNum == LOCAL_MAX_IMG_NUM)
		{
			g_pRepresent->DrawPrimitives(nNum, &ImgList[0], RU_T_IMAGE, false);
			nNum = 0;
			pGi = &ImgList[0];
			if (ImgPart.szImage[0])
			{
				g_pRepresent->DrawPrimitives(1, &ImgPart, RU_T_IMAGE_PART, false);
				ImgPart.szImage[0] = 0;
			}
		}
	}

	if (nNum)
		g_pRepresent->DrawPrimitives(nNum, &ImgList[0], RU_T_IMAGE, false);
}

//##ModelId=3DDBD8C80309
void KScenePlaceRegionC::PaintGround()
{
	if (m_pPrerenderGroundImg)
		g_pRepresent->DrawPrimitives(1, m_pPrerenderGroundImg, RU_T_IMAGE, false);
	else
		PaintGroundDirect();
}

#ifdef SWORDONLINE_SHOW_DBUG_INFO
void KScenePlaceRegionC::PaintObstacle()
{
	KRULine	Line[4];
	int		nNumLine, i, j, nX, nY;

	for (i = 0; i < 4; i++)
	{
		Line[i].Color.Color_dw = 0xffffffff;
		Line[i].oPosition.nZ = 0;
		Line[i].oEndPos.nZ = 0;
	}
	
	nX = m_LeftTopCornerScenePos.x;
	for (i = 0; i < 16; i++, nX += RWP_OBSTACLE_WIDTH)
	{
		nY = m_LeftTopCornerScenePos.y;
		for (j = 0; j < 32; j++, nY += RWP_OBSTACLE_HEIGHT)
		{
			long nTrap = m_TrapInfo[i][j];
			if (nTrap > 0)
			{
				char	szDebugTrap[32];
				sprintf(szDebugTrap, "%x", nTrap);
				g_pRepresent->OutputText(12, szDebugTrap, KRF_ZERO_END, nX, nY, 0xffffffff, 0, 0);
			}

			int nIdx = SubWorld[0].FindRegion(MAKELONG(m_RegionIndex.x, m_RegionIndex.y));
			if (nIdx >= 0)
			{
				int nRef = SubWorld[0].m_Region[nIdx].GetRef(i, j, obj_npc);
				char	szDebugNpcRef[32];
				if (nRef)
				{
					sprintf(szDebugNpcRef, "%d", nRef);
					g_pRepresent->OutputText(12, szDebugNpcRef, KRF_ZERO_END, nX, nY, 0xffffffff, 0, 0);
				}
			}
			long nType = (m_ObstacleInfo[i][j] >> 4) & 0x0000000f;
			if (nType == Obstacle_Empty)
				continue;
			Line[2].oEndPos.nX = Line[1].oEndPos.nX = Line[0].oEndPos.nX =
				Line[2].oPosition.nX = Line[1].oPosition.nX = Line[0].oPosition.nX = nX;
			Line[2].oEndPos.nY = Line[1].oEndPos.nY = Line[0].oEndPos.nY =
				Line[2].oPosition.nY = Line[1].oPosition.nY = Line[0].oPosition.nY = nY;
			nNumLine = 0;
			switch(nType)
			{
			case Obstacle_Full:
				Line[0].oEndPos.nX += 32;	//上
				Line[1].oEndPos.nY += 32;	//左
				Line[2].oPosition.nX    += 32;	//右
				Line[2].oEndPos.nX += 32;
				Line[2].oEndPos.nY += 32;
				Line[3].oPosition.nX	  = Line[0].oPosition.nX;	//下
				Line[3].oPosition.nY    = Line[2].oEndPos.nY;
				Line[3].oEndPos.nX = Line[2].oEndPos.nX;
				Line[3].oEndPos.nY = Line[2].oEndPos.nY;
				nNumLine = 4;
				break;
			case Obstacle_LT:
				Line[0].oEndPos.nX += 32;	//上
				Line[1].oEndPos.nY += 32;	//左
				Line[2].oPosition.nY	  += 32;	//左斜
				Line[2].oEndPos.nX += 32;
				nNumLine = 3;
				break;
			case Obstacle_RT:
				Line[0].oEndPos.nX += 32;	//上
				Line[1].oPosition.nX    += 32;	//右
				Line[1].oEndPos.nX += 32;
				Line[1].oEndPos.nY += 32;
				Line[2].oEndPos.nX += 32;	//右斜
				Line[2].oEndPos.nY += 32;
				nNumLine = 3;
				break;
			case Obstacle_LB:
				Line[0].oEndPos.nY += 32;	//左
				Line[1].oPosition.nY    += 32;	//下
				Line[1].oEndPos.nX += 32;
				Line[1].oEndPos.nY += 32;
				Line[2].oEndPos.nX += 32;	//右斜
				Line[2].oEndPos.nY += 32;
				nNumLine = 3;
				break;
			case Obstacle_RB:
				Line[0].oPosition.nX    += 32;	//右
				Line[0].oEndPos.nX += 32;
				Line[0].oEndPos.nY += 32;
				Line[1].oPosition.nY	  += 32;	//左斜
				Line[1].oEndPos.nX += 32;
				Line[2].oPosition.nY    += 32;	//下
				Line[2].oEndPos.nX += 32;
				Line[2].oEndPos.nY += 32;
				nNumLine = 3;
				break;
			}
			g_pRepresent->DrawPrimitives(nNumLine, Line, RU_T_LINE, false);
		}
	}
}
#endif

//##ModelId=3DE29F360221
void KScenePlaceRegionC::PaintAboveHeadObj(KBuildinObj* pObj, RECT* pRepresentArea)
{
	//_ASSERT(pObj);

	KRUImage4	Img;
	Img.Color.Color_dw = 0;	
	Img.bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
	Img.nType = ISI_T_SPR;
	strcpy(Img.szImage, pObj->szImage);
	Img.nFrame = pObj->nFrame;
	Img.uImage = pObj->OBJ_IMAGE_ID;
	Img.nISPosition = pObj->OBJ_IMAGE_ISPOSITION;

	if (pObj->nAniSpeed == 0)
	{
		Img.bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
		if (SM_IsOutsideRepresentArea(pRepresentArea,
			pObj->ImgPos1.x, pObj->ImgPos1.y, pObj->ImgPos1.z,
			pObj->ImgPos2.x, pObj->ImgPos2.y, pObj->ImgPos2.z) == false)
		{
			Img.oPosition.nX = pObj->ImgPos1.x;
			Img.oPosition.nY = pObj->ImgPos1.y;
			Img.oPosition.nZ = pObj->ImgPos1.z;
			Img.oEndPos.nX = pObj->ImgPos3.x;
			Img.oEndPos.nY = pObj->ImgPos3.y;
			Img.oEndPos.nZ = pObj->ImgPos3.z;
			
			if ((pObj->Props & SPBIO_P_SORTMANNER_MASK) == SPBIO_P_SORTMANNER_POINT)
			{
				g_pRepresent->DrawPrimitives(1, &Img, RU_T_IMAGE, false);
			}
			else
			{
				Img.oSecondPos.nX = pObj->ImgPos2.x;
				Img.oSecondPos.nY = pObj->ImgPos2.y;
				Img.oSecondPos.nZ = pObj->ImgPos2.z;
				
				Img.oThirdPos.nX = pObj->ImgPos4.x;
				Img.oThirdPos.nY = pObj->ImgPos4.y;
				Img.oThirdPos.nZ = pObj->ImgPos4.z;
				
				Img.oImgLTPos.nX = 0;
				Img.oImgLTPos.nY = 0;
				Img.oImgRBPos.nX = pObj->nImgWidth;
				Img.oImgRBPos.nY = pObj->nImgHeight;
				
				g_pRepresent->DrawPrimitives(1, &Img, RU_T_IMAGE_4, false);
			}
		}
	}
	else
	{
		Img.bRenderFlag = RUIMAGE_RENDER_FLAG_REF_SPOT;
		Img.oPosition.nX = pObj->oPos1.x;
		Img.oPosition.nY = pObj->oPos1.y;
		Img.oPosition.nZ = pObj->oPos1.z;

		BuildinObjNextFrame(pObj);
	}
	pObj->OBJ_IMAGE_ID = Img.uImage;
	pObj->OBJ_IMAGE_ISPOSITION = Img.nISPosition;
}

//## 获取高空对象最大的绘图次序编号。
unsigned int KScenePlaceRegionC::GetAboveHeadLayer(KBuildinObj*& pObjsAboveHead)
{
	pObjsAboveHead = NULL;
	unsigned int uNum = 0;
	if (m_BiosData.Numbers.nNumBiosAbove && m_BiosData.pBios)
	{
		uNum = m_BiosData.Numbers.nNumBiosAbove;
		pObjsAboveHead = &m_BiosData.pBios[m_BiosData.Numbers.nNumBios - uNum];

	}
	return uNum;
}

//获得内建光源的信息
unsigned int KScenePlaceRegionC::GetBuildinLights(KBuildInLightInfo*& pLights)
{
	unsigned int uNum = 0;
	if (m_BiosData.Numbers.nNumBulidinLight && m_BiosData.pLights)
	{
		pLights = m_BiosData.pLights;
		uNum = m_BiosData.Numbers.nNumBulidinLight;
	}
	return uNum;
}


//##ModelId=3DE33AB30318
void KScenePlaceRegionC::GetBuildinObjs(
		KIpotBuildinObj*& pObjsPointList, unsigned int& nNumObjsPoint,
		KIpotBuildinObj*& pObjsLineList, unsigned int& nNumObjsLine,
		KIpotBuildinObj*& pObjsTreeList, unsigned int& nNumObjsTree)
{
	pObjsPointList = NULL;
	pObjsLineList = NULL;
	pObjsTreeList = NULL;
	nNumObjsPoint = 0;
	nNumObjsLine = 0;
	nNumObjsTree = 0;
	if (m_BiosData.pLeafs)
	{
		int	nCount = 0;
		if (nNumObjsPoint = m_BiosData.Numbers.nNumBiosPoint)
		{
			pObjsPointList = m_BiosData.pLeafs;
			nCount += nNumObjsPoint;
		}
		if (nNumObjsLine = m_BiosData.Numbers.nNumBiosLine)
		{
			pObjsLineList = &m_BiosData.pLeafs[nCount];
			nCount += nNumObjsLine;
		}
		if (nNumObjsTree = m_BiosData.Numbers.nNumBiosTree)
			pObjsTreeList = &m_BiosData.pLeafs[nCount];
	}
}

//## 获取区域内内建未初始化的对象的列表。
void KScenePlaceRegionC::GetBIOSBuildinObjs(
	KBuildinObj*& pObjsList, 
	unsigned int& nNumObjs 
)
{
    pObjsList = NULL;

    nNumObjs = m_BiosData.Numbers.nNumBios;
    if (nNumObjs > 0)
        pObjsList = m_BiosData.pBios;
}


void KScenePlaceRegionC::LeaveProcessArea()
{
	//用KRUImage::GROUND_IMG_OCCUPY_FLAG(bMatchReferenceSpot)来表示KRUImage对象是否被占用
	//用KRUImage::GROUND_IMG_OK_FLAG(bFrameDraw)来表示KRUImage对象是否已经预渲染好了
	if (m_pPrerenderGroundImg)
	{
		m_pPrerenderGroundImg->GROUND_IMG_OCCUPY_FLAG = false;
		m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = false;
	}
	m_pPrerenderGroundImg = NULL;
	if (m_BiosData.pLeafs)
	{
		free(m_BiosData.pLeafs);
		m_BiosData.pLeafs = NULL;
	}
}

void KScenePlaceRegionC::EnterProcessArea(KRUImage *pImage)
{
	//用KRUImage::GROUND_IMG_OCCUPY_FLAG(bMatchReferenceSpot)来表示KRUImage对象是否被占用
	//用KRUImage::GROUND_IMG_OK_FLAG(bFrameDraw)来表示KRUImage对象是否已经预渲染好了
	if (m_pPrerenderGroundImg != pImage)
	{
		if (m_pPrerenderGroundImg)
		{
			m_pPrerenderGroundImg->GROUND_IMG_OCCUPY_FLAG = false;
			m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = false;
		}
		if (m_pPrerenderGroundImg = pImage)
		{
			m_pPrerenderGroundImg->GROUND_IMG_OCCUPY_FLAG = true;
			m_pPrerenderGroundImg->GROUND_IMG_OK_FLAG = false;
			m_pPrerenderGroundImg->oPosition.nX = m_LeftTopCornerScenePos.x;
			m_pPrerenderGroundImg->oPosition.nY = m_LeftTopCornerScenePos.y;
			m_pPrerenderGroundImg->oEndPos.nX = m_pPrerenderGroundImg->oPosition.nX + RWPP_AREGION_WIDTH;
			m_pPrerenderGroundImg->oEndPos.nY = m_pPrerenderGroundImg->oPosition.nY + RWPP_AREGION_HEIGHT;
			m_pPrerenderGroundImg->oEndPos.nZ = m_pPrerenderGroundImg->oPosition.nZ = 0;
		}
	}

	unsigned int i;
	unsigned int nCount = m_BiosData.Numbers.nNumBios
							- m_BiosData.Numbers.nNumBiosAbove;
	KIpotBuildinObj* pLeaf = NULL;
	KBuildinObj*	 pObj  = NULL;

	if (m_BiosData.pLeafs == NULL && m_BiosData.pBios &&
		m_BiosData.Numbers.nNumBios > m_BiosData.Numbers.nNumBiosAbove)
	{		
		m_BiosData.pLeafs = (KIpotBuildinObj*) malloc(
			sizeof(KIpotBuildinObj) * nCount);
		if (m_BiosData.pLeafs)
		{
			pLeaf = m_BiosData.pLeafs;
			pObj = m_BiosData.pBios;
			for (i = 0; i < nCount; i++, pLeaf++, pObj++)
			{
				pLeaf->eLeafType = KIpotLeaf::IPOTL_T_BUILDIN_OBJ;

				pLeaf->uImage = 0;
				pLeaf->nISPosition = -1;
				pLeaf->pBio = pObj;

				pLeaf->oPosition.x = pObj->oPos1.x;
				pLeaf->oPosition.y = pObj->oPos1.y;
				pLeaf->oEndPos.x = pObj->oPos2.x;
				pLeaf->oEndPos.y = pObj->oPos2.y;

				pLeaf->fAngleXY  = pObj->fAngleXY;
				pLeaf->fNodicalY = pObj->fNodicalY;

				pLeaf->bClone = false;
				pLeaf->bImgPart = false;
				pLeaf->pBrother = NULL;
				pLeaf->pRChild = NULL;
				pLeaf->pLChild = NULL;
			}
			pLeaf = m_BiosData.pLeafs;
			nCount = m_BiosData.Numbers.nNumBiosPoint;
			for (i = 0; i < nCount; i++, pLeaf++)
				pLeaf->oPosition.y += POINT_LEAF_Y_ADJUST_VALUE;			
		}
	}
	else if(m_BiosData.pLeafs)
	{
		pLeaf = m_BiosData.pLeafs;
		pObj = m_BiosData.pBios;
		for (i = 0; i < nCount; i++, pLeaf++, pObj++)
		{
			pLeaf->oPosition.x = pObj->oPos1.x;
			pLeaf->oPosition.y = pObj->oPos1.y;
			pLeaf->oEndPos.x = pObj->oPos2.x;
			pLeaf->oEndPos.y = pObj->oPos2.y;
			pLeaf->fAngleXY  = pObj->fAngleXY;
			pLeaf->fNodicalY = pObj->fNodicalY;
			pLeaf->bImgPart = false;
		}
		pLeaf = m_BiosData.pLeafs;
		nCount = m_BiosData.Numbers.nNumBiosPoint;
		for (i = 0; i < nCount; i++, pLeaf++)
			pLeaf->oPosition.y += POINT_LEAF_Y_ADJUST_VALUE;			
	}
}

long KScenePlaceRegionC::GetObstacleInfo(int nX, int nY)
{
	int		nMpsX, nMpsY, nMapX, nMapY;
	long	lInfo, lRet, lType;
	nMpsX = nX - m_LeftTopCornerScenePos.x;
	nMpsY = nY - m_LeftTopCornerScenePos.y;

	nMapX = nMpsX / RWP_OBSTACLE_WIDTH;
	nMapY = nMpsY / RWP_OBSTACLE_HEIGHT;

	_ASSERT(nMapX >= 0 && nMapX < RWP_NUM_GROUND_CELL_H && nMapY >= 0 && nMapY < RWP_NUM_GROUND_CELL_V * 2);
	lInfo = m_ObstacleInfo[nMapX][nMapY];
	nMpsX -= nMapX * RWP_OBSTACLE_WIDTH;
	nMpsY -= nMapY * RWP_OBSTACLE_HEIGHT;
	lRet = lInfo & 0x0000000f;

	lType = (lInfo >> 4) & 0x0000000f;
	switch(lType)
	{
	case Obstacle_LT:
		if (nMpsX + nMpsY > RWP_OBSTACLE_WIDTH)
			lRet = Obstacle_NULL;
		break;
	case Obstacle_RT:
		if (nMpsX < nMpsY)
			lRet = Obstacle_NULL;
		break;
	case Obstacle_LB:
		if (nMpsX > nMpsY)
			lRet = Obstacle_NULL;
		break;
	case Obstacle_RB:
		if (nMpsX + nMpsY < RWP_OBSTACLE_WIDTH)
			lRet = Obstacle_NULL;
		break;
	default:
		break;
	}
	return lRet;
}

long KScenePlaceRegionC::GetObstacleInfoMin(int nX, int nY, int nOffX, int nOffY)
{

	int		nMpsX, nMpsY, nMapX, nMapY;
	long	lRet, lType;

	nMpsX = nX - m_LeftTopCornerScenePos.x;
	nMpsY = nY - m_LeftTopCornerScenePos.y;
	nMapX = nMpsX / RWP_OBSTACLE_WIDTH;
	nMapY = nMpsY / RWP_OBSTACLE_HEIGHT;
	nMpsX = ((nMpsX - nMapX * RWP_OBSTACLE_WIDTH) << 10) + nOffX;
	nMpsY = ((nMpsY - nMapY * RWP_OBSTACLE_HEIGHT) << 10) + nOffY;

	_ASSERT(nMapX >= 0 && nMapX < RWP_NUM_GROUND_CELL_H && nMapY >= 0 && nMapY < RWP_NUM_GROUND_CELL_V * 2);
	_ASSERT(nOffX >= 0 && nOffX < 1024 && nOffY >= 0 && nOffY < 1024);

	lRet = m_ObstacleInfo[nMapX][nMapY] & 0x0000000f;
	lType = (m_ObstacleInfo[nMapX][nMapY] >> 4) & 0x0000000f;

	switch(lType)
	{
	case Obstacle_LT:
		if (nMpsX + nMpsY > RWP_OBSTACLE_WIDTH << 10)
			lRet = Obstacle_NULL;
		break;
	case Obstacle_RT:
		if (nMpsX < nMpsY)
			lRet = Obstacle_NULL;
		break;
	case Obstacle_LB:
		if (nMpsX > nMpsY)
			lRet = Obstacle_NULL;
		break;
	case Obstacle_RB:
		if (nMpsX + nMpsY < RWP_OBSTACLE_WIDTH << 10)
			lRet = Obstacle_NULL;
		break;
	default:
		break;
	}
	return lRet;
}

void KScenePlaceRegionC::SetHightLightSpecialObject(unsigned int uBioIndex)
{
	if (m_BiosData.pBios && uBioIndex < m_BiosData.Numbers.nNumBios)
		m_BiosData.pBios[uBioIndex].Props |= SPBIO_F_HIGHT_LIGHT;
}
void KScenePlaceRegionC::UnsetHightLightSpecialObject(unsigned int uBioIndex)
{
	if (m_BiosData.pBios && uBioIndex < m_BiosData.Numbers.nNumBios)
		m_BiosData.pBios[uBioIndex].Props &= ~SPBIO_F_HIGHT_LIGHT;
}
