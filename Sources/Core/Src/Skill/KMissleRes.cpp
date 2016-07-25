/*******************************************************************************
// FileName			:	KMissleRes.cpp
// FileAuthor		:	RomanDou
// FileCreateDate	:	2002-7-8 16:21:44
// FileDescription	:	
// Revision Count	:	
*******************************************************************************/
#include "KEngine.h"
#include "KCore.h"
#include "KMissle.h"
#include "KMissleRes.h"
#include "KSubWorld.h"
#include "KSkillSpecial.h"
#include "ImgRef.h"
#include "KPlayer.h"
#ifndef _SERVER
#include "../../Represent/iRepresent/iRepresentshell.h"
#include "scene/KScenePlaceC.h"
#endif
#include "KSubWorldSet.h"
#ifndef _SERVER

KMissleRes::KMissleRes()
{
	m_pSndNode = NULL;
	m_bLoopAnim = 0;
	m_nLastShadowLifeTime = 0;
	m_bHaveEnd = FALSE;
	for (int i = 0 ; i < MAX_MISSLE_STATUS; i ++)
	{
		m_RUImage[i].nType = ISI_T_SPR;
		m_RUImage[i].Alpha = 32;
		m_RUImage[i].bRenderStyle = IMAGE_RENDER_STYLE_ALPHA;
		m_RUImage[i].uImage = 0;
		m_RUImage[i].nISPosition = 0;
		m_RUImage[i].bMatchReferenceSpot = 1;
		m_RUImage[i].bFrameDraw = FALSE;
	}
	
#ifdef TOOLVERSION
	m_pSprNode = NULL;
#endif
}

