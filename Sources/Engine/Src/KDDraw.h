//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDDraw.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KDDraw_H
#define KDDraw_H
//---------------------------------------------------------------------------
#define FULLSCREEN			0
#define WINDOWMODE			1
#define RGB_555 			0x7fff
#define RGB_565 			0xffff

#define	WND_INIT_WIDTH		800
#define	WND_INIT_HEIGHT		600

#include "KMemBase.h"

//---------------------------------------------------------------------------
class ENGINE_API KDirectDraw
{
private:
	LPDIRECTDRAW		m_lpDirectDraw;
	LPDIRECTDRAWSURFACE	m_lpDDSPrimary;
	LPDIRECTDRAWSURFACE	m_lpDDSBackBuf;
	LPDIRECTDRAWCLIPPER m_lpClipper;
private:
	DWORD		m_dwScreenMode;
	DWORD		m_dwScreenWidth;
	DWORD		m_dwScreenHeight;
	DWORD		m_dwScreenPitch;
	DWORD		m_dwRGBBitCount;
	DWORD		m_dwRGBBitMask16;
	DWORD		m_dwRGBBitMask32;
private:
	BOOL		CreateDirectDraw();
	BOOL		CreateClipper();
	BOOL		CreateSurface();
	BOOL		GetSurfaceDesc();
	BOOL		GetDisplayMode();
	BOOL		SetDisplayMode();
	void		SetWindowStyle();

public:
	KDirectDraw();
	~KDirectDraw();
	void		Mode(BOOL bFullScreen, int nWidth, int nHeight);
	BOOL		Init();
	void		Exit();
	void		WaitForVerticalBlankBegin();
	void		WaitForVerticalBlankEnd();
	BOOL		RestoreSurface();
	void		SetClipperHWnd(HWND hWnd);
	PVOID		LockPrimaryBuffer();
	void		UnLockPrimaryBuffer();
	PVOID		LockBackBuffer();
	void		UnLockBackBuffer();
	void		FillBackBuffer(DWORD dwColor);
	void		UpdateScreen(LPRECT lpRect);
	void		UpdateScreenZoom(LPRECT lpRect);
	LPDIRECTDRAWSURFACE CreateSurface(int nWidth, int nHeight);
	void		BltToFrontBuffer(LPDIRECTDRAWSURFACE pSurface, RECT* pDestRect, RECT* pSrcRect);
	void		BltToBackBuffer(LPDIRECTDRAWSURFACE pSurface, RECT* pDestRect, RECT* pSrcRect);

public:
	DWORD		GetScreenMode(){return m_dwScreenMode;};
	DWORD		GetScreenWidth(){return m_dwScreenWidth;};
	DWORD		GetScreenHeight(){return m_dwScreenHeight;};
	DWORD		GetScreenPitch(){return m_dwScreenPitch;};
	DWORD		GetRGBBitCount(){return m_dwRGBBitCount;};
	DWORD		GetRGBBitMask16(){return m_dwRGBBitMask16;};
	DWORD		GetRGBBitMask32(){return m_dwRGBBitMask32;};
};


//---------------------------------------------------------------------------
// 函数:	Lock Primary Buffer
// 功能:	开始绘制，锁定表面
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
inline LPVOID KDirectDraw::LockPrimaryBuffer()
{
	DDSURFACEDESC	ddsd;
	HRESULT			hres;
	
	// check back buffer
	if (m_lpDDSPrimary == NULL)
		return NULL;
	
	// set struct size
	g_MemZero(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	// lock back buffer
	hres = m_lpDDSPrimary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if (DD_OK == hres)
		return ddsd.lpSurface;
	
	return NULL;
}
//---------------------------------------------------------------------------
// 函数:	UnLock Primary Buffer
// 功能:	结束绘制，解锁表面
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
inline void KDirectDraw::UnLockPrimaryBuffer()
{
	if (m_lpDDSPrimary)
		m_lpDDSPrimary->Unlock(NULL);
}
//---------------------------------------------------------------------------
// 函数:	LockBackBuffer
// 功能:	开始绘制，锁定背景表面
// 参数:	void
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
inline LPVOID KDirectDraw::LockBackBuffer()
{
	DDSURFACEDESC	ddsd;
	HRESULT			hres;
	
	// check back buffer
	if (m_lpDDSBackBuf == NULL)
		return NULL;
	
	// set struct size
	g_MemZero(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	// lock back buffer
	hres = m_lpDDSBackBuf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if (DD_OK == hres)
		return ddsd.lpSurface;

	return NULL;
}
//---------------------------------------------------------------------------
// 函数:	UnLockBackBuffer
// 功能:	结束绘制，解锁背景表面
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
inline void KDirectDraw::UnLockBackBuffer()
{
	if (m_lpDDSBackBuf)
		m_lpDDSBackBuf->Unlock(NULL);
}

//---------------------------------------------------------------------------
// 函数:	UpdateScreen
// 功能:	刷新屏幕
// 参数:	LPRECT
// 返回:	void
//---------------------------------------------------------------------------
inline void KDirectDraw::UpdateScreen(LPRECT lpRect)
{
    //if (m_dwScreenMode == FULLSCREEN)
    //{
    //    速度是快,但是会导致输入法闪烁  
    //    m_lpDDSPrimary->Flip(NULL, DDFLIP_WAIT);
    //}
    //else
    //{
	    if (m_lpDDSPrimary)
	    {
		    m_lpDDSPrimary->Blt(lpRect, m_lpDDSBackBuf, lpRect, DDBLT_WAIT, NULL);
	    }
    //}
}




extern ENGINE_API KDirectDraw* g_pDirectDraw;

//---------------------------------------------------------------------------
#endif
