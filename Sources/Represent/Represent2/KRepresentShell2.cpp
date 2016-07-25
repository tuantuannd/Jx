/*****************************************************************************************
//  表现模块的对外接口的二维版本实现。
//	Copyright : Kingsoft 2002
//	Author	:   Spe(huyi)
//	CreateTime:	2002-11-11
*****************************************************************************************/
#define _REPRESENT_INTERNAL_SIGNATURE_
#include "KRepresentShell2.h"
#include "../iRepresent/KRepresentUnit.h"
#include "../../Engine/src/KColors.h"
#include "../iRepresent/Font/KFont2.h"
#include "../iRepresent/Image/ImageOperation.h"
#include "..\iRepresent\RepresentUtility.h"
#include "..\..\engine\src\KWin32Wnd.h"
#include "..\..\engine\src\KBmpFile24.h"
#include <assert.h>


//根据SPR头指针，获取调色版缓冲区指针
#define GET_SPR_PALETTE(pHeader)	( ((char*)pHeader) + sizeof(SPRHEAD))

//=========创建一个iRepresentShell接口的实例===============
extern "C" __declspec(dllexport)
iRepresentShell* CreateRepresentShell()
{
	return (new KRepresentShell2);
}

IInlinePicEngineSink* g_pIInlinePicSinkRP = NULL;	//嵌入式图片的处理接口[wxb 2003-6-20]
HRESULT KRepresentShell2::AdviseRepresent(IInlinePicEngineSink* pSink)
{
	assert(NULL == g_pIInlinePicSinkRP);	//一般不会挂接两次
	g_pIInlinePicSinkRP = pSink;
	return S_OK;
}

HRESULT KRepresentShell2::UnAdviseRepresent(IInlinePicEngineSink* pSink)
{
	if (pSink == g_pIInlinePicSinkRP)
		g_pIInlinePicSinkRP = NULL;
	return S_OK;
}

//##ModelId=3DD20C90004D
KRepresentShell2::KRepresentShell2()
{
	m_nLeft = 0;
	m_nTop = 0;
	memset(m_FontTable, 0, sizeof(m_FontTable));
}

//##ModelId=3DD20C900089
KRepresentShell2::~KRepresentShell2()
{
	m_DirectDraw.Exit();
	m_ImageStore.Free();
	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].pFontObj)
		{
			m_FontTable[i].pFontObj->Release();
			m_FontTable[i].pFontObj = NULL;
		}
	}
}

//设置偏色列表
unsigned int KRepresentShell2::SetAdjustColorList(unsigned int* puColorList, unsigned int uCount)
{
	return m_ImageStore.SetAdjustColorList(puColorList, uCount);
}



bool KRepresentShell2::Create(int nWidth, int nHeight, bool bFullScreen)
{
	m_DirectDraw.Mode(bFullScreen, nWidth, nHeight);
	if (m_DirectDraw.Init())
	{
		m_Canvas.Init(nWidth, nHeight);
		m_ImageStore.Init();
		RIO_Set16BitImageFormat(m_DirectDraw.GetRGBBitMask16() == RGB_565);
		// 初始化Gdi+
		InitGdiplus();
		return true;
	}
	return false;
}

//##ModelId=3DCA0B230317
bool KRepresentShell2::CreateAFont(const char* pszFontFile, CHARACTER_CODE_SET CharaSet, int nId)
{
	int nFirstFree = -1;
	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].pFontObj == NULL && nFirstFree == -1)
			nFirstFree = i;
		else if (m_FontTable[i].nId == nId)
		{
			nFirstFree = i;
			break;
		}
	}
	if (nFirstFree == -1 || pszFontFile == NULL || nId == 0)
		return false;

	if (m_FontTable[nFirstFree].pFontObj)
	{
		m_FontTable[nFirstFree].pFontObj->Release();
		m_FontTable[nFirstFree].pFontObj = NULL;
	}

	if (pszFontFile[0] == '#')
	{
		//共享已经打开的字库
		int nShareWithId = atoi(pszFontFile + 1);
		for (int j = 0; j < RS2_MAX_FONT_ITEM_NUM; j++)
		{
			if (nFirstFree != j &&	m_FontTable[j].nId == nShareWithId &&
				m_FontTable[j].pFontObj)
			{
				m_FontTable[nFirstFree].nId = nId;
				m_FontTable[nFirstFree].pFontObj = m_FontTable[j].pFontObj->Clone();
				return true;
			}
		}
		return false;
	}

	if ((m_FontTable[nFirstFree].pFontObj = new KFont2) == NULL)
		return false;

	m_FontTable[nFirstFree].pFontObj->Init(&m_Canvas);
	if (m_FontTable[nFirstFree].pFontObj->Load((LPSTR)pszFontFile/*, CharaSet*/))
	{
		m_FontTable[nFirstFree].nId = nId;
		m_FontTable[nFirstFree].pFontObj->SetOutputSize(nId, nId + 1);
	}
	else
	{
		m_FontTable[nFirstFree].pFontObj->Release();
		m_FontTable[nFirstFree].pFontObj = NULL;
	}
	
	return (m_FontTable[nFirstFree].pFontObj != NULL);
}