KMissleRes::~KMissleRes()
{
	while(m_ShadowList.GetTail())
	{
		KShadowNode * pShadowNode = (KShadowNode*)m_ShadowList.GetTail();
		delete pShadowNode;
		m_ShadowList.RemoveTail();
	}
}
/*!*****************************************************************************
// Function		: KMissleRes::LoadResource
// Purpose		: 
// Return		: void 
// Argumant		: eMissleStatus nStatus
// Argumant		: char * MissleImage
// Argumant		: char * MissleSound
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissleRes::LoadResource(int nStatus, char * MissleImage, char * MissleSound)
{
	strcpy(m_MissleRes[nStatus].AnimFileName, MissleImage);
	strcpy(m_MissleRes[nStatus].SndFileName,	MissleSound);
}

BOOL KMissleRes::Init()
{
	Clear();
	return TRUE;
}

void KMissleRes::Remove()
{
}

void KMissleRes::Clear()
{
	m_bNeedShadow = FALSE;
	while (m_ShadowList.GetHead())
		m_ShadowList.RemoveHead();
	m_nLastShadowLifeTime = 0;
	
	while(m_SkillSpecialList.GetHead())
	{
		KSkillSpecialNode * pNode = (KSkillSpecialNode*)m_SkillSpecialList.GetHead();
		pNode->Remove();
		delete pNode;
	}
	
	for(int i = 0; i <MAX_MISSLE_STATUS; i ++)
	{
		m_RUImage[i].szImage[0] = 0;
		m_RUImage[i].uImage = 0;
	}
}

/*!*****************************************************************************
// Function		: KMissleRes::Draw
// Purpose		: 
// Return		: void 
// Argumant		: int nX 实际像素点坐标
// Argumant		: int nY 实际像素点坐标
// Argumant		: int nZ
// Argumant		: int nFrame
// Comments		:
// Author		: RomanDou
*****************************************************************************/
int KMissleRes::Draw(int eStatus,  int nX, int nY , int nZ, int nDir, int nAllFrame,  int nCurLifeFrame)
{
	//当nAllFrame == 0时，表示为默认数
	char SprFileName[80];
	g_SetFilePath("\\");
	if (eStatus == MS_DoFly)
	{
		if (nCurLifeFrame < 0 || (nAllFrame != 0 && nAllFrame < nCurLifeFrame)) return FALSE;
		
		if (!m_RUImage[eStatus].szImage[0])
		{
			g_GetFullPath(SprFileName, m_MissleRes[eStatus].AnimFileName);
			g_StrCpy(m_RUImage[eStatus].szImage, SprFileName);
		}
		
		int nSprDir = m_MissleRes[eStatus].nDir;
		int nSprFrames = m_MissleRes[eStatus].nTotalFrame;
		if (nSprDir && nSprFrames)
		{
			//处理子弹显示时实际方向
			int	nImageDir = (nDir / (64 / nSprDir));
			int nImageDir1 = (nDir % (64 / nSprDir));
			if (nImageDir1 >= 32 / nSprDir) 	nImageDir ++;
			if (nImageDir >= nSprDir)		nImageDir = 0;
			int nFramePerDir = (nSprFrames / nSprDir);
			if (nAllFrame == 0) nAllFrame = nFramePerDir;
			int nFirstFrame = nImageDir * nFramePerDir;
			int nTotalFrame = nSprFrames / nSprDir;
			int nFrame = nCurLifeFrame ;

			{
				if (m_bLoopAnim) //如果是循环播放的话则每帧都换帧
				{
					if (!m_bSubLoop)//无子显示循环
					{	nFrame = (nCurLifeFrame / m_MissleRes[eStatus].nInterval)  % nTotalFrame;
					}
					else
					{
						//  未显示到循环播放的开始帧
						if ( (nCurLifeFrame / m_MissleRes[eStatus].nInterval) < m_nSubStart)
							nFrame = nCurLifeFrame / m_MissleRes[eStatus].nInterval;
						else
						{
							if (m_nSubStart == m_nSubStop) nFrame = m_nSubStart;
							else
								nFrame = m_nSubStart + ((nCurLifeFrame - m_nSubStart) / m_MissleRes[eStatus].nInterval)  % (m_nSubStop - m_nSubStart);
						}
					}
				}
				else
				{
					nFrame = nTotalFrame * nCurLifeFrame / nAllFrame;
				}
				
				if (nFrame > (nTotalFrame - 1)) 
					return FALSE;
			}
			nFrame = nFirstFrame + nFrame;
#ifdef TOOLVERSION
			KSprite * pSprite = NULL;
			m_pSprNode = (KCacheNode *)g_SpriteCache.GetNode(m_RUImage[eStatus].szImage ,  (KCacheNode *)m_pSprNode);
			pSprite = (KSprite*)m_pSprNode->m_lpData;
			if (pSprite)
			{
				
				pSprite->DrawAlpha(nX - pSprite->GetCenterX(), nY - pSprite->GetCenterY(), nFrame);
			}
#else			
			m_RUImage[eStatus].oPosition.nX = nX;
			m_RUImage[eStatus].oPosition.nY = nY;
			m_RUImage[eStatus].oPosition.nZ = nZ;
			m_RUImage[eStatus].nFrame = nFrame;
			g_pRepresent->DrawPrimitives(1, &m_RUImage[eStatus], RU_T_IMAGE, 0);
			
			
			if (Player[0].m_DebugMode)
			{
				KRULine		Line;
				Line.oPosition.nX = nX;
				Line.oPosition.nY = nY;
				Line.oPosition.nZ = nZ;
				Line.oEndPos.nX = Line.oPosition.nX +32;
				Line.oEndPos.nY = nY;
				Line.oEndPos.nZ = nZ;
				
				Line.Color.Color_dw = 0xffffffff;
				g_pRepresent->DrawPrimitives(1, &Line, RU_T_LINE, 0);
			}

#endif
			int nExAlpha = 0;
			nExAlpha = nCurLifeFrame % 32;
			KShadowNode * pDrawShadowNode = NULL;
			int nCurAlpha = 0;
			
			/*			if (m_bNeedShadow)
			{
			
			  pDrawShadowNode = (KShadowNode*)m_ShadowList.GetHead();
			  
				while (pDrawShadowNode)
				{
				int nSrcX = pDrawShadowNode->X;
				int nSrcY = pDrawShadowNode->Y;
				SubWorld[Missle[m_nMissleId].m_nSubWorldId].Mps2Screen(&nSrcX, &nSrcY);
				pSprite->DrawAlpha(nSrcX- pSprite->GetCenterX(), nSrcY- pSprite->GetCenterY(), pDrawShadowNode->Frame, nCurAlpha );
				nCurAlpha += (32 / m_nMaxShadowNum) ;
				pDrawShadowNode = (KShadowNode*)pDrawShadowNode->GetNext();
				}
				
				  if (nCurLifeFrame - m_nLastShadowLifeTime > 2)
				  {
				  
					if (m_ShadowList.GetNodeCount() >= m_nMaxShadowNum)
					m_ShadowList.RemoveHead();
					
					  KShadowNode * pShadowNode = new KShadowNode;
					  pShadowNode->X		= nX ;
					  pShadowNode->Y      = nY ;
					  pShadowNode->Frame	= nFrame;
					  pShadowNode->Alpha	= 100;
					  m_ShadowList.AddTail(pShadowNode);
					  m_nLastShadowLifeTime = nCurLifeFrame;
					  }
					  
						}
			*/
			//			int nSrcX = nX;
			//			int nSrcY = nY;
			//SubWorld[Missle[m_nMissleId].m_nSubWorldId].Mps2Screen(&nSrcX, &nSrcY);
			//pSprite->DrawAlpha(nSrcX - pSprite->GetCenterX(), nSrcY - pSprite->GetCenterY(), nFrame);
			/*			if (m_pObj)
			{
			cPoint			stPoint;
			ChangeMissleAnimation(m_pObj, SprFileName);
			ChangeMissleFrame(m_pObj, nFrame);
			stPoint.x = nX;
			stPoint.y = nY;
			stPoint.z = nZ;
			OffsetMissleToScale(m_pObj, stPoint);
			OffsetLightToScale(m_pLight, stPoint);
			
			  
				if (!m_pObj->IsInGraphics())
				//					::AddMissle(g_pGraphics, m_pObj);
				
				  if (!m_pLight->IsInGraphics())
				  //::AddLight(g_pGraphics, m_pLight);
				  }
			*/
		}
	}	
	KSkillSpecialNode * pNode = (KSkillSpecialNode*)m_SkillSpecialList.GetHead(); 
	while (pNode)
	{
		DWORD dwCurrentTime =  g_SubWorldSet.GetGameTime();
		if(pNode->m_pSkillSpecial->m_nEndTime <= dwCurrentTime)
		{
			KSkillSpecialNode * pDelNode = (KSkillSpecialNode*)pNode->GetNext();
			pNode->Remove();
			delete pNode;
			pNode = pDelNode;
			continue;
		}
		else
		{
			if (pNode->m_pSkillSpecial->m_nBeginTime <= dwCurrentTime)
				pNode->m_pSkillSpecial->Draw(dwCurrentTime);
		}
		
		pNode = (KSkillSpecialNode*)pNode->GetNext();
	}
	return TRUE;
}

