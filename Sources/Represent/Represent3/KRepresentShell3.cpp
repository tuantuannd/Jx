/*****************************************************************************************
//  表现模块的对外接口的三维版本实现。
//	Copyright : Kingsoft 2002
//	Author	:   cp(Chenpeng)
//	CreateTime:	2003-3-4
*****************************************************************************************/
#include "precompile.h"
#include "KRepresentShell3.h"
#include "..\iRepresent\Font\KFont3.h"
#include "d3d_shell.h"
#include "d3d_device.h"
#include "d3d_utils.h"
#include <dxerr.h>

#include "TextureRes.h"
HWND	g_hWnd = NULL;
int		g_ntest = 0;

D3DFORMAT g_16BitFormat = D3DFMT_R5G6B5;

bool	g_bRefRast = false;
bool	g_bRunWindowed = true;

int32	g_nScreenWidth = DEFAULT_SCREENWIDTH;
int32	g_nScreenHeight = DEFAULT_SCREENHEIGHT;

uint32	g_nAntiAliasFSOverSample = 0;
uint32	g_nBackBufferCount = 1;
bool	g_bForceSwapEffectBlt = false;
bool	g_bForce32Bit = true;

int		g_nFogEnable = 0;

int		g_nWireframe = 0;

RenderModel	g_renderModel = RenderModel3DPerspective;

bool	g_bUse4444Texture = true;

bool	g_bNonPow2Conditional = false;

bool Test3D()
{
	IDirectDraw7 *pDDraw7;
	DirectDrawCreateEx(NULL, (void**)&pDDraw7, IID_IDirectDraw7, NULL);
	DDCAPS hel_ddcaps, hal_ddcpas ;
	hel_ddcaps.dwSize = sizeof(DDCAPS);
	hal_ddcpas.dwSize = sizeof(DDCAPS);

	if(!pDDraw7)
		return false;

	pDDraw7->GetCaps(&hel_ddcaps, &hal_ddcpas);
	pDDraw7->Release();

	// 如果显存小于32兆则返回假
	if(hel_ddcaps.dwVidMemTotal < 33554432)
		return false;

	MEMORYSTATUS stat;
	GlobalMemoryStatus (&stat);
	// 如果物理内存小于128兆则返回假
	if(stat.dwTotalPhys < 134217728)
		return false;

	return true;
}

static inline void DOWRDToRGB(DWORD color, DWORD &a, DWORD &r, DWORD &g, DWORD &b)
{
	a = color >> 24;
	r = (color & 0x00ff0000) >> 16;
	g = (color & 0x0000ff00) >> 8;
	b = (color & 0x000000ff);
}

static inline DWORD ARGBToDWORD(DWORD a, DWORD r, DWORD g, DWORD b)
{
	return (a<<24) | (r<<16) | (g<<8) | b;
}

// 将color1用color2做偏色
static inline DWORD ScaleColor(DWORD color1, DWORD color2)
{
	DWORD a1, r1, g1, b1, a2, r2, g2, b2;
	DOWRDToRGB(color1, a1, r1, g1, b1);
	DOWRDToRGB(color2, a2, r2, g2, b2);

	r1 = r1 * r2 / 256;     // 255
	g1 = g1 * g2 / 256;     // 255
	b1 = b1 * b2 / 256;     // 255

	return ARGBToDWORD(a1, r1, g1, b1);
}

static inline DWORD ScaleColor(DWORD color1, DWORD r, DWORD g, DWORD b)
{
	DWORD a1, r1, g1, b1;
	DOWRDToRGB(color1, a1, r1, g1, b1);
	r1 = r1 * r / 256;     // 255
	g1 = g1 * g / 256;     // 255
	b1 = b1 * b / 256;     // 255

	return ARGBToDWORD(a1, r1, g1, b1);
}

// inline函数放在文件的前部，可以使得Release版本效率更高
inline unsigned int KRepresentShell3::GetPoint3dLighting(D3DXVECTOR3& v)
{
	if(!m_bDoLighting)
		return 0xff404040;

	unsigned uX = (unsigned)(int)(((int)v.x) - m_nLightingAreaX);
    if (uX >= 1536)
        return pLightingArray[0];
    
	unsigned uY = (unsigned)(int)(((int)v.y) - m_nLightingAreaY);
    if (uY >= 3072)
        return pLightingArray[0];
        
	return pLightingArray[
		// 这里不能写成uY * LIGHTING_GRID_WIDTH / LIGHTING_GRID_SIZEY,因为uY / LIGHTING_GRID_SIZEY这一步舍去小数部分是必须的
        (uY / LIGHTING_GRID_SIZEY * LIGHTING_GRID_WIDTH) +
        (uX / LIGHTING_GRID_SIZEX)
    ];
}

void __fastcall KRepresentShell3::SetPoint3dLighting(VERTEX3D& pDes, VERTEX3D& pSrc, DWORD color)
{
	pDes = pSrc;
	pDes.color = GetPoint3dLighting(pDes.position);
	if(color != 0xffffffff)
		pDes.color = ScaleColor(pDes.color, color);
}


static WORD g_A8ToA4[256] =
{
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,
    0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,
    0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,
    0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,
    0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,
    0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,
    0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,
    0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,
    0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,
    0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,
    0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,
    0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,
    0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,
    0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,
    0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,
    0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,
    0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,
    0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,
    0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,
    0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,
    0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,
    0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,
    0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,
    0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,
    0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,
    0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000
};

// 将spr数据转换到A4R4G4B4缓冲区
void RenderToA4R4G4B4(
    WORD* pDest, uint32 nPitch, PBYTE pData,
    RECT &rect, uint32 nWidth, uint32 nHeight, 
    WORD* pPalette
)
{
	uint32 nPixelCount = 0;							// 总字节计数
	uint32 nTotlePixel = nWidth * nHeight;
    uint32 nNextWidth = nWidth;
	uint32 pixelNum;
	BYTE *pTexLine = (BYTE*)pDest;					// 贴图每一行数据
	pTexLine += rect.top * nPitch + rect.left * 2;
	BYTE *pLine = pTexLine;
	for(;;)
	{
		pixelNum = *pData++;
		nPixelCount += pixelNum;
        uint32 alpha = *pData++;
		uint32 uAlpha = g_A8ToA4[alpha];

		if (uAlpha == 0)
		{
			pLine += pixelNum * 2;
            if (alpha != 0)
                pData += pixelNum;    
		}
		else
		{
			while (pixelNum--)
			{
				*((WORD*)pLine) = uAlpha | pPalette[*pData++];
				pLine += 2;
			}
		}

		// 如果一帧数据解完则停止
		if (nPixelCount >= nNextWidth)
        {
			pLine = pTexLine = pTexLine + nPitch;
            nNextWidth += nWidth;
        }

		assert(nPixelCount <= nTotlePixel);
		if(nPixelCount >= nTotlePixel)
			break;
	}
}

// 渲染窗口的窗口函数
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

// 将世界坐标转换到屏幕坐标
D3DXVECTOR3* WorldToScreen
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DVIEWPORT9 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld)
{
	D3DXMATRIX mat = (*pView) * (*pProjection);
	D3DXVec3TransformCoord(pOut, pV, &mat);
	pOut->x = (pOut->x + 1.0f) * pViewport->Width / 2;
	pOut->y = (1.0f - pOut->y) * pViewport->Height / 2;
	return pOut;
}

// fd2,fd2目标区域，fs1,fs2原区域，点在原区域中的位置
// 返回点在目标区域的位置
inline float ChaZhi(float fd1, float fd2, float fs1, float fs2, float fs)
{
	return (fs - fs1) / (fs2 - fs1) * (fd2 - fd1) + fd1;
}

// 按fs相对于fs1和fs2的位置对v1和v2进行差值
inline D3DXVECTOR3 ChaZhi(D3DXVECTOR3 v1, D3DXVECTOR3 v2, float fs1, float fs2, float fs)
{
	return (v2 - v1) * (fs - fs1) / (fs2 - fs1) + v1;
}

//=========测试模块性能，判断是否推荐使用===============
extern "C" __declspec(dllexport)
bool RepresentIsModuleRecommended()
{
	return Test3D();
}

//=========创建一个iRepresentShell接口的实例===============
extern "C" __declspec(dllexport)
iRepresentShell* CreateRepresentShell()
{
	return (new KRepresentShell3);
}

IInlinePicEngineSink* g_pIInlinePicSinkRP = NULL;	//嵌入式图片的处理接口[wxb 2003-6-20]
HRESULT KRepresentShell3::AdviseRepresent(IInlinePicEngineSink* pSink)
{
	assert(NULL == g_pIInlinePicSinkRP);	//一般不会挂接两次
	g_pIInlinePicSinkRP = pSink;
	return S_OK;
}
HRESULT KRepresentShell3::UnAdviseRepresent(IInlinePicEngineSink* pSink)
{
	if (pSink == g_pIInlinePicSinkRP)
		g_pIInlinePicSinkRP = NULL;
	return S_OK;
}

KRepresentShell3::KRepresentShell3()
{
	m_nLeft = 0;
	m_nTop = 0;
	m_pPreRenderTexture128 = NULL;
	m_pPreRenderTexture256 = NULL;
	m_pVB2D = NULL;
	m_pVB3D = NULL;
	m_bDeviceLost = false;
	m_bDoLighting = true;
	memset(m_FontTable, 0, sizeof(m_FontTable));
}

KRepresentShell3::~KRepresentShell3()
{
	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].pFontObj)
		{
			m_FontTable[i].pFontObj->Release();
			m_FontTable[i].pFontObj = NULL;
			m_FontTable[i].nId = 0;
		}
	}
}

void KRepresentShell3::SetOption(RepresentOption eOption,	bool bOn)
{
	switch (eOption)
	{
	case DYNAMICLIGHT:
		if(bOn)
			m_bDoLighting = true;
		else
			m_bDoLighting = false;
		break;
	case PERSPECTIVE:
		{	if(bOn)
				g_renderModel = RenderModel3DPerspective;
			else
				g_renderModel = RenderModel3DOrtho;

			SetUpProjectionMatrix();
		}
		break;
	}
}

void KRepresentShell3::SetUpProjectionMatrix()
{
	// 根据g_renderModel设置投影变换矩阵
	float fAspect = (float)(g_nScreenWidth / g_nScreenHeight * 1.37);
	if(g_renderModel == RenderModel3DOrtho)
		D3DXMatrixOrthoLH(&m_matProj, 800, 600, 1.0f, 20000.0f );
	else
		D3DXMatrixPerspectiveFovLH( &m_matProj, D3DX_PI/24, fAspect, 1.0f, 20000.0f );
	PD3DDEVICE->SetTransform( D3DTS_PROJECTION, &m_matProj );
}

bool KRepresentShell3::Create(int nWidth, int nHeight, bool bFullScreen)
{
	if (!g_D3DShell.Create())
		return false;
	g_DebugLog("[D3DRender]g_D3DShell create ok!");

	// 取得窗口句柄
	g_hWnd = g_GetMainHWnd();
	if(!g_hWnd)
		return false;

	g_nScreenWidth = nWidth;
	g_nScreenHeight = nHeight;
	g_bRunWindowed = !bFullScreen;

	if(!g_bRunWindowed)
	{
		LONG winLong = GetWindowLong(g_hWnd, GWL_STYLE);
		winLong &= ~WS_BORDER;
		winLong &= ~WS_CAPTION;
		SetWindowLong(g_hWnd, GWL_STYLE, winLong);
	}

	D3DAdapterInfo* pAdapterInfo	 = NULL;
	D3DDeviceInfo*  pDeviceInfo		 = NULL;
	D3DModeInfo*	pModeInfo		 = NULL;

	pDeviceInfo = g_D3DShell.PickDefaultDev(&pAdapterInfo);
	if (!pDeviceInfo)
	{
		// 无法找到合适的D3D设备
		D3DTerm();
		g_DebugLog("[D3DRender]Can't find any d3d devices to use!");
		return false; 
	}

	pModeInfo = g_D3DShell.PickDefaultMode(pDeviceInfo,DEFAULT_BITDEPTH);
	if (!pModeInfo)
	{
		// 无法找到合适的显示模式
		D3DTerm();
		g_DebugLog("[D3DRender]Can't find an appropriate display mode!");
		return false;
	}

	// 创建设备
	if (!g_Device.CreateDevice(pAdapterInfo,pDeviceInfo,pModeInfo))
	{
		D3DTerm();
		g_DebugLog("[D3DRender]Couldn't create D3D Device!");
		return false;
	}

	// 设置显示模式
	if (!Reset(nWidth, nHeight, bFullScreen))
		return false;
	g_DebugLog("[D3DRender]Device reset ok!");

	if (!RestoreDeviceObjects())
		return false;
	g_DebugLog("[D3DRender]RestoreDeviceObjects ok!");

	if (!InitDeviceObjects())
		return false;
	g_DebugLog("[D3DRender]InitDeviceObjects ok!");

	PD3DDEVICE->SetDialogBoxMode(true);

	PD3DDEVICE->GetGammaRamp(0, &m_ramp);

	SetGamma(50);

	// 初始化Gdi+
	InitGdiplus();

	return true;
}

bool KRepresentShell3::InitDeviceObjects()
{
	// 创建预渲染主角的贴图
	if (FAILED(PD3DDEVICE->CreateTexture(SPR_PRERENDER_TEXSIZE1, SPR_PRERENDER_TEXSIZE1, 1,
								0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pPreRenderTexture128, NULL)))
		return false;
	if (FAILED(PD3DDEVICE->CreateTexture(SPR_PRERENDER_TEXSIZE2, SPR_PRERENDER_TEXSIZE2, 1,
								0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pPreRenderTexture256, NULL)))
		return false;
	return true;
}

void KRepresentShell3::Release()
{
	DeleteDeviceObjects();
	D3DTerm();
	ShutdownGdiplus();
	delete this;
}

void KRepresentShell3::DeleteDeviceObjects()
{

	SAFE_RELEASE( m_pPreRenderTexture128 );
	SAFE_RELEASE( m_pPreRenderTexture256 );
	SAFE_RELEASE( m_pVB3D );
	SAFE_RELEASE( m_pVB2D );
	m_TextureResMgr.Free();
}

bool KRepresentShell3::InvalidateDeviceObjects()
{
	SAFE_RELEASE( m_pVB2D );
	SAFE_RELEASE( m_pVB3D );

	if(!m_TextureResMgr.InvalidateDeviceObjects())
		return false;

	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].pFontObj)
		{
			m_FontTable[i].pFontObj->InvalidateDeviceObjects();
		}
	}
	return true;
}

