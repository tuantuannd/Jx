// ***************************************************************************************
// 场景对象树上的树叶的函数
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11
// ***************************************************************************************
#include "KCore.h"
#include "Windows.h"
#include "KIpotLeaf.h"
#include "KIpotBranch.h"
#include "../CoreDrawGameObj.h"
#include "../ImgRef.h"
#include "../../Represent/iRepresent/iRepresentShell.h"
#include <crtdbg.h>
#define	ONE_OF_COS_30					1.1547005383792515290182975610039

void KIpotRuntimeObj::Pluck()
{
	if (pAheadBrother)
		pAheadBrother->pBrother = pBrother;
	if (pParentBranch)
	{
		if (//(pParentBranch->m_uFlag & KIpotBranch::IPOT_BF_HAVE_LEFT_BRANCH) == 0 &&
			pParentBranch->m_pLeafs[0] == (KIpotLeaf*)this)
		{
			pParentBranch->m_pLeafs[0] = pBrother;
		}
		else if (//(pParentBranch->m_uFlag & KIpotBranch::IPOT_BF_HAVE_RIGHT_BRANCH) == 0 &&
			pParentBranch->m_pLeafs[1] == (KIpotLeaf*)this)
		{
			pParentBranch->m_pLeafs[1] = pBrother;
		}
		pParentBranch = NULL;
	}
	if (pParentLeaf)
	{
		if (pParentLeaf->pLChild == (KIpotLeaf*)this)
			pParentLeaf->pLChild = pBrother;
		else if (pParentLeaf->pRChild == (KIpotLeaf*)this)
			pParentLeaf->pRChild = pBrother;
		pParentLeaf = NULL;
	}

	//_ASSERT(pLChild == NULL && pRChild == NULL);
	if (pBrother && pBrother->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		((KIpotRuntimeObj*)pBrother)->pAheadBrother = pAheadBrother;
	pAheadBrother = NULL;
	pBrother = NULL;
}

void KIpotLeaf_EnumerateObjects(KIpotLeaf* pLeaf, void* p, ObjectsCallbackFn CallbackFn)
{
	while(pLeaf)
	{
		if (pLeaf->pLChild)
			KIpotLeaf_EnumerateObjects(pLeaf->pLChild, p, CallbackFn);
		CallbackFn(p, pLeaf);
		if (pLeaf->pRChild)
			KIpotLeaf_EnumerateObjects(pLeaf->pRChild, p, CallbackFn);
		pLeaf = pLeaf->pBrother;
	};
}

extern int		g_bShowObstacle;

void KIpotLeaf_PaintObjectLayer(KIpotLeaf* pLeaf, RECT* pRepresentArea)
{
	while(pLeaf)
	{
		if (pLeaf->pLChild)
			KIpotLeaf_PaintObjectLayer(pLeaf->pLChild, pRepresentArea);
		//paint self
		if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		{
			KIpotRuntimeObj* pObj = (KIpotRuntimeObj*)pLeaf;
			if ((pObj->eLayerParam & IPOT_RL_OBJECT) &&
				pObj->oPosition.y >= pRepresentArea->top &&
				pObj->oPosition.y < pRepresentArea->bottom &&
				pObj->oPosition.x >= pRepresentArea->left &&
				pObj->oPosition.x < pRepresentArea->right)
			{
				CoreDrawGameObj(pObj->uGenre, pObj->nId, pObj->oPosition.x,
					pObj->oPosition.y, 0, 0, IPOT_RL_OBJECT);
			}
			#ifdef SWORDONLINE_SHOW_DBUG_INFO
			if (g_bShowObstacle)
			{
				KRURect	FootSpot;
				FootSpot.Color.Color_dw = 0x000FFFF;
				FootSpot.oPosition.nX = pObj->oPosition.x - 3;
				FootSpot.oPosition.nY = pObj->oPosition.y - POINT_LEAF_Y_ADJUST_VALUE;
				FootSpot.oPosition.nZ = 0;
				FootSpot.oEndPos.nX = pObj->oPosition.x + 4;
				FootSpot.oEndPos.nY = pObj->oPosition.y;
				FootSpot.oEndPos.nZ = 0;
				g_pRepresent->DrawPrimitives(1, &FootSpot, RU_T_RECT, false);
			}
			#endif
		}
		else// if (eLeafType == IPOTL_T_BUILDIN_OBJ)
		{
			((KIpotBuildinObj*)pLeaf)->PaintABuildinObject(pRepresentArea);
		}

		if (pLeaf->pRChild)
			KIpotLeaf_PaintObjectLayer(pLeaf->pRChild, pRepresentArea);

		pLeaf = pLeaf->pBrother;
	}
}

void KIpotLeaf_PaintNoneObjectLayer(KIpotLeaf* pLeaf, RECT* pRepresentArea, int eLayer)
{
	while(pLeaf)
	{
		if (pLeaf->pLChild)
			KIpotLeaf_PaintNoneObjectLayer(pLeaf->pLChild, pRepresentArea, eLayer);

		else if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ &&
				((((KIpotRuntimeObj*)pLeaf)->eLayerParam) & eLayer))
		{
			if (pLeaf->oPosition.x >= pRepresentArea->left && pLeaf->oPosition.x < pRepresentArea->right &&
				pLeaf->oPosition.y >= pRepresentArea->top && pLeaf->oPosition.y < pRepresentArea->bottom)
			{
				KIpotRuntimeObj* pObj = (KIpotRuntimeObj*)pLeaf;
				CoreDrawGameObj(pObj->uGenre, pObj->nId, pObj->oPosition.x,
					pObj->oPosition.y, 0, 0, eLayer);
			}
		}
	
		if (pLeaf->pRChild)
			KIpotLeaf_PaintNoneObjectLayer(pLeaf->pRChild, pRepresentArea, eLayer);

		pLeaf = pLeaf->pBrother;
	}
}

