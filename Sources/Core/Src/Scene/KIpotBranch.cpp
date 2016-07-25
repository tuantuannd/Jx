// ***************************************************************************************
// 场景内对象树的树枝
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-19
// ---------------------------------------------------------------------------------------
// ***************************************************************************************
#include "KCore.h"
#include "Windows.h"
#include "KIpotLeaf.h"
#include "KIpotBranch.h"
#include "SceneMath.h"
#include <crtdbg.h>
#include "../CoreDrawGameObj.h"
#include "../ImgRef.h"
#include "../../Represent/iRepresent/iRepresentShell.h"
#include <math.h>

unsigned int	KIpotBranch::m_BranchFlag[2] = {IPOT_BF_HAVE_LEFT_BRANCH, IPOT_BF_HAVE_RIGHT_BRANCH};
#define	IS_BRANCH(i) (m_uFlag & m_BranchFlag[i])

//##ModelId=3DDAC4AC0238
KIpotBranch::KIpotBranch()
{
	m_uFlag = 0;
	m_pParent  = NULL;
	m_pSubBranch[0] = m_pSubBranch[1] = NULL;
	m_oHeadPoint.x = m_oEndPoint.x  = 0;
	m_oHeadPoint.y = m_oEndPoint.y = 0;
	m_pFirstObject = NULL;
	m_pObjectList = NULL;
	m_nNumObjects = 0;
	m_nNumObjectSpace = 0;
}

//##ModelId=3DDAC4AC0242
KIpotBranch::~KIpotBranch()
{
	Clear();
}

void KIpotBranch::EnumerateObjects(void* p, ObjectsCallbackFn CallbackFn)
{
	for (int i = 0; i < 2; i++)
	{
		if (IS_BRANCH(i))
		{
			m_pSubBranch[i]->EnumerateObjects(p, CallbackFn);
		}
		else if (m_pLeafs[i])
			KIpotLeaf_EnumerateObjects(m_pLeafs[i], p, CallbackFn);

		if (i == 0)
		{
			if (m_pFirstObject)
			{
				CallbackFn(p, m_pFirstObject);
				for (int j = 0; j < m_nNumObjects; j++)
					CallbackFn(p, m_pObjectList[j]);
			}
		}
	}
}

//##ModelId=3DDACA410279
void KIpotBranch::PaintObjectLayer(RECT* pRepresentArea)
{
	for (int i = 0; i < 2; i++)
	{
		if (IS_BRANCH(i))
		{
			m_pSubBranch[i]->PaintObjectLayer(pRepresentArea);
		}
		else if (m_pLeafs[i])
			KIpotLeaf_PaintObjectLayer(m_pLeafs[i], pRepresentArea);

		if (i == 0)
		{
			if (m_pFirstObject)
			{
				PaintABranchObject(m_pFirstObject, pRepresentArea);
				for (int j = 0; j < m_nNumObjects; j++)
					PaintABranchObject(m_pObjectList[j], pRepresentArea);
			}
		}
	}
}

