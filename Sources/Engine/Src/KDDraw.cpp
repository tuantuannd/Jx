//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDDraw.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Direct Draw Related Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KDError.h"
#include "KColors.h"
#include "KMemBase.h"
#include "KWin32Wnd.h"
#include "KDDraw.h"
//#include "KMouse.h"
//---------------------------------------------------------------------------
ENGINE_API KDirectDraw* g_pDirectDraw = NULL;
//---------------------------------------------------------------------------
// 函数:	KDirectDraw
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KDirectDraw::KDirectDraw()
{
	g_pDirectDraw		= this;
	m_lpDirectDraw		= NULL;
	m_lpDDSPrimary		= NULL;
	m_lpDDSBackBuf		= NULL;
	m_lpClipper			= NULL;
	m_dwScreenMode		= WINDOWMODE;
	m_dwScreenWidth		= WND_INIT_WIDTH;
	m_dwScreenHeight	= WND_INIT_HEIGHT;
	m_dwRGBBitCount		= 16;
}
//---------------------------------------------------------------------------
// 函数:	~KDirectDraw
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KDirectDraw::~KDirectDraw()
{
	Exit();
}
//---------------------------------------------------------------------------
// 函数:	Mode
// 功能:	Set screen mode
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::Mode(BOOL bFullScreen, int nWidth, int nHeight)
{
	m_dwScreenMode   = bFullScreen? FULLSCREEN : WINDOWMODE;
	m_dwScreenWidth  = nWidth;
	m_dwScreenHeight = nHeight;
}