void KIpotBuildinObj::PaintABuildinObject(RECT* pRepresentArea)
{
	KRUImage4	Image;
	Image.Color.Color_dw = 0;
	Image.nType = ISI_T_SPR;
	Image.nFrame = pBio->nFrame;
	Image.nISPosition = nISPosition;
	strcpy(Image.szImage, pBio->szImage);
	Image.uImage = uImage;
	if ((pBio->Props & SPBIO_F_HIGHT_LIGHT) == 0)
	{
		if (pBio->nImgWidth > 128 && pBio->nImgHeight > 128)
			Image.bRenderStyle = IMAGE_RENDER_STYLE_3LEVEL;
		else
		{
			Image.bRenderStyle = IMAGE_RENDER_STYLE_ALPHA;
			Image.Color.Color_b.a = 255;
		}
	}
	else
		Image.bRenderStyle = IMAGE_RENDER_STYLE_BORDER;

	if (pBio->nAniSpeed == 0)
	{
		Image.bRenderFlag = RUIMAGE_RENDER_FLAG_FRAME_DRAW;
		Image.oPosition.nX = pBio->ImgPos1.x;
		Image.oPosition.nY = pBio->ImgPos1.y;
		Image.oPosition.nZ = pBio->ImgPos1.z;
		
		Image.oEndPos.nX = pBio->ImgPos3.x;
		Image.oEndPos.nY = pBio->ImgPos3.y;
		Image.oEndPos.nZ = pBio->ImgPos3.z;

		if ((pBio->Props & SPBIO_P_SORTMANNER_MASK) == SPBIO_P_SORTMANNER_POINT)
		{
			if (Image.oEndPos.nY >= pRepresentArea->top)
				g_pRepresent->DrawPrimitives(1, &Image, RU_T_IMAGE, false);
		}
		else
		{
			if (Image.oEndPos.nY > pRepresentArea->top &&
				pBio->ImgPos3.x >= pRepresentArea->left &&
				pBio->ImgPos4.x < pRepresentArea->right)
			{
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

				g_pRepresent->DrawPrimitives(1, &Image, RU_T_IMAGE_4, false);
			}
		}
	}
	else
	{
		if (pBio->oPos1.y >= pRepresentArea->top)
		{
			Image.bRenderFlag = RUIMAGE_RENDER_FLAG_REF_SPOT;
			Image.oPosition.nX = pBio->oPos1.x;
			Image.oPosition.nY = pBio->oPos1.y;
			Image.oPosition.nZ = pBio->oPos1.z;
			g_pRepresent->DrawPrimitives(1, &Image, RU_T_IMAGE, false);
		}
		BuildinObjNextFrame(pBio);
	}
}

void KIpotLeaf_RemoveAllRtoLeafs(KIpotLeaf* pLeaf, KIpotLeaf* pPermanentRto)
{
	//_ASSERT(pPermanentRto);
	while(pLeaf)
	{
		if (pLeaf->pLChild)
			KIpotLeaf_RemoveAllRtoLeafs(pLeaf->pLChild, pPermanentRto);
		if (pLeaf->pRChild)
			KIpotLeaf_RemoveAllRtoLeafs(pLeaf->pRChild, pPermanentRto);

		KIpotLeaf* pBrother = pLeaf->pBrother;

		if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		{
			((KIpotRuntimeObj*)pLeaf)->Pluck();
			if (pPermanentRto->pBrother)
			{
				pLeaf->pBrother = pPermanentRto->pBrother;
				((KIpotRuntimeObj*)pLeaf->pBrother)->pAheadBrother = pLeaf;
			}
			pPermanentRto->pBrother = pLeaf;
			((KIpotRuntimeObj*)pLeaf)->pAheadBrother = pPermanentRto;
		}
		pLeaf = pBrother;
	};
}