bool KRepresentShell3::RestoreDeviceObjects()
{
	int i;
	if(!m_pVB2D)
	{
		// 创建非透视状态使用的顶点缓冲区
		if( FAILED(PD3DDEVICE->CreateVertexBuffer( VERTEX_BUFFER_SIZE*sizeof(VERTEX2D),
						D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC | D3DUSAGE_SOFTWAREPROCESSING, 
						D3DFVF_VERTEX2D, D3DPOOL_DEFAULT, &m_pVB2D, NULL)))
			return false;
	}

	if((g_renderModel == RenderModel3DOrtho || g_renderModel == RenderModel3DPerspective) && !m_pVB3D)
	{
		// 创建透视状态使用的顶点缓冲区
		if( FAILED(PD3DDEVICE->CreateVertexBuffer( VERTEX_BUFFER_SIZE*sizeof(VERTEX3D),
						D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC | D3DUSAGE_SOFTWAREPROCESSING, 
						D3DFVF_VERTEX3D, D3DPOOL_DEFAULT, &m_pVB3D, NULL)))
			return false;
	}

	for (i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].pFontObj)
		{
			m_FontTable[i].pFontObj->RestoreDeviceObjects();
		}
	}

	if(!m_TextureResMgr.RestoreDeviceObjects())
		return false;

	// 设置渲染状态
	// 使用Alpha混合
	PD3DDEVICE->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    PD3DDEVICE->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    PD3DDEVICE->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
	// 关闭Alpha测试
    PD3DDEVICE->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
    PD3DDEVICE->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
    PD3DDEVICE->SetRenderState( D3DRS_CULLMODE,   D3DCULL_NONE );
    PD3DDEVICE->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    PD3DDEVICE->SetRenderState( D3DRS_CLIPPING,         FALSE );
    PD3DDEVICE->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
    PD3DDEVICE->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
    PD3DDEVICE->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
    PD3DDEVICE->SetRenderState( D3DRS_FOGENABLE,        FALSE );
	PD3DDEVICE->SetRenderState( D3DRS_LIGHTING,FALSE );
	// 设置贴图渲染阶段0
	// 设置颜色混合模式
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	// 设置Alpha混合模式
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	// 设置过滤模式
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    PD3DDEVICE->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	// 关闭0以上的贴图渲染阶段
    PD3DDEVICE->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    PD3DDEVICE->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	g_Device.SetupViewport(0, g_nScreenWidth, 0, g_nScreenHeight, 0.0f, 1.0f);

	if(g_renderModel == RenderModel3DPerspective || g_renderModel == RenderModel3DOrtho)
	{
		// 设置世界坐标变换矩阵
		D3DXMATRIX matWorld;
		D3DXMatrixIdentity( &matWorld );
		PD3DDEVICE->SetTransform( D3DTS_WORLD, &matWorld );

		// 设置投影变换矩阵
		SetUpProjectionMatrix();
		PD3DDEVICE->SetTransform( D3DTS_PROJECTION, &m_matProj );
	}

	PD3DDEVICE->SetPixelShader( NULL );

	return true;
}

bool KRepresentShell3::Reset(int nWidth, int nHeight, bool bFullScreen)
{
	g_Device.ReleaseDevObjects();

	D3DAdapterInfo* pAdapterInfo	 = NULL;
	D3DDeviceInfo*  pDeviceInfo		 = NULL;
	D3DModeInfo*	pModeInfo		 = NULL;

	pAdapterInfo  = g_Device.GetAdapterInfo();
	pDeviceInfo	  = g_Device.GetDeviceInfo();
	pModeInfo	  = g_D3DShell.PickDefaultMode(pDeviceInfo,DEFAULT_BITDEPTH);

	if (!pModeInfo)
	{
		// 无法找到合适的显示模式
		D3DTerm();
		g_DebugLog("[D3DRender]Can't find an appropriate display mode!");
		return false;
	}

	if (!g_Device.SetMode(pModeInfo))
	{
		// 无法设置合适的显示模式
		D3DTerm();
		g_DebugLog("[D3DRender]Can't find an appropriate display mode!");
		return false;
	}
	
	if((pModeInfo->Format == D3DFMT_X1R5G5B5))
		g_16BitFormat = D3DFMT_X1R5G5B5;
	else
		g_16BitFormat = D3DFMT_R5G6B5;
	
	g_Device.SetDefaultRenderStates();

	g_Device.RestoreDevObjects();

	return true;
}

bool KRepresentShell3::CreateAFont(const char* pszFontFile, CHARACTER_CODE_SET CharaSet, int nId)
{
	if(m_bDeviceLost)
		return false;

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
				m_FontTable[nFirstFree].pFontObj = (KFont3*)m_FontTable[j].pFontObj->Clone();
				return true;
			}
		}
		return false;
	}
	
	if ((m_FontTable[nFirstFree].pFontObj = new KFont3) == NULL)
		return false;

	m_FontTable[nFirstFree].pFontObj->Init(PD3DDEVICE);
	if (!m_FontTable[nFirstFree].pFontObj->Load((LPSTR)pszFontFile/*, CharaSet*/))
	{
		m_FontTable[nFirstFree].pFontObj->Release();
		m_FontTable[nFirstFree].pFontObj = NULL;
	}
	
	m_FontTable[nFirstFree].nId = nId;
	return (m_FontTable[nFirstFree].pFontObj != NULL);
}

unsigned int KRepresentShell3::CreateImage(const char* pszName, int nWidth, int nHeight, int nType)
{
	if(m_bDeviceLost)
		return 0;

	return m_TextureResMgr.CreateImage(pszName, nWidth, nHeight, nType);
}

void KRepresentShell3::DrawPrimitives(int nPrimitiveCount, KRepresentUnit* pPrimitives, unsigned int uGenre, int bSinglePlaneCoord)
{
	if(!pPrimitives)
	{
		assert(pPrimitives);
		return;
	}

	if(m_bDeviceLost)
		return;

	int i = 0;
	
	switch(uGenre)
	{
	case RU_T_IMAGE:
	case RU_T_IMAGE_4:
		{
			if( g_renderModel == RenderModel2D || bSinglePlaneCoord)
				DrawImage2D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord);
			else
				DrawImage3D(uGenre, nPrimitiveCount, pPrimitives, bSinglePlaneCoord);
		}
		break;
	case RU_T_IMAGE_STRETCH:
		if(bSinglePlaneCoord)
			DrawImage2DStretch(nPrimitiveCount, pPrimitives);
		break;
	case RU_T_IMAGE_PART:
		{
			KRUImagePart* pTemp = (KRUImagePart *)pPrimitives;
			for (i = 0; i < nPrimitiveCount; i++, pTemp++)
			{				
				switch(pTemp->nType)
				{
				case ISI_T_SPR:
					{
						TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
							pTemp->szImage,	pTemp->uImage,
							pTemp->nISPosition, pTemp->nFrame, pTemp->nType);
						if (!pSprite || pTemp->nFrame >= pSprite->m_nFrameNum)
							break;

						int nX = pTemp->oPosition.nX;
						int nY = pTemp->oPosition.nY;
						if (!bSinglePlaneCoord)
							CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
						if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
						{
							nX -= pSprite->GetCenterX();
							nY -= pSprite->GetCenterY();
						}

						if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
						{
							nX += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX;
							nY += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY;
						}

						// Clipper
						RECT rc;
						rc.left  = nX;
						rc.top   = nY;
						nX -= pTemp->oImgLTPos.nX;
						nY -= pTemp->oImgLTPos.nY;
						rc.right = nX + pTemp->oImgRBPos.nX;
						rc.bottom= nY + pTemp->oImgRBPos.nY;
						
						switch(pTemp->bRenderStyle)
						{
						case IMAGE_RENDER_STYLE_ALPHA:
						case IMAGE_RENDER_STYLE_3LEVEL:
						case IMAGE_RENDER_STYLE_OPACITY:
						case IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT:
						{
							DrawSpritePartAlpha(nX, nY, pSprite->m_pFrameInfo[pTemp->nFrame].nWidth, 
								pSprite->m_pFrameInfo[pTemp->nFrame].nHeight, pTemp->nFrame, pSprite, rc);
							break;
						}
						}
					}
					break;
				case ISI_T_BITMAP16:
					{
						TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
						pTemp->szImage,	pTemp->uImage,
						pTemp->nISPosition, pTemp->nFrame, pTemp->nType);

						if (pBitmap)
						{
							int nX = pTemp->oPosition.nX;
							int nY = pTemp->oPosition.nY;
							if (bSinglePlaneCoord == false)
								CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
	
							RECT rc;
							rc.left  = pTemp->oImgLTPos.nX;
							rc.top   = pTemp->oImgLTPos.nY;
							rc.right = pTemp->oImgRBPos.nX;
							rc.bottom= pTemp->oImgRBPos.nY;
							DrawBitmap16Part(nX, nY,
								pBitmap->GetWidth(), pBitmap->GetHeight(), pBitmap, rc);
						}
					}
					break;
				}
			}
		}
		break;
	case RU_T_POINT:
		{	
			DrawPoint(nPrimitiveCount, pPrimitives, bSinglePlaneCoord);
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
				DrawLine(nX1, nY1, nX2, nY2, D3DCOLOR_ARGB((DWORD)pTemp->Color.Color_b.a, (DWORD)pTemp->Color.Color_b.r, 
							(DWORD)pTemp->Color.Color_b.g, (DWORD)pTemp->Color.Color_b.b));
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
				DrawRectFrame(nX1, nY1, nX2, nY2, D3DCOLOR_ARGB(255, (DWORD)pTemp->Color.Color_b.r, 
							(DWORD)pTemp->Color.Color_b.g, (DWORD)pTemp->Color.Color_b.b));
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
					CoordinateTransform(nX2, nY2, pTemp->oEndPos.nZ);	// No nEndZ? Must be Single Plane Coord?
				}
				DrawRect(nX1, nY1, nX2 - nX1, nY2 - nY1, D3DCOLOR_ARGB(255 - (pTemp->Color.Color_b.a<<3), 
					(DWORD)pTemp->Color.Color_b.r, (DWORD)pTemp->Color.Color_b.g, (DWORD)pTemp->Color.Color_b.b));
			}
		}
		break;
	default:
		break;
	}
}

void KRepresentShell3::DrawImage2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord)
{
	// 判断是否主角类
	if(nPrimitiveCount >= 4)
	{
		DrawPlayer2D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord);
		return;
	}

	int i;
	KRUImage* pTemp = (KRUImage*)pPrimitives;

	for (i = 0; i < nPrimitiveCount; i++, pTemp++)
	{	
		switch(pTemp->nType)
		{
		case ISI_T_SPR:
			{
				TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
					pTemp->szImage,	pTemp->uImage,
					pTemp->nISPosition, pTemp->nFrame, pTemp->nType);
				if (!pSprite || pTemp->nFrame >= pSprite->m_nFrameNum)
					continue;

				int nX = pTemp->oPosition.nX;
				int nY = pTemp->oPosition.nY;
				if (!bSinglePlaneCoord)
					CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
				
				if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
				{
#define CENTERX		160
#define	CENTERY		192
					int nCenterX = pSprite->GetCenterX();
					int nCenterY = pSprite->GetCenterY();
					if (nCenterX || nCenterY)
					{
						nX -= nCenterX;
						nY -= nCenterY;
					}
					else if (pSprite->GetWidth() > CENTERX)
					{
						nX -= CENTERX;
						nY -= CENTERY;
					}
				}
				
				if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
				{
					nX += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX;
					nY += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY;
				}
				DrawSpriteAlpha(nX, nY, pSprite->m_pFrameInfo[pTemp->nFrame].nWidth, 
									pSprite->m_pFrameInfo[pTemp->nFrame].nHeight, 
									pTemp->nFrame, pSprite, pTemp->Color.Color_dw, pTemp->bRenderStyle);
			}
			break;
		case ISI_T_BITMAP16:
			{
				TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
					pTemp->szImage,	pTemp->uImage,
					pTemp->nISPosition, pTemp->nFrame, pTemp->nType);
				if (!pBitmap)
					break;
			
				int nX = pTemp->oPosition.nX;
				int nY = pTemp->oPosition.nY;
				if (!bSinglePlaneCoord)
					CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
				DrawBitmap16(nX, nY, pBitmap->GetWidth(), pBitmap->GetHeight(), pBitmap);
			}
			break;
		}
	}
}

void KRepresentShell3::DrawImage2DStretch(int nPrimitiveCount, KRepresentUnit* pPrimitives)
{
	int i;
	KRUImageStretch* pTemp = (KRUImageStretch*)pPrimitives;

	for (i = 0; i < nPrimitiveCount; i++, pTemp++)
	{	
		// 只处理ISI_T_BITMAP16类资源
		if(pTemp->nType != ISI_T_BITMAP16)
			break;

		TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
			pTemp->szImage,	pTemp->uImage,
			pTemp->nISPosition, pTemp->nFrame, pTemp->nType);
		if (!pBitmap)
			break;
	
		DrawBitmap16(pTemp->oPosition.nX, pTemp->oPosition.nY, pTemp->oEndPos.nX - pTemp->oPosition.nX, 
					pTemp->oEndPos.nY - pTemp->oPosition.nY, pBitmap, true);
	}
}

void KRepresentShell3::DrawPlayer2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord)
{
	RECT rcBound;
	RECTFLOAT rcBound1;

	// 取得主角所有部件的外包矩形
	GetBoundBox2D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord, rcBound);

	// 如果图素超出屏幕范围则不渲染
	if(rcBound.right < 0 || rcBound.left > g_nScreenWidth || rcBound.bottom < 0 || rcBound.top > g_nScreenHeight)
		return;

	rcBound1.left = (float)(rcBound.left);
	rcBound1.top = (float)(rcBound.top);
	rcBound1.right = (float)(rcBound.right);
	rcBound1.bottom = (float)(rcBound.bottom);

	// 绘制主角类
	if(rcBound.right - rcBound.left <= SPR_PRERENDER_TEXSIZE1 
		&& rcBound.bottom - rcBound.top <= SPR_PRERENDER_TEXSIZE1)
	{
		DrawSprOnTexture2D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord, rcBound, rcBound1, SPR_PRERENDER_TEXSIZE1, false);
		g_ntest++;
	}
	else if(rcBound.right - rcBound.left <= SPR_PRERENDER_TEXSIZE2 
		&& rcBound.bottom - rcBound.top <= SPR_PRERENDER_TEXSIZE2)
	{
		DrawSprOnTexture2D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord, rcBound, rcBound1, SPR_PRERENDER_TEXSIZE2, false);
		g_ntest++;
	}
}

void KRepresentShell3::GetBoundBox2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord, RECT &rcBound)
{
	int i;
	KRUImage* pTemp = (KRUImage*)pPrimitives;

	bool bFirstOne = true;
	// 所有图素的计算外包矩形
	for (i = 0; i < nPrimitiveCount; i++, pTemp++)
	{
		TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
			pTemp->szImage,	pTemp->uImage,
			pTemp->nISPosition, pTemp->nFrame, pTemp->nType, false);
		if (!pSprite || pTemp->nFrame >= pSprite->m_nFrameNum)
			continue;

		int nX = pTemp->oPosition.nX;
		int nY = pTemp->oPosition.nY;
		if (!bSinglePlaneCoord)
			CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
		
		if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
		{
#define CENTERX		160
#define	CENTERY		192
			int nCenterX = pSprite->GetCenterX();
			int nCenterY = pSprite->GetCenterY();
			if (nCenterX || nCenterY)
			{
				nX -= nCenterX;
				nY -= nCenterY;
			}
			else if (pSprite->GetWidth() > CENTERX)
			{
				nX -= CENTERX;
				nY -= CENTERY;
			}
		}
		
		if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
		{
			nX += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX;
			nY += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY;
		}

		if(bFirstOne)
		{
			bFirstOne = false;
			rcBound.left = nX, rcBound.top = nY;
			rcBound.right = nX + pSprite->m_pFrameInfo[pTemp->nFrame].nWidth;
			rcBound.bottom = nY + pSprite->m_pFrameInfo[pTemp->nFrame].nHeight;
		}
		else
		{
			if(nX < rcBound.left)
				rcBound.left = nX;
			if(nY < rcBound.top)
				rcBound.top = nY;
			if(nX + pSprite->m_pFrameInfo[pTemp->nFrame].nWidth > rcBound.right)
				rcBound.right = nX + (pSprite->m_pFrameInfo[pTemp->nFrame].nWidth);
			if(nY + pSprite->m_pFrameInfo[pTemp->nFrame].nHeight > rcBound.bottom)
				rcBound.bottom = nY + (pSprite->m_pFrameInfo[pTemp->nFrame].nHeight);
		}
	}
}