//---------------------------------------------------------------------------
// 函数:	Init
// 功能:	创建DirectDraw对象
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::Init()
{
	// free prior object
	if (m_lpDirectDraw)
		Exit();
	
	// change window style
	SetWindowStyle();

	// create direct draw
	if (!CreateDirectDraw())
	{
		g_MessageBox("DirectDraw : CreateDirectDraw() Failed");
		return FALSE;
	}
	
	// set display mode
	if (!SetDisplayMode())
	{
		g_MessageBox("DirectDraw : SetDisplayMode() Failed");
		return FALSE;
	}
	
	// get display mode
	if (!GetDisplayMode())
	{
		g_MessageBox("DirectDraw : GetDisplayMode() Failed");
		return FALSE;
	}

	// create surface
	if (!CreateSurface())
	{
		g_MessageBox("DirectDraw : CreateSurface() Failed");
		return FALSE;
	}
	
	// get surface desc
	if (!GetSurfaceDesc())
	{
		g_MessageBox("DirectDraw : GetSurfaceDesc() Failed");
		return FALSE;
	}

	// create cliper
	if (!CreateClipper())
	{
		g_MessageBox("DirectDraw : CreateClipper() Failed");
		return FALSE;
	}
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Exit
// 功能:	释放DirectDraw对象
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::Exit()
{
	if (m_lpDirectDraw)
	{
		m_lpDirectDraw->RestoreDisplayMode();
	}

	if (m_lpDDSBackBuf)
	{
		m_lpDDSBackBuf->Release();
		m_lpDDSBackBuf = NULL;
	}
	
	if (m_lpDDSPrimary)
	{
		m_lpDDSPrimary->Release();
		m_lpDDSPrimary = NULL;
	}
	
	if (m_lpClipper)
	{
		m_lpClipper->Release();
		m_lpClipper = NULL;
	}
	
	if (m_lpDirectDraw)
	{
		m_lpDirectDraw->Release();
		m_lpDirectDraw = NULL;
	}
}
//---------------------------------------------------------------------------
// 函数:	CreateDirectDraw
// 功能:	创建DirectDraw对象
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::CreateDirectDraw()
{
	HRESULT	hres;
	DWORD	dwFlags;
	
	// create direct draw
	hres = DirectDrawCreate(
		NULL,
		&m_lpDirectDraw,
		NULL); 
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}
	
	// set cooperative level
	if (m_dwScreenMode == FULLSCREEN)
	{
		dwFlags = DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
	}
	else
	{
		dwFlags = DDSCL_NORMAL;
		DDSURFACEDESC	dsd;
		memset(&dsd, 0, sizeof(dsd));
		dsd.dwSize = sizeof(dsd);
		if (m_lpDirectDraw->GetDisplayMode(&dsd) != DD_OK ||
			(dsd.ddpfPixelFormat.dwRGBBitCount != 15 &&
				dsd.ddpfPixelFormat.dwRGBBitCount != 16))
		{
			return FALSE;
		}
	}
	hres = m_lpDirectDraw->SetCooperativeLevel(g_GetMainHWnd(), dwFlags);
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	SetDisplayMode
// 功能:	设置显示模式
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::SetDisplayMode()
{
	HRESULT		hres;
	
	// return true if in window mode
	if (m_dwScreenMode == WINDOWMODE)
		return TRUE;
	
	// set display mode
	hres = m_lpDirectDraw->SetDisplayMode(
		m_dwScreenWidth,	// Screen Width
		m_dwScreenHeight,	// Screen Height
		m_dwRGBBitCount);	// Bits Per Pixel
	
	// check return value
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}
	
	// output debug message
	g_DebugLog("g_SetDisplayMode()...");
	g_DebugLog("Screen Width  = %d", m_dwScreenWidth);
	g_DebugLog("Screen Height = %d", m_dwScreenHeight);
	g_DebugLog("Screen BPP    = %d", m_dwRGBBitCount);
	
	// return true
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetDisplayMode
// 功能:	取得显示模式
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::GetDisplayMode()
{
	DDSURFACEDESC	ddsd;
	HRESULT			hres;
	
	// return true if in fullscreen mode
	if (m_dwScreenMode == FULLSCREEN)
		return TRUE;

	// init ddsd struct
	g_MemZero(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_ALL;
	
	// get display mode
	hres = m_lpDirectDraw->GetDisplayMode(&ddsd);

	// check return value
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}
	
	// get screen caps
	m_dwScreenWidth  = ddsd.dwWidth;
	m_dwScreenHeight = ddsd.dwHeight;
	m_dwScreenPitch  = ddsd.lPitch;
	
	// output debug message
	g_DebugLog("g_GetDisplayMode()...");
	g_DebugLog("Screen Width  = %d", m_dwScreenWidth);
	g_DebugLog("Screen Height = %d", m_dwScreenHeight);
	g_DebugLog("Screen Pitch  = %d", m_dwScreenPitch);
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	CreateSurface
// 功能:	创建表面
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::CreateSurface()
{
	DDSURFACEDESC	ddsd;
	DDSCAPS			ddscaps;
	HRESULT			hres;
	
	// check ddraw object
	if (m_lpDirectDraw == NULL)
		return FALSE;
	
	// setup ddsd struct
	g_MemZero(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	if (m_dwScreenMode == FULLSCREEN)
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | 
			DDSCAPS_FLIP | DDSCAPS_COMPLEX; 
		ddsd.dwBackBufferCount = 1;
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE; 
	}

	// create primary surface
	hres = m_lpDirectDraw->CreateSurface(&ddsd, &m_lpDDSPrimary, NULL);

	// check return vlue
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}

	// create back buffer surface
	if (m_dwScreenMode == FULLSCREEN)
	{
		g_MemZero(&ddscaps, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER; 
		hres = m_lpDDSPrimary->GetAttachedSurface(&ddscaps, &m_lpDDSBackBuf);
	}
	else
	{
		g_MemZero(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = m_dwScreenWidth;
		ddsd.dwHeight = m_dwScreenHeight;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		hres = m_lpDirectDraw->CreateSurface(&ddsd, &m_lpDDSBackBuf, NULL);
	}

	// check return value
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}

	return TRUE;
}

LPDIRECTDRAWSURFACE KDirectDraw::CreateSurface(int nWidth, int nHeight)
{
	LPDIRECTDRAWSURFACE	pSurface = NULL;
	if (m_lpDirectDraw)
	{
		DDSURFACEDESC	ddsd;
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = nWidth;
		ddsd.dwHeight = nHeight;
		ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
		if (m_lpDirectDraw->CreateSurface(&ddsd, &pSurface, NULL) != DD_OK)
			pSurface = NULL;
	}
	return pSurface;
}

//---------------------------------------------------------------------------
// 函数:	RestoreSurface
// 功能:	恢复丢失的表面
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::RestoreSurface()
{
	if (m_lpDDSPrimary)
	{
		if (m_lpDDSPrimary->IsLost())
			m_lpDDSPrimary->Restore();
	}
	if (m_lpDDSBackBuf)
	{
		if (m_lpDDSBackBuf->IsLost())
			m_lpDDSBackBuf->Restore();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetSurfaceDesc
// 功能:	取得绘图表面的参数
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::GetSurfaceDesc()
{
	DDSURFACEDESC	ddsd;
	HRESULT			hres;
	
	// check back surface pointer
	if (m_lpDDSBackBuf == NULL)
		return FALSE;
	
	// set ddsd struct size
	g_MemZero(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_ALL;
	
	// get surface desc
	hres = m_lpDDSBackBuf->GetSurfaceDesc(&ddsd);
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}
	
	// set globle value
	m_dwScreenPitch  = ddsd.lPitch;
	m_dwRGBBitCount	 = ddsd.ddpfPixelFormat.dwRGBBitCount;
	m_dwRGBBitMask16 = ddsd.ddpfPixelFormat.dwRBitMask |
						ddsd.ddpfPixelFormat.dwGBitMask |
						ddsd.ddpfPixelFormat.dwBBitMask;
	m_dwRGBBitMask32 = (m_dwRGBBitMask16 & 0x8000)? 0x07e0f81f : 0x03e07c1f;
	g_RGB = (m_dwRGBBitMask16 & 0x8000)? g_RGB565 : g_RGB555;
	
	// output debug message
	g_DebugLog("g_GetSurfaceDesc()...");
	g_DebugLog("Surface Width   = %d", ddsd.dwWidth);
	g_DebugLog("Surface Height  = %d", ddsd.dwHeight);
	g_DebugLog("Surface Picth   = %d", m_dwScreenPitch);
	g_DebugLog("RGB Bit Count   = %d", m_dwRGBBitCount);
	g_DebugLog("RGB Bit Mask16  = %x", m_dwRGBBitMask16);
	g_DebugLog("RGB Bit Mask32  = %x", m_dwRGBBitMask32);
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	CreateClipper
// 功能:	创建裁剪器
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KDirectDraw::CreateClipper()
{
	HRESULT  hres;
	
	// check ddraw object
	if (m_lpDirectDraw == NULL)
		return FALSE;
	
	// return true in full screen mode
	//if (m_dwScreenMode == FULLSCREEN)
	//	return TRUE;
	
	// create a window clipper
	hres = m_lpDirectDraw->CreateClipper(0, &m_lpClipper, NULL);
	if (hres != DD_OK)
	{
		g_DDError(hres);
		return FALSE;
	}

	// set clipper hwnd
	SetClipperHWnd(g_GetDrawHWnd());
    m_lpDirectDraw->FlipToGDISurface();
	
	// attach clipper to primary surface
	hres = m_lpDDSPrimary->SetClipper(m_lpClipper);
	if( hres != DD_OK )
	{
		g_DDError(hres);
		return FALSE;
	}

    //m_lpDirectDraw->FlipToGDISurface();
    
    //m_lpClipper->Release();
    //m_lpClipper = NULL;




	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	SetClipperHWnd
// 功能:	设置裁剪器的窗口句柄
// 参数:	hWnd	窗口句柄
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::SetClipperHWnd(HWND hWnd)
{
	static HWND hWndClip;

	// check handle of window
	if (hWndClip == hWnd)
		return;
	
	// save the handle of window
	hWndClip = hWnd;
	
	// set handle of window
	if (m_lpClipper)
		m_lpClipper->SetHWnd(0, hWndClip);
}
//---------------------------------------------------------------------------
// 函数:	WaitForVerticalBlankBegin
// 功能:	等待垂直回扫周期开始
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::WaitForVerticalBlankBegin()
{
	if (m_lpDirectDraw)
		m_lpDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
}
//---------------------------------------------------------------------------
// 函数:	WaitForVerticalBlankEnd
// 功能:	等待垂直回扫周期结束
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::WaitForVerticalBlankEnd()
{
	if (m_lpDirectDraw)
		m_lpDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
}
//---------------------------------------------------------------------------
// 函数:	ClearBackBuffer
// 功能:	用指定颜色填充绘图表面
// 参数:	dwColor		颜色值
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::FillBackBuffer(DWORD dwColor)
{
	DDBLTFX ddbltfx;
	if (m_lpDDSBackBuf == NULL)
		return;
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = dwColor;
	m_lpDDSBackBuf->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);
}
//---------------------------------------------------------------------------
// 函数:	UpdateScreenZoom
// 功能:	刷新屏幕
// 参数:	LPRECT
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::UpdateScreenZoom(LPRECT lpRect)
{
	if (m_lpDDSPrimary)
		m_lpDDSPrimary->Blt(NULL, m_lpDDSBackBuf, lpRect, DDBLT_WAIT, NULL);
}
//---------------------------------------------------------------------------
// 函数:	SetWindowStyle
// 功能:	根据显示模式改变窗口风格
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KDirectDraw::SetWindowStyle()
{
	RECT	rc = {0, 0, m_dwScreenWidth, m_dwScreenHeight};
	HWND	hWnd = g_GetMainHWnd();
	DWORD	dwStyle = 0;
	
	if (GetScreenMode() == FULLSCREEN)
	{
		dwStyle = WS_VISIBLE | WS_SYSMENU | WS_POPUP;
		SetWindowLong(hWnd, GWL_STYLE, dwStyle);
	}
	else
	{
		dwStyle = WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED | 
			WS_CAPTION | WS_MINIMIZEBOX;
		SetWindowLong(hWnd, GWL_STYLE, dwStyle);
		AdjustWindowRectEx(&rc,
			GetWindowLong(hWnd, GWL_STYLE),
			GetMenu(hWnd) != NULL,
			GetWindowLong(hWnd, GWL_EXSTYLE));
		SetWindowPos(hWnd,
			HWND_NOTOPMOST,
			0,
			0,
			rc.right - rc.left,
			rc.bottom - rc.top,
			SWP_NOACTIVATE);
	}
}
//---------------------------------------------------------------------------

void KDirectDraw::BltToFrontBuffer(LPDIRECTDRAWSURFACE pSurface, RECT* pDestRect, RECT* pSrcRect)
{
	if (m_lpDDSPrimary && pSurface)
	{
		m_lpDDSPrimary->Blt(pDestRect, pSurface, pSrcRect, DDBLT_WAIT, NULL);
	}
}

void KDirectDraw::BltToBackBuffer(LPDIRECTDRAWSURFACE pSurface, RECT* pDestRect, RECT* pSrcRect)
{
	if (m_lpDDSBackBuf && pSurface)
	{
		m_lpDDSBackBuf->Blt(pDestRect, pSurface, pSrcRect, DDBLT_WAIT, NULL);
	}
}