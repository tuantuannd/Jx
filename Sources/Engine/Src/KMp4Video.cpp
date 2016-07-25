//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp4Video.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Mpeg4 Video Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KDDraw.h"
#include "KWin32Wnd.h"
#include "KMemBase.h"
#include "KCanvas.h"
#include "KFilePath.h"
#include "KMp4Video.h"
//---------------------------------------------------------------------------
// decore options
#define DEC_OPT_INIT	0x00008000
#define DEC_OPT_RELEASE 0x00010000
#define DEC_OPT_SETPP	0x00020000 // set postprocessing mode
#define DEC_OPT_SETOUT  0x00040000 // set output mode

// decore return values
#define DEC_OK			0
#define DEC_MEMORY		1
#define DEC_BAD_FORMAT	2

// supported output formats
#define YUV12	1
#define RGB32	2 // -biHeight
#define RGB24	3 // -biHeight
#define RGB555	4 // -biHeight
#define RGB565  5 // -biHeight
#define YUV2    6
#define UYVY    7

// decore params
typedef struct _DEC_PARAM_ 
{
	int x_dim; // x dimension of the frames to be decoded
	int y_dim; // y dimension of the frames to be decoded
	unsigned long color_depth; // leaved for compatibility (new value must be NULL)
	int output_format;
} DEC_PARAM;

// decore frame
typedef struct _DEC_FRAME_
{
	void *bmp; // the 24-bit decoded bitmap 
	void *bitstream; // the decoder buffer
	long length; // the lenght of the decoder stream
	int render_flag;
} DEC_FRAME;

typedef int (*decore)(unsigned long handle,// handle - the handle of the calling entity, must be unique
					  unsigned long dec_opt,// dec_opt - the option for docoding, see below
					  void *param1,// param1 - the parameter 1 (it's actually meaning depends on dec_opt
					  void *param2);// param2 - the parameter 2 (it's actually meaning depends on dec_opt

