//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCanvas.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	256 Color bitmap file class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KWin32Wnd.h"
#include "KDebug.h"
#include "KDDraw.h"
#include "KMemManager.h"
#include "KDrawBase.h"
#include "KDrawFont.h"
#include "KDrawFade.h"
#include "KDrawBitmap.h"
#include "KDrawBitmap16.h"
#include "KDrawSprite.h"
#include "KDrawSpriteAlpha.h"
#include "KCanvas.h"

//---------------------------------------------------------------------------
//#define BLOCK_WIDTH 640
//#define BLOCK_HEIGHT 480
//---------------------------------------------------------------------------
ENGINE_API KCanvas* g_pCanvas = NULL;
//---------------------------------------------------------------------------
// 函数:	KCanvas
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KCanvas::KCanvas()
{
	g_pCanvas = this;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nMask16 = 0;
	m_nMask32 = 0;
	m_bChanged = TRUE;
	m_pSurface = NULL;
//	m_nSubCols = 0;
//	m_nSubRows = 0;
}

KCanvas::~KCanvas()
{
	Terminate();
}
//---------------------------------------------------------------------------
// 函数:	Init canvas
// 功能:	初始化canvas
// 参数:	nWidth, nHeight
// 返回:	void
//---------------------------------------------------------------------------
bool KCanvas::Init(int nWidth, int nHeight)
{
	Terminate();

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_ClipRect.left = 0;
	m_ClipRect.top = 0;
	m_ClipRect.right = m_nWidth;
	m_ClipRect.bottom = m_nHeight;

	if (g_pDirectDraw && (nWidth % 4 == 0))
	{
		m_nMask16 = g_pDirectDraw->GetRGBBitMask16();
		m_nMask32 = g_pDirectDraw->GetRGBBitMask32();
		m_nScreenWidth  = g_pDirectDraw->GetScreenWidth();
		m_nScreenHeight = g_pDirectDraw->GetScreenHeight();
		m_nScreenPitch = g_pDirectDraw->GetScreenPitch();
		m_dwScreenMode = g_pDirectDraw->GetScreenMode();

		m_pSurface = g_pDirectDraw->CreateSurface(m_nWidth, m_nHeight);
	}
//	m_pCanvasAlloc = m_Canvas.Alloc(m_nPitch * m_nHeight + 256);
//  unsigned long ulAddress = (unsigned long)m_pCanvasAlloc;
//	ulAddress = (ulAddress + 255) / 256 * 256;
//	m_pCanvas = (PVOID)ulAddress;
//	m_pCanvas = m_pCanvasAlloc;

//	m_nSubCols = m_nWidth / BLOCK_WIDTH;
//	m_nSubRows = m_nHeight / BLOCK_HEIGHT;
//	m_SubCanvas.Alloc(m_nSubCols * m_nSubRows * sizeof(KSubCanvas));
//	m_SubCanvas.Zero();// clear sub canvas
	return (m_pSurface != NULL);
}

void KCanvas::Terminate()
{
	if (m_pSurface)
	{
		m_pSurface->Release();
		m_pSurface = NULL;
	}
}