void KRepresentShell3::DrawSprOnTexture2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, 
						int bSinglePlaneCoord, RECT &rcBound, RECTFLOAT &rcRenderBound,int nTexSize, bool bLighting)
{
	int i;
	RECT rect;
	unsigned char cRenderStyle;
	KRUImage* pTemp = (KRUImage*)pPrimitives;
	uint32 nWidth = rcBound.right - rcBound.left;
	uint32 nHeight = rcBound.bottom - rcBound.top;
	unsigned int alpha = (((DWORD)pTemp->Color.Color_b.a) << 24);
	unsigned int color = 0x00404040 | alpha;
//	unsigned int color = 0xff404040;
	bool bLightGet = false;

	if(nWidth == 0 || nHeight == 0)
		return;

	D3DLOCKED_RECT LockedRect;
	rect.left = rect.top = 0;
	rect.right = nWidth;
	rect.bottom = nHeight;
	
	// 根据绘制区域不同大小锁定不同的贴图
	if(nTexSize == SPR_PRERENDER_TEXSIZE1)
	{
		if (FAILED(m_pPreRenderTexture128->LockRect(0, &LockedRect, &rect, 0)))
			return;
	}
	else
	{
		if (FAILED(m_pPreRenderTexture256->LockRect(0, &LockedRect, &rect, 0)))
			return;
	}

	// 清空贴图
	BYTE* p = (BYTE*)LockedRect.pBits;
	int nLen = (rect.right - rect.left) * 2;
	for(i = 0; i < rect.bottom - rect.top; i++)
	{
		memset(p, 0, nLen);
		p += LockedRect.Pitch;
	}

	for(i = 0; i < nPrimitiveCount; i++, pTemp++)
	{
		TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
			pTemp->szImage,	pTemp->uImage,
			pTemp->nISPosition, pTemp->nFrame, pTemp->nType, false);
		if (!pSprite || pTemp->nFrame >= pSprite->m_nFrameNum)
			continue;

		if(!bLightGet)
		{
			bLightGet = true;
			if(bLighting && m_bDoLighting)
			{
				D3DXVECTOR3 v;
				v.x = (float)(pTemp->oPosition.nX);
				v.y = (float)(pTemp->oPosition.nY);
				v.z = (float)(pTemp->oPosition.nZ);
				color = (GetPoint3dLighting(v) & 0x00ffffff) | alpha;
			}

			if(pTemp->bRenderStyle == IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST)
			{
				// 处理偏色
				color = ScaleColor(color, pTemp->Color.Color_b.r, pTemp->Color.Color_b.g, pTemp->Color.Color_b.b);
			}
		}

		cRenderStyle = pTemp->bRenderStyle;

		int nX = pTemp->oPosition.nX;
		int nY = pTemp->oPosition.nY;
		if (!bSinglePlaneCoord)
			CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
		
		if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
		{
#define CENTERX		160
#define	CENTERY		192
			int nCenterX = pSprite->GetCenterX();
			int nCenterY = pSprite->GetCenterY();
			if (nCenterX || nCenterY)
			{
				nX -= nCenterX;
				nY -= nCenterY;
			}
			else if (pSprite->GetWidth() > CENTERX)
			{
				nX -= CENTERX;
				nY -= CENTERY;
			}
		}
		
		if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
		{
			nX += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX;
			nY += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY;
		}

		rect.left = nX - rcBound.left, rect.top = nY - rcBound.top;
		rect.right = rect.left + pSprite->m_pFrameInfo[pTemp->nFrame].nWidth;
		rect.bottom = rect.top + pSprite->m_pFrameInfo[pTemp->nFrame].nHeight;

		assert(pSprite->m_pFrameInfo[pTemp->nFrame].pRawData);
		if(pSprite->m_pFrameInfo[pTemp->nFrame].pRawData)
		{
			// 将spr原始数据直接转换到A4R4G4B4的贴图上
			RenderToA4R4G4B4((WORD*)LockedRect.pBits, LockedRect.Pitch,
				pSprite->m_pFrameInfo[pTemp->nFrame].pRawData, rect, 
				pSprite->m_pFrameInfo[pTemp->nFrame].nWidth, 
				pSprite->m_pFrameInfo[pTemp->nFrame].nHeight,
				pSprite->m_pPal16);
		}
	}

	if(nTexSize == SPR_PRERENDER_TEXSIZE1)
		m_pPreRenderTexture128->UnlockRect(0);
	else
		m_pPreRenderTexture256->UnlockRect(0);

	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	m_pVB2D->Lock( 0, 4*sizeof(VERTEX2D), (void**)&pvb, 0 );

	float fX1, fY1, fX2, fY2;
	fX1 = rcRenderBound.left;
	fY1 = rcRenderBound.top;
	fX2 = rcRenderBound.right;
	fY2 = rcRenderBound.bottom;

	float fU2, fV2;
	if(nTexSize == SPR_PRERENDER_TEXSIZE1)
	{
		fU2 = (float)nWidth / (float)SPR_PRERENDER_TEXSIZE1;
		fV2 = (float)nHeight / (float)SPR_PRERENDER_TEXSIZE1;

		PD3DDEVICE->SetTexture( 0, m_pPreRenderTexture128 );
	}
	else
	{
		fU2 = (float)nWidth / (float)SPR_PRERENDER_TEXSIZE2;
		fV2 = (float)nHeight / (float)SPR_PRERENDER_TEXSIZE2;

		PD3DDEVICE->SetTexture( 0, m_pPreRenderTexture256 );
	}

	float ft1,ft2;
	ft1 = 0.5f / (float)SPR_PRERENDER_TEXSIZE2;
	ft2 = 0.5f / (float)SPR_PRERENDER_TEXSIZE2;

	pvb[0].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[0].color    = color;
    pvb[0].tu       = 0.0f + ft1;
    pvb[0].tv       = 0.0f + ft2;

	pvb[1].position = D3DXVECTOR4( fX2,fY1, 100, 1 );
    pvb[1].color    = color;
    pvb[1].tu       = fU2 - ft1;
    pvb[1].tv       = 0.0f + ft2;

	pvb[2].position = D3DXVECTOR4( fX1,fY2, 100, 1 );
    pvb[2].color    = color;
    pvb[2].tu       = 0.0f + ft1;
    pvb[2].tv       = fV2 - ft2;

	pvb[3].position = D3DXVECTOR4( fX2,fY2, 100, 1 );
    pvb[3].color    = color;
    pvb[3].tu       = fU2 - ft1;
    pvb[3].tv       = fV2 - ft2;

	m_pVB2D->Unlock();

	if(cRenderStyle == IMAGE_RENDER_STYLE_BORDER)
		PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD );
	else
		PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
}

// 为和2D模式大小一致，特定的spr缩放一个比例
#define SCALE_RATE_SPRITE_WIDTH	 1.05f
#define SCALE_RATE_SPRITE_HEIGHT 1.12f

void KRepresentShell3::DrawImage3D(unsigned int uGenre, int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord)
{
	if(uGenre != RU_T_IMAGE && uGenre != RU_T_IMAGE_4)
		return;

	// 判断是否主角类
	if(nPrimitiveCount >= 4)
	{
		DrawPlayer3D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord);
		return;
	}

	int i;
	RenderParam3D renderParam;
	KRUImage* pTemp = (KRUImage*)pPrimitives;

	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB3D, 0, sizeof(VERTEX3D) )))
		return;

	PD3DDEVICE->SetFVF( D3DFVF_VERTEX3D );
	// 由于是透视模式，将过滤方式设为D3DTEXF_LINEAR防止贴图抖动
	if(g_renderModel == RenderModel3DPerspective)
	{
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}

	for (i = 0; i < nPrimitiveCount; i++, pTemp++)
	{
		float fX1, fY1, fZ1, fX2, fY2, fZ2, fX3, fY3, fZ3, fX4, fY4, fZ4;
		fX1 = (float)pTemp->oPosition.nX;
		fY1 = (float)pTemp->oPosition.nY;
		fZ1 = (float)pTemp->oPosition.nZ;
		fX3 = (float)pTemp->oEndPos.nX;
		fY3 = (float)pTemp->oEndPos.nY;
		fZ3 = (float)pTemp->oEndPos.nZ;

		switch(pTemp->nType)
		{
		case ISI_T_SPR:
			{
				TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
					pTemp->szImage,	pTemp->uImage,
					pTemp->nISPosition, pTemp->nFrame, pTemp->nType);

				if (!pSprite || pTemp->nFrame >= pSprite->m_nFrameNum)
					break;

				if(!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT) && (pTemp->oEndPos.nX == 0 || pTemp->oEndPos.nY == 0))
					break;

				if(uGenre != RU_T_IMAGE_4)
				{
					// 矩形图素
					if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
					{
#define CENTERX		160
#define	CENTERY		192
						int nCenterX = pSprite->GetCenterX();
						int nCenterY = pSprite->GetCenterY();
						if (nCenterX || nCenterY)
						{
							fX1 -= nCenterX * SCALE_RATE_SPRITE_WIDTH;
							fZ1 += nCenterY * SCALE_RATE_SPRITE_HEIGHT;
						}
						else if (pSprite->GetWidth() > CENTERX)
						{
							fX1 -= CENTERX * SCALE_RATE_SPRITE_WIDTH;
							fZ1 += CENTERY * SCALE_RATE_SPRITE_HEIGHT;
						}
						fX3 = fX1 + pSprite->m_pFrameInfo[pTemp->nFrame].nWidth * SCALE_RATE_SPRITE_WIDTH;
						fZ3 = fZ1 - pSprite->m_pFrameInfo[pTemp->nFrame].nHeight * SCALE_RATE_SPRITE_HEIGHT;
						fY3 = fY1;
					}
			
					float fXOff = 0;
					float fYOff = 0;
					if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
					{
						fXOff += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX * SCALE_RATE_SPRITE_WIDTH;
						fYOff += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY * SCALE_RATE_SPRITE_HEIGHT;
						if(fZ1 == fZ3)
						{
							fX1 += fXOff;
							fX3 += fXOff;
							fY1 += fYOff * 2;
							fY3 += fYOff * 2;
						}
						else
						{
							float fW,fH,fM;
							fW = (float)(fabs(fX3 - fX1));
							fH = (float)(fabs(fY3 - fY1));
							fM = (float)(sqrt(fW * fW + fH * fH));
							fW = fXOff * fW / fM;
							fH = fXOff * fH / fM;
							fX1 += fW;
							fX3 += fW;
							fY1 += fH * 2;
							fY3 += fH * 2;
							fZ1 -= fYOff;
							fZ3 -= fYOff;
						}
					}
					
					if(fZ1 == fZ3)
					{
						// 水平图素
						renderParam.m_pos[0] = D3DXVECTOR3( fX1,fY1, fZ1 );
						renderParam.m_pos[1] = D3DXVECTOR3( fX3,fY1, fZ1 );
						renderParam.m_pos[2] = D3DXVECTOR3( fX3,fY3, fZ3 );
						renderParam.m_pos[3] = D3DXVECTOR3( fX1,fY3, fZ3 );
					}
					else
					{
						// 垂直图素
						renderParam.m_pos[0] = D3DXVECTOR3( fX1,fY1, fZ1 );
						renderParam.m_pos[1] = D3DXVECTOR3( fX3,fY3, fZ1 );
						renderParam.m_pos[2] = D3DXVECTOR3( fX3,fY3, fZ3 );
						renderParam.m_pos[3] = D3DXVECTOR3( fX1,fY1, fZ3 );
					}

					if(m_bDoLighting && pTemp->bRenderStyle != IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT)
						DrawSpriteAlpha3DLighting(renderParam, pTemp->nFrame, pSprite,
													pTemp->Color.Color_dw, pTemp->bRenderStyle, NULL);
					else
						DrawSpriteAlpha3D(renderParam, pTemp->nFrame, pSprite, pTemp->Color.Color_dw, 
													pTemp->bRenderStyle, NULL);
				}
				else
				{
					// 平行四边形图素
					KRUImage4 *pTemp4 = (KRUImage4*)pTemp;
					fX2 = (float)pTemp4->oSecondPos.nX;
					fY2 = (float)pTemp4->oSecondPos.nY;
					fZ2 = (float)pTemp4->oSecondPos.nZ;
					fX4 = (float)pTemp4->oThirdPos.nX;
					fY4 = (float)pTemp4->oThirdPos.nY;
					fZ4 = (float)pTemp4->oThirdPos.nZ;
					
					renderParam.m_pos[0] = D3DXVECTOR3( fX1,fY1, fZ1 );
					renderParam.m_pos[1] = D3DXVECTOR3( fX2,fY2, fZ2 );
					renderParam.m_pos[2] = D3DXVECTOR3( fX3,fY3, fZ3 );
					renderParam.m_pos[3] = D3DXVECTOR3( fX4,fY4, fZ4 );

					// Clipper
					RECT rc;
					rc.left  = pTemp4->oImgLTPos.nX;
					rc.top   = pTemp4->oImgLTPos.nY;
					rc.right = pTemp4->oImgRBPos.nX;
					rc.bottom= pTemp4->oImgRBPos.nY;

					if(m_bDoLighting && pTemp->bRenderStyle != IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT)
						DrawSpriteAlpha3DLighting(renderParam, pTemp->nFrame, pSprite, 
												pTemp->Color.Color_dw, pTemp->bRenderStyle, &rc);
					else
						DrawSpriteAlpha3D(renderParam, pTemp->nFrame, pSprite, pTemp->Color.Color_dw, pTemp->bRenderStyle, &rc);
				}
			}
			break;
		case ISI_T_BITMAP16:
			{
				TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
					pTemp->szImage,	pTemp->uImage,
					pTemp->nISPosition, pTemp->nFrame, pTemp->nType);
				if (!pBitmap)
					break;
				if(fZ1 == fZ3)
				{
					// 水平图素
					renderParam.m_pos[0] = D3DXVECTOR3( fX1,fY1, fZ1 );
					renderParam.m_pos[1] = D3DXVECTOR3( fX3,fY1, fZ1 );
					renderParam.m_pos[2] = D3DXVECTOR3( fX3,fY3, fZ3 );
					renderParam.m_pos[3] = D3DXVECTOR3( fX1,fY3, fZ3 );
				}
				else
				{
					// 垂直图素
					renderParam.m_pos[0] = D3DXVECTOR3( fX1,fY1, fZ1 );
					renderParam.m_pos[1] = D3DXVECTOR3( fX3,fY3, fZ1 );
					renderParam.m_pos[2] = D3DXVECTOR3( fX3,fY3, fZ3 );
					renderParam.m_pos[3] = D3DXVECTOR3( fX1,fY1, fZ3 );
				}
				if(m_bDoLighting)
					DrawBitmap163DLighting(renderParam, pBitmap);
				else
					DrawBitmap163D(renderParam, pBitmap);
			}
			break;
		}
	}
	// 恢复缺省过滤模式
	if(g_renderModel == RenderModel3DPerspective)
	{
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}
}

