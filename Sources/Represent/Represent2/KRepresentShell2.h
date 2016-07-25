/*****************************************************************************************
//  表现模块的对外接口的二维版本实现。
//	Copyright : Kingsoft 2002-2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-11-11
------------------------------------------------------------------------------------------
*****************************************************************************************/

#ifndef KREPRESENTSHELL2_H_HEADER_INCLUDED_C2314D70
#define KREPRESENTSHELL2_H_HEADER_INCLUDED_C2314D70
#include "../iRepresent/iRepresentShell.h"
#include "KImageStore2.h"
#include "../../Engine/Src/KEngine.h"
#include "../iRepresent/Text/TextProcess.h"

struct KImageParam;
struct iFont;

//##ModelId=3DB6678F0243
//##Documentation
//## 表现模块的对外接口的二维版本实现。
class KRepresentShell2 : public iRepresentShell
{
public:
	//##ModelId=3DD20C90004D
	KRepresentShell2();
//=======================================
//=====iRepresentShell定义的接口函数=====
//=======================================

    //##ModelId=3DCA72E102FE
    //## 释放销毁接口对象
    //##Documentation
    //## 释放销毁接口对象
    void Release();

//---------绘图设备相关---------------

    //##ModelId=3DB6692300B3
    //## 创建绘图设备，构造绘图环境。
    //## 直接调用Reset方法。
    bool Create(
        //##Documentation
        //## 设备（窗口）横宽（单位：像素点）
        int nWidth, 
        //##Documentation
        //## 设备（窗口）纵宽（单位：像素点）
        int nHeight, 
        //##Documentation
        //## 是否独占全屏幕
        bool bFullScreen);

    //##ModelId=3DB69EC0023A
    //##Documentation
    //## 重置绘图设备
    bool Reset(
        //##Documentation
        //## 设备（窗口）横宽（单位：像素点）
        int nWidth, 
        //##Documentation
        //## 设备（窗口）纵宽（单位：像素点）
        int nHeight, 
        //##Documentation
        //## 是否独占全屏幕
        bool bFullScreen);

//---------字体与文字输出---------------

    //##ModelId=3DCA0B230317
    //##Documentation
    //## 创建一个字体对象。
	bool CreateAFont(
        //##Documentation
        //## 字库文件名。
        const char* pszFontFile, 
        //##Documentation
        //## 字库使用的字符编码集。
        CHARACTER_CODE_SET CharaSet, 
        //##Documentation
        //## 字体对象id.
        int nId);

    //##ModelId=3DCA0BAE00E4
    //##Documentation
    //## 输出文字。
    void OutputText(
        //##Documentation
        //## 使用的字体对象id。
        int nFontId, 
        //##Documentation
        //## 要输出的字符串。
        const char* psText, 
        //##Documentation
        //## 要输出的字符串的长度(BYTE)。
        //## 当nCount大于等于0时，字符串可以不包括特殊的结束符号来表示输出字符的结束。
        //## 当nCount小于0时，表示此字符串是以'\0'结尾，将根据结束字符来确定输出字符串的长度。
        //## 默认值为-1。
        int nCount, 
        //##Documentation
        //## 字符串显示起点坐标X，如果传入值为KF_FOLLOW，
        //## 则此字符串紧接在上次字符串的输出位置之后。
        //## 默认值为KF_FOLLOW。
        int nX, 
        //##Documentation
        //## 字符串显示起点坐标Y, 如果传入值为KF_FOLLOW，
        //## 此字符串与前一次输出字符串在同一行的位置。
        //## 默认值为KF_FOLLOW。
        int nY, 
        //##Documentation
        //## 字符串显示颜色，默认为黑色，用32bit数以ARGB的格
        //## 式表示颜色，每个分量8bit。
        unsigned int Color, 
        //##Documentation
        //## 自动换行的行宽限制，如果其值小于一个全角字符宽度则不做自动换行处理。默认值为0，既不做自动换行处理。
        int nLineWidth = 0,
		int nZ = TEXT_IN_SINGLE_PLANE_COORD,
		//字的边缘颜色
		unsigned int BorderColor = 0);