//##ModelId=3DCD8DEA01BB
unsigned int KRepresentShell2::CreateImage(const char* pszName, int nWidth, int nHeight, int nType)
{
	return m_ImageStore.CreateImage(pszName, nWidth, nHeight, nType);
}

//##ModelId=3DB69FE401DA
void KRepresentShell2::DrawPrimitives(int nPrimitiveCount, KRepresentUnit* pPrimitives, unsigned int uGenre, int bSinglePlaneCoord)
{
	int i = 0;

	switch(uGenre)
	{
	case RU_T_IMAGE:
		{
			KRUImage* pTemp = (KRUImage*)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{
				switch(pTemp->nType)
				{
				case ISI_T_SPR:
					{
						SPRFRAME* pFrame;
						SPRHEAD* pSprHeader = (SPRHEAD*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType, (void*&)pFrame);
						if (pSprHeader == NULL)
							break;

						int nX = pTemp->oPosition.nX;
						int nY = pTemp->oPosition.nY;
						if (bSinglePlaneCoord == false)
							CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
						if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
						{
					//****to be modify****
#define CENTERX		160
#define	CENTERY		192
							int nCenterX = pSprHeader->CenterX;
							int nCenterY = pSprHeader->CenterY;
							if (nCenterX || nCenterY)
							{
								nX -= nCenterX;
								nY -= nCenterY;
							}
							else if (pSprHeader->Width > CENTERX)
							{
								nX -= CENTERX;
								nY -= CENTERY;
							}
						}
						//****to be modify end****

//						Check Current Draw Device??;
						if ((pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW) == 0)
						{
							nX += pFrame->OffsetX;
							nY += pFrame->OffsetY;
						}

						char* pPalette = GET_SPR_PALETTE(pSprHeader);

						switch(pTemp->bRenderStyle)
						{
						case IMAGE_RENDER_STYLE_ALPHA:
						case IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT:
							m_Canvas.DrawSpriteAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette, pTemp->Color.Color_b.a / 8);
							break;
						case IMAGE_RENDER_STYLE_3LEVEL:
							m_Canvas.DrawSprite3LevelAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette);
							break;
						case IMAGE_RENDER_STYLE_OPACITY:
							m_Canvas.DrawSprite(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette);
							break;
						case IMAGE_RENDER_STYLE_BORDER:
//							m_Canvas.DrawSpriteBorder(nX, nY, pFrame->Width, pFrame->Height,
//								///g_RGB(pTemp->Color.Color_b.r, pTemp->Color.Color_b.g, pTemp->Color.Color_b.b),
//								g_RGB(200, 200, 0),
//								pFrame->Sprite);
							break;
						case IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST:
							pPalette = m_ImageStore.GetAdjustColorPalette(pTemp->nISPosition, pTemp->Color.Color_dw);
							m_Canvas.DrawSpriteAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette, pTemp->Color.Color_b.a / 8);
							break;
						}
					}
					break;
				case ISI_T_BITMAP16:
					{
						void* pFrame;
						KSGImageContent* pBitmap = (KSGImageContent *)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							0, ISI_T_BITMAP16, pFrame);
						if (pBitmap)
						{
							int nX = pTemp->oPosition.nX;
							int nY = pTemp->oPosition.nY;
							if (bSinglePlaneCoord == false)
								CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
							m_Canvas.DrawBitmap16(nX, nY,
								pBitmap->nWidth, pBitmap->nHeight, pBitmap->Data);
						}
					}
					break;
				}
			}
		}
		break;
	case RU_T_IMAGE_4:
		{
			KRUImage4* pTemp = (KRUImage4*)pPrimitives;
			RECT rcOld;
			m_Canvas.GetClipRect(&rcOld);
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{
				//_ASSERT(pTemp->nType == ISI_T_SPR);
					{
						SPRFRAME* pFrame;
						SPRHEAD* pSprHeader = (SPRHEAD*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType, (void*&)pFrame);
						if (pSprHeader == NULL)
							break;

						int nX = pTemp->oPosition.nX;
						int nY = pTemp->oPosition.nY;
						if (!bSinglePlaneCoord)
							CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
						if ((pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW) == 0)
						{
							nX += pFrame->OffsetX;
							nY += pFrame->OffsetY;
						}

						RECT	rc;
						rc.left  = nX;
						rc.top   = nY;
						nX -= pTemp->oImgLTPos.nX;
						nY -= pTemp->oImgLTPos.nY;
						rc.right = nX + pTemp->oImgRBPos.nX;
						rc.bottom= nY + pTemp->oImgRBPos.nY;
						if (rc.left < rcOld.left)
							rc.left = rcOld.left;
						if (rc.right > rcOld.right)
							rc.right = rcOld.right;
						if (rc.top < rcOld.top)
							rc.top = rcOld.top;
						if (rc.bottom > rcOld.bottom)
							rc.bottom = rcOld.bottom;
						m_Canvas.SetClipRect(&rc);

						char* pPalette = GET_SPR_PALETTE(pSprHeader);

						switch(pTemp->bRenderStyle)
						{
						case IMAGE_RENDER_STYLE_ALPHA:
						case IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT:
							m_Canvas.DrawSpriteAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette, pTemp->Color.Color_b.a / 8);
							break;
						case IMAGE_RENDER_STYLE_3LEVEL:
							m_Canvas.DrawSprite3LevelAlpha(nX, nY, pFrame->Width,
								pFrame->Height, pFrame->Sprite, pPalette);
							break;
						case IMAGE_RENDER_STYLE_OPACITY:
							m_Canvas.DrawSprite(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette);
							break;
						case IMAGE_RENDER_STYLE_BORDER:
//							m_Canvas.DrawSpriteBorder(nX, nY, pFrame->Width, pFrame->Height,
//								///g_RGB(pTemp->Color.Color_b.r, pTemp->Color.Color_b.g, pTemp->Color.Color_b.b),
//								g_RGB(200, 200, 0),
//								pFrame->Sprite);
							break;
						case IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST:
							pPalette = m_ImageStore.GetAdjustColorPalette(pTemp->nISPosition, pTemp->Color.Color_dw);
							m_Canvas.DrawSpriteAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette, pTemp->Color.Color_b.a / 8);
							break;
						}
					}
			}
			m_Canvas.SetClipRect(&rcOld);
		}
		break;
	case RU_T_IMAGE_PART:
		{
			KRUImagePart* pTemp = (KRUImagePart *)pPrimitives;
			RECT rcOld;
			m_Canvas.GetClipRect(&rcOld);
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{				
				switch(pTemp->nType)
				{
				case ISI_T_SPR:
					{
						SPRFRAME* pFrame;
						SPRHEAD* pSprHeader = (SPRHEAD*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType, (void*&)pFrame);
						if (pSprHeader == NULL)
							break;

						int nX = pTemp->oPosition.nX;
						int nY = pTemp->oPosition.nY;
						if (bSinglePlaneCoord == false)
							CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
						if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
						{
							nX -= pSprHeader->CenterX;
							nY -= pSprHeader->CenterY;
						}
//						Check Current Draw Device??;

						// Clipper
						if ((pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW) == 0)
						{
							nX += pFrame->OffsetX;
							nY += pFrame->OffsetY;
						}

						RECT	rc;
						rc.left  = nX;
						rc.top   = nY;
						nX -= pTemp->oImgLTPos.nX;
						nY -= pTemp->oImgLTPos.nY;
						rc.right = nX + pTemp->oImgRBPos.nX;
						rc.bottom= nY + pTemp->oImgRBPos.nY;
						if (rc.left < rcOld.left)
							rc.left = rcOld.left;
						if (rc.right > rcOld.right)
							rc.right = rcOld.right;
						if (rc.top < rcOld.top)
							rc.top = rcOld.top;
						if (rc.bottom > rcOld.bottom)
							rc.bottom = rcOld.bottom;
						m_Canvas.SetClipRect(&rc);

						char* pPalette = GET_SPR_PALETTE(pSprHeader);

						switch(pTemp->bRenderStyle)
						{
						case IMAGE_RENDER_STYLE_ALPHA:
						case IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT:
							m_Canvas.DrawSpriteAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette, pTemp->Color.Color_b.a / 8);
							break;
						case IMAGE_RENDER_STYLE_3LEVEL:
							m_Canvas.DrawSprite3LevelAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette);
							break;
						case IMAGE_RENDER_STYLE_OPACITY:
							m_Canvas.DrawSprite(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette);
							break;
						case IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST:
							pPalette = m_ImageStore.GetAdjustColorPalette(pTemp->nISPosition, pTemp->Color.Color_dw);
							m_Canvas.DrawSpriteAlpha(nX, nY, pFrame->Width, pFrame->Height,
								pFrame->Sprite, pPalette, pTemp->Color.Color_b.a / 8);
							break;
						}
					}
					break;
				case ISI_T_BITMAP16:
					{
						void* pFrame;
						KSGImageContent* pBitmap = (KSGImageContent*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType, pFrame);
						if (pBitmap)
						{
							int nX = pTemp->oPosition.nX;
							int nY = pTemp->oPosition.nY;
							if (bSinglePlaneCoord == false)
								CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
	
							RECT	rc;
							rc.left  = nX;
							rc.top   = nY;
							nX -= pTemp->oImgLTPos.nX;
							nY -= pTemp->oImgLTPos.nY;
							rc.right = nX + pTemp->oImgRBPos.nX;
							rc.bottom= nY + pTemp->oImgRBPos.nY;
							if (rc.left < rcOld.left)
								rc.left = rcOld.left;
							if (rc.right > rcOld.right)
								rc.right = rcOld.right;
							if (rc.top < rcOld.top)
								rc.top = rcOld.top;
							if (rc.bottom > rcOld.bottom)
								rc.bottom = rcOld.bottom;
							m_Canvas.SetClipRect(&rc);

							m_Canvas.DrawBitmap16(nX, nY,
								pBitmap->nWidth, pBitmap->nHeight, pBitmap->Data);
						}
					}
					break;
				}
			}
			m_Canvas.SetClipRect(&rcOld);
		}
		break;
	case RU_T_POINT:
		{
			KRUPoint* pTemp = (KRUPoint *)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{				
				int nX = pTemp->oPosition.nX;
				int nY = pTemp->oPosition.nY;
				if (!bSinglePlaneCoord)
					CoordinateTransform(nX, nY, pTemp->oPosition.nZ);				
				m_Canvas.DrawPixel(nX, nY, g_RGB(pTemp->Color.Color_b.r,
					pTemp->Color.Color_b.g, pTemp->Color.Color_b.b));
			}
		}
		break;
	case RU_T_LINE:
		{
			KRULine* pTemp = (KRULine *)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{
				int	nX1 = pTemp->oPosition.nX;
				int nY1 = pTemp->oPosition.nY;
					
				int nX2 = pTemp->oEndPos.nX;
				int nY2 = pTemp->oEndPos.nY;
				if (!bSinglePlaneCoord)
				{
					CoordinateTransform(nX1, nY1, pTemp->oPosition.nZ);
					CoordinateTransform(nX2, nY2, pTemp->oEndPos.nZ);
				}
				if (pTemp->Color.Color_b.a >= 248)
				{
					m_Canvas.DrawLine(nX1, nY1, nX2, nY2, g_RGB(pTemp->Color.Color_b.r,
						pTemp->Color.Color_b.g, pTemp->Color.Color_b.b));
				}
				else if (pTemp->Color.Color_b.a >= 8)
				{
					m_Canvas.DrawLineAlpha(nX1, nY1, nX2, nY2,
						g_RGB(pTemp->Color.Color_b.r, pTemp->Color.Color_b.g, pTemp->Color.Color_b.b),
						31 - pTemp->Color.Color_b.a / 8);
				}
			}
		}
		break;
	case RU_T_RECT:
		{
			KRURect* pTemp = (KRURect *)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{
				int	nX1 = pTemp->oPosition.nX;
				int nY1 = pTemp->oPosition.nY;
					
				int nX2 = pTemp->oEndPos.nX;
				int nY2 = pTemp->oEndPos.nY;
				if (!bSinglePlaneCoord)
				{
					CoordinateTransform(nX1, nY1, pTemp->oPosition.nZ);
					CoordinateTransform(nX2, nY2, pTemp->oEndPos.nZ);
				}
				int	Color = g_RGB(pTemp->Color.Color_b.r,
					pTemp->Color.Color_b.g, pTemp->Color.Color_b.b);
				m_Canvas.DrawLine(nX1, nY1, nX2, nY1, Color);	//上边
				m_Canvas.DrawLine(nX1, nY2, nX2, nY2, Color);	//下边
				m_Canvas.DrawLine(nX1, nY1, nX1, nY2, Color);	//左边
				m_Canvas.DrawLine(nX2, nY1, nX2, nY2, Color);	//右边
			}
		}
		break;
	case RU_T_SHADOW:
		{
			KRUShadow* pTemp =(KRUShadow *)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{				
				int nX1 = pTemp->oPosition.nX;
				int nY1 = pTemp->oPosition.nY;
				int nX2 = pTemp->oEndPos.nX;
				int	nY2 = pTemp->oEndPos.nY;
				if (!bSinglePlaneCoord)
				{
					CoordinateTransform(nX1, nY1, pTemp->oPosition.nZ);
					CoordinateTransform(nX2, nY2, pTemp->oEndPos.nZ);
				}
				m_Canvas.ClearAlpha(nX1, nY1, nX2 - nX1, nY2 - nY1, g_RGB(pTemp->Color.Color_b.r,
					pTemp->Color.Color_b.g, pTemp->Color.Color_b.b), pTemp->Color.Color_b.a);
			}
		}
		break;
	case RU_T_IMAGE_STRETCH:
		if (bSinglePlaneCoord)
		{
			KRUImageStretch* pTemp = (KRUImage*)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{
				if (pTemp->nType == ISI_T_BITMAP16)
				{
					LPDIRECTDRAWSURFACE pSurface;
					KSGImageContent* pBitmap = (KSGImageContent*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							0, ISI_T_BITMAP16, (void*&)pSurface);

					if (pBitmap)
					{
						if (pSurface)
						{
							RECT	rc;
							rc.left = pTemp->oPosition.nX;
							rc.top = pTemp->oPosition.nY;
							rc.right = pTemp->oEndPos.nX;
							rc.bottom = pTemp->oEndPos.nY;
							m_Canvas.BltSurface(pSurface, &rc);
						}
						else
						{
							m_ImageStore.CreateBitmapSurface(pTemp->szImage, pTemp->uImage, pTemp->nISPosition);
						}
					}
				}
			}
		}
		break;
	}
}