/*
//---------------------------------------------------------------------------
// 函数:	Sub Mark Changed
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::SubMarkChanged()
{
	KSubCanvas* pSubCanvas = (KSubCanvas*)m_SubCanvas.GetMemPtr();
	int n, i;
	for (n = 0; n < m_nSubCols * m_nSubRows; n++)
	{
		if (pSubCanvas->changed)
		{
			for (i = 0; i < pSubCanvas->num; i++)
				pSubCanvas->node[i]->m_bChanged = TRUE;
		}
		pSubCanvas++;
	}
}
//---------------------------------------------------------------------------
// 函数:	Sub Add Node
// 功能:	
// 参数:	nCol, nRow, KDrawNode
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::SubAddNode(int nX,int nY,int nW,int nH,KDrawNode* pNode)
{
	int left = nX / BLOCK_WIDTH;
	int top = nY / BLOCK_HEIGHT;
	int right = (nX + nW - 1) / BLOCK_WIDTH;
	int bottom = (nY + nH - 1) / BLOCK_HEIGHT;
	if (left < 0)
		left = 0;
	if (top < 0)
		top = 0;
	if (right >= m_nSubCols)
		right = m_nSubCols - 1;
	if (bottom >= m_nSubRows)
		bottom = m_nSubRows - 1;
	int row, col;
	for (row = top; row <= bottom; row++)
		for (col = left; col <= right; col++)
			SubAdd(col, row, pNode);
}
//---------------------------------------------------------------------------
// 函数:	Sub Add node
// 功能:	
// 参数:	nCol, nRow, KDrawNode
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::SubAdd(int nCol, int nRow, KDrawNode* pNode)
{
	KSubCanvas* pSubCanvas = (KSubCanvas*)m_SubCanvas.GetMemPtr();
	pSubCanvas += nCol + nRow * m_nSubCols;
	pSubCanvas->node[pSubCanvas->num] = pNode;
	if (pSubCanvas->num < 9)
		pSubCanvas->num++;
}
//---------------------------------------------------------------------------
// 函数:	Sub Changed Node
// 功能:	
// 参数:	nCol, nRow, KDrawNode
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::SubChangedNode(int nX,int nY,int nW,int nH,KDrawNode* pNode)
{
	if (!pNode->m_bChanged)
		return;
	int left = nX / BLOCK_WIDTH;
	int top = nY / BLOCK_HEIGHT;
	int right = (nX + nW - 1) / BLOCK_WIDTH;
	int bottom = (nY + nH - 1) / BLOCK_HEIGHT;
	if (left < 0)
		left = 0;
	if (top < 0)
		top = 0;
	if (right >= m_nSubCols)
		right = m_nSubCols - 1;
	if (bottom >= m_nSubRows)
		bottom = m_nSubRows - 1;
	int row, col;
	for (row = top; row <= bottom; row++)
		for (col = left; col <= right; col++)
			SubChanged(col, row);
}
//---------------------------------------------------------------------------
// 函数:	Sub Changed
// 功能:	
// 参数:	nCol, nRow
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::SubChanged(int nCol, int nRow)
{
	KSubCanvas* pSubCanvas = (KSubCanvas*)m_SubCanvas.GetMemPtr();
	pSubCanvas += nCol + nRow * m_nSubCols;
	pSubCanvas->changed = 1;
}*/

bool KCanvas::GetUpdateRect(LPRECT lpSrc, LPRECT lpDest)
{
	//_ASSERT(lpSrc && lpDest);
	if (lpSrc->left < 0)
		lpSrc->left = 0;
	if (lpSrc->top < 0)
		lpSrc->top = 0;
	if (lpSrc->right > m_nWidth)
		lpSrc->right = m_nWidth;
	if (lpSrc->bottom > m_nHeight)
		lpSrc->bottom = m_nHeight;

	*lpDest = *lpSrc;

	g_ClientToScreen(lpDest);
	if (lpDest->left < 0)
	{
		lpSrc->left -= lpDest->left;
		lpDest->left = 0;
	}

	if (lpDest->top < 0)
	{
		lpSrc->top -= lpDest->top;
		lpDest->top = 0;
	}

	if (lpDest->right > m_nScreenWidth)
	{
		lpSrc->right += m_nScreenWidth - lpDest->right;
		lpDest->right = m_nScreenWidth;
	}

	if (lpDest->bottom > m_nScreenHeight)
	{
		lpSrc->bottom += m_nScreenHeight - lpDest->bottom;
		lpDest->bottom = m_nScreenHeight;
	}

	return ((lpDest->left < lpDest->right) && (lpDest->top < lpDest->bottom));
}