/*!*****************************************************************************
// Function		: KMissleRes::PlaySound
// Purpose		: 
// Return		: void 
// Argumant		: int nLoop
// Argumant		: int nPan
// Argumant		: int nVal
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissleRes::PlaySound(int eStatus, int nLoop, int nPan, int nVal)
{
	if (m_MissleRes[eStatus].SndFileName[0] == 0)	return;
	
	KWavSound * pSound = NULL;
	char SndFileName[200];
	g_SetFilePath("\\");
	g_GetFullPath(SndFileName, m_MissleRes[eStatus].SndFileName);
	m_pSndNode	= (KCacheNode*) g_SoundCache.GetNode(SndFileName, (KCacheNode * ) m_pSndNode);
	pSound		= (KWavSound*) m_pSndNode->m_lpData;
	if (pSound)
	{
		pSound->Play(nPan, nVal, nLoop);
	}
	m_nLastSndIndex = eStatus;
}

/*!*****************************************************************************
// Function		: KMissleRes::StopSound
// Purpose		: 
// Return		: void 
// Comments		:
// Author		: RomanDou
*****************************************************************************/
void KMissleRes::StopSound()
{
	if (m_MissleRes[m_nLastSndIndex].SndFileName[0] == 0)		return;
	
	KWavSound * pSound = NULL;
	m_pSndNode	= (KCacheNode*) g_SoundCache.GetNode(m_MissleRes[m_nLastSndIndex].SndFileName, (KCacheNode * ) m_pSndNode);
	pSound		= (KWavSound*) m_pSndNode->m_lpData;
	if (pSound)
	{
		pSound->Stop();
	}
}
#endif