void KRepresentShell2::DrawPrimitivesOnImage(int nPrimitiveCount, KRepresentUnit* pPrimitives, 
        unsigned int uGenre, const char* pszImage, unsigned int uImage, short& nImagePosition)
{
	KSGImageContent* pDestBitmap = (KSGImageContent*)m_ImageStore.GetExistedCreateBitmap(
		pszImage, uImage, nImagePosition);

	if (pDestBitmap == NULL)
		return;

	int   i = 0;
	int   nDestWidth  = pDestBitmap->nWidth;
	int   nDestHeight = pDestBitmap->nHeight;
	void* pDestBuffer = pDestBitmap->Data;

	switch(uGenre)
	{
	case RU_T_IMAGE:
		KRUImage* pTemp = (KRUImage*)pPrimitives;
		for (i = 0; i < nPrimitiveCount; i++, pTemp++)
		{
			switch(pTemp->nType)
			{
			case ISI_T_SPR:
				{
					SPRFRAME* pFrame;
					SPRHEAD* pSprHeader = (SPRHEAD*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType, (void*&)pFrame);
					if (pSprHeader == NULL)
						break;

					int nX = pTemp->oPosition.nX;
					int nY = pTemp->oPosition.nY;

					if ((pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW) == 0)
					{
						nX += pFrame->OffsetX;
						nY += pFrame->OffsetY;
						if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
						{
							nX -= pSprHeader->CenterX;
							nY -= pSprHeader->CenterY;
						}
					}

					char* pPalette = GET_SPR_PALETTE(pSprHeader);

					switch(pTemp->bRenderStyle)
					{
					case IMAGE_RENDER_STYLE_ALPHA:
					case IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT:
						RIO_CopySprToBufferAlpha(pFrame->Sprite, pFrame->Width, pFrame->Height,
							pPalette, pDestBuffer, nDestWidth, nDestHeight, nX, nY);
						break;
					case IMAGE_RENDER_STYLE_3LEVEL:
						RIO_CopySprToBuffer3LevelAlpha(pFrame->Sprite, pFrame->Width, pFrame->Height,
							pPalette, pDestBuffer, nDestWidth, nDestHeight, nX, nY);
						break;
					case IMAGE_RENDER_STYLE_OPACITY:
						RIO_CopySprToBuffer(pFrame->Sprite, pFrame->Width, pFrame->Height,
							pPalette, pDestBuffer, nDestWidth, nDestHeight, nX, nY);
						break;
					case IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST:
						pPalette = m_ImageStore.GetAdjustColorPalette(pTemp->nISPosition, pTemp->Color.Color_dw);
						RIO_CopySprToBufferAlpha(pFrame->Sprite, pFrame->Width, pFrame->Height,
							pPalette, pDestBuffer, nDestWidth, nDestHeight, nX, nY);
						break;
					}						
				}
				break;
			case ISI_T_BITMAP16:
				{
					void* pFrame;
					KSGImageContent* pBitmap = (KSGImageContent*)m_ImageStore.GetImage(
							pTemp->szImage,	pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType, pFrame);
					if (pBitmap)
					{
						RIO_CopyBitmap16ToBuffer(pBitmap->Data, pBitmap->nWidth, pBitmap->nHeight, pDestBitmap,
							nDestWidth, nDestHeight, pTemp->oPosition.nX, pTemp->oPosition.nY);
					}
				}
				break;
			}
		}
		break;
	}
}

