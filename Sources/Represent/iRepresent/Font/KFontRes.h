/*******************************************************************************
// FileName			:	KFontRes.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2001-9-12 9:51:04
// FileDescription	:	3D绘制方式字体图形资源类头文件
// Revision Count	:	2002-9-4改写，改为基于d3d。Wooy
*******************************************************************************/
#pragma once
#include <D3d9.h>
#include "KFontData.h"

//3D绘制方式字体图形资源类
class KFontRes
{
public:
	//启用/禁用文字边缘效果
	static void EnableTextBorder(bool bEnable);
	//初始化
	bool Init(const char* pszFontFile, LPDIRECT3DDEVICE9 pd3dDevice);
	//结束，清除操作
	void Terminate();
	//得到字体的宽度高度，贴图可以缓存的字符的个数
	void GetInfo(int& nWidth, int& nHeight, int& nHCount, int& nCount) const;
	//得到字符在贴图里的宽度高度，水平间隔，贴图可以缓存的字符的个数
	void GetInfo(float& fWidth, float& fHeight, float& fHInterval, float& fVInterval) const;
	//更新缓存字符
	void Update(unsigned short* pUpdates, int nCount);
	
	//!得到贴图句柄
	LPDIRECT3DTEXTURE9 GetTexture()	const {	return m_pCharTexture;	}

	KFontRes();
	virtual ~KFontRes();

private:
	//更新单个字符的点阵数据
	void Update(unsigned char* pCharImage, unsigned char* pTexData, int nTexPitch);
		
private:
	KFontData		m_FontData;					//字库资源
	int				m_nFontW;					//字符宽
	int				m_nFontH;					//字符高
	int				m_nNumCountH;				//贴图里一行存储字符的个数
	int				m_nMaxCount;				//贴图里最多能容纳的字符个数	

	unsigned int		m_nTextureSideWidth;	//字体贴图的边长
	LPDIRECT3DDEVICE9	m_pd3dDevice;			//Direct 3d device接口的实例的指针
	LPDIRECT3DTEXTURE9	m_pCharTexture;			//贴图句柄	

	static	int		m_bEnableTextBorder;
};
