/*****************************************************************************************
//  无贴图的图形资源管理
//	Copyright : Kingsoft 2002-2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-11-11
------------------------------------------------------------------------------------------
*****************************************************************************************/
#ifndef KIMAGESTORE2_H_HEADER_INCLUDED_C23109DC
#define KIMAGESTORE2_H_HEADER_INCLUDED_C23109DC

#include "../iRepresent/KRepresentUnit.h"
#include "../../Engine/Src/KCriticalSection.h"
#include <ddraw.h>
struct KImageParam;

//## 无贴图的图形资源管理。
//## 里面相同id的图形对象只存一个。
class KImageStore2
{
public:
    KImageStore2();
    ~KImageStore2();

//====针对ImageStore整体的操作====

	//## 初始化iResStore接口对象。
    bool Init();

    //## 清空释放全部的图形对象，既包括动态加载的，也包括通过调用CreateImage生成的。
    void Free();

    //## 设置图形动态加载平衡参数。
    void SetBalanceParam(
        //## 加载图形的数目的平衡值。
        int nNumImage, 
        //## 每多少次引用图形对象后作一次平衡检查。
		unsigned int uCheckPoint = 10000);

//====针对单个图形的操作====

	//## 如果内存图形对象失败，则返回0，否则返回根据图形名字转换而成的数值id。
    unsigned int CreateImage(
        //## 图形的名字。
        const char* pszName, 
        //## 图形横宽
        int nWidth, 
        //## 图形纵宽
        int nHeight, 
        //## 图形类型
        int nType);

    //## 获取已经存在的通过CreateImage创建的图形
    void* GetExistedCreateBitmap(
		//## 图形名。
        const char* pszImage,
		//## 图形id
		unsigned int uImage,
		//## 图形原来在iImageStore内的位置。
        short& nImagePosition);

	bool CreateBitmapSurface(
		//## 图形名。
        const char* pszImage,
		//## 图形id
		unsigned int& uImage,
		//## 图形原来在iImageStore内的位置。
        short& nImagePosition);

	//## 指定释放某个图资源。
    void FreeImage(
        //## 图形文件名/图形名。
        const char* pszImage);

    //## 获取图形的数据缓冲区。
    void* GetImage(
        //## 图资源文件名称/图形名。
        const char* pszImage, 
        //## 图形id
        unsigned int& uImage, 
        //## 图形原来在iImageStore内的位置。
        short& nImagePosition, 
        //## 要获取图形的帧的索引。
        int nFrame,
        //## 图资源类型
        int nType,
		//额外需要的帧数据buffer
		void*& pFrameData);

    //## 获取某个图形资源对象的信息。
    bool GetImageParam(
        //## 指向保存图形资源文件名/图形名的缓冲区
        const char* pszImage,
        //## 图资源类型
		int nType,
		//## 图形信息存储结构的指针
        KImageParam* pImageData);
	
	//## 获取某个图形资源某帧的信息。
	bool GetImageFrameParam(
        //## 指向保存图形资源文件名/图形名的缓冲区
        const char* pszImage,
        //## 图资源类型
		int nType,
		//图形帧索引
		int		nFrame,
        //## 帧图形相对于整个图形的偏移
		KRPosition2* pOffset,
		//## 帧图形大小
		KRPosition2* pSize);

    //## 获取图形指定帧的某个像点的alpha值。
    int GetImagePixelAlpha(
        //## 图形资源文件名/图形名
        const char* pszImage, 
        //## 图资源类型
        int nType,
		//## 图形的帧索引。
        int nFrame, 
        //## 像点在图中横坐标
        int nX, 
        //## 像点在图中纵坐标
        int nY);

    //## 保存图形到文件。保存文件只对ISI_T_BITMAP16格式的图形有效。返回指表明图形是否成功保存到文件。
    bool SaveImage(
        //## 保存文件的名字。
        const char* pszFile, 
        //## 图形名
        const char* pszImage, 
        //## 目标图形文件的类型
        int nFileType);

	unsigned int SetAdjustColorList(unsigned int* puColorList, unsigned int uCount);

	char* GetAdjustColorPalette(
		int nISPosition,	//图形在iImageStore内的位置。
		unsigned uColor);//偏色量

private:

	struct _KISImageFrameObj
	{        
		bool			bRef;	//在本次绘图循环中是否被引用到
		bool			bReserved;
		unsigned short	sOffTableSize;
		union
		{
			//图形帧数据偏移表，表后紧接各帧数据
			void*			pOffsetTable;
			//帧数据缓冲区
			void*			pFrameData;
		};
	};