//##Documentation
//## 清除图形数据
void KRepresentShell2::ClearImageData(const char* pszImage, unsigned int uImage, short nImagePosition)
{
	void* pFrame;
	KSGImageContent* pBitmap = (KSGImageContent*)m_ImageStore.GetImage(
			pszImage,	uImage, nImagePosition, 0, ISI_T_BITMAP16, pFrame);
	if (pBitmap)
		memset(pBitmap->Data, 0, 2 * pBitmap->nWidth * pBitmap->nHeight);
}

//##ModelId=3DCD8E9200E8
void KRepresentShell2::FreeAllImage()
{
	m_ImageStore.Free();
}

//##ModelId=3DCD8EF60316
void KRepresentShell2::FreeImage(const char* pszImage)
{
	m_ImageStore.FreeImage(pszImage);
}

//##ModelId=3DCD8FA900EE
void* KRepresentShell2::GetBitmapDataBuffer(const char* pszImage, KBitmapDataBuffInfo* pInfo)
{
	unsigned int uImage = 0;
	short		nISPosition = -1;
	void*		pBuffer = NULL;

	LPDIRECTDRAWSURFACE pSurface;
	KSGImageContent* pDestBitmap = (KSGImageContent *)m_ImageStore.GetImage(
					pszImage, uImage, nISPosition, 0, ISI_T_BITMAP16, (void*&)pSurface);
	if (pDestBitmap)
	{
		int nPitch = pDestBitmap->nWidth * 2;
		pBuffer = pDestBitmap->Data;
		if (pSurface)
		{
			DDSURFACEDESC	desc;
			desc.dwSize = sizeof(desc);
			if (pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL) == DD_OK)
			{
				pBuffer = desc.lpSurface;
				nPitch = desc.lPitch;
			}
		}

		 if (pInfo)
		 {
			pInfo->nWidth = pDestBitmap->nWidth;
			pInfo->nHeight = pDestBitmap->nHeight;
			pInfo->nPitch = nPitch;
			pInfo->pData = pBuffer;
			pInfo->eFormat = (m_DirectDraw.GetRGBBitMask16() == RGB_565) ? BDBF_16BIT_565 : BDBF_16BIT_555;
		}
	}
	return pBuffer;
}