void KIpotBranch::PaintABranchObject(KIpotBuildinObj* pObject,
									RECT* pRepresentArea) const
{
	_ASSERT(pObject && pRepresentArea);

	KRUImage4	Image;
	KBuildinObj* pBio = pObject->pBio;
	Image.Color.Color_dw = 0;
	Image.bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
	Image.bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
	Image.nType = ISI_T_SPR;
	Image.nFrame = pBio->nFrame;
	Image.nISPosition = pObject->nISPosition;
	strcpy(Image.szImage, pBio->szImage);
	Image.uImage = pObject->uImage;

	if (pBio->ImgPos3.y > pRepresentArea->top &&
		pBio->ImgPos3.x > pRepresentArea->left &&
		pBio->ImgPos4.x < pRepresentArea->right)
	{
		if (pObject->bImgPart == false)
		{
			Image.oPosition.nX = pBio->ImgPos1.x;
			Image.oPosition.nY = pBio->ImgPos1.y;
			Image.oPosition.nZ = pBio->ImgPos1.z;
			Image.oEndPos.nX = pBio->ImgPos3.x;
			Image.oEndPos.nY = pBio->ImgPos3.y;
			Image.oEndPos.nZ = pBio->ImgPos3.z;

			Image.oSecondPos.nX = pBio->ImgPos2.x;
			Image.oSecondPos.nY = pBio->ImgPos2.y;
			Image.oSecondPos.nZ = pBio->ImgPos2.z;

			Image.oThirdPos.nX = pBio->ImgPos4.x;
			Image.oThirdPos.nY = pBio->ImgPos4.y;
			Image.oThirdPos.nZ = pBio->ImgPos4.z;

			Image.oImgLTPos.nX = 0;
			Image.oImgLTPos.nY = 0;
			Image.oImgRBPos.nX = pBio->nImgWidth;
			Image.oImgRBPos.nY = pBio->nImgHeight;
		}
		else
		{
			int nEntireXRange = pBio->oPos2.x - pBio->oPos1.x;
			int nStartPart = pObject->oPosition.x - pBio->oPos1.x;
			int nEndPart = pObject->oEndPos.x - pBio->oPos1.x;

			Image.oSecondPos.nX = pBio->ImgPos2.x - pBio->ImgPos1.x;
			Image.oSecondPos.nY = pBio->ImgPos2.y - pBio->ImgPos1.y;
			Image.oSecondPos.nZ = pBio->ImgPos2.z - pBio->ImgPos1.z;

			Image.oEndPos.nX = pBio->ImgPos3.x - pBio->ImgPos4.x;
			Image.oEndPos.nY = pBio->ImgPos3.y - pBio->ImgPos4.y;
			Image.oEndPos.nZ = pBio->ImgPos3.z - pBio->ImgPos4.z;

			Image.oPosition.nX = pBio->ImgPos1.x + (Image.oSecondPos.nX * nStartPart) / nEntireXRange;
			Image.oPosition.nY = pBio->ImgPos1.y + (Image.oSecondPos.nY * nStartPart) / nEntireXRange;
			Image.oPosition.nZ = pBio->ImgPos1.z + (Image.oSecondPos.nZ * nStartPart) / nEntireXRange;

			Image.oSecondPos.nX = pBio->ImgPos1.x + (Image.oSecondPos.nX * nEndPart) / nEntireXRange;
			Image.oSecondPos.nY = pBio->ImgPos1.y + (Image.oSecondPos.nY * nEndPart) / nEntireXRange;
			Image.oSecondPos.nZ = pBio->ImgPos1.z + (Image.oSecondPos.nZ * nEndPart) / nEntireXRange;

			Image.oThirdPos.nX = pBio->ImgPos4.x + (Image.oEndPos.nX * nStartPart) / nEntireXRange;
			Image.oThirdPos.nY = pBio->ImgPos4.y + (Image.oEndPos.nY * nStartPart) / nEntireXRange;
			Image.oThirdPos.nZ = pBio->ImgPos4.z + (Image.oEndPos.nZ * nStartPart) / nEntireXRange;

			Image.oEndPos.nX = pBio->ImgPos4.x + (Image.oEndPos.nX * nEndPart) / nEntireXRange;
			Image.oEndPos.nY = pBio->ImgPos4.y + (Image.oEndPos.nY * nEndPart) / nEntireXRange;
			Image.oEndPos.nZ = pBio->ImgPos4.z + (Image.oEndPos.nZ * nEndPart) / nEntireXRange;

			Image.oImgLTPos.nX = (pBio->nImgWidth * nStartPart) / nEntireXRange;
			Image.oImgLTPos.nY = 0;
			Image.oImgRBPos.nX = (pBio->nImgWidth * nEndPart) / nEntireXRange;
			Image.oImgRBPos.nY = pBio->nImgHeight;
		}
		g_pRepresent->DrawPrimitives(1, &Image, RU_T_IMAGE_4, false);

	}
	//BuildinObjNextFrame(pBio);
}

