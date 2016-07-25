// D3D_Device.h
//	Contains the active D3D Device. CD3D_Shell & CD3D_Device are the main containers of D3D shtuff...

#include "precompile.h"

#include "d3d_device.h"
#include "d3d_shell.h"
#include "d3d_utils.h"

// The global D3D Shell...
CD3D_Device g_Device;

// 创建显示设备
bool CD3D_Device::CreateDevice(D3DAdapterInfo* pAdapter,D3DDeviceInfo* pDevice,D3DModeInfo* pMode)
{
	FreeDevice();								// Make sure it's all released and groovie...

	m_pAdapter	= pAdapter;
	m_pDevice	= pDevice;
	m_pMode		= pMode;

 	uint32 BehaviorFlags = D3DCREATE_MULTITHREADED;

	if (pDevice->d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) BehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
	else BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	
	SetPresentationParams(m_PresentationParam,pMode);

	HRESULT hResult = PDIRECT3D->CreateDevice(pAdapter->iAdapterNum,pDevice->DeviceType,g_hWnd,BehaviorFlags,&m_PresentationParam,&m_pD3DDevice);
	if ((hResult != D3D_OK) || !m_pD3DDevice) 
	{
		// 创建设备失败
		assert(0);
		g_DebugLog("[D3DRender]Warning: Device create failed.");
		return false;
	}

	if (FAILED(m_pD3DDevice->GetDeviceCaps(&m_DeviceCaps)))
	{
		FreeAll(); 
		return false; 
	}
	
	// 检测是否支持非二的幂次贴图长宽
//	if(m_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
//	{
//		g_bNonPow2Conditional = true;
//		g_DebugLog("[D3DRender]D3DPTEXTURECAPS_NONPOW2CONDITIONAL is supported.");
//	}
//	else
//		g_DebugLog("[D3DRender]D3DPTEXTURECAPS_NONPOW2CONDITIONAL is not supported");
	
	// 设置缺省的渲染状态
	SetDefaultRenderStates();

	// 创建了一个软件渲染设备，显示一个警告
	if (m_pDevice->DeviceType == D3DDEVTYPE_REF)
		g_DebugLog("[D3DRender]Warning: Couldnt' find any HAL devices, Using reference rasterizer");

	return true;
}

bool CD3D_Device::SetMode(D3DModeInfo* pMode)
{
	SetPresentationParams(m_PresentationParam,pMode);
	if (m_pD3DDevice->Reset(&m_PresentationParam) != D3D_OK)
		return false;

	m_rcViewport.left			= 0;	// Force a viewport reset...
	m_rcViewport.right			= 0;
	m_rcViewport.top			= 0;
	m_rcViewport.bottom			= 0;
	return true;
}

bool CD3D_Device::ResetDevice()
{
	if (!m_pD3DDevice)
		return false;

	if (m_pD3DDevice->Reset(&m_PresentationParam) != D3D_OK)
		return false;

	return true;
}

// Releases the device (and resets all that should be reset on device release)...
void CD3D_Device::FreeDevice()
{
	if (m_pD3DDevice)
	{
		uint32 iRefCnt = m_pD3DDevice->Release();
	} // assert(iRefCnt==0);

	ResetDeviceVars();
}

// 释放设备对象
bool CD3D_Device::ReleaseDevObjects()
{
	return true; 
}

// 恢复设备对象
bool CD3D_Device::RestoreDevObjects()
{
	return true;
}