void KRepresentShell3::DrawPlayer3D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord)
{
	RECTFLOAT rcBound3D;
	RECT rcBound2D;

	// 计算非透视模式和透视模式的外包矩形
	GetBoundBox2D(nPrimitiveCount, pPrimitives, bSinglePlaneCoord, rcBound2D);
	GetBoundBox3D(nPrimitiveCount, pPrimitives, rcBound3D);

	// 如果图素超出屏幕范围则不渲染
	if(rcBound3D.right < 0 || rcBound3D.left > g_nScreenWidth || rcBound3D.bottom < 0 || rcBound3D.top > g_nScreenHeight)
		return;

	if(rcBound3D.left - rcBound3D.right > 260 || rcBound3D.right - rcBound3D.left > 260)
		return;

	// 绘制主角类
	if(rcBound2D.right - rcBound2D.left <= SPR_PRERENDER_TEXSIZE1 
		&& rcBound2D.bottom - rcBound2D.top <= SPR_PRERENDER_TEXSIZE1)
	{
		DrawSprOnTexture2D(nPrimitiveCount, pPrimitives, false, rcBound2D, rcBound3D, SPR_PRERENDER_TEXSIZE1, true);
		g_ntest++;
	}
	else if(rcBound2D.right - rcBound2D.left <= SPR_PRERENDER_TEXSIZE2 
		&& rcBound2D.bottom - rcBound2D.top <= SPR_PRERENDER_TEXSIZE2)
	{
		DrawSprOnTexture2D(nPrimitiveCount, pPrimitives, false, rcBound2D, rcBound3D, SPR_PRERENDER_TEXSIZE2, true);
		g_ntest++;
	}
}

void KRepresentShell3::GetBoundBox3D(int nPrimitiveCount, KRepresentUnit* pPrimitives, RECTFLOAT &rcBound)
{
	int i;
	KRUImage* pTemp = (KRUImage*)pPrimitives;

	bool bFirstOne = true;
	// 所有图素的计算外包矩形
	for (i = 0; i < nPrimitiveCount; i++, pTemp++)
	{
		D3DXVECTOR3 v1,v2;
		v1.x = (float)pTemp->oPosition.nX;
		v1.y = (float)pTemp->oPosition.nY;
		v1.z = (float)pTemp->oPosition.nZ;
		v2.x = (float)pTemp->oEndPos.nX;
		v2.y = (float)pTemp->oEndPos.nY;
		v2.z = (float)pTemp->oEndPos.nZ;

		TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
			pTemp->szImage,	pTemp->uImage,
			pTemp->nISPosition, pTemp->nFrame, pTemp->nType, false);
		if (!pSprite || pTemp->nFrame >= pSprite->m_nFrameNum)
			continue;

		if(!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT) && (pTemp->oEndPos.nX == 0 || pTemp->oEndPos.nY == 0))
			continue;

		// 矩形图素
		if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
		{
#define CENTERX		160
#define	CENTERY		192
			int nCenterX = pSprite->GetCenterX();
			int nCenterY = pSprite->GetCenterY();
			if (nCenterX || nCenterY)
			{
				v1.x -= nCenterX * SCALE_RATE_SPRITE_WIDTH;
				v1.z += nCenterY * SCALE_RATE_SPRITE_HEIGHT;
			}
			else if (pSprite->GetWidth() > CENTERX)
			{
				v1.x -= CENTERX * SCALE_RATE_SPRITE_WIDTH;
				v1.z += CENTERY * SCALE_RATE_SPRITE_HEIGHT;
			}
			v2.x = v1.x + pSprite->m_pFrameInfo[pTemp->nFrame].nWidth * SCALE_RATE_SPRITE_WIDTH;
			v2.z = v1.z - pSprite->m_pFrameInfo[pTemp->nFrame].nHeight * SCALE_RATE_SPRITE_HEIGHT;
			v2.y = v1.y;
		}
			
		float fXOff = 0;
		float fYOff = 0;
		if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
		{
			fXOff += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX * SCALE_RATE_SPRITE_WIDTH;
			fYOff += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY * SCALE_RATE_SPRITE_HEIGHT;

			float fW,fH,fM;
			fW = (float)(fabs(v2.x - v1.x));
			fH = (float)(fabs(v2.y - v1.y));
			fM = (float)(sqrt(fW * fW + fH * fH));
			fW = fXOff * fW / fM;
			fH = fXOff * fH / fM;
			v1.x += fW;
			v2.x += fW;
			v1.y += fH * 2;
			v2.y += fH * 2;
			v1.z -= fYOff;
			v2.z -= fYOff;
		}

		// 将坐标转化到屏幕空间
		D3DXVECTOR3 vPos1, vPos2;
		D3DVIEWPORT9 viewportData = g_Device.GetViewport();
		
		D3DXVec3Project(&vPos1, &v1, &viewportData, &m_matProj, &m_matView, NULL);
		D3DXVec3Project(&vPos2, &v2, &viewportData, &m_matProj, &m_matView, NULL);

		if(bFirstOne)
		{
			bFirstOne = false;
			rcBound.left = vPos1.x, rcBound.top = vPos1.y;
			rcBound.right = vPos2.x;
			rcBound.bottom = vPos2.y;
		}
		else
		{
			if(vPos1.x < rcBound.left)
				rcBound.left = vPos1.x;
			if(vPos1.y < rcBound.top)
				rcBound.top = vPos1.y;
			if(vPos2.x > rcBound.right)
				rcBound.right = vPos2.x;
			if(vPos2.y > rcBound.bottom)
				rcBound.bottom = vPos2.y;
		}
	}
}

void KRepresentShell3::DrawPrimitivesOnImage(int nPrimitiveCount, KRepresentUnit* pPrimitives, 
        unsigned int uGenre, const char* pszImage, unsigned int uImage, short &nImagePosition)
{
	if(!pPrimitives)
	{
		assert(pPrimitives);
		return;
	}

	if(!pszImage || !pszImage[0])
		return;

	if(m_bDeviceLost)
		return;

	TextureResBmp* pDestBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
		pszImage, uImage, nImagePosition, 0, ISI_T_BITMAP16);
	if (pDestBitmap == NULL)
		return;

	if(!pDestBitmap->m_FrameInfo.texInfo[0].pTexture)
	{
		assert(pDestBitmap->m_FrameInfo.texInfo[0].pTexture);
		return;
	}

	IDirect3DSurface9 *pDesSurface, *pOldSurface;
	if (FAILED(PD3DDEVICE->GetRenderTarget( 0, &pOldSurface )))
		return;

	if (FAILED(pDestBitmap->m_FrameInfo.texInfo[0].pTexture->GetSurfaceLevel( 0, &pDesSurface)))
	{
		pOldSurface->Release();
		return;
	}

	if (FAILED(PD3DDEVICE->SetRenderTarget( 0, pDesSurface)))
	{
		pDesSurface->Release();
		pOldSurface->Release();
		return;
	}

	PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) );
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	int i;
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
					TextureResSpr* pSprite = (TextureResSpr *)m_TextureResMgr.GetImage(
						pTemp->szImage, pTemp->uImage,
						pTemp->nISPosition, pTemp->nFrame, pTemp->nType);
					if (pSprite == NULL || pTemp->nFrame >= pSprite->m_nFrameNum)
						break;

					int nX = pTemp->oPosition.nX;
					int nY = pTemp->oPosition.nY;

					if (!(pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_FRAME_DRAW))
					{
						nX += pSprite->m_pFrameInfo[pTemp->nFrame].nOffX;
						nY += pSprite->m_pFrameInfo[pTemp->nFrame].nOffY;
						if (pTemp->bRenderFlag & RUIMAGE_RENDER_FLAG_REF_SPOT)
						{
							nX -= pSprite->GetCenterX();
							nY -= pSprite->GetCenterY();
						}
					}

					switch(pTemp->bRenderStyle)
					{
					case IMAGE_RENDER_STYLE_ALPHA:
					case IMAGE_RENDER_STYLE_3LEVEL:
					case IMAGE_RENDER_STYLE_OPACITY:
					case IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT:
						RIO_CopySprToBufferAlpha(pSprite, pTemp->nFrame, pDestBitmap, nX, nY);
						break;
					}						
				}
				break;
			case ISI_T_BITMAP16:
				{
					TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
						pTemp->szImage, pTemp->uImage, pTemp->nISPosition,
							pTemp->nFrame, pTemp->nType);
						if (!pBitmap)
							break;
						RIO_CopyBitmap16ToBuffer(pBitmap, pDestBitmap, pTemp->oPosition.nX, pTemp->oPosition.nY);
				}
				break;
			}
		}
		break;
	}

	PD3DDEVICE->SetRenderTarget( 0, pOldSurface );
	pDesSurface->Release();
	pOldSurface->Release();
}

//## 清除图形数据
void KRepresentShell3::ClearImageData(const char* pszImage, unsigned int uImage, short nImagePosition)
{
	if(!pszImage || !pszImage[0])
		return;

	if(m_bDeviceLost)
		return;

	TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
		pszImage, uImage, nImagePosition, 0, ISI_T_BITMAP16);
	if (pBitmap)
	{
		byte* pBuffer;
		int32 nPitch;
		if( pBitmap->LockData((void**)(&pBuffer), nPitch) )
		{
			for(int i=0; i<pBitmap->GetHeight(); i++)
			{
				memset(pBuffer, 0, nPitch);
				pBuffer += nPitch;
			}
			pBitmap->UnLockData();
		}
	}
}


void KRepresentShell3::FreeAllImage()
{
	m_TextureResMgr.Free();
}

void KRepresentShell3::FreeImage(const char* pszImage)
{
	if(!pszImage || !pszImage[0])
		return;

	m_TextureResMgr.FreeImage(pszImage);
}

void* KRepresentShell3::GetBitmapDataBuffer(const char* pszImage, KBitmapDataBuffInfo* pInfo)
{
	if(!pszImage || !pszImage[0])
		return NULL;

	if(m_bDeviceLost)
		return NULL;

	short nImagePosition;
	unsigned int uImage = 0;
	TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
							pszImage, uImage, nImagePosition, 0, ISI_T_BITMAP16);
	if (!pBitmap)
		return NULL;

	void *pData;
	int32 nPitch;
	if(!pBitmap->LockData(&pData, nPitch))
		return NULL;

	if(pInfo)
	{
		pInfo->nWidth = pBitmap->GetWidth();
		pInfo->nHeight = pBitmap->GetHeight();
		pInfo->nPitch = nPitch;
		pInfo->pData = pData;
		if(g_16BitFormat == D3DFMT_R5G6B5)
			pInfo->eFormat = BDBF_16BIT_565;
		else
			pInfo->eFormat = BDBF_16BIT_555;
	}

	return pData;
}

void KRepresentShell3::ReleaseBitmapDataBuffer(const char* pszImage, void* pBuffer)
{
	if(!pszImage || !pszImage[0])
		return;

	if(m_bDeviceLost)
		return;

	short nImagePosition;
	unsigned int uImage = 0;
	TextureResBmp* pBitmap = (TextureResBmp *)m_TextureResMgr.GetImage(
							pszImage, uImage, nImagePosition, 0, ISI_T_BITMAP16);
	if (!pBitmap)
		return;

	pBitmap->UnLockData();
}

bool KRepresentShell3::GetImageParam(const char* pszImage, KImageParam* pImageData, int nType)
{
	if(!pImageData || !pszImage || !pszImage[0])
		return false;

	return m_TextureResMgr.GetImageParam(pszImage, pImageData, nType);
}

bool KRepresentShell3::GetImageFrameParam( const char* pszImage, int nFrame,
										  KRPosition2* pOffset, KRPosition2* pSize, int nType)
{
	if(!pOffset || !pszImage || !pszImage[0])
		return false;

	return m_TextureResMgr.GetImageFrameParam(pszImage, nFrame, pOffset, pSize, nType);
}

int KRepresentShell3::GetImagePixelAlpha(const char* pszImage, int nFrame, int nX, int nY, int nType)
{
	if(!pszImage || !pszImage[0])
		return 0;

	if(m_bDeviceLost)
		return 0;

	return m_TextureResMgr.GetImagePixelAlpha(pszImage, nFrame, nX, nY, nType);
}

void KRepresentShell3::LookAt(int nX, int nY, int nZ)
{
	if(m_bDeviceLost)
		return;

	m_nLeft = nX - g_nScreenWidth / 2;
	m_nTop  = nY / 2 - ((nZ * 887) >> 10) - g_nScreenHeight / 2;

	if(g_renderModel == RenderModel3DOrtho || g_renderModel == RenderModel3DPerspective)
	{
		float fX = (float)nX;
		float fY = (float)nY;
		float fZ = (float)nZ;
		// 摄像机后退，抬高成30度角
		m_vCamera1.x = fX;
		m_vCamera1.y = fY;
		m_vCamera1.z = fZ;
		m_vCamera.x = fX;
		m_vCamera.y = fY + 4000.0f;
		m_vCamera.z = (float)(fZ + 4000.0f*0.58);
		D3DXMatrixLookAtLH( &m_matView, &m_vCamera,
							&D3DXVECTOR3( fX, fY, fZ ), &D3DXVECTOR3( 0, 0, 1 ) );
		PD3DDEVICE->SetTransform( D3DTS_VIEW,  &m_matView );
		D3DXMatrixInverse( &m_matViewInverse, NULL, &m_matView );
	}
}

void KRepresentShell3::OutputText(int nFontId, const char* psText, int nCount, int nX, 
								  int nY, unsigned int Color, int nLineWidth, int nZ, unsigned int BorderColor)
{
	if(!psText || !psText[0])
		return;

	if(m_bDeviceLost)
		return;
	
	Color |= 0xff000000;
	int i;
	for ( i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nFontId)
			break;
	}
	
	if (i == RS2_MAX_FONT_ITEM_NUM)
		return;

	if (!m_FontTable[i].pFontObj)
		return;

	if(nZ != TEXT_IN_SINGLE_PLANE_COORD)
	{
		// 将3D坐标转化为屏幕坐标
		if(m_dwWindowStyle == RenderModel3DPerspective)
		{
			D3DXVECTOR3 vPos((float)(nX), (float)(nY), (float)(nZ));
			D3DVIEWPORT9 viewportData = g_Device.GetViewport();
			D3DXVec3Project(&vPos, &vPos, &viewportData, &m_matProj, &m_matView, NULL);
			nX = (int)vPos.x, nY = (int)vPos.y;
		}
		else
			CoordinateTransform(nX, nY, nZ);
	}
	
	m_FontTable[i].pFontObj->SetBorderColor(BorderColor);
	m_FontTable[i].pFontObj->SetOutputSize(nFontId, nFontId + 1);
	m_FontTable[i].pFontObj->OutputText(psText, nCount, nX, nY, Color, nLineWidth);
}

