#ifndef __TEXTURERESMGR_H__
#define __TEXTURERESMGR_H__

#include <vector>
using namespace std;

#include "../../Engine/Src/KCriticalSection.h"

enum IS_BALANCE_PARAM { 
    //## 扩展m_pObjectList，每次操作增加的元素的数目
    ISBP_EXPAND_SPACE_STEP = 128, 
    //## 每多少次引用图形对象后作一次平衡检查的默认值。
    ISBP_CHECK_POINT_DEF = 24, 
    //## 图形数目平衡的默认值
    ISBP_BALANCE_NUM_DEF128 = 16 * 1024 * 1024, 
	ISBP_BALANCE_NUM_DEF256 = 32 * 1024 * 1024, 
	ISBP_BALANCE_NUM_DEF512 = 48 * 1024 * 1024, 
    //## 靠近范围优先查找的范围大小。
    ISBP_TRY_RANGE_DEF = 8 };

class TextureRes;

// 资源链表的节点
struct ResNode
{
	uint32		m_nLastUsedTime;			// 上一次渲染的时间标签
	bool		m_bCacheable;				// 是否是无硬盘对应文件的资源
	uint32		m_nID;						// 资源ID，由资源名生成
	uint32		m_nType;					// 资源类型
	TextureRes*	m_pTextureRes;				// 指向资源的指针
};

class TextureResMgr
{
public:
	uint32 m_nLoadCount;
	uint32 m_nReleaseCount;
    uint32 m_nMaxReleaseCount;
public:
	TextureResMgr();
	~TextureResMgr();

	//## 检查图资源存储量的平衡状况。
    void CheckBalance();

	//## 设置图形动态加载平衡参数。
    void SetBalanceParam(
        //## 加载图形的数目的平衡值。
        int32 nNumImage, 
        //## 每多少次引用图形对象后作一次平衡检查。
        uint32 uCheckPoint = 3000);

	//## 如果内存图形对象失败，则返回0，否则返回根据图形名字转换而成的数值id。
    uint32 CreateImage(
        //## 图形的名字。
        const char* pszName, 
        //## 图形横宽
        int32 nWidth, 
         //## 图形纵宽
        int32 nHeight, 
        //## 图形类型
        int32 nType);

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
    TextureRes* GetImage(
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
		//## 是否保证这一帧贴图可用
		bool bPrepareTex = true);
	
	//## 获取图形指定帧的某个像点的alpha值。
    int32 GetImagePixelAlpha(
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

	// 开始记录统计信息
	void StartProfile();

	// 结束记录统计信息
	void EndProfile();

	// 获得统计信息字符串
	void GetProfileString(char* str, int BufLen);
	
	bool InvalidateDeviceObjects();
	bool RestoreDeviceObjects();

private:
	//## 获得给定id在纪录表中的位置。
    //## 返回值含义：
    //##  返回值 >= 0 --> 给定的id在纪录表中的位置（以0开始索引数值）
    //##  返回值 < 0  --> 表中没有给定的id，如果需要插入此id的话，插入位置应该为(-返回值-1)
    int FindImage(
        //## 图形的id
        uint32 uImage, 
        //## 图形纪录在m_pObjectList中可能的位置。
        int nPossiblePosition);

	//## 生成图形对象，载入指定图形资源，如果载入失败，则销毁刚生成的对象。
    TextureRes* LoadImage(
        //## 图形文件名
        const char* pszImageFile, 
        //## 图形类型
        uint32 nType) const;

private:
	//## 对图形对象访问总次数的累加纪录值。
    DWORD m_tmLastCheckBalance;
	//## 加载在内存中图形数目的平衡值
    int32 m_nBalanceNum;
	//## 调整平衡的时刻设置。
    uint32 m_uCheckPoint;
	vector<ResNode>	m_TextureResList;	// 贴图资源链表
	
	// 用于性能统计
	uint32 m_nHitCount;					// 命中计数
	uint32 m_nUseCount;					// 使用计数
	bool m_bDoProfile;					// 是否在性能统计区间
	uint32 m_uTexCacheMemUsed;			// 缓冲区消耗的内存,单位字节
	uint32 m_uMemDrawingUsed;			// 上一帧用于渲染的内存（包括贴图的空白区域）,单位字节

    KCriticalSection    m_ImageProcessLock;
};

#endif