//##释放对(通过GetBitmapDataBuffer调用获取得的)图形像点数据缓冲区的控制
void KRepresentShell2::ReleaseBitmapDataBuffer(const char* pszImage, void* pBuffer)
{
	unsigned int uImage = 0;
	short		nISPosition = -1;
	LPDIRECTDRAWSURFACE pSurface;
	KSGImageContent* pDestBitmap = (KSGImageContent *)m_ImageStore.GetImage(
					pszImage, uImage, nISPosition, 0, ISI_T_BITMAP16, (void*&)pSurface);
	if (pSurface && pDestBitmap)
	{
		pSurface->Unlock(NULL);
	}
}

//##ModelId=3DCA6EBC000F
bool KRepresentShell2::GetImageParam(const char* pszImage, KImageParam* pImageData, int nType) 
{
	return m_ImageStore.GetImageParam(pszImage, nType, pImageData);
}

bool KRepresentShell2::GetImageFrameParam(const char* pszImage, int nFrame,
			KRPosition2* pOffset, KRPosition2* pSize, int nType)
{
	return m_ImageStore.GetImageFrameParam(pszImage, nType, nFrame, pOffset, pSize);
}

//##ModelId=3DCA72620157
int KRepresentShell2::GetImagePixelAlpha(const char* pszImage, int nFrame, int nX, int nY, int nType)
{
	return m_ImageStore.GetImagePixelAlpha(pszImage, nType, nFrame, nX, nY);
}