void KIpotBranch::PaintNoneObjectLayer(RECT* pRepresentArea, int eLayer)
{
	for (int i = 0; i < 2; i++)
	{
		if (IS_BRANCH(i))
			m_pSubBranch[i]->PaintNoneObjectLayer(pRepresentArea, eLayer);
		else if (m_pLeafs[i])
			KIpotLeaf_PaintNoneObjectLayer(m_pLeafs[i], pRepresentArea, eLayer);
	}
}

//##ModelId=3DDACA8B015D
void KIpotBranch::RemoveAllRtoLeafs(KIpotLeaf* pPermanentRto)
{
	for (int i = 0; i < 2; i++)
	{
		if (IS_BRANCH(i))
			m_pSubBranch[i]->RemoveAllRtoLeafs(pPermanentRto);
		else if (m_pLeafs[i])
			KIpotLeaf_RemoveAllRtoLeafs(m_pLeafs[i], pPermanentRto);
	}
}

void KIpotBranch::AddAObject(KIpotBuildinObj* pBranchObj)
{
//	_ASSERT(pBranchObj);
#define SPACE_EXPAND_STEP 8

	if (m_pFirstObject == NULL)
	{
		m_pFirstObject = pBranchObj;
		m_oHeadPoint = pBranchObj->oPosition;
		m_oEndPoint  = pBranchObj->oEndPos;
	}
	else
	{
		if (m_nNumObjects == m_nNumObjectSpace)
		{
			KIpotBuildinObj** pNewList = (KIpotBuildinObj**)realloc(m_pObjectList,
				sizeof(KIpotBuildinObj*) * (m_nNumObjectSpace + SPACE_EXPAND_STEP));
			if (pNewList == NULL)
			{
				if (pBranchObj->bClone)
					free(pBranchObj);
				return;
			}
			m_pObjectList = pNewList;
			m_nNumObjectSpace += SPACE_EXPAND_STEP;
		}

		m_pObjectList[m_nNumObjects] = pBranchObj;
		m_nNumObjects ++;
		if (m_oHeadPoint.x > pBranchObj->oPosition.x)
			m_oHeadPoint = pBranchObj->oPosition;
		if (m_oEndPoint.x < pBranchObj->oEndPos.x)
			m_oEndPoint = pBranchObj->oEndPos;
	}
}

//##ModelId=3DDACD580262
void KIpotBranch::AddBranch(KIpotBuildinObj* pBranchObj)
{
	_ASSERT(pBranchObj && m_pFirstObject);

	POINT	poi, p1, p2;
	p1 = pBranchObj->oPosition;
	p2 = pBranchObj->oEndPos;

	_ASSERT(p1.x != p2.x && m_oHeadPoint.x != m_oEndPoint.x);

	RELATION_ENUM eRelate;

	if (m_pFirstObject->fAngleXY == pBranchObj->fAngleXY &&
		m_pFirstObject->fNodicalY == pBranchObj->fNodicalY)
	{
		eRelate = RELATION_ON;
	}
	else
	{
		eRelate = SM_Relation_LineLine_CheckCut(p1, p2,
					m_oHeadPoint, m_oEndPoint, poi);
	}

	if (eRelate != RELATION_CROSS)
	{
		if (eRelate == RELATION_UP)
			AddSubBranch(0, pBranchObj);
		else if (eRelate == RELATION_DOWN)
			AddSubBranch(1, pBranchObj);
		else
			AddAObject(pBranchObj);
	}
	else
	{
		KIpotBuildinObj* pClone = pBranchObj->Clone(&poi);
		eRelate = SM_Relation_PointLine(p1, m_oHeadPoint, m_oEndPoint);
		int nOrigIndex, nCloneIndex;
		if (eRelate == RELATION_DOWN)
		{
			nOrigIndex  = 1;
			nCloneIndex = 0;
		}
		else
		{
			nOrigIndex  = 0;
			nCloneIndex = 1;
		}
		if (pBranchObj->oPosition.x != pBranchObj->oEndPos.x)
			AddSubBranch(nOrigIndex, pBranchObj);
		else if (pBranchObj->bClone == true)
		{
			free(pBranchObj);
			pBranchObj = NULL;
		}
		if (pClone)
			AddSubBranch(nCloneIndex, pClone);
	}
}