//---------------------------------------------------------------------------
// 函数:	Clip Rect
// 功能:	裁剪背景矩形
// 参数:	lpRect	矩形区域
// 返回:	void
//---------------------------------------------------------------------------
BOOL KCanvas::RectClip(LPRECT lpRect)
{
	// left clip
	if (lpRect->left < 0)
		lpRect->left = 0;
	
	// top clip
	if (lpRect->top < 0)
		lpRect->top = 0;
	
	// right clip
	if (lpRect->right > m_nScreenWidth)
		lpRect->right = m_nScreenWidth;
	
	// bottom clip
	if (lpRect->bottom > m_nScreenHeight)
		lpRect->bottom = m_nScreenHeight;

	// error check
	if ((lpRect->left > lpRect->right) || (lpRect->top > lpRect->bottom))
		return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Make Clip
// 功能:	根据绘制进行裁减
// 参数:	pClipper
// 返回:	void
//---------------------------------------------------------------------------
BOOL KCanvas::MakeClip(long nX, long nY, long nWidth, long nHeight, KClipper* pClipper)
{
	// 初始化裁减量
	pClipper->x = nX;
	pClipper->y = nY;
	pClipper->width = nWidth;
	pClipper->height = nHeight;
	pClipper->top = 0;
	pClipper->left = 0;
	pClipper->right = 0;
	
	// 上边界裁减
	if (pClipper->y < m_ClipRect.top)
	{
		pClipper->y = m_ClipRect.top;
		pClipper->top = m_ClipRect.top - nY;
		pClipper->height -= pClipper->top;
	}
	if (pClipper->height <= 0)
		return FALSE;
	
	// 下边界裁减
	if (pClipper->height > m_ClipRect.bottom - pClipper->y)
	{
		pClipper->height = m_ClipRect.bottom - pClipper->y;
	}
	if (pClipper->height <= 0)
		return FALSE;
	
	// 左边界裁减
	if (pClipper->x < m_ClipRect.left)
	{
		pClipper->x = m_ClipRect.left;
		pClipper->left = m_ClipRect.left - nX;
		pClipper->width -= pClipper->left;
	}
	if (pClipper->width <= 0)
		return FALSE;
	
	// 右边界裁减
	if (pClipper->width > m_ClipRect.right - pClipper->x)
	{
		pClipper->right = pClipper->width + pClipper->x - m_ClipRect.right;
		pClipper->width -= pClipper->right;
	}
	if (pClipper->width <= 0)
		return FALSE;
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Fill Canvas
// 功能:	清除背景缓存(MMX版本)
// 参数:	dwColor	填充颜色
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::FillCanvas(WORD wColor)
{
	if (m_pSurface)
	{
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(ddbltfx);
		ddbltfx.dwFillColor = wColor;
		m_pSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);
	}

/*	if (m_pSurface)
	{
		DDSURFACEDESC	desc;
		desc.dwSize = sizeof(desc);
		if (m_pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL) == DD_OK)
		{
			long lPitch = desc.lPitch;
			long nWidth  = desc.dwWidth;
			long nHeight = desc.dwHeight;
			WORD FillColor[4];
			FillColor[0] = wColor;
			FillColor[1] = wColor;
			FillColor[2] = wColor;
			FillColor[3] = wColor;
			__asm
			{
				lea		edi, FillColor
				movq	mm0, [edi]
				mov		esi, desc.lpSurface
				mov		eax, desc.dwWidth
				shr		eax, 3
				mov		ebx, desc.dwHeight
				mul		ebx
			loc_fill_loop:
				movq	[esi], mm0
				movq	[esi + 8], mm0
				add		esi, 16

				dec     eax
				jnz		loc_fill_loop
				emms
			}
		}
	}*/
}

//---------------------------------------------------------------------------
// 函数:	Update canvas
// 功能:	刷新画布
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::UpdateCanvas()
{
/*	SubMarkChanged();
	KDrawNode* pNode;
	while (pNode = (KDrawNode *)m_DrawList.GetHead())
	{
		if (TRUE)//pNode->m_bChanged)
			pNode->m_pDrawFunc(pNode, this);
		pNode->Remove();
		g_MemManager.Free(pNode);
	}*/
}
/*
//---------------------------------------------------------------------------
// 函数:	Update Screen Zoom
// 功能:	刷新屏幕
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::UpdateScreen(LPRECT lpRect)
{
	int cx = lpRect->left;
	int cy = lpRect->top;
	int width = lpRect->right - lpRect->left;
	int height = lpRect->bottom - lpRect->top;
	PWORD pScreenBuf = (PWORD)g_pDirectDraw->LockPrimaryBuffer();
	PWORD pCanvasBuf = (PWORD)m_pCanvas;
	int sx, sy;
	for (sy = 0; sy < m_nScreenHeight; sy++)
		for (sx = 0; sx < m_nScreenWidth; sx++)
			pScreenBuf[sx + sy * m_nScreenPitch / 2] = 
			pCanvasBuf[sx * width / m_nScreenWidth + cx +
			(sy * height / m_nScreenHeight + cy ) * m_nWidth];
	g_pDirectDraw->UnLockPrimaryBuffer();
	m_SubCanvas.Zero();// clear sub canvas
}*/

//---------------------------------------------------------------------------
// 函数: 把Surface整个Blt到画布上
// 参数: pDest画布目标位置
//---------------------------------------------------------------------------
void KCanvas::BltSurface(LPDIRECTDRAWSURFACE pSurface, RECT* pDestRect)
{
	if (m_pSurface && pSurface && pDestRect)
	{
		m_pSurface->Blt(pDestRect, pSurface, NULL, DDBLT_WAIT, NULL);
	}
}

//---------------------------------------------------------------------------
// 函数:	Blit canvas to back buffer
// 功能:	把Canvas的内容Copy到显存的BackBuffer上
// 参数:	LPRECT
// 返回:	void
//---------------------------------------------------------------------------
inline void KCanvas::UpdateBackBuffer(LPRECT lpRect)
{
	RECT	SrcRc, DestRc;

	if (lpRect == NULL)
	{
		SrcRc.left = 0;
		SrcRc.top = 0;
		SrcRc.right = m_nWidth;
		SrcRc.bottom = m_nHeight;
	}
	else
	{
		SrcRc = *lpRect;
	}

	if (GetUpdateRect(&SrcRc, &DestRc))
	{
		g_pDirectDraw->BltToBackBuffer(m_pSurface, &DestRc, &SrcRc);
		g_pDirectDraw->UpdateScreen(&DestRc);
	}

/*	RECT rc = *lpRect;

	g_ClientToScreen(&rc);
	int cx = (rc.left < 0)?(lpRect->left - rc.left):lpRect->left;
	int cy = (rc.top < 0)?(lpRect->top - rc.top):lpRect->top;
	if (!RectClip(&rc))
		return;
	
	int x = rc.left;
	int y = rc.top;
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	char* pScreenBuf = (char*)g_pDirectDraw->LockBackBuffer();
	if (pScreenBuf == NULL)
		return;
	pScreenBuf = pScreenBuf + y * m_nScreenPitch + x * 2;
	
	char* pCanvasBuf = (char*)m_pCanvas;
	pCanvasBuf = pCanvasBuf + cy * m_nPitch + cx * 2;
	
	long line_esi = m_nPitch - width * 2;
	long line_edi = m_nScreenPitch - width * 2;



	__asm
	{
		mov		esi, pCanvasBuf
		mov		edi, pScreenBuf
		mov		ebx, width
		mov		edx, height

loc_back_loop1:
		mov		ecx, ebx
		shr		ecx, 3
		jcxz	loc_back_loop3

loc_back_loop2:
		movq	mm0, [esi]
		movq    mm1, [esi + 8]
		add		esi, 16
        
        movq	[edi], mm0
        movq	[edi + 8], mm1
        add     edi, 16

        dec     ecx
		jnz		loc_back_loop2

loc_back_loop3:
		mov		ecx, ebx
		and		ecx, 7
		rep		movsw

		add		esi, line_esi
		add		edi, line_edi
        dec     edx
		jnz		loc_back_loop1
		emms
	}

	
//	__asm
//	{
//		mov		esi, pCanvasBuf
//		mov		edi, pScreenBuf
//		mov		ebx, width
//		mov		edx, height
//
//loc_back_loop1:
//		mov		ecx, ebx
//		shr		ecx, 2
//		jcxz	loc_back_loop3
//
//loc_back_loop2:
//		movq	mm0, [esi]
//		add		esi, 8
//		movq	[edi], mm0
//		add		edi, 8
//		dec		ecx
//		jnz		loc_back_loop2
//
//loc_back_loop3:
//		mov		ecx, ebx
//		and		ecx, 3
//		rep		movsw
//		add		esi, line_esi
//		add		edi, line_edi
//		dec		edx
//		jnz		loc_back_loop1
//		emms
//	}

	g_pDirectDraw->UnLockBackBuffer();
	g_pDirectDraw->UpdateScreen(&rc);*/
}
//---------------------------------------------------------------------------
// 函数:	Blit Canvas to front buffer
// 功能:	把Canvas的内容Copy到主显存上
// 参数:	LPRECT
// 返回:	void
//---------------------------------------------------------------------------
inline void KCanvas::UpdateFrontBuffer(LPRECT lpRect)
{
	RECT	SrcRc, DestRc;
	if (lpRect == NULL)
	{
		SrcRc.left = 0;
		SrcRc.top = 0;
		SrcRc.right = m_nWidth;
		SrcRc.bottom = m_nHeight;
	}
	else
	{
		SrcRc = *lpRect;
	}

	if (GetUpdateRect(&SrcRc, &DestRc))
	{
		g_pDirectDraw->BltToFrontBuffer(m_pSurface, &DestRc, &SrcRc);
		g_pDirectDraw->UpdateScreen(&DestRc);
	}

/*
	RECT rc = *lpRect;
	
	g_ClientToScreen(&rc);
	int cx = (rc.left < 0)?(lpRect->left - rc.left):lpRect->left;
	int cy = (rc.top < 0)?(lpRect->top - rc.top):lpRect->top;
	if (!RectClip(&rc))
		return;
	
	int x = rc.left;
	int y = rc.top;
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

    //g_pDirectDraw->WaitForVerticalBlankBegin();
    //g_pDirectDraw->WaitForVerticalBlankEnd();

	
	char* pScreenBuf = (char*)g_pDirectDraw->LockPrimaryBuffer();
	if (pScreenBuf == NULL)
		return;
	pScreenBuf = pScreenBuf + y * m_nScreenPitch + x * 2;
	
	char* pCanvasBuf = (char*)m_pCanvas;
	pCanvasBuf = pCanvasBuf + cy * m_nPitch + cx * 2;
	
	long line_esi = m_nPitch - width * 2;
	long line_edi = m_nScreenPitch - width * 2;

	__asm
	{
		mov		esi, pCanvasBuf
		mov		edi, pScreenBuf
		mov		ebx, width
		mov		edx, height

loc_front_loop1:
		mov		ecx, ebx
		shr		ecx, 3
		jcxz	loc_front_loop3

loc_front_loop2:
		movq	mm0, [esi]
		movq    mm1, [esi + 8]
		add		esi, 16
        
        movq	[edi], mm0
        movq	[edi + 8], mm1
        add     edi, 16

		dec		ecx
		jnz		loc_front_loop2

loc_front_loop3:
		mov		ecx, ebx
		and		ecx, 7
		rep		movsw

		add		esi, line_esi
		add		edi, line_edi
		dec		edx
		jnz		loc_front_loop1
		emms
	}
	
//	__asm
//	{
//		mov		esi, pCanvasBuf
//		mov		edi, pScreenBuf
//		mov		ebx, width
//		mov		edx, height
//
//loc_front_loop1:
//		mov		ecx, ebx
//		shr		ecx, 2
//		jcxz	loc_front_loop3
//
//loc_front_loop2:
//		movq	mm0, [esi]
//		add		esi, 8
//		movq	[edi], mm0
//		add		edi, 8
//		dec		ecx
//		jnz		loc_front_loop2
//
//loc_front_loop3:
//		mov		ecx, ebx
//		and		ecx, 3
//		rep		movsw
//		add		esi, line_esi
//		add		edi, line_edi
//		dec		edx
//		jnz		loc_front_loop1
//		emms
//	}

	g_pDirectDraw->UnLockPrimaryBuffer();*/
}

//---------------------------------------------------------------------------
// 函数:	Update Screen
// 功能:	刷新屏幕
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::UpdateScreen()
{
/*	KSubCanvas* pSubCanvas = (KSubCanvas*)m_SubCanvas.GetMemPtr();
	RECT rc;
	int row, col;
	for (row = 0; row < m_nSubRows; row++)
	{
		for (col = 0; col < m_nSubCols; col++)
		{
			if (TRUE)//pSubCanvas->changed)
			{
				rc.left = col * BLOCK_WIDTH;
				rc.right = rc.left + BLOCK_WIDTH;
				rc.top = row * BLOCK_HEIGHT;
				rc.bottom = rc.top + BLOCK_HEIGHT;
				UpdateBackBuffer(&rc);
			}
			pSubCanvas->changed = 0;
			pSubCanvas->num = 0;
			pSubCanvas++;
		}
	}
	*/
/*	RECT	rc;
	rc.top		= 0;
	rc.left		= 0;
	rc.bottom	= m_nHeight;
	rc.right	= m_nWidth;
*/

    //if (m_dwScreenMode == FULLSCREEN)
    //{  
    //    速度是快,但是会导致输入法闪烁  
    //    UpdateFrontBuffer(&rc);
    //    return;
    //}
	//else
    //{
        UpdateBackBuffer(NULL);
    //    return;
    //}
}

//---------------------------------------------------------------------------
// 函数:	Draw pixel
// 功能:	绘制象素
// 参数:	nX, nY, nColor
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawPixel(int nX, int nY, int nColor)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawPixel;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nColor = nColor;
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, 1, 1, pNode);
	SubChangedNode(nX, nY, 1, 1, pNode);*/
	g_DrawPixel(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw pixel alpha
// 功能:	绘制象素(带透明度)
// 参数:	nX, nY, nColor, nAlpha
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawPixelAlpha(int nX, int nY, int nColor, int nAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawPixelAlpha;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nColor = nColor;
	Node.m_nAlpha = nAlpha;
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, 1, 1, pNode);
	SubChangedNode(nX, nY, 1, 1, pNode);*/
	g_DrawPixelAlpha(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw line
// 功能:	绘制直线
// 参数:	nX1, nY1, nX2, nY2, nColor
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawLine(int nX1, int nY1, int nX2, int nY2, int nColor)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawLine;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX1;
	Node.m_nY = nY1;
	Node.m_nWidth = nX2;
	Node.m_nHeight = nY2;
	Node.m_nColor = nColor;
//	m_DrawList.AddTail(pNode);
	int nX = min(nX1, nY1);
	int nY = min(nY1, nY2);
	int nW = abs(nX1 - nX2);
	int nH = abs(nY1 - nY2);
/*	SubAddNode(nX, nY, nW, nH, pNode);
	SubChangedNode(nX, nY, nW, nH, pNode);*/
	g_DrawLine(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw line alpha
// 功能:	绘制直线
// 参数:	nX1, nY1, nX2, nY2, nColor, nAlpha
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawLineAlpha(int nX1, int nY1, int nX2, int nY2, int nColor, int nAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawLineAlpha;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX1;
	Node.m_nY = nY1;
	Node.m_nWidth = nX2;
	Node.m_nHeight = nY2;
	Node.m_nColor = nColor;
	Node.m_nAlpha = nAlpha;
//	m_DrawList.AddTail(pNode);
	int nX = min(nX1, nY1);
	int nY = min(nY1, nY2);
	int nW = abs(nX1 - nX2);
	int nH = abs(nY1 - nY2);
//	SubAddNode(nX, nY, nW, nH, pNode);
//	SubChangedNode(nX, nY, nW, nH, pNode);
	g_DrawLineAlpha(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw sprite on canvas
// 功能:	绘制动画精灵位图
// 参数:	nX, nY, nWidth, nHeight, lpSprite, lpPalette
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawSprite(int nX, int nY, int nWidth, int nHeight,
						 void* lpSprite, void* lpPalette)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawSprite;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpSprite;
	Node.m_pPalette = lpPalette;
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, nWidth, nHeight, pNode);
	SubChangedNode(nX, nY, nWidth, nHeight, pNode);*/
	g_DrawSprite(&Node, this);
}

//---------------------------------------------------------------------------
// 函数:	Draw sprite Mix color on canvas
// 功能:	混色绘制动画精灵位图
// 参数:	nX, nY, nWidth, nHeight, lpSprite, lpPalette,nColor,nAlpha
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawSpriteMixColor(int nX, int nY, int nWidth, int nHeight,
						 void* lpSprite, void* lpPalette,int nColor,int nAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawSprite;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpSprite;
	Node.m_pPalette = lpPalette;
    Node.m_nColor=nColor;
	Node.m_nAlpha=nAlpha;
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, nWidth, nHeight, pNode);
	SubChangedNode(nX, nY, nWidth, nHeight, pNode);*/
	g_DrawSpriteMixColor(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw sprite with color on canvas
// 功能:	混色绘制动画精灵位图
// 参数:	nX, nY, nWidth, nHeight, lpSprite, lpPalette,nColor,nAlpha
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawSpriteWithColor(int nX, int nY, int nWidth, int nHeight,
						 void* lpSprite, void* lpPalette,int nColor,int nAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawSprite;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpSprite;
	Node.m_pPalette = lpPalette;
    Node.m_nColor=nColor;
	Node.m_nAlpha=nAlpha;
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, nWidth, nHeight, pNode);
	SubChangedNode(nX, nY, nWidth, nHeight, pNode);*/
	g_DrawSpriteWithColor(&Node, this);
}


//---------------------------------------------------------------------------
// 函数:	Draw sprite alpha on canvas
// 功能:	绘制动画精灵位图
// 参数:	nX, nY, nWidth, nHeight, lpSprite, lpPalette
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawSpriteAlpha(int nX, int nY, int nWidth, int nHeight,
							  void* lpSprite, void* lpPalette, int nExAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawSpriteAlpha;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpSprite;
	Node.m_pPalette = lpPalette;
	
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, nWidth, nHeight, pNode);
	SubChangedNode(nX, nY, nWidth, nHeight, pNode);*/
	if (nExAlpha >= 31)
		g_DrawSpriteAlpha(&Node, this);
	else
		g_DrawSpriteAlpha(&Node, this, nExAlpha);
}

void KCanvas::DrawSpriteBorder(int nX, int nY, int nWidth, int nHeight, int nColor, void* lpSprite)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawSpriteAlpha;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpSprite;
	g_DrawSpriteBorder(&Node, this);
}

void KCanvas::DrawSprite3LevelAlpha(int nX, int nY, int nWidth, int nHeight, void* lpSprite, void* lpPalette)
{
	KDrawNode	Node;
//	Node.m_pPrev = NULL;
//	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawSpriteAlpha;
//	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpSprite;
	Node.m_pPalette = lpPalette;
	
/*	m_DrawList.AddTail(pNode);
	SubAddNode(nX, nY, nWidth, nHeight, pNode);
	SubChangedNode(nX, nY, nWidth, nHeight, pNode);*/

	g_DrawSprite3LevelAlpha(&Node, this);
}

//---------------------------------------------------------------------------
// 函数:	Draw font on canvas
// 功能:	绘制字体
// 参数:	nX, nY, nWidth, nHeight, nColor, lpFont
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawFont(int nX, int nY, int nWidth, int nHeight, int nColor, int nAlpha,void* lpFont)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawFont;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_nColor = nColor;
	Node.m_pBitmap = lpFont;
	Node.m_nAlpha =  nAlpha;

	g_DrawFont(&Node, this);
}