//##ModelId=3DC0A08D0085
void KRepresentShell2::LookAt(int nX, int nY, int nZ)
{
	m_nLeft = nX - m_Canvas.GetWidth() / 2;
	m_nTop  = nY / 2 - ((nZ * 887) >> 10) - m_Canvas.GetHeight() / 2;
}

//##ModelId=3DCA0BAE00E4
void KRepresentShell2::OutputText(int nFontId, const char* psText, int nCount, int nX, int nY, unsigned int Color, int nLineWidth, int nZ, unsigned int BorderColor)
{
	int i;
	for (i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nFontId)
			break;
	}
	
	if ( i < RS2_MAX_FONT_ITEM_NUM && m_FontTable[i].pFontObj)
	{
		if (nZ != TEXT_IN_SINGLE_PLANE_COORD)
			CoordinateTransform(nX, nY, nZ);
		m_FontTable[i].pFontObj->SetBorderColor(BorderColor);
		m_FontTable[i].pFontObj->SetOutputSize(nFontId, nFontId + 1);
		m_FontTable[i].pFontObj->OutputText(psText, nCount, nX, nY, Color, nLineWidth);
	}
}

//##ModelId=3DB655B2000E
//##Documentation
//## 输出文字。
int KRepresentShell2::OutputRichText(int nFontId, KOutputTextParam* pParam, 
		const char* psText, int nCount, int nLineWidth)
{
	if (pParam == NULL)
		return 0;
	int i;
	for ( i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nFontId)
			break;
	}

	if (i < RS2_MAX_FONT_ITEM_NUM && m_FontTable[i].pFontObj)
	{
		KTextProcess	tp(psText, nCount, nLineWidth * 2 / nFontId);
		if (pParam->nZ != TEXT_IN_SINGLE_PLANE_COORD)
		{
			int x, y, z;
			x = pParam->nX;
			y = pParam->nY;
			z = pParam->nZ;
			CoordinateTransform(x, y, z);
			pParam->nX = x;
			pParam->nY = y;
		}
		m_FontTable[i].pFontObj->SetBorderColor(pParam->BorderColor);
		m_FontTable[i].pFontObj->SetOutputSize(nFontId, nFontId + 1);
		return tp.DrawTextLine(m_FontTable[i].pFontObj, nFontId, pParam);
	}
	return 0;
}

//## 返回指定坐标在字符串中最近的字符偏移
int KRepresentShell2::LocateRichText(int nX, int nY,
						int nFontId, KOutputTextParam* pParam, 
						const char* psText, int nCount, int nLineWidth)
{
	if (pParam == NULL)
		return -1;
	int i;
	for (i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nFontId)
			break;
	}

	if (i < RS2_MAX_FONT_ITEM_NUM && m_FontTable[i].pFontObj)
	{
		KTextProcess	tp(psText, nCount, nLineWidth * 2 / nFontId);
		if (pParam->nZ != TEXT_IN_SINGLE_PLANE_COORD)
		{
			int x, y, z;
			x = pParam->nX;
			y = pParam->nY;
			z = pParam->nZ;
			CoordinateTransform(x, y, z);
			pParam->nX = x;
			pParam->nY = y;
		}
		m_FontTable[i].pFontObj->SetBorderColor(pParam->BorderColor);
		m_FontTable[i].pFontObj->SetOutputSize(nFontId, nFontId + 1);
		return tp.TransXYPosToCharOffset(nX, nY, m_FontTable[i].pFontObj, nFontId, pParam);
	}
	return -1;
}