static HMODULE	m_hDll = NULL;
static decore	Mp4Decode = NULL;
static BOOL LoadMp4Lib();
static void FreeMp4Lib();
//---------------------------------------------------------------------------
// 函数:	Load Mp4 Lib
// 功能:	加载 Mp4Lib.dll
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL LoadMp4Lib()
{
	if (m_hDll)
		return TRUE;
	char szPathName[MAXPATH];
	g_GetFullPath(szPathName, "\\mp4lib.dll");
	m_hDll = LoadLibrary(szPathName);
	if (m_hDll == NULL)
	{
		g_DebugLog("Can't load mp4lib.dll!");
		return FALSE;
	}
	Mp4Decode = (decore)GetProcAddress(m_hDll, "decore");
	if (!Mp4Decode)
	{
		g_DebugLog("There are errors in mp4lib.dll!");
		FreeMp4Lib();
		return FALSE;
	}
	g_DebugLog("mp4lib.dll load Ok!");
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FreeMp4Lib
// 功能:	
// 返回:	void
//---------------------------------------------------------------------------
void FreeMp4Lib()
{
	if (m_hDll)
		FreeLibrary(m_hDll);
	m_hDll = NULL;
}
//---------------------------------------------------------------------------
// 函数:	KMp4Video
// 功能:	构造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMp4Video::KMp4Video()
{
	m_dwHandle = 0;
}
//---------------------------------------------------------------------------
// 函数:	~KMp4Video
// 功能:	析构函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMp4Video::~KMp4Video()
{
	Close();
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开AVI文件
// 参数:	FileName	文件名
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KMp4Video::Open(LPSTR FileName)
{
	if (m_pVideo)
		Close();

	if (!m_AviFile.Open(FileName))
		return FALSE;

	if (!m_AviFile.GetVideoIndex())
		return FALSE;

	if (m_dwHandle)
		DivXExit();

	if (!DivXInit())
		return FALSE;

	return Init();
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭文件，释放资源
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Video::Close()
{
	// 必须先等线程结束，再释放内存，防止非法操作
	Stop();
	SetEvent(m_hEvent[1]);
	m_Thread.WaitForExit();
	FreeEventHandle();
	DivXExit();
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	移动当前帧
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Video::Seek(int nPercent)
{
	m_AviFile.VideoSeek(nPercent);
}
//---------------------------------------------------------------------------
// 函数:	Rewind
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Video::Rewind()
{
	m_AviFile.Rewind();
}
//---------------------------------------------------------------------------
// 函数:	NextFrame
// 功能:	下一帧
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KMp4Video::NextFrame()
{
	BOOL bRender = TRUE;

	// read next video frame
	m_nFrameSize = m_AviFile.NextFrame(m_pVideo);

	// drop this frame if necessary
	if ((DWORD)m_AviFile.CurrentFrame() * 1000 < m_Timer.GetElapse() * m_nFrameRate)
	{
		bRender = FALSE;
		SetEvent(m_hEvent[0]);
	}

	// if reach the video end
	if (m_nFrameSize >= 0)
	{	
		DivXDecode(bRender);
		if (bRender)
			DrawFrame();
		return TRUE;
	}
	else
	{
		PostMessage(g_GetMainHWnd(), WM_MP4MOVIE_END, 0, 0);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	DrawFrame
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Video::DrawFrame()
{
	RECT rc;
	
	rc.left = m_nX;
	rc.right = rc.left + m_nWidth;
	rc.top = m_nY;
	rc.bottom = rc.top + m_nHeight;

	g_pCanvas->Changed(TRUE);
	g_pCanvas->DrawBitmap16win(m_nX, m_nY, m_nWidth, m_nHeight, m_pFrame);
	g_pCanvas->UpdateCanvas();
	if (m_bZoomToWin)
		g_pCanvas->UpdateScreen();
	else
		g_pCanvas->UpdateScreen();
}
//---------------------------------------------------------------------------
// 函数:	DivXInit
// 功能:	初始化解码器
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KMp4Video::DivXInit()
{
	char				szMethod[8];
	BITMAPINFOHEADER	BmpInfoHead;

	// load mp4lib.dll
	if (!LoadMp4Lib())
		return FALSE;

	// check avi compress method
	m_AviFile.GetCompressMethod(szMethod);
	if (memcmp(szMethod, "divx", 4) != 0)
	{
		g_MessageBox("KMp4Video: Not a MPEG-4 compressed AVI file");
		return FALSE;
	}

	// get frame delay
	m_nFrameRate = m_AviFile.FrameRate();
	m_nFrameTime = (1000 + m_nFrameRate - 1) / m_nFrameRate;

	// get bitmap info header
	m_AviFile.GetBitmapInfoHeader(&BmpInfoHead);

	// check compress method
	if (memcmp(&BmpInfoHead.biCompression, "DIVX", 4) != 0)
	{
		g_MessageBox("KMp4Video: Not a MPEG-4 compressed AVI file");
		return FALSE;
	}
	
	// get frame width an height
	m_nWidth = BmpInfoHead.biWidth;
	m_nHeight = BmpInfoHead.biHeight;

	// get video buffer
	if (!m_VideoBuffer.Alloc(m_nWidth * m_nHeight * 2))
		return FALSE;
	m_pVideo = (PBYTE)m_VideoBuffer.GetMemPtr();

	// get frame buffer
	if (!m_FrameBuffer.Alloc(m_nWidth * m_nHeight * 2))
		return FALSE;
	m_pFrame = (PBYTE)m_FrameBuffer.GetMemPtr();

	// get a unique decore handle
	if (m_dwHandle == 0)
		m_dwHandle = GetTickCount();

	// get rgb bit mask
	DWORD dwRGBMask = g_pDirectDraw->GetRGBBitMask16();

	// init decore
	DEC_PARAM dec_param;

	dec_param.x_dim         = m_nWidth;
	dec_param.y_dim         = m_nHeight;
	dec_param.color_depth   = NULL;
	dec_param.output_format = (dwRGBMask == RGB_555)? RGB555 : RGB565;

	if (Mp4Decode(m_dwHandle, DEC_OPT_INIT, &dec_param, NULL) != DEC_OK)
	{
		g_MessageBox("KMp4Video: MPEG-4 decoder init fail");
		return FALSE;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	DivXDecode
// 功能:	解码
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Video::DivXDecode(BOOL bRender)
{
	DEC_FRAME dec_frame;

	dec_frame.length      = m_nFrameSize;
	dec_frame.bitstream   = m_pVideo;
	dec_frame.bmp         = m_pFrame;
	dec_frame.render_flag = bRender;
	
	if (m_dwHandle)
	{
		Mp4Decode(m_dwHandle, 0, &dec_frame, NULL);
	}
}
//---------------------------------------------------------------------------
// 函数:	DivXExit
// 功能:	释放解码器
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Video::DivXExit()
{
	if (m_dwHandle)
	{
		Mp4Decode(m_dwHandle, DEC_OPT_RELEASE, NULL, NULL);
		m_dwHandle = 0;
	}
}
//---------------------------------------------------------------------------
