#ifndef __TEXTURERES_H__
#define __TEXTURERES_H__

struct TextureInfo
{
	uint32	nWidth;						// 贴图宽度
	uint32	nHeight;					// 贴图高度
	LPDIRECT3DTEXTURE9 pTexture;		// 贴图指针
	uint32	nFrameX;					// 在这一帧图象上的左上角位置
	uint32	nFrameY;					// 在这一帧图象上的左上角位置
	int32	nFrameWidth;				// 贴图上填充的数据宽度
	int32	nFrameHeight;				// 贴图上填充的数据高度
};

struct FrameToTexture
{
	int32	nTexNum;					// 这一帧拆分为几张贴图
	int32	nWidth;						// 这一帧的宽度
	int32	nHeight;					// 这一帧的高度
	int32	nOffX;						// 相对于整个spr的X坐标偏移
	int32	nOffY;						// 相对于整个spr的Y坐标偏移
	TextureInfo texInfo[4];				// 贴图信息数组，一级拆分，最多4张贴图
	int32	nRawDataLen;				// 原始数据长度
	BYTE	*pRawData;					// 这一帧的原始数据
	void	*pFrame;					// 保存SprGetFrame返回的指针，供SprReleaseFrame使用
};

class TextureRes
{
public:

public:
	TextureRes();
	virtual ~TextureRes();

	// 将成员变量置为初始值
	virtual void ResetVar() = 0;

	// 创建内存资源
	virtual bool CreateImage(const char* szImage, int nWidth, int nHeight, uint32 nType) = 0;
	// 从文件载入资源
	virtual bool LoadImage(char* szImage, uint32 nType) = 0;

	// 释放内存
	virtual void Release() = 0;
	
	virtual bool InvalidateDeviceObjects(){return true;}
	virtual bool RestoreDeviceObjects(){return true;}

	virtual bool ReleaseAFrameData() = 0;

	int32 GetWidth(){ return (int32)m_nWidth; }
	int32 GetHeight(){ return (int32)m_nHeight; }

//private:
public:
	uint32		m_nType;					// 资源类型

	D3DFORMAT	m_eTextureFormat;			// 贴图格式
	int32		m_nWidth;					// 资源宽度
	int32		m_nHeight;					// 资源高度
	
	// 用于性能统计
	unsigned long m_nTexMemUsed;			// 贴图内存占用数量,单位字节
	unsigned long m_nSprMemUsed;			// 贴图内存占用数量,单位字节
	bool		m_bLastFrameUsed;			// 上一帧是否使用

private:
	
	friend class TextureResMgr;
};

class TextureResBmp : public TextureRes
{
public:

public:
	TextureResBmp();
	~TextureResBmp();

	// 将成员变量置为初始值
	virtual void ResetVar();

	// 创建内存资源
	virtual bool CreateImage(const char* szImage, int nWidth, int nHeight, uint32 nType);
	// 从文件载入资源
	virtual bool LoadImage(char* szImage, uint32 nType);

	// 释放内存
	virtual void Release();

	virtual bool InvalidateDeviceObjects();
	virtual bool RestoreDeviceObjects();

	virtual bool ReleaseAFrameData()
	{
		Release();
		return false;
	}

	// 锁定数据缓冲区
	bool LockData(void** pData, int32& nPitch);
	// 解锁数据缓冲区
	void UnLockData();

//private:
public:
	FrameToTexture		m_FrameInfo;		// 帧到铁图映射信息
	LPDIRECT3DTEXTURE9	m_pSysMemTexture;	// 系统内存中的对应贴图，用于更新和清除显存中的贴图

private:
	bool LoadJpegFile(char* szImage);
	
};

class TextureResSpr : public TextureRes
{
public:

public:
	TextureResSpr();
	~TextureResSpr();

	// 将成员变量置为初始值
	virtual void ResetVar();

	// 创建内存资源
	virtual bool CreateImage(const char* szImage, int nWidth, int nHeight, uint32 nType);
	// 从文件载入资源
	virtual bool LoadImage(char* szImage, uint32 nType);

	// 释放内存
	virtual void Release();

	virtual bool ReleaseAFrameData();

	bool PrepareFrameData(const char* szImage, int32 nFrame, bool bPrepareTex);
	// 取得nFrame帧的第nIdx张贴图，根据不同情况nIdx取值为1-4
	LPDIRECT3DTEXTURE9 GetTexture(int32 nFrame, int nIdx);

	uint32 GetCenterX(){ return m_nCenterX; }
	uint32 GetCenterY(){ return m_nCenterY; }
	uint32 GetFrameNum(){ return m_nFrameNum; }
	uint32 GetDirections(){ return m_nDirections; };
	uint32 GetInterval(){ return m_nInterval; };
	
	// 取得第nFrame帧图象x，y坐标上的象素点alpha值
	int32 GetPixelAlpha(int32 nFrame, int32 x, int32 y);

//private:
public:
	uint32		m_nCenterX;
	uint32		m_nCenterY;
	uint32		m_nColors;
	uint32		m_nDirections;
	uint32		m_nInterval;

	KPAL24*		m_pPal24;					// 调色板
	WORD*		m_pPal16;					// 4444调色板
	int32		m_nFrameNum;				// 资源帧数

	FrameToTexture*		m_pFrameInfo;		// 帧到铁图映射信息数组指针
	char*		m_pHeader;					// spr头部信息，如果不为空意味着分帧压缩，否则为整体压缩
	SPROFFS*	m_pOffset;					// 帧偏移量信息

private:
	bool LoadSprFile(char* szImage);
	void CreateTexture16Bit(const char* szImage, int32 nFrame);
	void CreateTexture32Bit(uint32 nFrame);

	// 拆分贴图，将贴图拆分为1到4张以充分利用贴图空间
	int SplitTexture(uint32 nFrame);
	
};

#endif