//##ModelId=3DCA72E102FE
void KRepresentShell2::Release()
{
	m_Canvas.Terminate();
	ShutdownGdiplus();
	delete this;
}

//##ModelId=3DCA0B8102F3
void KRepresentShell2::ReleaseAFont(int nId)
{
	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nId)
		{
			m_FontTable[i].nId = 0;
			if (m_FontTable[i].pFontObj)
			{
				m_FontTable[i].pFontObj->Release();
				m_FontTable[i].pFontObj = NULL;
			}
			break;
		}
	}
}

//##ModelId=3DB69EC0023A
bool KRepresentShell2::Reset(int nWidth, int nHeight, bool bFullScreen)
{
	return Create(nWidth, nHeight, bFullScreen);
}

//##ModelId=3DCD90910361
bool KRepresentShell2::SaveImage(const char* pszFile, const char* pszImage, int nFileType)
{
	return m_ImageStore.SaveImage(pszFile, pszImage, nFileType);
}

//##ModelId=3DCD90F30011
void KRepresentShell2::SetImageStoreBalanceParam(int nNumImage, unsigned int uCheckPoint)
{
	m_ImageStore.SetBalanceParam(nNumImage, uCheckPoint);
}

//##ModelId=3DD00EEE0149
bool KRepresentShell2::CopyDeviceImageToImage(const char* pszName, int nDeviceX, int nDeviceY, int nImageX, int nImageY, int nWidth, int nHeight)
{
	if (nWidth > m_Canvas.GetWidth() - nDeviceX || nHeight > m_Canvas.GetHeight() - nDeviceY)
		return false;

	short nISPosition = -1;
	KSGImageContent* pBitmap = (KSGImageContent*)m_ImageStore.GetExistedCreateBitmap(
		pszName, 0, nISPosition);

	if (pBitmap)
	{
		if (pBitmap->nWidth >= nImageX + nWidth && pBitmap->nHeight >= nImageY + nHeight)
		{
			int nPitch;
			void* pDevice = m_Canvas.LockCanvas(nPitch);
			if (pDevice)
			{
				unsigned short*	pBuffer = &pBitmap->Data[pBitmap->nWidth * nImageY + nImageX];
				pDevice = (char*)pDevice + nDeviceY * nPitch + nDeviceX * 2;
				int nCopyLen = nWidth * 2;
				for (int i = 0; i < nHeight; i++)
				{
					memcpy(pBuffer, pDevice, nCopyLen);
					pBuffer += pBitmap->nWidth;
					pDevice = (char*)pDevice + nPitch;
				}
				m_Canvas.UnlockCanvas();
				return true;
			}
		}
	}
	return false;
}

//##ModelId=3DCFED410049
void KRepresentShell2::CoordinateTransform(int& nX, int& nY, int nZ)
{
	nX = nX - m_nLeft;
	nY = nY / 2 - m_nTop - ((nZ * 887) >> 10);	// * sqrt(3) / 2
}

//##ModelId=3DD20C45002A
bool KRepresentShell2::RepresentBegin(int bClear, unsigned int Color)
{
	KRColor	c;
	c.Color_dw = Color;
	if (bClear)
		m_Canvas.FillCanvas(g_RGB(c.Color_b.r, c.Color_b.g, c.Color_b.b));
	return true;
}

//##ModelId=3DD20C450066
void KRepresentShell2::RepresentEnd()
{
	m_Canvas.Changed(true);
	m_Canvas.UpdateScreen();
}

//视图/绘图设备坐标 转化为空间坐标
void KRepresentShell2::ViewPortCoordToSpaceCoord(int& nX,	int& nY, int  nZ)
{
	nX = nX + m_nLeft;
	nY = (nY + m_nTop + ((nZ * 887) >> 10)) * 2;
}
/*
bool KRepresentShell2::SaveScreenToFile(const char* pszName)
{
	if(!pszName || !pszName[0])
		return 0;

	DWORD n = m_Canvas.m_nWidth;

	int nPicWidth, nPicHeight, nDesktopWidth, nDesktopHeight, nPicOffX, nPicOffY;
	{
		//全屏模式参数设定
		nPicOffX = 0;
		nPicOffY = 0;
		nDesktopWidth = nPicWidth = m_Canvas.m_nWidth;
		nDesktopHeight = nPicHeight = m_Canvas.m_nHeight;
	}

	WORD *pSrc;
	BYTE *pDes, *pTemp;

	// 分配r8g8b8缓冲区	
	pTemp = pDes = new BYTE[nPicWidth * nPicHeight * 3];
	if(!pDes)
		return false;

	pSrc = (WORD*)m_Canvas.m_pCanvas;
	
	// 拷贝屏幕数据到缓冲区
	for(int i=0; i<nPicHeight; i++)
	{
		for(int j=0; j<nPicWidth; j++)
		{
			if(m_DirectDraw.GetRGBBitMask16() == RGB_565)
			{
				pDes[2] = ((*pSrc) & 0xf800) >> 8;
				pDes[1] = ((*pSrc) & 0x07e0) >> 3;
				pDes[0] = ((*pSrc) & 0x001f) << 3;
			}
			else
			{
				pDes[2] = ((*pSrc) & 0x7c00) >> 7;
				pDes[1] = ((*pSrc) & 0x03e0) >> 2;
				pDes[0] = ((*pSrc) & 0x001f) << 3;
			}
			pDes += 3;
			pSrc++;
		}
	}

	// 生成24位bmp文件
	if(!KBmpFile24::SaveBuffer24((char*)pszName, pTemp, nPicWidth*3, nPicWidth, nPicHeight))
	{
		delete[] pTemp;
		return false;
	}

	delete[] pTemp;
	return true;
}*/

