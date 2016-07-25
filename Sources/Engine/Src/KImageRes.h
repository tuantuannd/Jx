#ifndef __KIMAGERES_H__
#define __KIMAGERES_H__

#include "KSprite.h"

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

//##ModelId=3DC0F495038B
//##Documentation
//## 图形类型定义
enum KIS_IMAGE_TYPE
{
    //##Documentation
    //## 16位单帧位图。
    //## ISI_T_BITMAP16的格式为 D3DFMT_R5G6B5 或者 D3DFMT_X1R5G5B5
    //## ，具体为哪种格式由iImageStore内部确定。可以通过方法iImageStore::IsBitmapFormat565来知道是用哪种格式。
    ISI_T_BITMAP16, 
    //##Documentation
    //##  spr格式的带alpha压缩图形，若干帧
    ISI_T_SPR,
	//##Documentation
    //##  可被修改的，spr格式图形，不作动态加载处理
	ISI_T_SPR_CUSTOM,
};

class KImageRes
{
public:

public:
	KImageRes();
	virtual ~KImageRes();

	// 将成员变量置为初始值
	virtual void ResetVar() = 0;

	// 创建内存资源
	virtual bool CreateImage(const char* szImage, int nWidth, int nHeight, unsigned int nType) = 0;
	// 从文件载入资源
	virtual bool LoadImage(char* szImage, unsigned int nType) = 0;

	// 释放内存
	virtual void Release() = 0;
	
	int GetWidth(){ return (int)m_nWidth; }
	int GetHeight(){ return (int)m_nHeight; }

//private:
public:
	unsigned int		m_nType;					// 资源类型
	unsigned int		m_nWidth;					// 资源宽度
	unsigned int		m_nHeight;					// 资源高度
	
	unsigned long m_nMemUsed;				// 内存占用数量,单位字节
	bool		m_bLastFrameUsed;			// 上一帧是否使用
};

class ImageResBmp : public KImageRes
{
public:

public:
	ImageResBmp();
	~ImageResBmp();

	// 将成员变量置为初始值
	virtual void ResetVar();

	// 创建内存资源
	virtual bool CreateImage(const char* szImage, int nWidth, int nHeight, unsigned int nType);
	// 从文件载入资源
	virtual bool LoadImage(char* szImage, unsigned int nType);

	// 释放内存
	virtual void Release();

//private:
public:
	BYTE *m_pData;		// 图象数据

private:
	bool LoadJpegFile(char* szImage);
	
};

class ImageResSpr : public KImageRes
{
public:

public:
	ImageResSpr();
	~ImageResSpr();

	// 将成员变量置为初始值
	virtual void ResetVar();

	// 创建内存资源
	virtual bool CreateImage(const char* szImage, int nWidth, int nHeight, unsigned int nType);
	// 从文件载入资源
	virtual bool LoadImage(char* szImage, unsigned int nType);

	// 释放内存
	virtual void Release();

	unsigned int GetCenterX(){ return m_Header.CenterX; }
	unsigned int GetCenterY(){ return m_Header.CenterY; }
	int GetFrameNum(){ return m_Header.Frames; }
	unsigned int GetDirections(){ return m_Header.Directions; };
	unsigned int GetInterval(){ return m_Header.Interval; };
	
	// 取得第nFrame帧图象x，y坐标上的象素点alpha值
	int GetPixelAlpha(int nFrame, int x, int y);

	// 准备一帧数据
	bool PrepareFrameData(char *pszImage, int nFrame);

//private:
public:
	bool		m_bInPackage;				// 这个spr是否在包内
	KPAL24*		m_pPal24;					// 调色板
	WORD*		m_pPal16;					// 4444调色板

	SPRHEAD		m_Header;					// spr头部信息
	SPRFRAME**	m_pFrameInfo;				// 帧到铁图映射信息数组指针

private:
	bool LoadSprFile(char* szImage);
};

#endif