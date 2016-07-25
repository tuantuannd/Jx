/*****************************************************************************************
//  表现模块的对外接口的二维版本实现。
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-11-11
------------------------------------------------------------------------------------------
*****************************************************************************************/

#ifndef KREPRESENTSHELL3_H_HEADER_INCLUDED_C2314D70
#define KREPRESENTSHELL3_H_HEADER_INCLUDED_C2314D70
#define _REPRESENT_INTERNAL_SIGNATURE_
#include "../iRepresent/iRepresentShell.h"
#include "../../Engine/Src/KEngine.h"
#include "../iRepresent/Text/TextProcess.h"

#include "TextureResMgr.h"

#define VERTEX_BUFFER_SIZE	3000	// 顶点缓冲的尺寸 
#define SPR_PRERENDER_TEXSIZE1 128	// 主角预渲染贴图尺寸
#define SPR_PRERENDER_TEXSIZE2 256	// 主角预渲染贴图尺寸

// 用于非透视渲染的顶点格式
#define D3DFVF_VERTEX2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
// 用于透视渲染的顶点格式
#define D3DFVF_VERTEX3D (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct VERTEX2D
{
    D3DXVECTOR4 position; // The transformed position for the vertex.
    DWORD		color;    // The vertex color.
	FLOAT       tu, tv;   // The texture coordinates.
};

struct VERTEX3D
{
    D3DXVECTOR3 position; // The transformed position for the vertex.
    DWORD		color;    // The vertex color.
	FLOAT       tu, tv;   // The texture coordinates.
};

struct RECTFLOAT
{
	float top;
	float left;
	float bottom;
	float right;
};

// 3D渲染参数
struct RenderParam3D
{
    D3DXVECTOR3	m_pos[4];
};

struct	KImageParam;
class	TextureResBmp;
class	TextureResSpr;
class	KFont3;


//## 表现模块的对外接口的三维版本实现。
class KRepresentShell3 : public iRepresentShell
{
public:
	KRepresentShell3();
//=======================================
//=====iRepresentShell定义的接口函数=====
//=======================================

    //## 释放销毁接口对象
    void Release();

//---------绘图设备相关---------------

    //## 创建绘图设备，构造绘图环境。
    //## 直接调用Reset方法。
    bool Create(
        //## 设备（窗口）横宽（单位：像素点）
        int nWidth, 
        //## 设备（窗口）纵宽（单位：像素点）
        int nHeight, 
        //## 是否独占全屏幕
        bool bFullScreen);

    //## 重置绘图设备
    bool Reset(
        //## 设备（窗口）横宽（单位：像素点）
        int nWidth, 
        //## 设备（窗口）纵宽（单位：像素点）
        int nHeight, 
        //## 是否独占全屏幕
        bool bFullScreen);

//---------字体与文字输出---------------

    //## 创建一个字体对象。
	bool CreateAFont(
        //## 字库文件名。
        const char* pszFontFile, 
        //## 字库使用的字符编码集。
        CHARACTER_CODE_SET CharaSet, 
        //## 字体对象id.
        int nId);

    //## 输出文字。
    void OutputText(
        //## 使用的字体对象id。
        int nFontId, 
        //## 要输出的字符串。
        const char* psText, 
        //## 要输出的字符串的长度(BYTE)。
        //## 当nCount大于等于0时，字符串可以不包括特殊的结束符号来表示输出字符的结束。
        //## 当nCount小于0时，表示此字符串是以'\0'结尾，将根据结束字符来确定输出字符串的长度。
        //## 默认值为-1。
        int nCount, 
        //## 字符串显示起点坐标X，如果传入值为KF_FOLLOW，
        //## 则此字符串紧接在上次字符串的输出位置之后。
        //## 默认值为KF_FOLLOW。
        int nX, 
        //## 字符串显示起点坐标Y, 如果传入值为KF_FOLLOW，
        //## 此字符串与前一次输出字符串在同一行的位置。
        //## 默认值为KF_FOLLOW。
        int nY, 
        //## 字符串显示颜色，默认为黑色，用32bit数以ARGB的格
        //## 式表示颜色，每个分量8bit。
        unsigned int Color, 
        //## 自动换行的行宽限制，如果其值小于一个全角字符宽度则不做自动换行处理。默认值为0，既不做自动换行处理。
        int nLineWidth = 0,
		int nZ = TEXT_IN_SINGLE_PLANE_COORD,
		//字的边缘颜色
		unsigned int BorderColor = 0);