//## 输出文字。
int KRepresentShell3::OutputRichText(int nFontId, KOutputTextParam* pParam, 
		const char* psText, int nCount, int nLineWidth)
{
	if(!pParam || !psText || !psText[0])
		return 0;

	if(m_bDeviceLost)
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
		if(pParam->nZ != TEXT_IN_SINGLE_PLANE_COORD)
		{
			int x, y, z;
			x = pParam->nX;
			y = pParam->nY;
			z = pParam->nZ;

			// 将3D坐标转化为屏幕坐标
			if(m_dwWindowStyle == RenderModel3DPerspective)
			{
				D3DXVECTOR3 vPos((float)x, (float)y, (float)z);
				D3DVIEWPORT9 viewportData = g_Device.GetViewport();
				D3DXVec3Project(&vPos, &vPos, &viewportData, &m_matProj, &m_matView, NULL);
				x = (int)vPos.x, y = (int)vPos.y;
			}
			else
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

int KRepresentShell3::LocateRichText(int nX, int nY,
					int nFontId, KOutputTextParam* pParam, 
					const char* psText, int nCount, int nLineWidth)
{
	if(!pParam || !psText || !psText[0])
		return -1;

	if(m_bDeviceLost)
		return -1;
	int i;
	for ( i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nFontId)
			break;
	}

	if (i < RS2_MAX_FONT_ITEM_NUM && m_FontTable[i].pFontObj)
	{
		KTextProcess	tp(psText, nCount, nLineWidth * 2 / nFontId);
		if(pParam->nZ != TEXT_IN_SINGLE_PLANE_COORD)
		{
			int x, y, z;
			x = pParam->nX;
			y = pParam->nY;
			z = pParam->nZ;

			// 将3D坐标转化为屏幕坐标
			if(m_dwWindowStyle == RenderModel3DPerspective)
			{
				D3DXVECTOR3 vPos((float)x, (float)y, (float)z);
				D3DVIEWPORT9 viewportData = g_Device.GetViewport();
				D3DXVec3Project(&vPos, &vPos, &viewportData, &m_matProj, &m_matView, NULL);
				x = (int)vPos.x, y = (int)vPos.y;
			}
			else
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

void KRepresentShell3::ReleaseAFont(int nId)
{
	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId == nId)
		{
			if (m_FontTable[i].pFontObj)
			{
				m_FontTable[i].pFontObj->Release();
				m_FontTable[i].pFontObj = NULL;
				m_FontTable[i].nId = 0;
				break;
			}
		}
	}
}

bool KRepresentShell3::SaveImage(const char* pszFile, const char* pszImage, int nFileType)
{
	if(!pszFile || !pszFile[0] || !pszImage || !pszImage[0])
		return false;

	if(m_bDeviceLost)
		return false;
	
	return true;
}

void KRepresentShell3::SetImageStoreBalanceParam(int nNumImage, unsigned int uCheckPoint)
{
	m_TextureResMgr.SetBalanceParam(nNumImage, uCheckPoint);
}

bool KRepresentShell3::CopyDeviceImageToImage(const char* pszName, int nDeviceX, int nDeviceY, int nImageX, int nImageY, int nWidth, int nHeight)
{
	if(!pszName || !pszName[0])
		return 0;

	if(m_bDeviceLost)
		return false;

	return true;
}

bool KRepresentShell3::RepresentBegin(int bClear, unsigned int Color)
{
	HRESULT hr;
	g_ntest = 0;
    // Test the cooperative level to see if it's okay to render
    if( FAILED( hr = PD3DDEVICE->TestCooperativeLevel() ) )
    {
		m_bDeviceLost = true;
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
        {
			return false;
		}

        // Check if the device needs to be resized.
        if( D3DERR_DEVICENOTRESET == hr )
        {
            if( !InvalidateDeviceObjects() )
				return false;
			if( FAILED( g_Device.ResetDevice()) )
				return false;
			if( !RestoreDeviceObjects() )
				return false;
			m_bDeviceLost = false;
        }
		return false;
    }

	// 清除背景
	PD3DDEVICE->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );

	// 开始性能统计
	m_TextureResMgr.StartProfile();

/*	float fAngleAdd = 0.03f;
	static float fAngel = 0.0f;
	fAngel += fAngleAdd;
	if(fAngel > 6.28f)
		fAngel = 0.0f;
	float fXAdd, fYAdd;
	fXAdd = sin(fAngel) * 50.0f;
	fYAdd = cos(fAngel) * 50.0f;

	static int c1 = 0x40, c2 = 0x60, c3 = 0x90;
	static int c1Add = 0x01, c2Add = 0x02, c3Add = 0x03;
	c1 += c1Add, c2 += c2Add, c3+= c3Add;
	if(c1 > 0xff)
		c1 = 0xff, c1Add = -0x01;
	if(c2 > 0xff)
		c2 = 0xff, c2Add = -0x02;
	if(c3 > 0xff)
		c3 = 0xff, c3Add = -0x04;
	if(c1 < 0)
		c1 = 0, c1Add = 0x01;
	if(c2 < 0)
		c2 = 0, c2Add = 0x01;
	if(c3 < 0)
		c3 = 0, c3Add = 0x01;

	static unsigned int dwTp[LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT];
	float fSenterX = LIGHTING_GRID_WIDTH * 16;
	float fSenterY = LIGHTING_GRID_HEIGHT * 16;
	float fx,fy,fd;
	for(int i=0; i<LIGHTING_GRID_HEIGHT; i++)
		for(int j=0; j<LIGHTING_GRID_WIDTH; j++)
		{
			fx = j * 32 - (fSenterX + fXAdd);
			fy = i * 32 - (fSenterY + fYAdd);
			fd = (float)(sqrt(fx * fx + fy * fy) / fSenterX);
			if(fd > 1.0f)
				fd = 1.0f;
			unsigned char c11,c22,c33;
			c11 = (1.0f - fd) * c1;
			c22 = (1.0f - fd) * c2;
			c33 = (1.0f - fd) * c3;
			dwTp[i * LIGHTING_GRID_WIDTH + j] = 0xff000000 | (c11<<16) | (c22<<8) | c33;
		}

//	SetLightInfo(112640, 197632, dwTp);
//	SetLightInfo(111104 + fXAdd, 195584 - 400 + fYAdd, dwTp);
*/
	if(FAILED(g_Device.Start3D()))
		return false;

	return true;
}

void KRepresentShell3::RepresentEnd()
{
	if(m_bDeviceLost)
		return;

	char cc[200];

	// 终止性能统计
	m_TextureResMgr.EndProfile();

	// 生成并显示统计信息
	m_TextureResMgr.GetProfileString(cc, 200);
	KOutputTextParam param;
	param.Color = 0xffff;
	param.nNumLine = 4;
	param.nX = 0;
	param.nY = 30;

	int nFontId;

	for (int i = 0; i < RS2_MAX_FONT_ITEM_NUM; i++)
	{
		if (m_FontTable[i].nId != 0)
		{
			nFontId = m_FontTable[i].nId;
			break;
		}
	}

//	OutputText(nFontId, cc, strlen(cc), 600, 20, 0xffffffff, 800);

	// 完成3D渲染
	g_Device.End3D();
	// 交换页面
	PD3DDEVICE->Present(NULL,NULL,NULL,NULL);
}

void KRepresentShell3::ViewPortCoordToSpaceCoord( int& nX, int& nY, int  nZ )
{
	if(g_renderModel == RenderModel3DPerspective)
	{
		D3DVIEWPORT9 viewportData = g_Device.GetViewport();
		D3DXVECTOR3 vPos((2.0f * (float)nX / viewportData.Width - 1.0f) / m_matProj._11, 
					(1.0f - 2.0f * (float)nY / viewportData.Height) / m_matProj._22, 1.0f);
		D3DXVECTOR3 vOut1, vOut2;
		
		// 鼠标确定的选择射线的方向向量
		vOut2.x  = vPos.x*m_matViewInverse._11 + vPos.y*m_matViewInverse._21 + vPos.z*m_matViewInverse._31;
        vOut2.y  = vPos.x*m_matViewInverse._12 + vPos.y*m_matViewInverse._22 + vPos.z*m_matViewInverse._32;
        vOut2.z  = vPos.x*m_matViewInverse._13 + vPos.y*m_matViewInverse._23 + vPos.z*m_matViewInverse._33;

		// 摄像机位置
		vOut1.x = m_matViewInverse._41;
        vOut1.y = m_matViewInverse._42;
        vOut1.z = m_matViewInverse._43;

		vOut1 = vOut1 - vOut2 * ((vOut1.z - nZ) / vOut2.z);

		nX = (int)vOut1.x;
		nY = (int)vOut1.y;
	}
	else
	{
		nX = nX + m_nLeft;
		nY = (nY + m_nTop + ((nZ * 887) >> 10)) * 2;
	}
}

void KRepresentShell3::D3DTerm()
{
	g_Device.ReleaseDevObjects();

	PD3DDEVICE->SetGammaRamp(0, D3DSGR_CALIBRATE, &m_ramp);
	g_Device.FreeAll(); 					// Free the Device...
	g_D3DShell.FreeAll();					// Free the Shell...
}

void KRepresentShell3::CoordinateTransform( int& nX, int& nY, int nZ)
{
	nX = nX - m_nLeft;
	nY = nY / 2 - m_nTop - ((nZ * 887) >> 10);
}

void KRepresentShell3::DrawRect(int32 x1, int32 y1, int32 nWidth, int32 nHeight, DWORD color)
{
	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	m_pVB2D->Lock( 0, 4*sizeof(VERTEX2D), (void**)&pvb, 0 );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)x1;
	fY1 = (float)y1;
	fX2 = fX1 + (float)nWidth;
	fY2 = fY1 + (float)nHeight;

	pvb[0].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[0].color    = color;

	pvb[1].position = D3DXVECTOR4( fX2,fY1, 100, 1 );
    pvb[1].color    = color;

	pvb[2].position = D3DXVECTOR4( fX1,fY2, 100, 1 );
    pvb[2].color    = color;

	pvb[3].position = D3DXVECTOR4( fX2,fY2, 100, 1 );
    pvb[3].color    = color;

	m_pVB2D->Unlock();

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
}

void KRepresentShell3::DrawPoint(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord)
{
	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	if(FAILED(m_pVB2D->Lock( 0, nPrimitiveCount * sizeof(VERTEX2D), (void**)&pvb, 0 )))
		return;

	KRUPoint* pTemp = (KRUPoint *)pPrimitives;
	for (int i = 0; i < nPrimitiveCount; i++, pTemp++)
	{				
		int nX = pTemp->oPosition.nX;
		int nY = pTemp->oPosition.nY;
		if (!bSinglePlaneCoord)
			CoordinateTransform(nX, nY, pTemp->oPosition.nZ);
		DWORD color = D3DCOLOR_ARGB(255, (DWORD)pTemp->Color.Color_b.r, 
					(DWORD)pTemp->Color.Color_b.g, (DWORD)pTemp->Color.Color_b.b);

		pvb[i].position = D3DXVECTOR4( (float)nX, (float)nY, 100, 1 );
		pvb[i].color    = color;
	}

	m_pVB2D->Unlock();

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->DrawPrimitive( D3DPT_POINTLIST, 0, nPrimitiveCount );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
}

void KRepresentShell3::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2, DWORD color)
{
	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	m_pVB2D->Lock( 0, 2*sizeof(VERTEX2D), (void**)&pvb, 0 );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)x1;
	fY1 = (float)y1;
	fX2 = (float)x2;
	fY2 = (float)y2;

	pvb[0].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[0].color    = color;

	pvb[1].position = D3DXVECTOR4( fX2,fY2, 100, 1 );
    pvb[1].color    = color;

	m_pVB2D->Unlock();

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->DrawPrimitive( D3DPT_LINELIST, 0, 1 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
}

void KRepresentShell3::DrawRectFrame(int32 x1, int32 y1, int32 x2, int32 y2, DWORD color)
{
	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	m_pVB2D->Lock( 0, 5*sizeof(VERTEX2D), (void**)&pvb, 0 );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)x1;
	fY1 = (float)y1;
	fX2 = (float)x2;
	fY2 = (float)y2;

	pvb[0].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[0].color    = color;

	pvb[1].position = D3DXVECTOR4( fX2,fY1, 100, 1 );
    pvb[1].color    = color;

	pvb[2].position = D3DXVECTOR4( fX2,fY2, 100, 1 );
    pvb[2].color    = color;

	pvb[3].position = D3DXVECTOR4( fX1,fY2, 100, 1 );
    pvb[3].color    = color;

	pvb[4].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[4].color    = color;

	m_pVB2D->Unlock();

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	PD3DDEVICE->DrawPrimitive( D3DPT_LINESTRIP, 0, 4 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
}

void KRepresentShell3::DrawBitmap16(int32 nX, int32 nY, int32 nWidth,
									int32 nHeight, TextureResBmp* pBitmap, bool bStretch)
{
	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	m_pVB2D->Lock( 0, 4*sizeof(VERTEX2D), (void**)&pvb, 0 );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)nX;
	fY1 = (float)nY;
	fX2 = fX1 + (float)nWidth;
	fY2 = fY1 + (float)nHeight;

	float fU2, fV2;
	fU2 = (float)pBitmap->m_nWidth / (float)pBitmap->m_FrameInfo.texInfo[0].nWidth;
	fV2 = (float)pBitmap->m_nHeight / (float)pBitmap->m_FrameInfo.texInfo[0].nHeight;

	pvb[0].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[0].color    = 0xffffffff;
    pvb[0].tu       = 0.0f;
    pvb[0].tv       = 0.0f;

	pvb[1].position = D3DXVECTOR4( fX2,fY1, 100, 1 );
    pvb[1].color    = 0xffffffff;
    pvb[1].tu       = fU2;
    pvb[1].tv       = 0.0f;

	pvb[2].position = D3DXVECTOR4( fX1,fY2, 100, 1 );
    pvb[2].color    = 0xffffffff;
    pvb[2].tu       = 0.0f;
    pvb[2].tv       = fV2;

	pvb[3].position = D3DXVECTOR4( fX2,fY2, 100, 1 );
    pvb[3].color    = 0xffffffff;
    pvb[3].tu       = fU2;
    pvb[3].tv       = fV2;

	m_pVB2D->Unlock();

	if(!pBitmap->m_FrameInfo.texInfo[0].pTexture)
	{
		assert(pBitmap->m_FrameInfo.texInfo[0].pTexture);
		return;
	}
	PD3DDEVICE->SetTexture( 0, pBitmap->m_FrameInfo.texInfo[0].pTexture );
	if(bStretch)
	{
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}
	else
		PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
}

