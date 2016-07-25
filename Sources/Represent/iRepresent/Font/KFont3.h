/*****************************************************************************
					基于Direct3D中文显示系统之字体接口

应用：
	界面系统对外接口：iFont。
	外部客户直接引用此系统要包含头文件iFont.h。

接口功能：
	绘制显示字符串。

使用：
	接口使用前的初始化：
		使用接口前需先调用其接口方法Init，初始化时提供Direct3D Device接口的
		实例的指针。调用时间最好是在Direct3D Device已初始化之后。
		初始化将使已经载入字库数据丢失。

依赖性与耦合关系：
	*基于M$ DirectX 8.0*

本地化：
	改变KFR_MAX_NUM_CHAR值的定义，以适应目标语言字符集的字符数目。

    如果要同时存在多个字符集，可以考虑把KFR_MAX_NUM_CHAR改成KFontRes的数据成员，
	在载入字库文件时动态确定，同时动态分配KFontRes::m_dwOffs的空间，及其他一些额外改动。

	如果目标字符集要缓存的字符数目较少，可以调小DESIRE_TEXTURE_SIDE_WIDTH值的定义。

	此系统主要在于提供了缓存机制，对于字符数目很大的字符集，只在贴图里维护一些
	缓冲字符的贴图。如果目标语言字符集的字符数目较少，不需要缓冲机制，又想利用
	此系统其它特点的话，可以考虑改造KCharSet，把KMRU分离出来。

	不只支持中文，双字节编码的文字都支持。
	单字节编码的文字也可以固定把高字节置零，扩展成双字节使用此系统。
	也可以改造成支持单字节或者四字节编码的文字：改变与字符编码宽度以及字符集字
	符总数目相关的结构数据、接口参数的类型以及对它们的相关操作。

	KFont3::TextOut中“字符的判断与处理”以及KFontRes的对外接口方法Update与编码
	规则相关的，要根据本地文字的编码做相应的修改。

文件：
	此系统当前包含如下头文件：
		KFont3.h, KFontRes.h, KMRU.h, KCharSet.h
	包含如下实现代码文件：
		KFont3.cpp, KFontRes.cpp, KMRU.cpp, KCharSet.cpp

*******************************************************************************/


/*******************************************************************************
// FileName			:	KFont.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2001-9-13 10:18:42
// FileDescription	:	字体类
// Revision Count	:	2002-9-4改写，改为基于d3d，去除文本控制标记的判断。wooy
*******************************************************************************/
#pragma once
#include "KCharSet.h"
#include "FontDef.h"
#include "iFont.h"

typedef void (*fnRenderText)(KFontVertex* pPolyVertices, unsigned int nNumPolys);

class KFont3 : public iFont
{
public:
//--------iKFont接口方法---------
	//初始化
	bool	Init(void* pDrawDevice);
	//载入字库
	bool	Load(const char* pszFontFile);
	//结束，清除操作
	void	Terminate();
	//设置绘制时字符边缘的颜色，如alpha为0表示字符边缘不单独处理
	void	SetBorderColor(unsigned int uColor);
	//获取字体大小信息
	void	GetFontSize(int* pWidth, int* pHeight);
	//设置字符绘制大小
	void	SetOutputSize(int nOutputWith, int nOutputHeight);
	//显示字符串
	void	OutputText(const char* pszText, int nCount = KRF_ZERO_END,
					int nX = KRF_FOLLOW, int nY = KRF_FOLLOW,
					unsigned int nColor = 0xff000000, int nLineWidth = 0);
	//克隆一个接口对象指针
	iFont* Clone();
	//释放接口对象
	void Release();
	//使Device状态切换会受影响的绘图设备相关内容失效
	void	InvalidateDeviceObjects();
	//在D3d Device由lost state恢复为operational state后KFont作d3d Device相关内容的恢复操作。
	bool	RestoreDeviceObjects();
	//构造函数
	KFont3();
private:
	~KFont3();
	//绘制字符串
	static fnRenderText	ms_RenderText;	//指向当前绘制行为函数的指针
	static void	RenderTextDirect(KFontVertex* pPolyVertices, unsigned int nNumPolys);
	static void	RenderTextCmp(KFontVertex* pPolyVertices, unsigned int nNumPolys);
	static void	RenderTextCmpReverse(KFontVertex* pPolyVertices, unsigned int nNumPolys);

	static void	CheckAlphaCmpCaps();//检查绘图设设备所支持的alpha检测方法

private:
	bool				m_bLoaded;			//是否已经载入字库
	unsigned short		m_nRefCount;		//引用计数
	KCharSet			m_Resources;		//字体资源
	float				m_fFontWidth;		//字体的宽
	float				m_fFontHeight;		//字体的高
	int					m_nFontHalfWidth[2];//字体的半宽
	int					m_nOutputWidth;		//输出宽度
	int					m_nOutputHeight;	//输出高度

	static unsigned int	ms_uBorderColor;

	LPDIRECT3DTEXTURE9	m_pTexture;			//字体贴图句柄
	LPDIRECT3DSTATEBLOCK9 m_pStateBlockSaved;	//文字绘制前D3d device的状态纪录
    LPDIRECT3DSTATEBLOCK9 m_pStateBlockDrawText;//文字绘制时D3d device的状态纪录
	int					m_nLastPosH;		//上次文字绘制后的后续坐标水平值
	int					m_nLastPosV;		//上次文字绘制后的后续坐标垂直值

private:
	static LPDIRECT3DDEVICE9	ms_pd3dDevice;		//Direct 3d device接口的实例的指针
	static unsigned int	ms_AlphaCmpMethod;	//字体贴图采用的alpha判断方法
	static unsigned int	ms_AlphaRef1, ms_AlphaRef2;	//alpha判断时引用到的alpha值
};