    //## 输出文字。
    int OutputRichText(
        //## 使用的字体对象id。
        int nFontId, 
		KOutputTextParam* pParam,
        //## 要输出的字符串。
        const char* psText, 
        //## 要输出的字符串的长度(BYTE)。
        //## 当nCount大于等于0时，字符串可以不包括特殊的结束符号来表示输出字符的结束。
        //## 当nCount小于0时，表示此字符串是以'\0'结尾，且中间不存在'\0'的字符。
        //## 默认值为-1
        int nCount = KRF_ZERO_END, 
        //##Documentation
        //## 自动换行的行宽限制，如果其值小于一个全角字符宽度则不做自动换行处理。默认值为0，既不做自动换行处理。
        int nLineWidth = 0);

    //## 返回指定坐标在字符串中最近的字符偏移
    int LocateRichText(
		//## 指定的坐标
		int nX, int nY,

        //## 使用的字体对象id。
        int nFontId, 
		KOutputTextParam* pParam,
        //## 要输出的字符串。
        const char* psText, 
        //## 要输出的字符串的长度(BYTE)。
        //## 当nCount大于等于0时，字符串可以不包括特殊的结束符号来表示输出字符的结束。
        //## 当nCount小于0时，表示此字符串是以'\0'结尾，且中间不存在'\0'的字符。
        //## 默认值为-1。		
        int nCount = KRF_ZERO_END, 
        //##Documentation
        //## 自动换行的行宽限制，如果其值小于一个全角字符宽度则不做自动换行处理。默认值为0，既不做自动换行处理。
        int nLineWidth = 0);

    //## 释放一个字体对象
    void ReleaseAFont(
        //## 字体对象的id
        int nId);

//---------图资源相关---------------

    //## 如果内存图形对象失败，则返回0，否则返回根据图形名字转换而成的数值id。
    unsigned int CreateImage(
        //##Documentation
        //## 图形的名字。
        const char* pszName, 
        //##Documentation
        //## 图形横宽
        int nWidth, 
        //##Documentation
        //## 图形纵宽
        int nHeight,
		//##Documentation
        //## 图形类型
        int nType);

    //## 指定释放某个图资源。
    void FreeImage(
        //## 图形文件名/图形名。
        const char* pszImage 
        );

    //## 释放全部的图形对象。
    void FreeAllImage();

    //## 获取图形像点数据缓冲区，返回空，则表示失败
    virtual void* GetBitmapDataBuffer(
        //## 图形名
        const char* pszImage,
		//用于获取图形数据缓冲区的相关信息数据，如果传入空指针，则忽略这些信息。
		KBitmapDataBuffInfo* pInfo);

	//##释放对(通过GetBitmapDataBuffer调用获取得的)图形像点数据缓冲区的控制
	virtual void ReleaseBitmapDataBuffer(
		//## 图形名
		const char* pszImage,
		//通过GetBitmapDataBuffer调用获取得的图形像点数据缓冲区指针
		void* pBuffer);

    //## 获取某个图形信息。
    bool GetImageParam(
        //## 图形的资源文件名/图形名
        const char* pszImage,
        //## 图形信息存储结构的指针
        KImageParam* pImageData,
		//## 图形类型
		int nType);
	
	//## 获取某个图形某帧的信息
	virtual bool GetImageFrameParam(
        //## 指向保存图形资源文件名/图形名的缓冲区
        const char* pszImage,
		//图形帧索引
		int		nFrame,
        //## 帧图形相对于整个图形的偏移
		KRPosition2* pOffset,
		//## 帧图形大小
		KRPosition2* pSize,
        //## 图资源类型
		int nType);