void KRepresentShell3::DrawSpriteAlpha(int32 nX, int32 nY, int32 nWidth, int32 nHeight, 
									   int32 nFrame, TextureResSpr* pSprite, DWORD color, int32 nRenderStyle)
{
	int i;

	if(nFrame >= pSprite->m_nFrameNum)
		return;

	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)nX;
	fY1 = (float)nY;
	fX2 = fX1 + (float)nWidth;
	fY2 = fY1 + (float)nHeight;

	// 如果图素超出屏幕范围则不渲染
	if(fX2 < 0 || fX1 > g_nScreenWidth || fY2 < 0 || fY1 > g_nScreenHeight)
		return;

	VERTEX2D* pvb = NULL;
	if(FAILED(m_pVB2D->Lock( 0, 16*sizeof(VERTEX2D), (void**)&pvb, 0 )))
		return;

	if(nRenderStyle != IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST)
	{
		// 不偏色，将颜色改为白
		color = 0xffffffff;
	}

	// 根据贴图数目把矩形拆分成多个小矩形，计算坐标及纹理
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		float fU2, fV2;
		float x1, y1, x2, y2;
		fU2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth / 
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		fV2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight /
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;

		x1 = ChaZhi(fX1, fX2, 0, (float)nWidth, (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX);
		y1 = ChaZhi(fY1, fY2, 0, (float)nHeight, (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY);
		x2 = ChaZhi(fX1, fX2, 0, (float)nWidth, (float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + 
										pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth));
		y2 = ChaZhi(fY1, fY2, 0, (float)nHeight, (float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + 
										pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight));

		x1 -= 0.5f,	y1 -= 0.5f,	x2 -= 0.5f,	y2 -= 0.5f;

		int nBase = i*4;
		pvb[nBase+0].position = D3DXVECTOR4( x1,y1, 100, 1 );
		pvb[nBase+0].color    = color;
		pvb[nBase+0].tu       = 0.0f;
		pvb[nBase+0].tv       = 0.0f;

		pvb[nBase+1].position = D3DXVECTOR4( x2,y1, 100, 1 );
		pvb[nBase+1].color    = color;
		pvb[nBase+1].tu       = fU2;
		pvb[nBase+1].tv       = 0.0f;

		pvb[nBase+2].position = D3DXVECTOR4( x1,y2, 100, 1 );
		pvb[nBase+2].color    = color;
		pvb[nBase+2].tu       = 0.0f;
		pvb[nBase+2].tv       = fV2;

		pvb[nBase+3].position = D3DXVECTOR4( x2,y2, 100, 1 );
		pvb[nBase+3].color    = color;
		pvb[nBase+3].tu       = fU2;
		pvb[nBase+3].tv       = fV2;
	}

	m_pVB2D->Unlock();

	// 绘制多边形
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		LPDIRECT3DTEXTURE9 pTex = pSprite->GetTexture(nFrame, i);
		if(!pTex)
			continue;

		PD3DDEVICE->SetTexture( 0, pTex );
		
		if( nRenderStyle == IMAGE_RENDER_STYLE_BORDER )
		{
			// 选中加亮效果
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
			PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, i*4, 2 );
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		}
		else
			PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, i*4, 2 );
	}
}

void KRepresentShell3::DrawSpritePartAlpha(int32 nX, int32 nY, int32 nWidth, int32 nHeight, 
									   int32 nFrame, TextureResSpr* pSprite, RECT &rect)
{
	int i;

	if(nFrame >= pSprite->m_nFrameNum)
		return;

	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)nX;
	fY1 = (float)nY;
	fX2 = fX1 + (float)nWidth;
	fY2 = fY1 + (float)nHeight;

	// 如果图素超出屏幕范围则不渲染
	if(fX2 < 0 || fX1 > g_nScreenWidth || fY2 < 0 || fY1 > g_nScreenHeight)
		return;

	VERTEX2D* pvb = NULL;
	if(FAILED(m_pVB2D->Lock( 0, 16*sizeof(VERTEX2D), (void**)&pvb, 0 )))
		return;

	bool bDraw[4];

	// 根据贴图数目把矩形拆分成多个小矩形，计算坐标及纹理
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		float fU1, fV1, fU2, fV2, u2, v2;
		float x1, y1, x2, y2, fRcX1, fRcY1, fRcX2, fRcY2;

		x1 = ChaZhi(fX1, fX2, 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
			(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX);
		y1 = ChaZhi(fY1, fY2, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
			(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY);
		x2 = ChaZhi(fX1, fX2, 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
			(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth));
		y2 = ChaZhi(fY1, fY2, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
			(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight));

		x1 -= 0.5f,	y1 -= 0.5f,	x2 -= 0.5f,	y2 -= 0.5f;

		fRcX1 = (float)rect.left, fRcY1 = (float)rect.top;
		fRcX2 = (float)rect.right, fRcY2 = (float)rect.bottom;
		if(fRcX1 > x2 || fRcX2 < x1 || fRcY1 > y2 || fRcY2 < y1)
		{
			// 如果这个面片的剪彩矩形在图素矩形之外则不画
			bDraw[i] = false;
			continue;
		}
		bDraw[i] = true;
		if(fRcX1 < x1) fRcX1 = x1;
		if(fRcY1 < y1) fRcY1 = y1;
		if(fRcX2 > x2) fRcX2 = x2;
		if(fRcY2 > y2) fRcY2 = y2;

		u2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth / 
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		v2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight /
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;
		fU1 = ChaZhi(0, u2, x1, x2, fRcX1);
		fV1 = ChaZhi(0, v2, y1, y2, fRcY1);
		fU2 = ChaZhi(0, u2, x1, x2, fRcX2);
		fV2 = ChaZhi(0, v2, y1, y2, fRcY2);

		int nBase = i*4;
		pvb[nBase+0].position = D3DXVECTOR4( fRcX1,fRcY1, 100, 1 );
		pvb[nBase+0].color    = 0xffffffff;
		pvb[nBase+0].tu       = fU1;
		pvb[nBase+0].tv       = fV1;

		pvb[nBase+1].position = D3DXVECTOR4( fRcX2,fRcY1, 100, 1 );
		pvb[nBase+1].color    = 0xffffffff;
		pvb[nBase+1].tu       = fU2;
		pvb[nBase+1].tv       = fV1;

		pvb[nBase+2].position = D3DXVECTOR4( fRcX1,fRcY2, 100, 1 );
		pvb[nBase+2].color    = 0xffffffff;
		pvb[nBase+2].tu       = fU1;
		pvb[nBase+2].tv       = fV2;

		pvb[nBase+3].position = D3DXVECTOR4( fRcX2,fRcY2, 100, 1 );
		pvb[nBase+3].color    = 0xffffffff;
		pvb[nBase+3].tu       = fU2;
		pvb[nBase+3].tv       = fV2;
	}

	m_pVB2D->Unlock();

	// 绘制多边形
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		LPDIRECT3DTEXTURE9 pTex = pSprite->GetTexture(nFrame, i);
		if(!pTex || !bDraw[i])
			continue;
		PD3DDEVICE->SetTexture( 0, pTex );
		PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, i*4, 2 );	
	}
}

void KRepresentShell3::DrawBitmap16Part(int32 nX, int32 nY, int32 nWidth, int32 nHeight, TextureResBmp* pBitmap, RECT &rc)
{
	if(FAILED(PD3DDEVICE->SetStreamSource( 0, m_pVB2D, 0, sizeof(VERTEX2D) )))
		return;
	PD3DDEVICE->SetFVF( D3DFVF_VERTEX2D );

	VERTEX2D* pvb;
	m_pVB2D->Lock( 0, 4*sizeof(VERTEX2D), (void**)&pvb, 0 );

	float fX1, fY1, fX2, fY2;
	fX1 = (float)nX;
	fY1 = (float)nY;
	fX2 = fX1 - (float)rc.left + (float)rc.right;
	fY2 = fY1 - (float)rc.top + (float)rc.bottom;

	float fU1, fV1, fU2, fV2, u2, v2;
	float fWidth = (float)pBitmap->GetWidth();
	float fHeight = (float)pBitmap->GetHeight();
	u2 = fWidth / (float)pBitmap->m_FrameInfo.texInfo[0].nWidth;
	v2 = fHeight / (float)pBitmap->m_FrameInfo.texInfo[0].nHeight;
	fU1 = ChaZhi(0, u2, 0, fWidth, (float)rc.left);
	fV1 = ChaZhi(0, v2, 0, fHeight, (float)rc.top);
	fU2 = ChaZhi(0, u2, 0, fWidth, (float)rc.right);
	fV2 = ChaZhi(0, v2, 0, fHeight, (float)rc.bottom);

	fX1 -= 0.5f,	fY1 -= 0.5f,	fX2 -= 0.5f,	fY2 -= 0.5f;

	pvb[0].position = D3DXVECTOR4( fX1,fY1, 100, 1 );
	pvb[0].color    = 0xffffffff;
    pvb[0].tu       = fU1;
    pvb[0].tv       = fV1;

	pvb[1].position = D3DXVECTOR4( fX2,fY1, 100, 1 );
    pvb[1].color    = 0xffffffff;
    pvb[1].tu       = fU2;
    pvb[1].tv       = fV1;

	pvb[2].position = D3DXVECTOR4( fX1,fY2, 100, 1 );
    pvb[2].color    = 0xffffffff;
    pvb[2].tu       = fU1;
    pvb[2].tv       = fV2;

	pvb[3].position = D3DXVECTOR4( fX2,fY2, 100, 1 );
    pvb[3].color    = 0xffffffff;
    pvb[3].tu       = fU2;
    pvb[3].tv       = fV2;

	m_pVB2D->Unlock();

	if(!pBitmap->m_FrameInfo.texInfo[0].pTexture)
	{
		assert(pBitmap->m_FrameInfo.texInfo[0].pTexture);
		return;
	}
	PD3DDEVICE->SetTexture( 0, pBitmap->m_FrameInfo.texInfo[0].pTexture );
	PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
}

void KRepresentShell3::DrawBitmap163D(RenderParam3D &param, TextureResBmp* pBitmap)
{
	float fU2, fV2;
	fU2 = (float)pBitmap->m_nWidth / (float)pBitmap->m_FrameInfo.texInfo[0].nWidth;
	fV2 = (float)pBitmap->m_nHeight / (float)pBitmap->m_FrameInfo.texInfo[0].nHeight;

	float ft1,ft2;
	ft1 = 0.5f / (float)pBitmap->m_FrameInfo.texInfo[0].nWidth;
	ft2 = 0.5f / (float)pBitmap->m_FrameInfo.texInfo[0].nHeight;

	VERTEX3D* pvb = NULL;
	if(FAILED(m_pVB3D->Lock( 0, 4*sizeof(VERTEX3D), (void**)&pvb, 0 )))
		return;

	pvb[0].position = param.m_pos[0];
	pvb[1].position = param.m_pos[1];
	pvb[2].position = param.m_pos[3];
	pvb[3].position = param.m_pos[2];

	pvb[0].color    = 0xffffffff;
    pvb[0].tu       = 0.0f + ft1;
    pvb[0].tv       = 0.0f + ft2;

    pvb[1].color    = 0xffffffff;
    pvb[1].tu       = fU2 - ft1;
    pvb[1].tv       = 0.0f + ft2;

    pvb[2].color    = 0xffffffff;
    pvb[2].tu       = 0.0f + ft1;
    pvb[2].tv       = fV2 - ft2;

    pvb[3].color    = 0xffffffff;
    pvb[3].tu       = fU2 - ft1;
    pvb[3].tv       = fV2 - ft2;

	m_pVB3D->Unlock();

	if(!pBitmap->m_FrameInfo.texInfo[0].pTexture)
	{
		assert(pBitmap->m_FrameInfo.texInfo[0].pTexture);
		return;
	}

	PD3DDEVICE->SetTexture( 0, pBitmap->m_FrameInfo.texInfo[0].pTexture );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
}

void KRepresentShell3::DrawBitmap163DLighting(RenderParam3D &param, TextureResBmp* pBitmap)
{
	float fU2, fV2;
	fU2 = (float)pBitmap->m_nWidth / (float)pBitmap->m_FrameInfo.texInfo[0].nWidth;
	fV2 = (float)pBitmap->m_nHeight / (float)pBitmap->m_FrameInfo.texInfo[0].nHeight;

	float ft1,ft2;
	ft1 = 0.5f / (float)pBitmap->m_FrameInfo.texInfo[0].nWidth;
	ft2 = 0.5f / (float)pBitmap->m_FrameInfo.texInfo[0].nHeight;

	VERTEX3D vbSrc[4];

	vbSrc[0].position = param.m_pos[0];
	vbSrc[1].position = param.m_pos[1];
	vbSrc[2].position = param.m_pos[2];
	vbSrc[3].position = param.m_pos[3];
	
	vbSrc[0].color    = 0xffffffff;
    vbSrc[0].tu       = 0.0f + ft1;
    vbSrc[0].tv       = 0.0f + ft2;

    vbSrc[1].color    = 0xffffffff;
    vbSrc[1].tu       = fU2 - ft1;
    vbSrc[1].tv       = 0.0f + ft2;

    vbSrc[2].color    = 0xffffffff;
	vbSrc[2].tu       = fU2 - ft1;
    vbSrc[2].tv       = fV2 - ft2;

    vbSrc[3].color    = 0xffffffff;
    vbSrc[3].tu       = 0.0f + ft1;
    vbSrc[3].tv       = fV2 - ft2;

	VERTEX3D* pvb = NULL;
	if(FAILED(m_pVB3D->Lock( 0, VERTEX_BUFFER_SIZE*sizeof(VERTEX3D), (void**)&pvb, 0 )))
		return;

	// 生成网格，计算亮度
	uint32 nCount  = BuildMesh3D(vbSrc, pvb, VERTEX_BUFFER_SIZE, 0xffffffff);

	m_pVB3D->Unlock();

	if(nCount <= 2)
		return;

	if(!pBitmap->m_FrameInfo.texInfo[0].pTexture)
	{
		assert(pBitmap->m_FrameInfo.texInfo[0].pTexture);
		return;
	}

	PD3DDEVICE->SetTexture( 0, pBitmap->m_FrameInfo.texInfo[0].pTexture );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );
	PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, nCount - 2 );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
}