void KIpotBranch::AddSubBranch(int nSubIndex, KIpotBuildinObj* pBranchObj)
{
//	_ASSERT(pBranchObj);
	if (m_pSubBranch[nSubIndex] == NULL)
	{
		KIpotBranch* pBranch = new KIpotBranch;

		if (pBranch)
		{
			pBranch->m_pParent = this;
			pBranch->AddAObject(pBranchObj);
			m_pSubBranch[nSubIndex] = pBranch;
			m_uFlag |= m_BranchFlag[nSubIndex];
		}
		else if (pBranchObj->bClone)
		{
			free(pBranchObj);
			pBranchObj = NULL;
		}
	}
	else
	{
//		_ASSERT(m_uFlag & m_BranchFlag[nSubIndex]);
		m_pSubBranch[nSubIndex]->AddBranch(pBranchObj);
	}
}

//##ModelId=3DDC082502F0
void KIpotBranch::Clear()
{
	int	i;
	m_pParent = NULL;
	if (m_pFirstObject && m_pFirstObject->bClone)
		free(m_pFirstObject);
	m_pFirstObject = NULL;
	if (m_pObjectList)
	{
		for (i = 0; i < m_nNumObjects; i++)
		{
			if (m_pObjectList[i]->bClone)
				free(m_pObjectList[i]);
		}
		free(m_pObjectList);
		m_pObjectList = NULL;
	}
	m_nNumObjects = 0;
	m_nNumObjectSpace = 0;

	for (i = 0; i < 2; i++)
	{
		if (IS_BRANCH(i))
		{
			delete (m_pSubBranch[i]);
			m_pSubBranch[i] = NULL;
			m_uFlag &= ~m_BranchFlag[i];
		}
		else if (m_pLeafs[i])
		{
			KIpotLeaf_Clear(m_pLeafs[i]);
			m_pLeafs[i] = NULL;
		}
	}
}

void KIpotBranch::SetLine(POINT* pPoint1, POINT* pPoint2)
{
	if (pPoint1)
		m_oHeadPoint = *pPoint1;
	if (pPoint2)
		m_oEndPoint = *pPoint2;
}

//##ModelId=3DE2A08602E2
/*void KIpotBranch::RemoveOutsideRtoLeafs(RECT& rc)
{
	for (int i = 0; i < 2; i++)
	{
		if (IS_BRANCH(i))
			m_pSubBranch[i]->RemoveOutsideRtoLeafs(rc);
		else if (m_pLeafs[i])
			m_pLeafs[i]->RemoveOutsideRtoLeafs(rc);
	}
}*/

//##ModelId=3DE3CBBB03E7
void KIpotBranch::AddLeafLine(KIpotBuildinObj* pLeaf)
{
	_ASSERT(pLeaf);
	RELATION_ENUM		eRelate;

	if (SM_Distance_PointLine(pLeaf->oPosition, m_oHeadPoint, m_oEndPoint) >
		SM_Distance_PointLine(pLeaf->oEndPos, m_oHeadPoint, m_oEndPoint))
	{
		eRelate = SM_Relation_PointLine(pLeaf->oPosition, m_oHeadPoint, m_oEndPoint);
	}
	else
		eRelate = SM_Relation_PointLine(pLeaf->oEndPos, m_oHeadPoint, m_oEndPoint);


	int	nIndex = (eRelate == RELATION_UP) ? 0 : 1;

	if (IS_BRANCH(nIndex))
		m_pSubBranch[nIndex]->AddLeafLine(pLeaf);
	else
		AddLineLeafToList(m_pLeafs[nIndex], pLeaf);
}

