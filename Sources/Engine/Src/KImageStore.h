#ifndef __KIMAGESTORE_H__
#define __KIMAGESTORE_H__

#include "KImageRes.h"

// STL
#pragma warning(disable : 4786)
#include <vector>
#include <list>
#include <fstream>
#ifndef __CODEWARRIOR
using namespace std;
#endif

//整数表示的二维点坐标
struct KRPosition2
{
	int nX;
	int nY;
};

//整数表示的三维点坐标
struct KRPosition3
{
	int nX;
	int nY;
	int nZ;
};

//##ModelId=3DB511F30242
//##Documentation
//## 图形的信息。
struct KImageParam
{
    //##ModelId=3DB512190144
    //##Documentation
    //## 图形的总的帧数目
    short nNumFrames;

    //##ModelId=3DB51226012F
    //##Documentation
    //## 图形的帧间隔。
    short nInterval;

    //##ModelId=3DB5123B0158
    //##Documentation
    //## 图形横宽（单位：像素点）。
    short nWidth;

    //##ModelId=3DB5123E036F
    //##Documentation
    //## 图形纵宽（单位：像素点）
    short nHeight;

    //##ModelId=3DB512900118
    //##Documentation
    //## 图形参考点（重心）的横坐标值。
    short nReferenceSpotX;

    //##ModelId=3DB512A70306
    //##Documentation
    //## 图形参考点（重心）的纵坐标值。
    short nReferenceSpotY;

    //##ModelId=3DB512CD0280
    //##Documentation
    //## 图形的帧分组数目（通常应用为图形方向数目）。
    short nNumFramesGroup;

};

enum IS_BALANCE_PARAM { 
    //## 扩展m_pObjectList，每次操作增加的元素的数目
    ISBP_EXPAND_SPACE_STEP = 128, 
    //## 每多少次引用图形对象后作一次平衡检查的默认值。
    ISBP_CHECK_POINT_DEF = 40, 
    //## 图形数目平衡的默认值
    ISBP_BALANCE_NUM_DEF128 = 24, 
	ISBP_BALANCE_NUM_DEF256 = 30, 
	ISBP_BALANCE_NUM_DEF512 = 42, 
    //## 靠近范围优先查找的范围大小。
    ISBP_TRY_RANGE_DEF = 8 };

class KImageRes;

// 资源链表的节点
struct ResNode
{
	unsigned int		m_nLastUsedTime;			// 上一次渲染的时间标签
	bool		m_bCacheable;				// 是否是无硬盘对应文件的资源
	unsigned int		m_nID;						// 资源ID，由资源名生成
	unsigned int		m_nType;					// 资源类型
	KImageRes*	m_pTextureRes;				// 指向资源的指针
};

class iImageStore
{
public:
	//## 检查图资源存储量的平衡状况。
    virtual void CheckBalance() = 0;

	//## 设置图形动态加载平衡参数。
    virtual void SetBalanceParam(
        //## 加载图形的数目的平衡值。
        int nNumImage, 
        //## 每多少次引用图形对象后作一次平衡检查。
        unsigned int uCheckPoint = 3000) = 0;

	//## 如果内存图形对象失败，则返回0，否则返回根据图形名字转换而成的数值id。
    virtual unsigned int CreateImage(
        //## 图形的名字。
        const char* pszName, 
        //## 图形横宽
        int nWidth, 
         //## 图形纵宽
        int nHeight, 
        //## 图形类型
        int nType) = 0;

	//## 指定释放某个图资源。
    virtual void FreeImage(
        //## 图形文件名/图形名。
        const char* pszImage) = 0;

	//## 清空释放全部的图形对象，既包括动态加载的，也包括通过调用CreateImage生成的。
    virtual void Free() = 0;