    //## 获取某个图形某帧上指定像点的alpha值，如果不形不存在，或者象点落在图形范围之外则返回0。
    int GetImagePixelAlpha(
        //## 图形资源文件名/图形名
        const char* pszImage, 
        //## 图形的帧索引。
        int nFrame, 
        //## 像点在图中横坐标
        int nX, 
        //## 像点在图中纵坐标
        int nY,
        //## 图形类型
        int nType);

    //## 设置图形动态加载平衡参数。
    void SetImageStoreBalanceParam(
        //## 加载图形的数目的平衡值。
        int nNumImage, 
        //## 每多少次引用图形对象后作一次平衡检查。
        unsigned int uCheckPoint = 1000);

    //## 保存图形到文件。保存文件只对ISI_T_BITMAP16格式的图形有效。返回指表明图形是否成功保存到文件。
    bool SaveImage(
        //## 保存文件的名字。
        const char* pszFile, 
        //## 图形名
        const char* pszImage, 
        //## 目标图形文件的类型
        int nFileType);
    
	
//---------绘制操作---------------
	
    //## 绘制图元
    void DrawPrimitives(
        //## 绘制的图元的数目
        int nPrimitiveCount,
        //## 描述图元的结构的数组
        KRepresentUnit* pPrimitives, 
        //## Primitive类型，取值来自枚举值REPRESENT_UNIT_TYPE
        unsigned int uGenre,
        //## 图元绘制操作提供的坐标是否为垂直与视线的单平面坐标。如果不是，则图元绘制操作提供的坐标是三维空间坐标。
        int bSinglePlaneCoord);

	void DrawPrimitivesOnImage(
        //## 绘制的图元的数目
        int nPrimitiveCount,
        //## 描述图元的结构的数组
        KRepresentUnit* pPrimitives, 
        //## Primitive类型，取值来自枚举值REPRESENT_UNIT_TYPE
        unsigned int uGenre, 
        //## 图形名
        const char* pszImage, 
        //## 目标图形的id
        unsigned int uImage,
		short &nImagePosition);

	//## 清除图形数据
	void ClearImageData(
        //## 图形名
        const char* pszImage, 
        //## 目标图形的id
        unsigned int uImage,
		short nImagePosition);

    //## 设置视觉观察的目的焦点。
    //## (把输入的坐标作调整使它在z=0平面上)，
    //## 把调整后的坐标作为绘图设备中心点对应的坐标，根据坐标计算出绘图设备左上角点对应的坐标：
    //## m_nLeft = nX - Width(绘图设备) / 2
    //## m_nTop = nY - Height(绘图设备) / 2 - nZ
    void LookAt(int nX, int nY, int nZ);

    //## 复制绘图设备上的图到图形对象。
    //## 目标图形资源对象的类型目前只能是ISI_T_BITMAP16。如果目标图形对象不存在或者复制失败则返回false。
    bool CopyDeviceImageToImage(
        //## 图形的名字。
        const char* pszName, 
        //## 绘图设备图形复制范围的左上角点横坐标
        int nDeviceX, 
        //## 绘图设备图形复制范围的左上角点纵坐标
        int nDeviceY, 
        //## 目的图形复制到范围的左上角点横坐标
        int nImageX, 
        //## 目的图形复制到范围的左上角点纵坐标
        int nImageY, 
        //## 复制范围的横宽
        int nWidth, 
        //## 复制范围的横宽
        int nHeight);

	//## 开始一轮绘制操作
	bool RepresentBegin(
		//## 是否清除设备上当前的图形。
		int bClear, 
		//## 如果bClear为非0值，则Color指出用什么颜色值来清除设备原来的图形。
		unsigned int Color);

	//## 结束一轮绘制操作
	void RepresentEnd();

	//视图/绘图设备坐标 转化为空间坐标
	virtual void ViewPortCoordToSpaceCoord(
		int& nX,	//传入：视图/绘图设备坐标的x量，传出：空间坐标的x量
		int& nY,	//传入：视图/绘图设备坐标的y量，传出：空间坐标的y量
		int  nZ		//（期望）得到的空间坐标的z量
		);
	