//##ModelId=3DE3CC2401EA
void KIpotBranch::AddLeafPoint(KIpotLeaf* pLeaf)
{
	_ASSERT(pLeaf);

	RELATION_ENUM	eRelate = SM_Relation_PointLine(pLeaf->oPosition, m_oHeadPoint, m_oEndPoint);
	int	nIndex = (eRelate == RELATION_UP) ? 0 : 1;

	if (IS_BRANCH(nIndex))
		m_pSubBranch[nIndex]->AddLeafPoint(pLeaf);
	else
		AddPointLeafToList(m_pLeafs[nIndex], pLeaf, NULL);
}

void KIpotBranch::AddPointLeafToList(KIpotLeaf*& pFirst, KIpotLeaf* pLeaf,
						KIpotLeaf* pParentLeaf)
{
	_ASSERT(pLeaf);
	if (pFirst == NULL)
	{
		pFirst = pLeaf;
		if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		{
			if (pParentLeaf)
				((KIpotRuntimeObj*)pLeaf)->pParentLeaf = pParentLeaf;
			else
				((KIpotRuntimeObj*)pLeaf)->pParentBranch = this;
		}
		return;
	}


	POINT	lp, op1,op2;
	lp = pLeaf->oPosition;

	KIpotLeaf	*pL, *pMatchL = NULL;
	RELATION_ENUM	eRelate, eMatchRelate;

	for(pL = pFirst; pL; pL = pL->pBrother)
	{
		if (pL->eLeafType != KIpotLeaf::IPOTL_T_BUILDIN_OBJ)
			continue;
		if ((((KIpotBuildinObj*)pL)->pBio->Props & SPBIO_P_SORTMANNER_MASK) != SPBIO_P_SORTMANNER_LINE)
			continue;
		op1 = pL->oPosition;
		op2 = ((KIpotBuildinObj*)pL)->oEndPos;

		eRelate = SM_Relation_PointLine(lp, op1, op2);

		#define		WIDTH_EXPAND		13
		#define		HEIGHT_UP_EXPAND	3
		#define		HEIGHT_DOWN_EXPAND	128

		RECT	rcArea;
		if (op1.x < op2.x)
		{
			rcArea.left  = op1.x - WIDTH_EXPAND;
			rcArea.right = op2.x + WIDTH_EXPAND;
		}
		else
		{
			rcArea.left  = op2.x - WIDTH_EXPAND;
			rcArea.right = op1.x + WIDTH_EXPAND;
		}
		if (op1.y < op2.y)
		{
			rcArea.top    = op1.y;
			rcArea.bottom = op2.y;
		}
		else
		{
			rcArea.top    = op2.y;
			rcArea.bottom = op1.y;
		}

		if (eRelate != RELATION_UP)
		{
			rcArea.top    -= HEIGHT_UP_EXPAND;
			rcArea.bottom += HEIGHT_DOWN_EXPAND;
		}

		if (lp.x > rcArea.left && lp.x <  rcArea.right &&
			lp.y >= rcArea.top  && lp.y <=  rcArea.bottom)
		{
			pMatchL = pL;
			eMatchRelate = eRelate;
			if (eRelate == RELATION_UP)
				break;
		}
	}

	if (pMatchL)
	{
		if (eMatchRelate == RELATION_DOWN)
		{
			AddPointLeafToList(pMatchL->pRChild, pLeaf, pMatchL);
		}
		else	//RELATION_UP, RELATION_ON
		{
			AddPointLeafToList(pMatchL->pLChild, pLeaf, pMatchL);
		}
		return;
	}
	
	KIpotLeaf	Header;
	KIpotLeaf* pPreLeaf = &Header;
	Header.pBrother = pFirst;
	for(pL = pFirst; pL; pPreLeaf = pL, pL = pL->pBrother)
	{
		if (pL->eLeafType == KIpotLeaf::IPOTL_T_BUILDIN_OBJ &&
			(((KIpotBuildinObj*)pL)->pBio->Props & SPBIO_P_SORTMANNER_MASK)
				!= SPBIO_P_SORTMANNER_POINT)
		{
			if (lp.y < pL->oPosition.y &&
				lp.y < ((KIpotBuildinObj*)pL)->oEndPos.y)
				break;
		}
		else if (lp.y < pL->oPosition.y)
			break;			
	}

	pPreLeaf->pBrother = pLeaf;
	pLeaf->pBrother = pL;

	if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
	{
		if (pParentLeaf)
			((KIpotRuntimeObj*)pLeaf)->pParentLeaf = pParentLeaf;
		else
			((KIpotRuntimeObj*)pLeaf)->pParentBranch = this;

		((KIpotRuntimeObj*)pLeaf)->pAheadBrother = pPreLeaf;
	}

	if (pL && pL->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
	{
		((KIpotRuntimeObj*)pL)->pAheadBrother = pLeaf;
	}
	pFirst = Header.pBrother;
	if (pFirst->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		((KIpotRuntimeObj*)pFirst)->pAheadBrother = NULL;
}

void KIpotBranch::AddLineLeafToList(KIpotLeaf*& pFirst, KIpotBuildinObj* pLeaf)
{
	_ASSERT(pLeaf);
	if (pFirst == NULL)
	{
		pFirst = pLeaf;
		return;
	}

//	KIpotBuildinObj	Header;
//	Header.pBrother = pFirst;

	POINT	lp1, lp2, op1,op2;
	lp1 = pLeaf->oPosition;
	lp2 = pLeaf->oEndPos;

	KIpotBuildinObj	*pL;
	RELATION_ENUM	eRelate;

	for(pL = (KIpotBuildinObj*)pFirst; pL; pL = (KIpotBuildinObj*)(pL->pBrother))
	{
		//_ASSERT(pL->eLeafType == KIpotLeaf::IPOTL_T_BUILDIN_OBJ);
		#define fsize  7
		RECT	rcArea;
		op1 = pL->oPosition;
		op2 = pL->oEndPos;
		if (op1.x < op2.x)
		{
			rcArea.left  = op1.x - fsize;
			rcArea.right = op2.x + fsize;
		}
		else
		{
			rcArea.left  = op2.x - fsize;
			rcArea.right = op1.x + fsize;
		}
		if (op1.y < op2.y)
		{
			rcArea.top = op1.y;
			rcArea.bottom = op2.y;
		}
		else
		{
			rcArea.top = op2.y;
			rcArea.bottom = op1.y;
		}

		if ( lp1.x >= rcArea.left && lp1.x <=  rcArea.right  &&
			 lp1.y >= rcArea.top  && lp1.y <=  rcArea.bottom &&
			 lp2.x >= rcArea.left && lp2.x <=  rcArea.right  &&
			 lp2.y >= rcArea.top  && lp2.y <=  rcArea.bottom )
		{
			if (SM_Distance_PointLine(lp1, op1, op2) > SM_Distance_PointLine(lp2, op1, op2))
				eRelate = SM_Relation_PointLine(lp1, op1, op2);
			else
				eRelate = SM_Relation_PointLine(lp2, op1, op2);
			break;
		}
	}

	if (pL)
	{
		if (eRelate == RELATION_DOWN)
		{
			AddLineLeafToList(pL->pRChild, pLeaf);
		}
		else	//RELATION_UP, RELATION_ON
		{
			AddLineLeafToList(pL->pLChild, pLeaf);
		}
		return;
	}

	KIpotBuildinObj	Header;
	KIpotBuildinObj *pPreLeaf = &Header;
	Header.pBrother = pFirst;
	if (lp1.y > lp2.y)
		lp1.y = lp2.y;
	for(pL = (KIpotBuildinObj*)pFirst; pL; pPreLeaf = pL, pL = (KIpotBuildinObj*)(pL->pBrother))
	{
		if (lp1.y < pL->oPosition.y && lp1.y < pL->oEndPos.y)
			break;
	}
	pPreLeaf->pBrother = pLeaf;
	pLeaf->pBrother = pL;

	pFirst = Header.pBrother;
}