	//## 获取某个图形资源对象的信息。
    virtual bool GetImageParam(
        //## 指向保存图形资源文件名/图形名的缓冲区
        const char* pszImage,
        //## 图形信息存储结构的指针
        KImageParam* pImageData,
        //## 图资源类型
		int nType) = 0;
	
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
		int nType) = 0;

	//## 图形的数据对象指针。
    virtual KImageRes* GetImage(
        //## 图资源文件名称/图形名。
        const char* pszImage, 
        //## 图形id
        unsigned int& uImage, 
        //## 图形原来在iImageStore内的位置。
        int& nImagePosition, 
        //## 要获取图形的帧的索引。
        int nFrame,
        //## 图资源类型
        int nType) = 0;
	
	//## 获取图形指定帧的某个像点的alpha值。
    virtual int GetImagePixelAlpha(
        //## 图形资源文件名/图形名
        const char* pszImage, 
        //## 图形的帧索引。
        int nFrame, 
        //## 像点在图中横坐标
        int nX, 
        //## 像点在图中纵坐标
        int nY, 
        //## 图资源类型
        int nType) = 0;

	// 开始记录统计信息
	virtual void StartProfile() = 0;

	// 结束记录统计信息
	virtual void EndProfile() = 0;

	// 获得统计信息字符串
	virtual void GetProfileString(char* str, int BufLen) = 0;
	
	virtual bool InvalidateDeviceObjects() = 0;
	virtual bool RestoreDeviceObjects() = 0;
};

class KImageStore : public iImageStore
{
public:
	unsigned int m_nLoadCount;
	unsigned int m_nReleaseCount;
public:
	KImageStore();
	~KImageStore();

	//## 检查图资源存储量的平衡状况。
    void CheckBalance();

	//## 设置图形动态加载平衡参数。
    void SetBalanceParam(
        //## 加载图形的数目的平衡值。
        int nNumImage, 
        //## 每多少次引用图形对象后作一次平衡检查。
        unsigned int uCheckPoint = 3000);

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

	//## 指定释放某个图资源。
    void FreeImage(
        //## 图形文件名/图形名。
        const char* pszImage);

	//## 清空释放全部的图形对象，既包括动态加载的，也包括通过调用CreateImage生成的。
    void Free();

	//## 获取某个图形资源对象的信息。
    bool GetImageParam(
        //## 指向保存图形资源文件名/图形名的缓冲区
        const char* pszImage,
        //## 图形信息存储结构的指针
        KImageParam* pImageData,
        //## 图资源类型
		int nType);
	
	bool GetImageFrameParam(
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

	//## 图形的数据对象指针。
    KImageRes* GetImage(
        //## 图资源文件名称/图形名。
        const char* pszImage, 
        //## 图形id
        unsigned int& uImage, 
        //## 图形原来在iImageStore内的位置。
        int& nImagePosition, 
        //## 要获取图形的帧的索引。
        int nFrame,
        //## 图资源类型
        int nType);
	
	//## 获取图形指定帧的某个像点的alpha值。
    int GetImagePixelAlpha(
        //## 图形资源文件名/图形名
        const char* pszImage, 
        //## 图形的帧索引。
        int nFrame, 
        //## 像点在图中横坐标
        int nX, 
        //## 像点在图中纵坐标
        int nY, 
        //## 图资源类型
        int nType);

	bool InvalidateDeviceObjects();
	bool RestoreDeviceObjects();

private:
	//## 获得给定id在纪录表中的位置。
    //## 返回值含义：
    //##  返回值 >= 0 --> 给定的id在纪录表中的位置（以0开始索引数值）
    //##  返回值 < 0  --> 表中没有给定的id，如果需要插入此id的话，插入位置应该为(-返回值-1)
    int FindImage(
        //## 图形的id
        unsigned int uImage, 
        //## 图形纪录在m_pObjectList中可能的位置。
        int nPossiblePosition);

	//## 生成图形对象，载入指定图形资源，如果载入失败，则销毁刚生成的对象。
    KImageRes* LoadImage(
        //## 图形文件名
        const char* pszImageFile, 
        //## 图形类型
        unsigned int nType) const;

private:
	//## 对图形对象访问总次数的累加纪录值。
    DWORD m_tmLastCheckBalance;
	//## 加载在内存中图形数目的平衡值
    int m_nBalanceNum;
	//## 调整平衡的时刻设置。
    unsigned int m_uCheckPoint;
	vector<ResNode>	m_TextureResList;	// 贴图资源链表
	
	// 用于性能统计
	unsigned int m_nHitCount;					// 命中计数
	unsigned int m_nUseCount;					// 使用计数
	bool m_bDoProfile;					// 是否在性能统计区间
	float m_fCacheMemUsed;				// 缓冲区消耗的内存,单位M
	float m_fCacheMemUseful;			// 缓冲区消耗内存的有用部分（排除了贴图的空白区域）,单位M
	float m_fMemDrawingUsed;			// 上一帧用于渲染的内存（包括贴图的空白区域）,单位M
	float m_fHitPercent;				// 命中率
};

#endif