	//## 设置光照信息
	virtual void SetLightInfo(
		//## 当前9个区域左上角的x坐标
		int nX,
		//## 当前9个区域左上角的y坐标
		int nY,
		//## 当前9个区域的光照信息，DWORD数组，表示各网格节点上光照亮度值
		//## 如果pLighting为NULL，表示不做光照处理
		unsigned int *pLighting);
	
	//## 设置表现模块选项
	virtual void SetOption(
		//## 选项类型
		RepresentOption eOption,
		//## 选项是否打开
		bool bOn);
	
	//## 挂接嵌入文字的图片显示 [wxb 2003-6-20]
	virtual long AdviseRepresent(IInlinePicEngineSink*);
	virtual long UnAdviseRepresent(IInlinePicEngineSink*);
	
	virtual bool SaveScreenToFile(const char* pszName, ScreenFileType eType, unsigned int nQuality);
	
	virtual void SetGamma(int nGamma);
	
	//设置偏色列表
	virtual unsigned int SetAdjustColorList(
			unsigned int* puColorList,	//偏色的颜色列表，每个颜色为一个unsigned int数，
										//用32bit数以0RGB的格式表示颜色，每个分量8bit。
			unsigned int uCount			//颜色列表中颜色数的数目
			)
	{
		return uCount;
	}

private:
	class KRSFontItem
   {
      public:
        //## 字体对象的id，一般应用时就让id与字体的文字大小数值相关联，便于应用理解。
        int nId;

        //## 字体对象的指针。
        KFont3* pFontObj;

    };

	enum RS2_FONT_ITEM_PARAM { 
        //## 接口最多可以维护的字体对象的数目，操过此数目时，创建字体对象将失败。
        RS2_MAX_FONT_ITEM_NUM = 5 };

private:
	D3DGAMMARAMP m_ramp;				// 保存原始的gamma校准参数
	DWORD m_dwWindowStyle;				// 渲染窗口特性
	TextureResMgr m_TextureResMgr;		// 贴图资源管理器
	//## 字体对象的集合。
    KRSFontItem m_FontTable[RS2_MAX_FONT_ITEM_NUM];
	bool m_bDeviceLost;					// 渲染设备是否丢失的标志

	int m_nLeft;
    int m_nTop;

	LPDIRECT3DTEXTURE9 m_pPreRenderTexture128;	// 预渲染主角的贴图指针
	LPDIRECT3DTEXTURE9 m_pPreRenderTexture256;	// 预渲染主角的贴图指针
	LPDIRECT3DVERTEXBUFFER9 m_pVB2D;	// 顶点Buffer
	LPDIRECT3DVERTEXBUFFER9 m_pVB3D;	// 顶点Buffer
	D3DXMATRIX m_matView;				// 世界坐标到摄像机坐标系的变换矩阵
	D3DXMATRIX m_matViewInverse;		// 世界坐标到摄像机坐标系的变换矩阵的逆矩阵
	D3DXMATRIX m_matProj;				// 投影变换矩阵
	D3DXVECTOR3 m_vCamera;				// 摄像机位置
	D3DXVECTOR3 m_vCamera1;				// 摄像机位置

	bool m_bDoLighting;					// 是否做光照计算
	int m_nLightingAreaX;				// 当前9个区域左上角的x坐标
	int m_nLightingAreaY;				// 当前9个区域左上角的y坐标
	DWORD pLightingArray[LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT];	// 光照图信息数组

	// 性能统计相关

private:
	~KRepresentShell3();

	// 初始化设备对象
	bool InitDeviceObjects();

	// 释放设备对象
	void DeleteDeviceObjects();

	// 释放可能丢失的设备对象
	bool InvalidateDeviceObjects();

	// 恢复设备相关对象和状态
	bool RestoreDeviceObjects();