// Do all the resetting that we need to do (device release stuff)...
void CD3D_Device::ResetDeviceVars()
{
	m_pD3DDevice				= NULL;
	m_pAdapter					= NULL;
	m_pDevice					= NULL;
	m_pMode						= NULL;
	m_bWindowed					= false;
	m_bIn3D						= false;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Device::Reset()
{
	ResetDeviceVars();

	m_rcViewport.left			= 0;
	m_rcViewport.right			= 0;
	m_rcViewport.top			= 0;
	m_rcViewport.bottom			= 0;
}

// 释放所有成员变量，然后重置
void CD3D_Device::FreeAll()
{
	FreeDevice();
	Reset();
}
 
void CD3D_Device::SetPresentationParams(D3DPRESENT_PARAMETERS& PresentationParam,D3DModeInfo* pMode)
{
	ZeroMemory( &PresentationParam, sizeof(PresentationParam) );

	D3DSWAPEFFECT	SwapEffect		= D3DSWAPEFFECT_DISCARD;
	uint32			BackBufferCount	= g_nBackBufferCount;
	if (g_bRunWindowed || g_bForceSwapEffectBlt) { SwapEffect = D3DSWAPEFFECT_COPY ; BackBufferCount = 1; }

	PresentationParam.BackBufferWidth					= pMode->Width;
	PresentationParam.BackBufferHeight					= pMode->Height;
	PresentationParam.BackBufferFormat					= pMode->Format;
	PresentationParam.BackBufferCount					= BackBufferCount;						// Number of back buffers (1 "double buffer", 2 "triple buffer", etc)
	PresentationParam.MultiSampleType					= GetDefaultMultiSampleType(g_nAntiAliasFSOverSample);
	PresentationParam.SwapEffect						= SwapEffect;
	PresentationParam.hDeviceWindow						= g_hWnd;
	PresentationParam.Windowed							= g_bRunWindowed;
	PresentationParam.EnableAutoDepthStencil			= false;
//	PresentationParam.AutoDepthStencilFormat			= GetDefaultDepthStencilFormat(g_CV_ZBitDepth,g_CV_StencilBitDepth);
	// 要支持全屏模式下的系统输入法，这里必须是D3DPRESENTFLAG_LOCKABLE_BACKBUFFER，在某些显卡上可能有性能损失
	PresentationParam.Flags								= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	// puffs, but need it for the console (should get rid of this eventually)...
	//PresentationParam.Flags								= 0;
//	PresentationParam.Flags								= 0;
	PresentationParam.FullScreen_RefreshRateInHz		= D3DPRESENT_RATE_DEFAULT; //D3DPRESENT_RATE_UNLIMITED
}

/*
// Go for the best one that works and fallback till we find one...
//	Note: Need to watch for loops here. Would be very bad.
#define D3DDEVICE_CHECKVALID_ZSFORMAT(MyFormat) (PDIRECT3D->CheckDeviceFormat(m_pAdapter->iAdapterNum,m_pDevice->DeviceType,m_pMode->Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,MyFormat) == D3D_OK ? true : false)
D3DFORMAT CD3D_Device::GetDefaultDepthStencilFormat(uint32 iZBitDepth,uint32 iStencilBitDepth)
{
	D3DFORMAT DepthStencilFormat = D3DFMT_UNKNOWN;
	switch (iZBitDepth) {
	case 32 : 
		switch (iStencilBitDepth) {
		case 8  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(32,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D24X4S4;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,8); break;
		case 1  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(32,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D32;		if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,8); break;
		default : assert(0); return D3DFMT_UNKNOWN; break; } break;
	case 24 : 
		switch (iStencilBitDepth) {
		case 8  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D24X4S4;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,0); break;
		case 1  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D24X8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,8); break;
		default : assert(0); return D3DFMT_UNKNOWN; break; } break;
	case 16 : 
		switch (iStencilBitDepth) {
		case 8  : DepthStencilFormat = D3DFMT_D15S1;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D15S1;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,0); break;
		case 1  : DepthStencilFormat = D3DFMT_D15S1;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D16;		if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return D3DFMT_UNKNOWN; break;
		default : assert(0); return D3DFMT_UNKNOWN; break; } break;
	default : assert(0); return D3DFMT_UNKNOWN; }

	return DepthStencilFormat;
}*/

D3DMULTISAMPLE_TYPE CD3D_Device::GetDefaultMultiSampleType(uint32 Samples)
{
	D3DMULTISAMPLE_TYPE MultiSampleType = (D3DMULTISAMPLE_TYPE)Samples;
	if (PDIRECT3D->CheckDeviceMultiSampleType(m_pAdapter->iAdapterNum,m_pDevice->DeviceType,m_pMode->Format,g_bRunWindowed,MultiSampleType,NULL) != D3D_OK) {
		if ((uint32)MultiSampleType > 0) return GetDefaultMultiSampleType((uint32)MultiSampleType-1); else return (D3DMULTISAMPLE_TYPE)0; }
	return MultiSampleType;
}


void CD3D_Device::SetDefaultRenderStates()
{
	// Basic Render State defaults...
	m_pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
	m_pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE,  D3DFOG_NONE);

	m_pD3DDevice->SetRenderState(D3DRS_FOGENABLE, g_nFogEnable);
	m_pD3DDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	m_pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	m_pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, g_nWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	m_pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);

	// Disable Clipping..
 	m_pD3DDevice->SetRenderState(D3DRS_CLIPPING, TRUE);

	// HW Transform & Light...
