/*****************************************************************************************
//  文本串处理：获取文本串中单一性质的同一行的子串，以及文本串总的含数目。
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-29
------------------------------------------------------------------------------------------
//	Comment :  输入的文本串中包含控制符号，不是原始的标签控制符，是经过编码的。文本串中不
//	能出现无效字符，否则此模块可能会产生内存违例访问错误。参看Engine中的Text.h，获得对文
//	本串中的控制符进行编码以及过滤无效字符的功能。
//		可以在构造处理对象实例时传入要处理的字符串及相关数据，也可以调用SetContent来设置
//	要处理的字符串。
//		多次调用GetSimplexText将逐个获取字符串中同行的单一性质的子串，在字符串被遍厉完毕
//	之后将在设置调用参数TailCtrl中的行末控制码为文本串结束控制码。下次再调用又将重新从文
//	本串起始处获取子串。
*****************************************************************************************/

struct iFont;
#include "TextProcessDef.h"

class KTextProcess
{
private:
	//描述文本控制的结构
#pragma pack(push, 1)
	struct KTP_CTRL
	{
		unsigned char cCtrl;	//此成员取值为Text.h中梅举KTEXT_CTRL_CODE中的一个值
		union{
			struct {
				unsigned char cParam0;	//此及以下两个成员的取值与含义依据cCtrl的取值而定
				unsigned char cParam1;
				unsigned char cParam2;
			};
			unsigned short wParam;
		};
	};
#pragma pack(pop)
public:
	//在指定的位置绘制字符串中指定的内容
	int		DrawTextLine(iFont* pFont, int nFontSize, KOutputTextParam* pParam);

	//判断某一个坐标，在指定的位置输出的字符串中，所占的偏移量
	//注意：如果超出字符所占的矩形则返回的偏移为离指定坐标最近的一个字符的偏移
	int		TransXYPosToCharOffset(int nX, int nY, iFont* pFont, int nFontSize, KOutputTextParam* pParam);

	//跳到指定行开始处
	int		SeekToSpecialLine(int nLineIndex, KTP_CTRL& Ctrl1, KTP_CTRL& Ctrl2,
							  int& nSkipedHalfLines, int nFontSize, int bPicPackInSingleLine);
	//设置要处理的文本串内容
	void	SetContent(const char* pBuffer, int nCount, int nLineLen);

	void SetEncodePlugin(fnTextProcessEncodePlugin fnPlugin);

	void SetDrawPlugin(fnTextProcessEncodePlugin fnPlugin);

	//计算一行的高度 [wxb 2003-6-20]
	void GetCurLineHeight(int& nSpanLines, int& nHeight, int nFontSize, int bPicPackInSingleLine);
	//保存和恢复内部变量 [wxb 2003-6-20]
	void Backup()
	{
		//_ASSERT(!m_bBackuped);
		m_nCountB			= m_nCount;
		m_nCurrentLineLenB	= m_nCurrentLineLen;
		m_fCurrentLineLenB	= m_fCurrentLineLen;
		m_nMaxLineLenB		= m_nMaxLineLen;
		m_nReadPosB			= m_nReadPos;

		m_bBackuped = 1;	//true
	}
	void Restore()
	{
		//_ASSERT(m_bBackuped);
		m_nCount			= m_nCountB;
		m_nCurrentLineLen	= m_nCurrentLineLenB;
		m_fCurrentLineLen	= m_fCurrentLineLenB;
		m_nMaxLineLen		= m_nMaxLineLenB;
		m_nReadPos			= m_nReadPosB;

		m_bBackuped = 0;	//false
	}

	//构造函数
	KTextProcess();
	KTextProcess(const char* pBuffer, int nCount, int nLineLen);

private:
	//获取字符串里单一性质同行的一个子串
	int		GetSimplexText(int &nSimplexStartPos, KTP_CTRL& HeadCtrl, KTP_CTRL& TailCtrl, int nFontSize);

private:
	const char*	m_pBuffer;	//文本缓冲区的指针
	int			m_nCount;	//文本缓冲区数据的长度(以BYTE为单位)
	int			m_nCurrentLineLen;
	float		m_fCurrentLineLen;
	int			m_nMaxLineLen;	//每行可以容纳的（半角）字符的数目
	int			m_nReadPos;	//当前度读取的位置

	//for backup
	int			m_bBackuped;

	int			m_nCountB;
	int			m_nCurrentLineLenB;
	float		m_fCurrentLineLenB;
	int			m_nMaxLineLenB;
	int			m_nReadPosB;
};