bool KRepresentShell2::SaveScreenToFile(const char* pszName, ScreenFileType eType, unsigned int nQuality)
{
	if(!pszName || !pszName[0])
		return 0;

	DWORD n = m_Canvas.GetWidth();

	int nPicWidth, nPicHeight, nDesktopWidth, nDesktopHeight, nPicOffX, nPicOffY;
	if(m_DirectDraw.GetScreenMode() == WINDOWMODE)
	{
		// 窗口模式参数设定
		RECT rect;
		POINT ptLT, ptRB;
		HWND hWnd = g_GetMainHWnd();
		GetClientRect(hWnd, &rect);
		ptLT.x = rect.left, ptLT.y = rect.top;
		ptRB.x = rect.right, ptRB.y = rect.bottom;
		ClientToScreen(hWnd, &ptLT);
		ClientToScreen(hWnd, &ptRB);

		nDesktopWidth = m_DirectDraw.GetScreenWidth();
		nDesktopHeight = m_DirectDraw.GetScreenHeight();

		// 如果窗口客户区超出屏幕则返回
		if(ptLT.x >= nDesktopWidth || ptLT.y >= nDesktopHeight || ptRB.x <= 0 || ptRB.y <= 0)
			return false;
		if(ptLT.x < 0)
			ptLT.x = 0;
		if(ptLT.y < 0)
			ptLT.y = 0;
		if(ptRB.x > nDesktopWidth)
			ptRB.x = nDesktopWidth - 1;
		if(ptRB.y > nDesktopHeight)
			ptRB.y = nDesktopHeight - 1;

		nPicOffX = ptLT.x;
		nPicOffY = ptLT.y;
		nPicWidth = ptRB.x - ptLT.x;
		nPicHeight = ptRB.y - ptLT.y;
	}
	else
	{
		//全屏模式参数设定
		nPicOffX = 0;
		nPicOffY = 0;
		nDesktopWidth = nPicWidth = m_Canvas.GetWidth();
		nDesktopHeight = nPicHeight = m_Canvas.GetHeight();
	}

	WORD *pSrc;
	BYTE *pDes, *pTemp;

	// 分配r8g8b8缓冲区	
	pTemp = pDes = new BYTE[nPicWidth * nPicHeight * 3];
	if(!pDes)
		return false;

	if((pSrc = (WORD*)m_DirectDraw.LockPrimaryBuffer()) == NULL)
	{
		delete[] pTemp;
		return false;
	}
	int nPitch = m_DirectDraw.GetScreenPitch();
	pSrc += nPicOffY * nPitch / 2 + nPicOffX;
	int nLineAdd = nPitch / 2 - nPicWidth;
	
	// 拷贝屏幕数据到缓冲区
	for(int i=0; i<nPicHeight; i++)
	{
		for(int j=0; j<nPicWidth; j++)
		{
			if(m_DirectDraw.GetRGBBitMask16() == RGB_565)
			{
				pDes[2] = ((*pSrc) & 0xf800) >> 8;
				pDes[1] = ((*pSrc) & 0x07e0) >> 3;
				pDes[0] = ((*pSrc) & 0x001f) << 3;
			}
			else
			{
				pDes[2] = ((*pSrc) & 0x7c00) >> 7;
				pDes[1] = ((*pSrc) & 0x03e0) >> 2;
				pDes[0] = ((*pSrc) & 0x001f) << 3;
			}
			pDes += 3;
			pSrc++;
		}
		pSrc += nLineAdd;
	}

	m_DirectDraw.UnLockPrimaryBuffer();

	BOOL bRet;
	if(eType == SCRFILETYPE_BMP)
		// 保存24位bmp文件
		bRet = KBmpFile24::SaveBuffer24((char*)pszName, pTemp, nPicWidth*3, nPicWidth, nPicHeight);
	else
		// 保存24位jpg文件
		bRet = SaveBufferToJpgFile24((char*)pszName, pTemp, nPicWidth*3, nPicWidth, nPicHeight, nQuality);
	if(!bRet)
	{
		delete[] pTemp;
		return false;
	}

	delete[] pTemp;
	return true;
}