void KIpotLeaf_Clear(KIpotLeaf* pLeaf)
{
	while(pLeaf)
	{
		if (pLeaf->pLChild)
		{
			KIpotLeaf_Clear(pLeaf->pLChild);
			pLeaf->pLChild = NULL;
		}
		if (pLeaf->pRChild)
		{
			KIpotLeaf_Clear(pLeaf->pRChild);
			pLeaf->pRChild = NULL;
		}

		if (pLeaf->eLeafType == KIpotLeaf::IPOTL_T_RUNTIME_OBJ)
		{
			((KIpotRuntimeObj*)pLeaf)->pParentBranch = NULL;
			((KIpotRuntimeObj*)pLeaf)->pParentLeaf   = NULL;
			((KIpotRuntimeObj*)pLeaf)->pAheadBrother = NULL;
		}
		KIpotLeaf* pBrother = pLeaf->pBrother;
		pLeaf->pBrother = NULL;
		pLeaf = pBrother;
	};
}

//生成一个克隆体
KIpotBuildinObj* KIpotBuildinObj::Clone(POINT* pDivisionPos)
{
	_ASSERT(pDivisionPos);
	if (pDivisionPos->x == oEndPos.x)
		return NULL;
	KIpotBuildinObj* pClone = (KIpotBuildinObj*)malloc(sizeof(KIpotBuildinObj));
	if (pClone)
	{
		pClone->bClone = true;
		pClone->eLeafType = eLeafType;
		pClone->fAngleXY = fAngleXY;
		pClone->fNodicalY = fNodicalY;
		pClone->uImage = uImage;
		pClone->nISPosition = nISPosition;
		pClone->oPosition = *pDivisionPos;
		pClone->oEndPos = oEndPos;
		pClone->pBio = pBio;
		pClone->pBrother = NULL;
		pClone->bImgPart = true;

		oEndPos = *pDivisionPos;
		bImgPart = true;
	}
	return pClone;
}

void  BuildinObjNextFrame(KBuildinObj* pBio)
{
	_ASSERT(pBio);
	if (pBio->nAniSpeed && g_pRepresent)
	{
		if (pBio->nAniSpeed == 1)
		{
			KImageParam	Param;
			Param.nNumFrames = 0;
			g_pRepresent->GetImageParam(pBio->szImage, &Param, ISI_T_SPR);
			
			if (Param.nNumFrames > 1)
			{
				pBio->nImgNumFrames = Param.nNumFrames;
				pBio->uFlipTime = IR_GetCurrentTime();
				if ((pBio->nAniSpeed = Param.nInterval) < 20)
					pBio->nAniSpeed = 20;
			}
			else
			{
				pBio->nAniSpeed = 0;
			}
			
		}

		int nFrame = pBio->nFrame;
		IR_NextFrame(nFrame, pBio->nImgNumFrames, pBio->nAniSpeed, pBio->uFlipTime);
		pBio->nFrame = nFrame;
/*		if (nFrame != pBio->nFrame)
		{
			KRPosition2 Offset1, Offset2, Size1, Size2;
			if (pBio->fAngleXY == 0)
			{
				if (g_pRepresent->GetImageFrameParam(pBio->szImage, pBio->nFrame, &Offset1, &Size1, ISI_T_SPR) &&
					g_pRepresent->GetImageFrameParam(pBio->szImage, nFrame, &Offset2, &Size2, ISI_T_SPR))
				{
					float fEntireZRange = (float)(pBio->ImgPos3.z - pBio->ImgPos1.z);

					pBio->ImgPos3.x = pBio->ImgPos1.x +
						(Size2.nX + Offset2.nX - Offset1.nX);
					pBio->ImgPos1.x += (Offset2.nX - Offset1.nX);

					if (Offset1.nY != Offset2.nY)
						pBio->ImgPos1.z += (int)((Offset1.nY - Offset2.nY) * ONE_OF_COS_30);
					if (Size1.nY + Offset1.nY != Size2.nY + Offset2.nY)
						pBio->ImgPos3.z += (int)((Offset1.nY + Size1.nY - Offset2.nY - Size2.nY) * ONE_OF_COS_30);
					pBio->nFrame = nFrame;
				}
			}
			else
				pBio->nFrame = nFrame;
		}*/
	}
}