void KRepresentShell3::DrawSpriteAlpha3D(RenderParam3D &param, int32 nFrame, 
									TextureResSpr* pSprite, DWORD color, int32 nRenderStyle, RECT *rect)
{
	int i;
	if(nFrame >= pSprite->m_nFrameNum)
		return;

	// 在屏幕空间检测图素是否可见，如不可见则返回
	// 这里还可以优化
	D3DXVECTOR3 vPos1, vPos2, vPos3, vPos4;
	D3DVIEWPORT9 viewportData = g_Device.GetViewport();
	D3DXVec3Project(&vPos1, &param.m_pos[0], &viewportData, &m_matProj, &m_matView, NULL);
	D3DXVec3Project(&vPos2, &param.m_pos[1], &viewportData, &m_matProj, &m_matView, NULL);
	D3DXVec3Project(&vPos3, &param.m_pos[2], &viewportData, &m_matProj, &m_matView, NULL);
	D3DXVec3Project(&vPos4, &param.m_pos[3], &viewportData, &m_matProj, &m_matView, NULL);

	if(vPos2.x < 0 || vPos1.x > g_nScreenWidth || vPos3.x < 0 || vPos4.x > g_nScreenWidth)
		return;
	if(vPos3.y < 0 || vPos1.y > g_nScreenHeight || vPos4.y < 0 || vPos2.y > g_nScreenHeight)
		return;

	D3DXVECTOR3 v11, v22, v1, v2, v3, v4;

	VERTEX3D* pvb = NULL;
	if(FAILED(m_pVB3D->Lock( 0, 16*sizeof(VERTEX3D), (void**)&pvb, 0 )))
		return;

	bool bDraw[4];

	if(nRenderStyle != IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST)
	{
		// 不偏色，将颜色改为白
		color = 0xffffffff;
	}

	// 根据贴图数目把矩形拆分成多个小矩形，计算坐标及纹理
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		bDraw[i] = true;
		float fU1 = 0.0f, fV1 = 0.0f, fU2, fV2, fu2, fv2;
		fU2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth / 
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		fV2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight /
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;

		// 如果只画图素的一部分，则需要调整纹理坐标
		if(rect)
		{
			float fRcX1, fRcY1, fRcX2, fRcY2;
			float fFrameX1 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX;
			float fFrameY1 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY;
			float fFrameX2 = (float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth);
			float fFrameY2 = (float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight);
			fRcX1 = (float)rect->left, fRcY1 = (float)rect->top;
			fRcX2 = (float)rect->right, fRcY2 = (float)rect->bottom;
			if(fRcX1 > fFrameX2	|| fRcX2 < fFrameX1	|| fRcY1 > fFrameY2	|| fRcY2 < fFrameY1)
			{
				// 如果这个面片的剪彩矩形在图素矩形之外则不画
				bDraw[i] = false;
				continue;
			}
			if(fRcX1 < fFrameX1) fRcX1 = fFrameX1;
			if(fRcY1 < fFrameY1) fRcY1 = fFrameY1;
			if(fRcX2 > fFrameX2) fRcX2 = fFrameX2;
			if(fRcY2 > fFrameY2) fRcY2 = fFrameY2;

			fu2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth / 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
			fv2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight /
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;
			fU1 = ChaZhi(0, fu2, fFrameX1, fFrameX2, fRcX1);
			fV1 = ChaZhi(0, fv2, fFrameY1, fFrameY2, fRcY1);
			fU2 = ChaZhi(0, fu2, fFrameX1, fFrameX2, fRcX2);
			fV2 = ChaZhi(0, fv2, fFrameY1, fFrameY2, fRcY2);
		}

		float ft1,ft2;
		ft1 = 0.5f / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		ft2 = 0.5f / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;

		v11 = ChaZhi(param.m_pos[0], param.m_pos[1], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth,
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX);
		v22 = ChaZhi(param.m_pos[3], param.m_pos[2], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth,
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX);
		v1 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY);
		v4 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight));

		v11 = ChaZhi(param.m_pos[0], param.m_pos[1], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth));
		v22 = ChaZhi(param.m_pos[3], param.m_pos[2], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth));
		v2 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY);
		v3 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight));

		int nBase = i*4;
		pvb[nBase+0].position = v1;
		pvb[nBase+0].color    = color;
		pvb[nBase+0].tu       = fU1+ft1;
		pvb[nBase+0].tv       = fV1+ft2;

		pvb[nBase+1].position = v2;
		pvb[nBase+1].color    = color;
		pvb[nBase+1].tu       = fU2-ft1;
		pvb[nBase+1].tv       = fV1+ft2;

		pvb[nBase+2].position = v4;
		pvb[nBase+2].color    = color;
		pvb[nBase+2].tu       = fU1+ft1;
		pvb[nBase+2].tv       = fV2-ft2;

		pvb[nBase+3].position = v3;
		pvb[nBase+3].color    = color;
		pvb[nBase+3].tu       = fU2-ft1;
		pvb[nBase+3].tv       = fV2-ft2;
	}

	m_pVB3D->Unlock();

	// 绘制多边形
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		LPDIRECT3DTEXTURE9 pTex = pSprite->GetTexture(nFrame, i);
		if(!pTex || !bDraw[i])
			continue;
		PD3DDEVICE->SetTexture( 0, pTex );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		if( nRenderStyle == IMAGE_RENDER_STYLE_BORDER )
		{
			// 选中加亮效果
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
			PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, i*4, 2 );
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		}
		else
			PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, i*4, 2 );	
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}
}

void KRepresentShell3::DrawSpriteAlpha3DLighting(RenderParam3D &param, int32 nFrame, 
											TextureResSpr* pSprite, DWORD color, int32 nRenderStyle, RECT *rect)
{
	int i;
	if(nFrame >= pSprite->m_nFrameNum)
		return;

	// 在屏幕空间检测图素是否可见，如不可见则返回
	// 这里还可以优化
	D3DXVECTOR3 vPos1, vPos2, vPos3, vPos4;
	D3DVIEWPORT9 viewportData = g_Device.GetViewport();
	D3DXVec3Project(&vPos1, &param.m_pos[0], &viewportData, &m_matProj, &m_matView, NULL);
	D3DXVec3Project(&vPos2, &param.m_pos[1], &viewportData, &m_matProj, &m_matView, NULL);
	D3DXVec3Project(&vPos3, &param.m_pos[2], &viewportData, &m_matProj, &m_matView, NULL);
	D3DXVec3Project(&vPos4, &param.m_pos[3], &viewportData, &m_matProj, &m_matView, NULL);

	if(vPos2.x < 0 || vPos1.x > g_nScreenWidth || vPos3.x < 0 || vPos4.x > g_nScreenWidth)
		return;
	if(vPos3.y < 0 || vPos1.y > g_nScreenHeight || vPos4.y < 0 || vPos2.y > g_nScreenHeight)
		return;

	D3DXVECTOR3 v11, v22, v1, v2, v3, v4;
	VERTEX3D vbSrc[4];
	int nStripLen[4];

	VERTEX3D* pvb = NULL;
	if(FAILED(m_pVB3D->Lock( 0, 16*sizeof(VERTEX3D), (void**)&pvb, 0 )))
		return;

	bool bDraw[4];
	if(nRenderStyle != IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST)
	{
		// 不偏色，将颜色改为白
		color = 0xffffffff;
	}

	// 根据贴图数目把矩形拆分成多个小矩形，计算坐标及纹理
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		bDraw[i] = true;
		float fU1 = 0.0f, fV1 = 0.0f, fU2, fV2, fu2, fv2;
		fU2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth / 
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		fV2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight /
				(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;

		// 如果只画图素的一部分，则需要调整纹理坐标
		if(rect)
		{
			float fRcX1, fRcY1, fRcX2, fRcY2;
			float fFrameX1 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX;
			float fFrameY1 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY;
			float fFrameX2 = (float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth);
			float fFrameY2 = (float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight);
			fRcX1 = (float)rect->left, fRcY1 = (float)rect->top;
			fRcX2 = (float)rect->right, fRcY2 = (float)rect->bottom;
			if(fRcX1 > fFrameX2	|| fRcX2 < fFrameX1	|| fRcY1 > fFrameY2	|| fRcY2 < fFrameY1)
			{
				// 如果这个面片的剪彩矩形在图素矩形之外则不画
				bDraw[i] = false;
				continue;
			}
			if(fRcX1 < fFrameX1) fRcX1 = fFrameX1;
			if(fRcY1 < fFrameY1) fRcY1 = fFrameY1;
			if(fRcX2 > fFrameX2) fRcX2 = fFrameX2;
			if(fRcY2 > fFrameY2) fRcY2 = fFrameY2;

			fu2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth / 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
			fv2 = (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight /
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;
			fU1 = ChaZhi(0, fu2, fFrameX1, fFrameX2, fRcX1);
			fV1 = ChaZhi(0, fv2, fFrameY1, fFrameY2, fRcY1);
			fU2 = ChaZhi(0, fu2, fFrameX1, fFrameX2, fRcX2);
			fV2 = ChaZhi(0, fv2, fFrameY1, fFrameY2, fRcY2);
		}

		float ft1,ft2;
		ft1 = 0.5f / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		ft2 = 0.5f / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;

		v11 = ChaZhi(param.m_pos[0], param.m_pos[1], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX);
		v22 = ChaZhi(param.m_pos[3], param.m_pos[2], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX);
		v1 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY);
		v4 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight));

		v11 = ChaZhi(param.m_pos[0], param.m_pos[1], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth));
		v22 = ChaZhi(param.m_pos[3], param.m_pos[2], 0, (float)pSprite->m_pFrameInfo[nFrame].nWidth, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth));
		v2 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight,
					(float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY);
		v3 = ChaZhi(v11, v22, 0, (float)pSprite->m_pFrameInfo[nFrame].nHeight, 
					(float)(pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight));

		vbSrc[0].position = v1;
		vbSrc[0].color    = color;
		vbSrc[0].tu       = fU1+ft1;
		vbSrc[0].tv       = fV1+ft2;

		vbSrc[1].position = v2;
		vbSrc[1].color    = color;
		vbSrc[1].tu       = fU2-ft1;
		vbSrc[1].tv       = fV1+ft2;

		vbSrc[2].position = v3;
		vbSrc[2].color    = color;
		vbSrc[2].tu       = fU2-ft1;
		vbSrc[2].tv       = fV2-ft2;

		vbSrc[3].position = v4;
		vbSrc[3].color    = color;
		vbSrc[3].tu       = fU1+ft1;
		vbSrc[3].tv       = fV2-ft2;

		// 生成网格，计算亮度
		nStripLen[i] = BuildMesh3D(vbSrc, pvb, VERTEX_BUFFER_SIZE, color);
		pvb += nStripLen[i];
	}

	m_pVB3D->Unlock();

	// 绘制多边形
	int nBase = 0;
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		LPDIRECT3DTEXTURE9 pTex = pSprite->GetTexture(nFrame, i);
		if(!pTex || !bDraw[i])
			continue;
		PD3DDEVICE->SetTexture( 0, pTex );
		
		if( nRenderStyle == IMAGE_RENDER_STYLE_BORDER )
		{
			// 选中加亮效果
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD );
			PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, nBase, nStripLen[i] - 2 );
		}
		else
			PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, nBase, nStripLen[i] - 2 );	

		nBase += nStripLen[i];
	}
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	PD3DDEVICE->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
}

#define MESH_GRID_SIZE	32.0f
uint32 KRepresentShell3::BuildMesh3D(VERTEX3D *pvbSrc, VERTEX3D* pvbDes, uint32 nNumDes, DWORD color)
{
	uint32 nCount = 0;

	if(pvbSrc[0].position.z == pvbSrc[2].position.z)
	{
		VERTEX3D vt1 = pvbSrc[0];						// 扫描的前一行顶点
		VERTEX3D vt2 = pvbSrc[0];						// 扫描的后一行顶点
		float ftuAdd, ftvAdd;
		bool bAllDone = false;
		
		ftuAdd = (pvbSrc[1].tu - pvbSrc[0].tu) * MESH_GRID_SIZE / (pvbSrc[1].position.x - pvbSrc[0].position.x);
		ftvAdd = (pvbSrc[2].tv - pvbSrc[0].tv) * MESH_GRID_SIZE / (pvbSrc[2].position.y - pvbSrc[0].position.y);

		while(1)
		{
			vt1 = vt2;
			// 计算后一行起点
			vt2.position.y += MESH_GRID_SIZE;
			vt2.tv += ftvAdd;
			if(vt2.position.y >= pvbSrc[2].position.y)
			{
				vt2.position.y = pvbSrc[2].position.y;
				vt2.tv = pvbSrc[2].tv;
				bAllDone = true;
			}
			// 从左到右创建一行三角形条带
			while(1)
			{
				SetPoint3dLighting(pvbDes[nCount++], vt2, color);
				SetPoint3dLighting(pvbDes[nCount++], vt1, color);
				vt1.position.x += MESH_GRID_SIZE;
				vt2.position.x += MESH_GRID_SIZE;
				vt1.tu += ftuAdd;
				vt2.tu += ftuAdd;
				if(vt1.position.x >= pvbSrc[1].position.x)
				{
					vt1.position.x = pvbSrc[1].position.x;
					vt2.position.x = pvbSrc[1].position.x;
					vt1.tu = pvbSrc[1].tu;
					vt2.tu = pvbSrc[1].tu;
					SetPoint3dLighting(pvbDes[nCount++], vt2, color);
					SetPoint3dLighting(pvbDes[nCount++], vt1, color);
					break;
				}
			}
			if(bAllDone)
				break;
			
			vt1 = vt2;
			// 计算后一行起点
			vt2.position.y += MESH_GRID_SIZE;
			vt2.tv += ftvAdd;
			if(vt2.position.y >= pvbSrc[2].position.y)
			{
				vt2.position.y = pvbSrc[2].position.y;
				vt2.tv = pvbSrc[2].tv;
				bAllDone = true;
			}
			// 从右到左创建一行三角形条带
			while(1)
			{
				SetPoint3dLighting(pvbDes[nCount++], vt1, color);
				SetPoint3dLighting(pvbDes[nCount++], vt2, color);
				vt1.position.x -= MESH_GRID_SIZE;
				vt2.position.x -= MESH_GRID_SIZE;
				vt1.tu -= ftuAdd;
				vt2.tu -= ftuAdd;
				if(vt1.position.x <= pvbSrc[0].position.x)
				{
					vt1.position.x = pvbSrc[0].position.x;
					vt2.position.x = pvbSrc[0].position.x;
					vt1.tu = pvbSrc[0].tu;
					vt2.tu = pvbSrc[0].tu;
					SetPoint3dLighting(pvbDes[nCount++], vt1, color);
					SetPoint3dLighting(pvbDes[nCount++], vt2, color);
					break;
				}
			}
			if(bAllDone)
				break;
		}
	}
	else
	{
		VERTEX3D vt1 = pvbSrc[0];						// 扫描的前一行顶点
		VERTEX3D vt2 = pvbSrc[0];						// 扫描的后一行顶点
		float ftuAdd, ftvAdd;
		D3DXVECTOR3 fvAdd;
		float fEndZ1, fEndZ2;
		bool bAllDone = false;
		
		ftuAdd = (pvbSrc[1].tu - pvbSrc[0].tu) * (MESH_GRID_SIZE / (pvbSrc[1].position.x - pvbSrc[0].position.x));
		ftvAdd = (pvbSrc[3].tv - pvbSrc[0].tv) * (MESH_GRID_SIZE / (pvbSrc[0].position.z - pvbSrc[3].position.z));
		fvAdd = (pvbSrc[1].position - pvbSrc[0].position) * (MESH_GRID_SIZE / (pvbSrc[1].position.x - pvbSrc[0].position.x));

		while(1)
		{
			vt1 = vt2;
			// 计算后一行起点
			vt2.position += fvAdd;
			vt2.tu += ftuAdd;
			if(abs((int)(vt2.position.x - pvbSrc[0].position.x)) >=
				abs((int)(pvbSrc[1].position.x - pvbSrc[0].position.x)))
			{
				vt2 = pvbSrc[1];
				bAllDone = true;
			}
			fEndZ1 = vt1.position.z - (pvbSrc[0].position.z - pvbSrc[3].position.z);
			fEndZ2 = vt2.position.z - (pvbSrc[0].position.z - pvbSrc[3].position.z);
			// 从上到下创建一行三角形条带
			while(1)
			{
				SetPoint3dLighting(pvbDes[nCount++], vt1, color);
				SetPoint3dLighting(pvbDes[nCount++], vt2, color);
				vt1.position.z -= MESH_GRID_SIZE;
				vt2.position.z -= MESH_GRID_SIZE;
				vt1.tv += ftvAdd;
				vt2.tv += ftvAdd;
				if(vt1.position.z <= fEndZ1)
				{
					vt1.position.z = fEndZ1;
					vt2.position.z = fEndZ2;
					vt1.tv = pvbSrc[3].tv;
					vt2.tv = pvbSrc[3].tv;
					SetPoint3dLighting(pvbDes[nCount++], vt1, color);
					SetPoint3dLighting(pvbDes[nCount++], vt2, color);
					break;
				}
			}
			if(bAllDone)
				break;
			
			vt1 = vt2;
			// 计算后一行起点
			vt2.position += fvAdd;
			vt2.tu += ftuAdd;
			if(abs((int)(vt2.position.x - pvbSrc[3].position.x)) >=
				abs((int)(pvbSrc[2].position.x - pvbSrc[3].position.x)))
			{
				vt2 = pvbSrc[2];
				bAllDone = true;
			}
			fEndZ1 = vt1.position.z + (pvbSrc[0].position.z - pvbSrc[3].position.z);
			fEndZ2 = vt2.position.z + (pvbSrc[0].position.z - pvbSrc[3].position.z);
			// 从右到左创建一行三角形条带
			while(1)
			{
				SetPoint3dLighting(pvbDes[nCount++], vt2, color);
				SetPoint3dLighting(pvbDes[nCount++], vt1, color);
				vt1.position.z += MESH_GRID_SIZE;
				vt2.position.z += MESH_GRID_SIZE;
				vt1.tv -= ftvAdd;
				vt2.tv -= ftvAdd;
				if(vt1.position.z >= fEndZ1)
				{
					vt1.position.z = fEndZ1;
					vt2.position.z = fEndZ2;
					vt1.tv = pvbSrc[0].tv;
					vt2.tv = pvbSrc[0].tv;
					SetPoint3dLighting(pvbDes[nCount++], vt2, color);
					SetPoint3dLighting(pvbDes[nCount++], vt1, color);
					break;
				}
			}
			if(bAllDone)
				break;
		}
	}

	return nCount;
}


