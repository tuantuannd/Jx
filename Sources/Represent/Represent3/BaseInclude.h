#ifndef __BASEINCLUDE_H__
#define __BASEINCLUDE_H__

#define DEFAULT_SCREENWIDTH		800				// 缺省屏幕宽度
#define DEFAULT_SCREENHEIGHT	600				// 缺省屏幕高度
#define DEFAULT_BITDEPTH		16				// 缺省位深度

#define MAX_TEXTURE_SIZE		1024			// 最大贴图尺寸
#define MIN_TEXTURE_SIZE		8				// 最小贴图尺寸

enum RenderModel
{
	RenderModel2D,
	RenderModel3DOrtho,
	RenderModel3DPerspective,
};

extern HWND		g_hWnd;

extern D3DFORMAT g_16BitFormat;

extern bool		g_bRefRast;						// 是否使用软件3D加速
extern bool		g_bRunWindowed;					// 是否使用窗口模式

extern int32	g_nScreenWidth;					// 显示窗口宽度
extern int32	g_nScreenHeight;				// 显示窗口高度

extern uint32	g_nAntiAliasFSOverSample;		// 多采样级别
extern uint32	g_nBackBufferCount;				// BackBuffer个数
extern bool		g_bForceSwapEffectBlt;			// 是否强制显存交换方式为拷贝
extern bool		g_bForce32Bit;					// 是否强制以32位方式处理ISI_T_BITMAP16资源,暂时不用，都安32位处理

extern int		g_nFogEnable;					// 是否打开雾化效果

extern int		g_nWireframe;					// 是否打开线框模式

extern RenderModel		g_renderModel;				// 是否打开透视模式

extern bool		g_bUse4444Texture;				// spr资源是否使用4444格式贴图，如果否，则使用8888格式

extern bool		g_bNonPow2Conditional;			// 是否允许非二的幂次长宽的贴图

extern int		g_ntest;

#define YESNO(X) (X ? "Yes" : "No")

//#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#endif