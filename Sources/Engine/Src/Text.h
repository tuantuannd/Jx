/*****************************************************************************************
//	文字串处理
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-31
------------------------------------------------------------------------------------------
*****************************************************************************************/
#ifndef TEXT_H
#define TEXT_H

//字符串控制码的内部编码
enum KTEXT_CTRL_CODE
{
	KTC_INVALID			=	0,
	KTC_ENTER			=	0x0a,
	KTC_COLOR			=	0x02,	//后面跟随三个BYTE数据分别为RGB三色分量
	KTC_COLOR_RESTORE	=	0x03,	//回复到原来设置的颜色
	KTC_BORDER_COLOR	=	0x04,	//设置边缘色，后面跟随三个BYTE数据分别为RGB三色分量
	KTC_BORDER_RESTORE	=	0x05,	//设置边缘色还原
	KTC_INLINE_PIC		=	0x06,	//嵌入式图片[wxb 2003-6-19]
	KTC_TAB				=	0x09,	//tab
	KTC_SPACE			=	0x20,	//空格
	KTC_TAIL			=	0xFF,	//字符串结束
};

struct KTP_CTRL
{
	unsigned char	cCtrl;	//此成员取值为Text.h中梅举KTEXT_CTRL_CODE中的一个值
	union
	{
	    struct
	    {
   			unsigned char cParam0;	//此及以下两个成员的取值与含义依据cCtrl的取值而定
		    unsigned char cParam1;
		    unsigned char cParam2;
	    };
	    unsigned short wParam;
   };
};

#define MAX_SYSTEM_INLINE_PICTURES	4096	//系统预留的嵌入式图片个数
struct IInlinePicEngineSink
{
	//获取指定嵌入图片的大小
	virtual long GetPicSize(unsigned short wIndex, int& cx, int& cy) = 0;
	//绘制指定图片
	virtual long DrawPic(unsigned short wIndex, int x, int y) = 0;
	//动态加载图片,获取一个WORD,即图片的索引
	virtual long AddCustomInlinePic(unsigned short& wIndex, const char* szSprPathName) = 0;
	//动态卸载图片
	virtual long RemoveCustomInlinePic(unsigned short wIndex) = 0;
};

#ifndef ENGINE_EXPORTS

//接口 IInlinePicEngineSink 由应用层实现并挂接进 Engine 模块 [wxb 2003-6-19]
//相关挂接函数:
// AdviseEngine(IInlinePicEngineSink*);
// UnAdviseEngine(IInlinePicEngineSink*);
extern "C"
{
	//获取本行的下个显示字符
	const char* TGetSecondVisibleCharacterThisLine(const char* pCharacter, int nPos, int nLen);
	//检测某个字符是否为不许放置行首的字符，不是限制字符则返回0，否则返回字符占的子节数
	int TIsCharacterNotAlowAtLineHead(const char* pCharacter);
	//如果原字符串长度（包括结尾符）超过限定的长度，则截短它并加上..后缀
	const char* TGetLimitLenString(const char* pOrigString, int nOrigLen, char* pLimitLenString, int nLimitLen);
	//如果原(包含控制符)字符串长度（包括结尾符）超过限定的长度，则截短它并加上..后缀
	const char* TGetLimitLenEncodedString(const char* pOrigString, int nOrigLen, int nFontSize,
		int nWrapCharaNum, char* pLimitLenString, int& nShortLen, int nLineLimit, int bPicPackInSingleLine = false);
	//寻找分割字符串的合适位置
	int	TSplitString(const char* pString, int nDesirePos, int bLess);
	//在编码字串寻找分割字符串的合适位置
	int	TSplitEncodedString(const char* pString, int nCount, int nDesirePos, int bLess);
	//获得指定行的开始位置
	int TGetEncodeStringLineHeadPos(const char* pBuffer, int nCount, int nLine, int nWrapCharaNum, int nFontSize, int bPicPackInSingleLine = false);
	//对文本串中的控制标记进行转换，去除无效字符，缩短文本串存储长度
	int	TEncodeText(char* pBuffer, int nCount);
	//对文本串中的控制标记进行转换，去除无效字符，缩短文本串存储长度
	int TFilterEncodedText(char* pBuffer, int nCount);
	//去除编码文本中的控制符号
	int	TRemoveCtrlInEncodedText(char* pBuffer, int nCount);
	//获取编码文本的行数与最大行宽
	int	TGetEncodedTextLineCount(const char* pBuffer, int nCount, int nWrapCharaNum, int& nMaxLineLen, int nFontSize, int nSkipLine = 0, int nLineLimit = 0, int bPicSingleLine = false);
	//对已经编码的文本，从指定位置开始查找指定的控制符号的位置，返回-1表示未找到
	int	TFindSpecialCtrlInEncodedText(const char* pBuffer, int nCount, int nStartPos, char cControl);
	//对已经编码的文本，去除指定类型的控制符
	int	TClearSpecialCtrlInEncodedText(char* pBuffer, int nCount, char cControl);
	//对已经编码的文本，指定输出长度的在缓冲区中位置
	int TGetEncodedTextOutputLenPos(const char* pBuffer, int nCount, int& nLen, bool bLess, int nFontSize);
	//对已经编码的文本，指定的前段缓冲区中控制符，对后面的输出产生效果影响
	int TGetEncodedTextEffectCtrls(const char* pBuffer, int nSkipCount, KTP_CTRL& Ctrl0, KTP_CTRL& Ctrl1);

	long AdviseEngine(IInlinePicEngineSink*);
	long UnAdviseEngine(IInlinePicEngineSink*);
}

#endif

#endif