    //## 存储保存图形id、类型以及对象指针的结构
    struct _KISImageObj
    {
        //## 图形的id
        unsigned int uId;
		//## 表明为在内存中创建的图形，不能从磁盘动态加载，不作cache处理。
        bool bNotCacheable;
		//是否按单帧加载
        bool bSingleFrameLoad;
		//在本次绘图循环中是否被引用到
		bool bRef;
        //## 图形的类型
        unsigned char bType;
        //## 图形数据对象指针。
        void* pObject;
		union
		{
			//偏色色盘, ISI_T_SPR格式的图用
			char* pcAdjustColorPalettes;
			//DirectDraw Surface, ISI_T_BITMAP16格式的图用
            LPDIRECTDRAWSURFACE pSurface;
		};
		//图形的帧对象
		_KISImageFrameObj* pFrames;
    };

private:

	//## 扩展m_pObjectList的空间，使得存储更多图资源引用表述对象。
    bool ExpandSpace();

    //## 检查图资源存储量的平衡状况。
    void CheckBalance();

    //## 获得给定id在纪录表中的位置。
    //## 返回值含义：
    //##  返回值 >= 0 --> 给定的id在纪录表中的位置（以0开始索引数值）
    //##  返回值 < 0  --> 表中没有给定的id，如果需要插入此id的话，插入位置应该为(-返回值-1)
    int FindImage(
        //##Documentation
        //## 图形的id
        unsigned int uImage, 
        //##Documentation
        //## 图形纪录在m_pObjectList中可能的位置。
        int nPossiblePosition);

	//获取图形对象的某一帧的数据缓冲区
	void* GetSprFrame(
		const char* pszImageFile,
		_KISImageObj& ImgObject,
		int nFrame);

    //## 释放指定的图形对象。
    void FreeImageObject(
        //## 图形对象指针。
        _KISImageObj& ImgObject,
		//释放此图形的哪帧数据，如果传入值为负数，表示释放整个图形
		int nFrame = -1);

    //## 生成图形对象，载入指定图形资源，返回失败与否。
    void* LoadImage(
        //## 图形文件名
        const char* pszImageFile, 
        //## 图形类型
        _KISImageObj& ImgObj,
		//载入图形的哪一帧数据
		int nFrame,
		//用于获取帧数据的缓冲区指针
		void*& pFrameData);

	//文件名转化为字符串
	unsigned int ImageNameToId(const char* pszName);

	char* CreateAdjustColorPalette(const char* pOrigPalette, int nNumColor);

	//清除全部的偏色色盘
	void ClearAllAdjustColorPalette();

private:

    enum IS_BALANCE_PARAM
	{
        //## 扩展m_pObjectList，每次操作增加的元素的数目
        ISBP_EXPAND_SPACE_STEP = 128,
        //## 每多少次引用图形对象后作一次平衡检查的默认值。
        ISBP_CHECK_POINT_DEF = 256 + 128, 
        //## 图形数目平衡的默认值
        ISBP_BALANCE_NUM_DEF = 256 + 128,// + 128 + 512, 
        //## 靠近范围优先查找的范围大小。
        ISBP_TRY_RANGE_DEF = 8,
	};

private:

    //## 保存图形对象结构的线性表。内容依据图形的id按照升序排列。
    _KISImageObj* m_pObjectList;

    //## m_pObjectList向量中包含_KISImageObj元素的数目。暨当前向量做多可存储图形信息的数目。m_pObjectList空间大小改变时同时调整此值。
    int m_nNumReserved;

    //## 在载的图形资源的数目。这些图形的信息紧挨着存储在m_pObjectList从头开始的位置，按照图形的id升序排列。
    int m_nNumImages;

    //## 对图形对象访问总次数的累加纪录值。
    unsigned int m_uImageAccessCounter;

    //## 调整平衡的时刻设置。
    unsigned int m_uCheckPoint;

    //## 加载在内存中图形数目的平衡值
	int m_nBalanceNum;

	#define	MAX_ADJUSTABLE_COLOR_NUM	8
	unsigned int	m_uSprAdjustColorList[MAX_ADJUSTABLE_COLOR_NUM];
	unsigned int	m_uNumSprAdustColor;

    KCriticalSection    m_ImageProcessLock;

};


#endif /* KIMAGESTORE2_H_HEADER_INCLUDED_C23109DC */