	//##ModelId=3DB655B2000E
    //##Documentation
    //## 输出文字。
    int OutputRichText(
		//##Documentation
        //## 使用的字体对象id。
        int nFontId, 
		KOutputTextParam* pParam,
		//##Documentation
        //## 要输出的字符串。
        const char* psText, 
        //##Documentation
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

    //##ModelId=3DCA0B8102F3
    //##Documentation
    //## 释放一个字体对象
    void ReleaseAFont(
        //##Documentation
        //## 字体对象的id
        int nId);

//---------图资源相关---------------

    //##ModelId=3DCD8DEA01BB
    //##Documentation
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

    //##ModelId=3DCD8EF60316
    //##Documentation
    //## 指定释放某个图资源。
    void FreeImage(
		//##Documentation
        //## 图形文件名/图形名。
        const char* pszImage);

    //##ModelId=3DCD8E9200E8
    //##Documentation
    //## 释放全部的图形对象。
    void FreeAllImage();

    //##ModelId=3DCD8FA900EE
    //##Documentation
    //## 获取图形像点数据缓冲区
    void* GetBitmapDataBuffer(
        //##Documentation
        //## 图形名
        const char* pszImage,
		//用于获取图形数据缓冲区的相关信息数据，如果传入空指针，则忽略这些信息。
		KBitmapDataBuffInfo* pInfo);

	//##释放对(通过GetBitmapDataBuffer调用获取得的)图形像点数据缓冲区的控制
	void ReleaseBitmapDataBuffer(
		//## 图形名
		const char* pszImage,
		//通过GetBitmapDataBuffer调用获取得的图形像点数据缓冲区指针
		void* pBuffer);

    //##ModelId=3DCA6EBC000F
    //##Documentation
    //## 获取某个图形信息。
    bool GetImageParam(
        //##Documentation
        //## 图形的资源文件名/图形名
        const char* pszImage,
        //##Documentation
        //## 图形信息存储结构的指针
        KImageParam* pImageData,
		//##Documentation
		//## 图形类型
		int nType);

    //##Documentation
    //## 获取某个图形某帧的信息
	bool GetImageFrameParam(
        //##Documentation
        //## 指向保存图形资源文件名/图形名的缓冲区
        const char* pszImage,
		//##Documentation
		//图形帧索引
		int		nFrame,
        //##Documentation
        //## 帧图形相对于整个图形的偏移
		KRPosition2* pOffset,
		//## 帧图形大小
		KRPosition2* pSize,
        //##Documentation
        //## 图资源类型
		int nType);

    //##ModelId=3DCA72620157
    //##Documentation
    //## 获取某个图形某帧上指定像点的alpha值，如果不形不存在，或者象点落在图形范围之外则返回0。
    int GetImagePixelAlpha(
        //##Documentation
        //## 图形资源文件名/图形名
        const char* pszImage, 
        //##Documentation
        //## 图形的帧索引。
        int nFrame, 
        //##Documentation
        //## 像点在图中横坐标
        int nX, 
        //##Documentation
        //## 像点在图中纵坐标
        int nY,
		//##Documentation
        //## 图形类型
        int nType);

    //##ModelId=3DCD90F30011
    //##Documentation
    //## 设置图形动态加载平衡参数。
    void SetImageStoreBalanceParam(
        //##Documentation
        //## 加载图形的数目的平衡值。
        int nNumImage, 
        //##Documentation
        //## 每多少次引用图形对象后作一次平衡检查。
        unsigned int uCheckPoint = 10000);

    //##ModelId=3DCD90910361
    //##Documentation
    //## 保存图形到文件。保存文件只对ISI_T_BITMAP16格式的图形有效。返回指表明图形是否成功保存到文件。
    bool SaveImage(
        //##Documentation
        //## 保存文件的名字。
        const char* pszFile, 
        //##Documentation
        //## 图形名
        const char* pszImage, 
        //##Documentation
        //## 目标图形文件的类型
        int nFileType);
    
	
//---------绘制操作---------------
	
	//##ModelId=3DB69FE401DA
    //##Documentation
    //## 绘制图元
    void DrawPrimitives(
        //##Documentation
        //## 绘制的图元的数目
        int nPrimitiveCount,
        //##Documentation
        //## 描述图元的结构的数组
        KRepresentUnit* pPrimitives, 
        //##Documentation
        //## Primitive类型，取值来自枚举值REPRESENT_UNIT_TYPE
        unsigned int uGenre,
        //##Documentation
        //## 图元绘制操作提供的坐标是否为垂直与视线的单平面坐标。如果不是，则图元绘制操作提供的坐标是三维空间坐标。
        int bSinglePlaneCoord);

	void DrawPrimitivesOnImage(
		//##Documentation
        //## 绘制的图元的数目
        int nPrimitiveCount,
        //##Documentation
        //## 描述图元的结构的数组
        KRepresentUnit* pPrimitives, 
        //##Documentation
        //## Primitive类型，取值来自枚举值REPRESENT_UNIT_TYPE
        unsigned int uGenre, 
		//##Documentation
        //## 图形名
        const char* pszImage, 
        //##Documentation
        //## 目标图形的id
        unsigned int uImage,
		short&	nImagePosition);

	//##Documentation
	//## 清除图形数据
	void ClearImageData(
		//##Documentation
        //## 图形名
        const char* pszImage, 
        //##Documentation
        //## 目标图形的id
        unsigned int uImage,
		short nImagePosition);

    //##ModelId=3DC0A08D0085
    //##Documentation
    //## 设置视觉观察的目的焦点。
    //## (把输入的坐标作调整使它在z=0平面上)，
    //## 把调整后的坐标作为绘图设备中心点对应的坐标，根据坐标计算出绘图设备左上角点对应的坐标：
    //## m_nLeft = nX - Width(绘图设备) / 2
    //## m_nTop = nY - Height(绘图设备) / 2 - nZ
    void LookAt(int nX, int nY, int nZ);

    //##ModelId=3DD00EEE0149
    //##Documentation
    //## 复制绘图设备上的图到图形对象。
    //## 目标图形资源对象的类型目前只能是ISI_T_BITMAP16。如果目标图形对象不存在或者复制失败则返回false。
    bool CopyDeviceImageToImage(
        //##Documentation
        //## 图形的名字。
        const char* pszName, 
        //##Documentation
        //## 绘图设备图形复制范围的左上角点横坐标
        int nDeviceX, 
        //##Documentation
        //## 绘图设备图形复制范围的左上角点纵坐标
        int nDeviceY, 
        //##Documentation
        //## 目的图形复制到范围的左上角点横坐标
        int nImageX, 
        //##Documentation
        //## 目的图形复制到范围的左上角点纵坐标
        int nImageY, 
        //##Documentation
        //## 复制范围的横宽
        int nWidth, 
        //##Documentation
        //## 复制范围的横宽
        int nHeight);

	//##ModelId=3DD20C45002A
	//##Documentation
	//## 开始一轮绘制操作
	bool RepresentBegin(
		//##Documentation
		//## 是否清除设备上当前的图形。
		int bClear, 
		//##Documentation
		//## 如果bClear为非0值，则Color指出用什么颜色值来清除设备原来的图形。
		unsigned int Color);

	//##ModelId=3DD20C450066
	//##Documentation
	//## 结束一轮绘制操作
	void RepresentEnd();

	//视图/绘图设备坐标 转化为空间坐标
	void ViewPortCoordToSpaceCoord(
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
		unsigned int *pLighting){}
	
	//## 设置表现模块选项
	virtual void SetOption(
		//## 选项类型
		RepresentOption eOption,
		//## 选项是否打开
		bool bOn){}
	virtual HRESULT AdviseRepresent(IInlinePicEngineSink*);
	virtual HRESULT UnAdviseRepresent(IInlinePicEngineSink*);
	
	virtual bool SaveScreenToFile(const char* pszName, ScreenFileType eType, unsigned int nQuality);
	virtual void SetGamma(int nGamma){}

	//设置偏色列表
	virtual unsigned int SetAdjustColorList(
			unsigned int* puColorList,	//偏色的颜色列表，每个颜色为一个unsigned int数，
										//用32bit数以0RGB的格式表示颜色，每个分量8bit。
			unsigned int uCount			//颜色列表中颜色数的数目
			);
private:

    //##ModelId=3DB68CEB0350
    class KRSFontItem
   {
      public:
        //##ModelId=3DB68D04025D
        //##Documentation
        //## 字体对象的id，一般应用时就让id与字体的文字大小数值相关联，便于应用理解。
        int nId;

        //##ModelId=3DB68D1D0323
        //##Documentation
        //## 字体对象的指针。
        iFont* pFontObj;

    };

    //##ModelId=3DCFECA1036D
    enum RS2_FONT_ITEM_PARAM { 
        //##Documentation
        //## 接口最多可以维护的字体对象的数目，操过此数目时，创建字体对象将失败。
        RS2_MAX_FONT_ITEM_NUM = 5 };

private:

    //##ModelId=3DB66AD30124
    KCanvas m_Canvas;

    //##ModelId=3DB66AEA03BD
    KDirectDraw m_DirectDraw;

    //##ModelId=3DB6A25A0137
    //##Documentation
    //## 图形资源维护模块对象。
    KImageStore2 m_ImageStore;

    //##ModelId=3DCA0E4902B3
    int m_nLeft;

    //##ModelId=3DCA0E520312
    int m_nTop;

	//##ModelId=3DB68D8701FE
    //##Documentation
    //## 字体对象的集合。
    KRSFontItem m_FontTable[RS2_MAX_FONT_ITEM_NUM];

	KTextProcess	m_TextProcess;

private:

	//##ModelId=3DD20C900089
	~KRepresentShell2();
    //##ModelId=3DCFED410049
    //##Documentation
    //## 空间坐标到二维(显示面)平面坐标的变换
    void CoordinateTransform(
        //##Documentation
        //## 传入三维坐标x分量，传出二维坐标x(水平)分量
        int& nX, 
        //##Documentation
        //## 传入三维坐标y分量，传出二维坐标y(垂直)分量
        int& nY, 
        //##Documentation
        //## 传入三维坐标z分量
        int nZ);
};


#endif /* KREPRESENTSHELL2_H_HEADER_INCLUDED_C2314D70 */