void KCanvas::DrawFontWithBorder(int nX, int nY, int nWidth, int nHeight, int nColor, int nAlpha,void* lpFont, int nBorderColor)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_nColor = nColor;
	Node.m_pBitmap = lpFont;
	Node.m_nAlpha =  nBorderColor;

	g_DrawFontWithBorder(&Node, this);
}

//---------------------------------------------------------------------------
// 函数:	Draw font on canvas
// 功能:	绘制实心字体
// 参数:	nX, nY, nWidth, nHeight, nColor, lpFont
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawFontSolid(int nX, int nY, int nWidth, int nHeight, int nColor, int nAlpha,void* lpFont)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawFont;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_nColor = nColor;
	Node.m_pBitmap = lpFont;
	Node.m_nAlpha =  nAlpha;

	g_DrawFontSolid(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw bitmap on canvas
// 功能:	绘制256色位图
// 参数:	nX, nY, nWidth, nHeight, lpBitmap, lpPalette
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawBitmap(int nX, int nY, int nWidth, int nHeight, void* lpBitmap, void* lpPalette)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawBitmap;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpBitmap;
	Node.m_pPalette = lpPalette;

	g_DrawBitmap(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw fade on canvas
// 功能:	绘制阴影区域
// 参数:	nX, nY, nWidth, nHeight, nLevel
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawFade(int nX, int nY, int nWidth, int nHeight, int nAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawFade;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_nAlpha = nAlpha;

	g_DrawFade(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw bitmap on canvas
// 功能:	绘制16位色位图
// 参数:	nX, nY, nWidth, nHeight, lpBitmap
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawBitmap16(int nX, int nY, int nWidth, int nHeight, void* lpBitmap)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawBitmap16;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpBitmap;
	
    //g_DrawBitmap16(&Node, this);
    
    // Change by Freeway Chen in 2003.7.3
	g_DrawBitmap16mmx(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw bitmap on canvas
// 功能:	绘制16位色位图
// 参数:	nX, nY, nWidth, nHeight, lpBitmap
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawBitmap16mmx(int nX, int nY, int nWidth, int nHeight, void* lpBitmap)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawBitmap16mmx;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpBitmap;

	g_DrawBitmap16mmx(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Draw bitmap on canvas
// 功能:	绘制16位色位图
// 参数:	nX, nY, nWidth, nHeight, lpBitmap
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DrawBitmap16win(int nX, int nY, int nWidth, int nHeight, void* lpBitmap)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_DrawBitmap16win;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
	Node.m_pBitmap = lpBitmap;

	g_DrawBitmap16win(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Clear a region on canvas
// 功能:	用色彩nColor清除区域
// 参数:	nX, nY, nWidth, nHeight, nColor
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::Clear(int nX, int nY, int nWidth, int nHeight, int nColor)
{
	DDBLTFX ddbltfx;
	if (m_pSurface)
	{
		ddbltfx.dwSize = sizeof(ddbltfx);
		ddbltfx.dwFillColor = nColor;
		m_pSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);
	}

/*	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_Clear;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
    Node.m_nColor = nColor;

	g_Clear(&Node, this);*/
}
//---------------------------------------------------------------------------
// 函数:	DotClear a region on canvas
// 功能:	用色彩nColor隔点清除区域
// 参数:	nX, nY, nWidth, nHeight, nColor
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::DotClear(int nX, int nY, int nWidth, int nHeight, int nColor)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_Clear;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
    Node.m_nColor = nColor;

	g_DotClear(&Node, this);
}
//---------------------------------------------------------------------------
// 函数:	Clear a region on canvas
// 功能:	用色彩nColor带alpha清除区域清除区域
// 参数:	nX, nY, nWidth, nHeight, nColor
// 返回:	void
//---------------------------------------------------------------------------
void KCanvas::ClearAlpha(int nX, int nY, int nWidth, int nHeight, int nColor,int nAlpha)
{
	KDrawNode	Node;
	Node.m_pPrev = NULL;
	Node.m_pNext = NULL;
//	Node.m_pDrawFunc = g_ClearAlpha;
	Node.m_bChanged = m_bChanged;
	Node.m_nX = nX;
	Node.m_nY = nY;
	Node.m_nWidth = nWidth;
	Node.m_nHeight = nHeight;
    Node.m_nColor = nColor;
	Node.m_nAlpha = nAlpha;

	g_ClearAlpha(&Node, this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void* KCanvas::LockCanvas(int& nPitch)
{
	void* pBuffer = NULL;
	if (m_pSurface)
	{
		DDSURFACEDESC	desc;
		desc.dwSize = sizeof(desc);
		if (m_pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL) == DD_OK)
		{
			pBuffer = desc.lpSurface;
			nPitch = desc.lPitch;
		}
	}
	return pBuffer;
}

void KCanvas::UnlockCanvas()
{
	if (m_pSurface)
		m_pSurface->Unlock(NULL);
}