//	if (!(g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)) 
//		PD3DDEVICE->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,true);

	// Setup Material (for D3D lighting)...
	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_MATERIAL);
	m_pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_MATERIAL);
	m_pD3DDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_MATERIAL);
	m_pD3DDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL);

	// Texture Stage States...
	// 设置贴图渲染阶段0
	// 设置颜色混合模式
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	// 设置Alpha混合模式
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	// 设置过滤模式
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	// 关闭0以上的贴图渲染阶段
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	// 清除贴图Stage
	m_pD3DDevice->SetTexture(0, 0);

	assert(m_pD3DDevice);
}

void CD3D_Device::SetupViewport(uint32 iLeft, uint32 iRight, uint32 iTop, uint32 iBottom, float fMinZ, float fMaxZ)
{
	if (m_rcViewport.left == iLeft && m_rcViewport.right == iRight && m_rcViewport.top == iTop && m_rcViewport.bottom == iBottom) return;

	m_rcViewport.left	= iLeft;
	m_rcViewport.right	= iRight;
	m_rcViewport.top	= iTop;
	m_rcViewport.bottom = iBottom;

	m_viewportData.X		= m_rcViewport.left;	
	m_viewportData.Y		= m_rcViewport.top;
	m_viewportData.Width	= m_rcViewport.right  - m_rcViewport.left;
	m_viewportData.Height = m_rcViewport.bottom - m_rcViewport.top;
	m_viewportData.MinZ	= fMinZ;
	m_viewportData.MaxZ	= fMaxZ;

	HRESULT hResult = PD3DDEVICE->SetViewport(&m_viewportData);
	if (hResult != D3D_OK)
	{
		g_DebugLog("[D3DRender]IDirect3DDevice::SetCurrentViewport failed.");
		assert(0);
		return;
	}
}

// Call before you start rendering a frame...
bool CD3D_Device::Start3D()
{
	if (g_Device.m_bIn3D || !g_Device.m_pD3DDevice)
		return false;

	HRESULT hResult = g_Device.m_pD3DDevice->BeginScene();
	if (hResult == D3D_OK)
		return (g_Device.m_bIn3D = true); 
	else
		return false;
}

// Call when you are done rendering a frame...
bool CD3D_Device::End3D()
{
	if (!g_Device.m_bIn3D || !g_Device.m_pD3DDevice)
		return false;
	g_Device.m_bIn3D = false;

	HRESULT hResult = g_Device.m_pD3DDevice->EndScene();

	if (hResult == D3D_OK)
		return true;
	else
		return false;
}

bool CD3D_Device::IsIn3D()
{
	return g_Device.m_bIn3D;
}

// 打印输出设备能力
void CD3D_Device::ListDeviceCaps()
{
	if (!m_pD3DDevice || !m_pAdapter || !m_pMode)
		return;

	uint32 iBitCount, iAlphaMask, iRedMask, iGreenMask, iBlueMask;
	d3d_GetColorMasks(m_pMode->Format, iBitCount, iAlphaMask, iRedMask, iGreenMask, iBlueMask);

	g_DebugLog("[D3DRender]---------------------------------------------------------------");
	g_DebugLog("[D3DRender]Driver: %s", m_pAdapter->AdapterID.Driver);
	g_DebugLog("[D3DRender]Description: %s", m_pAdapter->AdapterID.Description);
	g_DebugLog("[D3DRender]Version: %x", m_pAdapter->AdapterID.DriverVersion);
	g_DebugLog("[D3DRender]VendorID: 0x%x, DeviceID: 0x%x, SubSysID: 0x%x, Revision: 0x%x",
		m_pAdapter->AdapterID.VendorId, m_pAdapter->AdapterID.DeviceId, 
		m_pAdapter->AdapterID.SubSysId, m_pAdapter->AdapterID.Revision);
	g_DebugLog("[D3DRender]Width: %d, Height: %d, BitDepth: %d",
		m_pMode->Width,m_pMode->Height,iBitCount);
	g_DebugLog("[D3DRender]---------------------------------------------------------------"); 
}