unsigned int KRepresentShell3::GetPoint3dLightingLerp(D3DXVECTOR3& v)
{
	DWORD dwLightColor;
	D3DXVECTOR3 vColor[4];
	KRPosition3 pt[4];
	float fOffX, fOffY;
	fOffX = (((int)v.x) % LIGHTING_GRID_SIZEX) / (float)LIGHTING_GRID_SIZEX;
	fOffY = (((int)v.y) % LIGHTING_GRID_SIZEY) / (float)LIGHTING_GRID_SIZEY;
	pt[0].nX = (int)v.x;
	pt[0].nY = (int)v.y;
	pt[0].nZ = (int)v.z;
	pt[1].nX = pt[0].nX + LIGHTING_GRID_SIZEX;
	pt[1].nY = pt[0].nY;
	pt[1].nZ = pt[0].nZ;
	pt[2].nX = pt[0].nX + LIGHTING_GRID_SIZEX;
	pt[2].nY = pt[0].nY + LIGHTING_GRID_SIZEY;
	pt[2].nZ = pt[0].nZ;
	pt[3].nX = pt[0].nX;
	pt[3].nY = pt[0].nY + LIGHTING_GRID_SIZEY;
	pt[3].nZ = pt[0].nZ;

	for(int i=0; i<4; i++)
	{
		if(pt[i].nX < m_nLightingAreaX || pt[i].nY < m_nLightingAreaY ||
					pt[i].nX >= m_nLightingAreaX + 1536 || pt[i].nY >= m_nLightingAreaY + 3072)
			dwLightColor = 0xff404040;
		else
		{
			pt[i].nX = (pt[i].nX - m_nLightingAreaX) / LIGHTING_GRID_SIZEX;
			pt[i].nY = (pt[i].nY - m_nLightingAreaY) / LIGHTING_GRID_SIZEY;
			dwLightColor = pLightingArray[pt[i].nY * LIGHTING_GRID_WIDTH + pt[i].nX];
		}
		vColor[i].x = (float)((dwLightColor & 0x00ff0000) >> 16);
		vColor[i].y = (float)((dwLightColor & 0x0000ff00) >> 8);
		vColor[i].z = (float)(dwLightColor & 0x000000ff);
	}

	vColor[0] = vColor[0] * fOffX + vColor[1] * (1.0f - fOffX);
	vColor[3] = vColor[3] * fOffX + vColor[2] * (1.0f - fOffX);
	vColor[0] = vColor[0] * fOffY + vColor[3] * (1.0f - fOffY);
	dwLightColor = 0xff000000 | (((DWORD) vColor[0].x)<<16) | (((DWORD) vColor[0].y)<<8) | ((DWORD) vColor[0].z);

	return dwLightColor;
}

void KRepresentShell3::RIO_CopyBitmap16ToBuffer(TextureResBmp* pBitmap, TextureResBmp* pDestBitmap, 
												int32 nX, int32 nY)
{
	return;
/*	int   nDestWidth  = pDestBitmap->GetWidth();
	int   nDestHeight = pDestBitmap->GetHeight();
	int   nSrcWidth  = pBitmap->GetWidth();
	int   nSrcHeight = pBitmap->GetHeight();

	if(!pBitmap->m_TextureInfo.pTexture)
	{
		assert(pBitmap->m_TextureInfo.pTexture);
		return;
	}

	if(!pDestBitmap->m_TextureInfo.pTexture)
	{
		assert(pDestBitmap->m_TextureInfo.pTexture);
		return;
	}

	IDirect3DSurface8* pSrcSurface, *pDesSurface;

	if ( pBitmap->m_TextureInfo.pTexture->GetSurfaceLevel( 0, &pSrcSurface) != D3D_OK )
		return;
	if ( pDestBitmap->m_TextureInfo.pTexture->GetSurfaceLevel( 0, &pDesSurface) != D3D_OK )
	{
		pSrcSurface->Release();
		return;
	}

	RECT SrcRect;
	POINT DesPoint;

	SrcRect.left = SrcRect.top = 0;
	DesPoint.x = nX;
	DesPoint.y = nY;


	if(nSrcWidth <= nDestWidth - nX)
		SrcRect.right = nSrcWidth;
	else
		SrcRect.right = nDestWidth - nX;

	if(nSrcHeight <= nDestHeight - nY)
		SrcRect.bottom = nSrcHeight;
	else
		SrcRect.bottom = nDestHeight - nY;

	PD3DDEVICE->CopyRects( pSrcSurface, &SrcRect, 1, pDesSurface, &DesPoint);
	pSrcSurface->Release();
	pDesSurface->Release();*/
}

inline WORD C32BitTo16Bit555(BYTE* pSrc)
{
	return (((WORD)(*pSrc))>>3) + ((((WORD)(*(pSrc+1)))<<2)&MASK_GREEN_555) + ((((WORD)(*(pSrc+2)))<<7)&MASK_RED_555);
}

inline void C32BitTo16Bit565(BYTE* pSrc, WORD *pDesColor)
{
	WORD rs, gs, bs, rd, gd, bd, as, ad;
	rs = (WORD)(*(pSrc+2));
	gs = (WORD)(*(pSrc+1));
	bs = (WORD)(*pSrc);
	as = (WORD)(*(pSrc+3));
	ad = 255 - as;
	rd = (*pDesColor & MASK_RED_565) >> 8;
	gd = (*pDesColor & MASK_GREEN_565) >> 3;
	bd = (*pDesColor & MASK_BLUE_565) << 3;

	*pDesColor = ((rs * as) & MASK_RED_565) + (((gs * as) >> 5) & MASK_GREEN_565) + ((bs * as) >> 11) +
					((rd * ad) & MASK_RED_565) + (((gd * ad) >> 5) & MASK_GREEN_565) + ((bd * ad) >> 11);

}

void KRepresentShell3::RIO_CopySprToBufferAlpha(TextureResSpr* pSprite, int32 nFrame, 
									TextureResBmp* pDestBitmap, int32 nX, int32 nY)
{
	if(nFrame >= pSprite->m_nFrameNum)
		return;

	int nx, ny, i;
	int32 nTargetWidth, nTargetHeight;

	VERTEX2D* pvb;
	if(FAILED(m_pVB2D->Lock( 0, 16*sizeof(VERTEX2D), (void**)&pvb, 0 )))
		return;

	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		int32 nDesWidth  = pDestBitmap->GetWidth();
		int32 nDesHeight = pDestBitmap->GetHeight();
		int32 nSrcWidth  = pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameWidth;
		int32 nSrcHeight = pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameHeight;
		nx = nX + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameX;
		ny = nY + pSprite->m_pFrameInfo[nFrame].texInfo[i].nFrameY;

		if(nx >= 0)
			nDesWidth -= nx;
		else
			nSrcWidth += nx;
		if(ny >= 0)
			nDesHeight -= ny;
		else
			nSrcHeight += ny;

		if(nSrcWidth <= nDesWidth)
			nTargetWidth = nSrcWidth;
		else
			nTargetWidth = nDesWidth;
		if(nSrcHeight <= nDesHeight)
			nTargetHeight = nSrcHeight;
		else
			nTargetHeight = nDesHeight;

		float fX1, fY1, fX2, fY2;
		fX1 = fY1 = 0.0f;
		if(nx >= 0)
			fX1 = (float)nx;
		if(ny >= 0)
			fY1 = (float)ny;
		fX2 = fX1 + (float)nTargetWidth;
		fY2 = fY1 + (float)nTargetHeight;

		float fU1, fV1, fU2, fV2;
		if(nx >= 0)
			fU1 = 0.0f;
		else
			fU1 = -(float)nx / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		if(ny >= 0)
			fV1 = 0.0f;
		else
			fV1 = -(float)ny / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;
		fU2 = fU1 + (float)nTargetWidth / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nWidth;
		fV2 = fV1 + (float)nTargetHeight / (float)pSprite->m_pFrameInfo[nFrame].texInfo[i].nHeight;

		fX1 -= 0.5f, fY1 -= 0.5f, fX2 -= 0.5f, fY2 -= 0.5f;

		int nBase = i*4;
		pvb[nBase+0].position = D3DXVECTOR4( fX1,fY1, 0, 1 );
		pvb[nBase+0].color    = 0xffffffff;
		pvb[nBase+0].tu       = fU1;
		pvb[nBase+0].tv       = fV1;

		pvb[nBase+1].position = D3DXVECTOR4( fX2,fY1, 0, 1 );
		pvb[nBase+1].color    = 0xffffffff;
		pvb[nBase+1].tu       = fU2;
		pvb[nBase+1].tv       = fV1;

		pvb[nBase+2].position = D3DXVECTOR4( fX1,fY2, 0, 1 );
		pvb[nBase+2].color    = 0xffffffff;
		pvb[nBase+2].tu       = fU1;
		pvb[nBase+2].tv       = fV2;

		pvb[nBase+3].position = D3DXVECTOR4( fX2,fY2, 0, 1 );
		pvb[nBase+3].color    = 0xffffffff;
		pvb[nBase+3].tu       = fU2;
		pvb[nBase+3].tv       = fV2;
	}

	m_pVB2D->Unlock();

	// 绘制多边形
	for(i=0; i<pSprite->m_pFrameInfo[nFrame].nTexNum; i++)
	{
		LPDIRECT3DTEXTURE9 pTex = pSprite->GetTexture(nFrame, i);
		if(!pTex)
			return;
		PD3DDEVICE->SetTexture( 0, pTex );
		PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLESTRIP, i*4, 2 );
	}
}

void KRepresentShell3::SetLightInfo(int nX, int nY, unsigned int *pLighting)
{
	if(!pLighting)
	{
		m_bDoLighting = false;
		return;
	}

	m_nLightingAreaX = nX;
	m_nLightingAreaY = nY;
	memcpy(pLightingArray, pLighting, LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT * sizeof(DWORD));
}

bool KRepresentShell3::SaveScreenToFile(const char* pszName, ScreenFileType eType, unsigned int nQuality)
{
	if(!pszName || !pszName[0])
		return 0;

	if(m_bDeviceLost)
		return false;

	int nPicWidth, nPicHeight, nDesktopWidth, nDesktopHeight, nPicOffX, nPicOffY;
	if(g_bRunWindowed)
	{
		RECT rect;
		POINT ptLT, ptRB;
		GetClientRect(g_hWnd, &rect);
		ptLT.x = rect.left, ptLT.y = rect.top;
		ptRB.x = rect.right, ptRB.y = rect.bottom;
		ClientToScreen(g_hWnd, &ptLT);
		ClientToScreen(g_hWnd, &ptRB);

		D3DDISPLAYMODE mode;
		PD3DDEVICE->GetDisplayMode(D3DADAPTER_DEFAULT, &mode);
		nDesktopWidth = mode.Width;
		nDesktopHeight = mode.Height;

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
		nPicOffX = 0;
		nPicOffY = 0;
		nDesktopWidth = nPicWidth = g_nScreenWidth;
		nDesktopHeight = nPicHeight = g_nScreenHeight;
	}

	BYTE *pData;
	// 创建用于存放截图的Surface
	IDirect3DSurface9* pSurface = NULL;
	if(FAILED(PD3DDEVICE->CreateOffscreenPlainSurface(nDesktopWidth, nDesktopHeight, 
												D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM , &pSurface, NULL)))
		goto error;

	// 获取屏幕截图
	if(FAILED(PD3DDEVICE->GetFrontBufferData(0, pSurface)))
    	goto error;

	D3DLOCKED_RECT lockedRect;
	if(FAILED(pSurface->LockRect(&lockedRect, NULL, D3DLOCK_READONLY)))
	{
		pSurface->Release();
		goto error;
	}
	
	pData = ((BYTE*)lockedRect.pBits) + nPicOffY * nDesktopWidth * 4 + nPicOffX * 4;
	BOOL bRet;
	if(eType == SCRFILETYPE_BMP)
		// 保存24位bmp文件
		bRet = KBmpFile24::SaveBuffer32((char*)pszName, pData, nDesktopWidth*4, nPicWidth, nPicHeight);
	else
		// 保存24位jpg文件
		bRet = SaveBufferToJpgFile32((char*)pszName, pData, nDesktopWidth*4, nPicWidth, nPicHeight, nQuality);
	if(!bRet)
	{
		pSurface->UnlockRect();
		goto error;
	}

	pSurface->UnlockRect();
	SAFE_RELEASE(pSurface);
	return true;

error:
	SAFE_RELEASE(pSurface);
	return false;
}

void KRepresentShell3::SetGamma(int nGamma)
{
	if(nGamma < 0 || nGamma > 100)
		return;

	// 把nGamma变为-100到100
	nGamma = nGamma * 2 - 100;

	D3DGAMMARAMP ramp;
	DWORD dwGama;
	int i;
	if(nGamma >= 0)
	{
		for(i=0; i<256; i++)
		{
			dwGama = i * 256 + i * nGamma;
			if(dwGama <= 65535)
				ramp.red[i] = ramp.green[i] = ramp.blue[i] = (WORD)dwGama;
			else
				ramp.red[i] = ramp.green[i] = ramp.blue[i] = 65535;
		}
	}
	else
	{
		for(i=0; i<256; i++)
		{
			dwGama = i * 256 + i * nGamma;
			if(dwGama <= 65535)
				ramp.red[i] = ramp.green[i] = ramp.blue[i] = (WORD)dwGama;
			else
				ramp.red[i] = ramp.green[i] = ramp.blue[i] = 0;
		}
	}
	PD3DDEVICE->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &ramp);
}