	void D3DTerm();
	 //## 空间坐标到二维(显示面)平面坐标的变换
    void CoordinateTransform(
        //## 传入三维坐标x分量，传出二维坐标x(水平)分量
        int& nX, 
        //## 传入三维坐标y分量，传出二维坐标y(垂直)分量
        int& nY, 
        //## 传入三维坐标z分量
        int nZ);
	
	// 根据g_renderModel设置投影变换矩阵
	void SetUpProjectionMatrix();

	// 渲染函数
	void DrawImage2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord);
	// 带缩放的2d绘制，用于video播放，只适用于单平面坐标和ISI_T_BITMAP16格式的资源
	void DrawImage2DStretch(int nPrimitiveCount, KRepresentUnit* pPrimitives);

	// 非透视模式绘制主角，预先绘制到贴图再绘制到屏幕
	void DrawPlayer2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord);
	
	// 透视模式绘制主角，预先绘制到贴图再绘制到屏幕
	void DrawPlayer3D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord);

	// 计算非透视模式下的主角外包矩形
	void GetBoundBox2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord, RECT &rect);

	// 计算透视模式下的主角外包矩形
	void GetBoundBox3D(int nPrimitiveCount, KRepresentUnit* pPrimitives, RECTFLOAT &rect);

	void DrawSprOnTexture2D(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord,
							RECT &rcBound, RECTFLOAT &rcRenderBound, int nTexSize, bool bLighting);
	void DrawImage3D(unsigned int uGenre, int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord);
	// bStretch是否缩放位图
	void DrawBitmap16(int32 nX, int32 nY, int32 nWidth, int32 nHeight, TextureResBmp* pBitmap, bool bStretch = false);
	void DrawSpriteAlpha(int32 nX, int32 nY, int32 nWidth, int32 nHeight, int32 nFrame, 
									TextureResSpr* pSprite, DWORD color, int32 nRenderStyle);
	void DrawSpritePartAlpha(int32 nX, int32 nY, int32 nWidth, int32 nHeight, int32 nFrame, TextureResSpr* pSprite, RECT &rect);
	void DrawBitmap16Part(int32 nX, int32 nY, int32 nWidth, int32 nHeight, TextureResBmp* pBitmap, RECT &rect);

	void DrawBitmap163D(RenderParam3D &param, TextureResBmp* pBitmap);
	void DrawBitmap163DLighting(RenderParam3D &param, TextureResBmp* pBitmap);

	void DrawSpriteAlpha3D(RenderParam3D &param, int32 nFrame, TextureResSpr* pSprite, 
												DWORD color, int32 nRenderStyle, RECT *rect);
	void DrawSpriteAlpha3DLighting(RenderParam3D &param, int32 nFrame, TextureResSpr* pSprite, 
										DWORD color, int32 nRenderStyle, RECT *rect);

	void __fastcall SetPoint3dLighting(VERTEX3D& pDes, VERTEX3D& pSrc, DWORD color);
	inline unsigned int GetPoint3dLighting(D3DXVECTOR3& v);
	unsigned int GetPoint3dLightingLerp(D3DXVECTOR3& v);
	

	void RIO_CopyBitmap16ToBuffer(TextureResBmp* pBitmap, TextureResBmp* pDestBitmap, int32 nX, int32 nY);
	void RIO_CopySprToBufferAlpha(TextureResSpr* pSpr, int32 nFrame, TextureResBmp* pDestBitmap, int32 nX, int32 nY);

	// 绘制一个实心矩形
	void DrawRect(int32 x1, int32 y1, int32 nWidth, int32 nHeight, DWORD color);

	// 绘制一批点
	void DrawPoint(int nPrimitiveCount, KRepresentUnit* pPrimitives, int bSinglePlaneCoord);

	// 绘制一条直线
	void DrawLine(int32 x1, int32 y1, int32 x2, int32 y2, DWORD color);

	// 绘制一个矩形线框
	void DrawRectFrame(int32 x1, int32 y1, int32 x2, int32 y2, DWORD color);
	// 生成网格，计算亮度
	uint32 BuildMesh3D(VERTEX3D *pvbSrc, VERTEX3D* pvbDes, uint32 nNumDes, DWORD color);
};

#endif
