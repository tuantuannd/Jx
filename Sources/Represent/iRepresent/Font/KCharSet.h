/*******************************************************************************
// FileName			:	KCharSet.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2001-9-13 10:05:57
// FileDescription	:	字符集类
// Revision Count	:	2002-9-4改写，改为基于d3d。Wooy
*******************************************************************************/
#pragma once

#include "KFontRes.h"
#include "KMru.h"

//绘制文字的三边形顶点结构
struct	KFontVertex
{
	float			x, y, z, w;
	unsigned    	color;
    float		 	u, v;
	//unsigned        uReserve;   // for Align
};

//一次可以提交给KCharSet的字符的最大数目
#define	KCS_CHAR_NUM_LIMIT	64

class KCharSet
{
public:
	//初始化
	bool	Init(const char* pszFontFile, LPDIRECT3DDEVICE9 pd3dDevice);
	//结束，清除操作
	void	Terminate();
	//提交字符串，计算串中各字符引用的字体贴图坐标，填入多边形内。
	void	CommitText(unsigned short* pString, int nNumChars, KFontVertex* pCharPolys);
	//获取字体信息
	void	GetInfo(int& nFontW, int& nFontH);
	//得到贴图句柄
	LPDIRECT3DTEXTURE9 GetTexture() const;
	
	KCharSet();
	virtual ~KCharSet();
	
private:
	int			m_nFontW;			//字体宽（单位：像素）
	int			m_nFontH;			//字体高
	int			m_nNumCharH;		//字体贴图中一行字符的数目
	float		m_fFontW;			//字体在贴图中宽（单位：贴图浮点坐标）
	float		m_fFontH;			//字体在贴图中高
	float		m_fCharHInterval;	//字符在贴图中水平距（字符间可以有间隔，所以字符在贴图中水平距与宽可能不同）
	float		m_fCharVInterval;	//字符在贴图中垂直距
	KFontRes	m_Resource;			//字库资源
	KMRU		m_MruTable;			//缓冲字符MRU表
};
