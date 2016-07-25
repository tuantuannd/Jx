
/*****************************************************************************************
//	文本串处理：获取文本串中单一性质的同一行的子串，以及文本串总的含数目。
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-29
*****************************************************************************************/
#include <crtdbg.h>
#include "TextProcess.h"
#include "../../../Engine/src/Text.h"
#include "../Font/iFont.h"

#ifndef SUCCEEDED
#define SUCCEEDED(Status) ((long)(Status) >= 0)
#endif

extern IInlinePicEngineSink* g_pIInlinePicSinkRP;	//嵌入式图片的处理接口[wxb 2003-6-20]

union	TP_COLOR
{
	struct {unsigned char b, g, r, a; } Color_b;
	unsigned int	Color_dw;
};

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KTextProcess::KTextProcess()
{
	m_pBuffer  = NULL;
	m_nCount   = 0;
	m_nMaxLineLen = 0;
	m_nReadPos = 0;
	m_nCurrentLineLen = 0;
	m_fCurrentLineLen = 0;
}

//--------------------------------------------------------------------------
//	功能：构造函数
//--------------------------------------------------------------------------
KTextProcess::KTextProcess(const char* pBuffer, int nCount, int nLineLen)
{
	SetContent(pBuffer, nCount, nLineLen);
}

//--------------------------------------------------------------------------
//	功能：设置要处理的文本串内容
//--------------------------------------------------------------------------
void KTextProcess::SetContent(const char* pBuffer, int nCount, int nLineLen)
{
	if (pBuffer && nCount > 0)
	{
		m_pBuffer  = pBuffer;
		m_nCount   = nCount;
		m_nMaxLineLen = nLineLen;
	}
	else
	{
		m_pBuffer  = NULL;
		m_nCount   = 0;
		m_nMaxLineLen = 0;
	}
	m_nCurrentLineLen = 0;
	m_fCurrentLineLen = 0;
	m_nReadPos = 0;
}

//--------------------------------------------------------------------------
//	功能：获取字符串里单一性质同行的一个子串
//	参数：int& nSimplexStartPos --> 子串头一个显示字符在字符串中的位置
//		　KTP_CTRL& HeadCtrl    --> 子串开始前的控制
//		　KTP_CTRL& TailCtrl    --> 子串结束后的控制
//	注释：此函数编码与GBK规范相关
//		多次调用GetSimplexText将逐个获取字符串中同行的单一性质的子串，在字符串
//	被遍厉完毕之后将在设置调用参数TailCtrl中的行末控制码为文本串结束控制码。下
//	次再调用又将重新从文本串起始处获取子串。
//--------------------------------------------------------------------------
int KTextProcess::GetSimplexText(int &nSimplexStartPos, KTP_CTRL& HeadCtrl, KTP_CTRL& TailCtrl, int nFontSize)
{
	int nBeforeLen = 0,
		nVisibleChars = 0;	//此次获取到的显示字符的个数
	nSimplexStartPos = m_nReadPos;
	HeadCtrl.cCtrl = KTC_INVALID;
	TailCtrl.cCtrl = KTC_INVALID;

	unsigned char cCode;

	if (m_nReadPos >= m_nCount)
		goto KS_EXIT;

	nBeforeLen = m_nCurrentLineLen;
	while(m_nReadPos < m_nCount)
	{
		cCode = m_pBuffer[m_nReadPos];
		//判断头一个字符是否是控制符号
		if (cCode > 0x80)	//可能是中文字符
		{	//字符串先被处理过，不会出现单个单字节数值大于0x80的字符
			nVisibleChars += 2;
			m_fCurrentLineLen += 2;
			m_nCurrentLineLen = (int)(m_fCurrentLineLen + 0.9999);
			if (m_nMaxLineLen == 0 || m_nCurrentLineLen < m_nMaxLineLen)
			{
				m_nReadPos += 2;
			}
			else if(m_nCurrentLineLen == m_nMaxLineLen || m_nCurrentLineLen == 0)
			{
				m_nReadPos += 2;
				TailCtrl.cCtrl = KTC_ENTER;
				break;
			}
			else
			{
				nVisibleChars -= 2;
				m_nCurrentLineLen -= 2;
				m_fCurrentLineLen -= 2;
				TailCtrl.cCtrl = KTC_ENTER;
				break;
			}
		}
		else if (cCode == KTC_INLINE_PIC)	//[wxb 2003-6-20]
		{
			TailCtrl.cCtrl	= KTC_INLINE_PIC;
			TailCtrl.wParam	= *((unsigned short*)(m_pBuffer + m_nReadPos + 1));
			if (g_pIInlinePicSinkRP)
			{
				int cx, cy;
				if (SUCCEEDED(g_pIInlinePicSinkRP->GetPicSize(TailCtrl.wParam, cx, cy)))
				{
					m_fCurrentLineLen += (float)cx * 2 / nFontSize;
					m_nCurrentLineLen = (int)(m_fCurrentLineLen + 0.9999);
					if (m_nMaxLineLen == 0 || m_nCurrentLineLen < m_nMaxLineLen)
					{
						m_nReadPos += 1 + sizeof(unsigned short);
					}
					else if (m_nCurrentLineLen == m_nMaxLineLen || m_nCurrentLineLen == 0)
					{
						m_nReadPos += 1 + sizeof(unsigned short);
						break;
					}
					else
					{
						m_fCurrentLineLen -= (float)cx * 2 / nFontSize;
						m_nCurrentLineLen = (int)(m_fCurrentLineLen + 0.9999);
						TailCtrl.cCtrl = KTC_ENTER;
						break;
					}
				}
				else
				{
					m_nReadPos += 1 + sizeof(unsigned short);
				}
			}
			else
				m_nReadPos += 1 + sizeof(unsigned short);
			break;
		}
		else if (cCode == KTC_COLOR || cCode == KTC_BORDER_COLOR)//颜色控制
		{
			if (m_nCurrentLineLen > nBeforeLen || HeadCtrl.cCtrl != KTC_INVALID)
			{
				(*(int*)(&TailCtrl)) = *(int*)(m_pBuffer + m_nReadPos);
				m_nReadPos += 4;
				break;
			}
			else
			{
				(*(int*)(&HeadCtrl)) = *(int*)(m_pBuffer + m_nReadPos);
				m_nReadPos += 4;
				nSimplexStartPos = m_nReadPos;
			}
		}
		else if (cCode == KTC_COLOR_RESTORE || cCode == KTC_BORDER_RESTORE)
		{
			m_nReadPos += 1;
			if (m_nCurrentLineLen > nBeforeLen || HeadCtrl.cCtrl != KTC_INVALID)
			{
				TailCtrl.cCtrl = cCode;
				break;
			}
			else
			{
				HeadCtrl.cCtrl = cCode;
				nSimplexStartPos = m_nReadPos;
			}
		}
		else if (cCode == KTC_ENTER)
		{
			TailCtrl.cCtrl = KTC_ENTER;
			m_nReadPos ++;
			break;
		}
		else
		{
			m_nCurrentLineLen ++;
			m_fCurrentLineLen += 1;
			nVisibleChars ++;
			m_nReadPos ++;
		}
		if (m_nMaxLineLen)
		{
			if (m_nCurrentLineLen >= m_nMaxLineLen)
			{
				TailCtrl.cCtrl = KTC_ENTER;
				break;
			}
			else if (m_nCurrentLineLen && m_nCurrentLineLen + 3 >= m_nMaxLineLen)
			{
				const char* pNext = TGetSecondVisibleCharacterThisLine((const char*)m_pBuffer, m_nReadPos, m_nCount);
				if (pNext && TIsCharacterNotAlowAtLineHead(pNext))
				{
					TailCtrl.cCtrl = KTC_ENTER;
					break;
				}
			}
		}
	}

	nBeforeLen = m_nCurrentLineLen - nBeforeLen;

KS_EXIT:
	if (m_nReadPos >= m_nCount)
	{
		if (KTC_INLINE_PIC == TailCtrl.cCtrl)
			return nVisibleChars;
		TailCtrl.cCtrl = KTC_TAIL;
		m_nReadPos = 0;
		m_nCurrentLineLen = 0;
		m_fCurrentLineLen = 0;
	}
	else if (TailCtrl.cCtrl == KTC_ENTER)
	{
		m_nCurrentLineLen = 0;
		m_fCurrentLineLen = 0;
	}
	
	return (nVisibleChars);
}

//--------------------------------------------------------------------------
//	功能：跳到指定行开始处
//	参数：int nLineIndex --> 指定行的行号（从0开始）
//		　KTP_CTRL& Ctrl --> 指定行开始前的控制
//	返回：如果文本串中存在，则返回文本串中指定行开始处的位置。
//		　如果指定行不存在，则返回-1，并把文本串的读取指针设置在本字符串开始处。
//--------------------------------------------------------------------------
int KTextProcess::SeekToSpecialLine(int nLineIndex, KTP_CTRL& Ctrl1, KTP_CTRL&Ctrl2,
									int& nSkipedHalfLines, int nFontSize, int bPicPackInSingleLine)
{
	m_nReadPos = 0;
	int nSimplexTextPos;
	KTP_CTRL HeadCtrl, TailCtrl;

	TP_COLOR	PreFontColor, PreBorderColor;
	PreFontColor.Color_b.a = 0;
	PreBorderColor.Color_b.a = 0;
	TailCtrl.cCtrl = Ctrl1.cCtrl = KTC_INVALID;
	Ctrl2.cCtrl = KTC_INVALID;
	int nLinesSpan = 0, nLineHeight = 0;
	unsigned char cPrevTailCtrl = 0xFF;

	nSkipedHalfLines = 0;
	while (nLineIndex > 0)
	{
		cPrevTailCtrl = TailCtrl.cCtrl;
		if (0 == nLinesSpan || KTC_ENTER == TailCtrl.cCtrl)
		{
			GetCurLineHeight(nLinesSpan, nLineHeight, nFontSize, bPicPackInSingleLine);
			Backup();
		}
		GetSimplexText(nSimplexTextPos, HeadCtrl, TailCtrl, nFontSize);
		switch(HeadCtrl.cCtrl)
		{
		case KTC_COLOR:
			if (Ctrl1.cCtrl == KTC_COLOR)
			{
				PreFontColor.Color_b.r = Ctrl1.cParam0;
				PreFontColor.Color_b.g = Ctrl1.cParam1;
				PreFontColor.Color_b.b = Ctrl1.cParam2;
				PreFontColor.Color_b.a = 0xFF;
			}
			Ctrl1 = HeadCtrl;
			break;
		case KTC_COLOR_RESTORE:
			if (PreFontColor.Color_b.a)
			{
				Ctrl1.cCtrl = KTC_COLOR;
				Ctrl1.cParam0 = PreFontColor.Color_b.r;
				Ctrl1.cParam1 = PreFontColor.Color_b.g;
				Ctrl1.cParam2 = PreFontColor.Color_b.b;
			}
			else
				Ctrl1.cCtrl = KTC_INVALID;
			break;
		case KTC_BORDER_COLOR:
			if (Ctrl2.cCtrl == KTC_BORDER_COLOR)
			{
				PreBorderColor.Color_b.r = Ctrl2.cParam0;
				PreBorderColor.Color_b.g = Ctrl2.cParam1;
				PreBorderColor.Color_b.b = Ctrl2.cParam2;
				PreBorderColor.Color_b.a = 0xFF;
			}
			Ctrl2 = HeadCtrl;
			break;
		case KTC_BORDER_RESTORE:
			if (PreBorderColor.Color_b.a)
			{
				Ctrl2.cCtrl = KTC_BORDER_COLOR;
				Ctrl2.cParam0 = PreBorderColor.Color_b.r;
				Ctrl2.cParam1 = PreBorderColor.Color_b.g;
				Ctrl2.cParam2 = PreBorderColor.Color_b.b;
			}
			else
				Ctrl2.cCtrl = KTC_INVALID;
			break;
		}

		switch (TailCtrl.cCtrl)
		{
		case KTC_COLOR:
			if (Ctrl1.cCtrl == KTC_COLOR)
			{
				PreFontColor.Color_b.r = Ctrl1.cParam0;
				PreFontColor.Color_b.g = Ctrl1.cParam1;
				PreFontColor.Color_b.b = Ctrl1.cParam2;
				PreFontColor.Color_b.a = 0xFF;
			}
			Ctrl1 = TailCtrl;
			break;
		case KTC_COLOR_RESTORE:
			if (PreFontColor.Color_b.a)
			{
				Ctrl1.cCtrl = KTC_COLOR;
				Ctrl1.cParam0 = PreFontColor.Color_b.r;
				Ctrl1.cParam1 = PreFontColor.Color_b.g;
				Ctrl1.cParam2 = PreFontColor.Color_b.b;
			}
			else
				Ctrl1.cCtrl = KTC_INVALID;
			break;
		case KTC_BORDER_COLOR:
			if (Ctrl2.cCtrl == KTC_BORDER_COLOR)
			{
				PreBorderColor.Color_b.r = Ctrl2.cParam0;
				PreBorderColor.Color_b.g = Ctrl2.cParam1;
				PreBorderColor.Color_b.b = Ctrl2.cParam2;
				PreBorderColor.Color_b.a = 0xFF;
			}
			Ctrl2 = TailCtrl;
			break;
		case KTC_BORDER_RESTORE:
			if (PreBorderColor.Color_b.a)
			{
				Ctrl2.cCtrl = KTC_BORDER_COLOR;
				Ctrl2.cParam0 = PreBorderColor.Color_b.r;
				Ctrl2.cParam1 = PreBorderColor.Color_b.g;
				Ctrl2.cParam2 = PreBorderColor.Color_b.b;
			}
			else
				Ctrl2.cCtrl = KTC_INVALID;
			break;
		case KTC_ENTER:
			nLineIndex -= nLinesSpan;
			if (nLineIndex < 0)
			{
				Restore();
				nSkipedHalfLines = -nLineIndex;
			}
			break;
		case KTC_INLINE_PIC:
			Ctrl2.cCtrl = KTC_INLINE_PIC;
			Ctrl2.wParam = TailCtrl.wParam;
			break;
		case KTC_TAIL:
			Ctrl1.cCtrl = KTC_INVALID;
			Ctrl2.cCtrl = KTC_INVALID;
			if (KTC_INLINE_PIC == cPrevTailCtrl)
			{
				nLineIndex -= nLinesSpan;
				if (nLineIndex < 0)
				{
					Restore();
					nSkipedHalfLines = -nLineIndex;
				}
				break;
			}
			m_nReadPos = 0;
			return -1;
			break;
		}
	}
	return m_nReadPos;
}

//--------------------------------------------------------------------------
//	功能：在指定的位置绘制字符串中指定的内容
//	返回：实际绘制的文字行数
//--------------------------------------------------------------------------
int KTextProcess::DrawTextLine(iFont* pFont, int nFontSize, KOutputTextParam* pParam)
{
	_ASSERT(pParam);
	if (pFont == NULL || pParam->nNumLine <= 0)
		return 0;

	KTP_CTRL	PreCtrl1, PreCtrl2, TailCtrl;
	int			nCount = 0, nPos = 0;
	int			x = pParam->nX, y = pParam->nY, nLineDrawed = 0;

	int			nHalfIndex = 0, nFontHalfWidth[2];
	int			nSkipedHalfLines = 0;	//因为嵌入式图片造成的跨半行现象
	nFontHalfWidth[0] = nFontSize / 2;
	nFontHalfWidth[1] = (nFontSize + 1) / 2;

	TP_COLOR	PreFontColor, CurFontColor;
	TP_COLOR	PreBorderColor, CurBorderColor;

	PreFontColor.Color_dw = CurFontColor.Color_dw = pParam->Color;
	PreBorderColor.Color_dw = CurBorderColor.Color_dw = 0xff000000;

#define	_SET_CURRENT_COLOR(c)				\
	{	PreFontColor = CurFontColor;				\
		CurFontColor.Color_b.r = c.cParam0;	\
		CurFontColor.Color_b.g = c.cParam1;	\
		CurFontColor.Color_b.b = c.cParam2;	}
#define	_SET_CURRENT_BORDER_COLOR(c)				\
	{	PreBorderColor = CurBorderColor;						\
		CurBorderColor.Color_b.r = c.cParam0;			\
		CurBorderColor.Color_b.g = c.cParam1;			\
		CurBorderColor.Color_b.b = c.cParam2;			\
		pFont->SetBorderColor(CurBorderColor.Color_dw);	}

	if (SeekToSpecialLine(pParam->nSkipLine, PreCtrl1, PreCtrl2, nSkipedHalfLines, nFontSize, pParam->bPicPackInSingleLine) < 0)
		return 0;
	
	if (PreCtrl1.cCtrl == KTC_COLOR)
		_SET_CURRENT_COLOR(PreCtrl1);
	if (PreCtrl2.cCtrl == KTC_BORDER_COLOR)
		_SET_CURRENT_BORDER_COLOR(PreCtrl2);

	int nSpanLines = 0, nLineHeight = 0;
	int bFirstLine = 1;	//true
	do
	{
		if (0 == nSpanLines || KTC_ENTER == TailCtrl.cCtrl)
		{
			//[wxb 2003-6-20][wxb 2003-7-10]
			GetCurLineHeight(nSpanLines, nLineHeight, nFontSize, pParam->bPicPackInSingleLine);
			if (nSpanLines > 1)
			{
				_ASSERT(nSpanLines * nFontSize - nLineHeight >= 0);
				switch (pParam->nVertAlign)
				{
				case 1:	//居上
					if (nLineHeight > nFontSize)
						y += (nLineHeight - nFontSize) / 2;
					break;
				case 2:	//居下
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize;
					if (nLineHeight > nFontSize)
						y -= (nLineHeight - nFontSize) / 2;
					break;
				case 0:	//居中
				default:
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize / 2;
					break;
				}
			}
		}
		nCount = GetSimplexText(nPos, PreCtrl1, TailCtrl, nFontSize);

		switch(PreCtrl1.cCtrl)
		{
		case KTC_COLOR:
			_SET_CURRENT_COLOR(PreCtrl1);
			break;
		case KTC_BORDER_COLOR:
			_SET_CURRENT_BORDER_COLOR(PreCtrl1);
			break;
		case KTC_COLOR_RESTORE:
			CurFontColor = PreFontColor;
			break;
		case KTC_BORDER_RESTORE:
			CurBorderColor = PreBorderColor;
			pFont->SetBorderColor(CurBorderColor.Color_dw);
			break;
		}

		if (nCount)
		{
			pFont->OutputText(m_pBuffer + nPos, nCount, x, y, CurFontColor.Color_dw, 0);
			x += nCount / 2 * nFontSize;
			if (nCount % 2)
			{
				x += nFontHalfWidth[nHalfIndex];
				nHalfIndex ^= 1;
			}
		}

		switch(TailCtrl.cCtrl)
		{
		case KTC_ENTER:
			x = pParam->nX;
			y += nFontSize;

			if (nSpanLines > 1)
			{
				//[wxb 2003-6-20][wxb 2003-7-10]
				switch (pParam->nVertAlign)
				{
				case 1:	//居上
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize;
					if (nLineHeight > nFontSize)
						y -= (nLineHeight - nFontSize) / 2;
					break;
				case 2:	//居下
					if (nLineHeight > nFontSize)
						y += (nLineHeight - nFontSize) / 2;
					break;
				case 0:	//居中
				default:
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize / 2;
					break;
				}
			}
			nLineDrawed += nSpanLines - (bFirstLine ? nSkipedHalfLines : 0);
			if (nLineDrawed >= pParam->nNumLine)
				TailCtrl.cCtrl = KTC_TAIL;	//为了中止循环
			else
				nHalfIndex = 0;

			bFirstLine = 0;	//false
			nSkipedHalfLines = 0;
			break;
		case  KTC_COLOR:
			_SET_CURRENT_COLOR(TailCtrl);
			break;
		case KTC_BORDER_COLOR:
			_SET_CURRENT_BORDER_COLOR(TailCtrl);
			break;
		case KTC_COLOR_RESTORE:
			CurFontColor = PreFontColor;
			break;
		case KTC_BORDER_RESTORE:
			CurBorderColor = PreBorderColor;
			pFont->SetBorderColor(CurBorderColor.Color_dw);
			break;
		case KTC_INLINE_PIC: //[wxb 2003-6-20][wxb 2003-7-10]
			if (g_pIInlinePicSinkRP)
			{
				int cx, cy;
				if (SUCCEEDED(g_pIInlinePicSinkRP->GetPicSize(TailCtrl.wParam, cx, cy)))
				{
					int nPicY = y;
					if (nSkipedHalfLines && cy > nFontSize)
					{
						switch (pParam->nVertAlign)
						{
							case 1:	//居上
								if (nLineHeight > nFontSize)
									nPicY -= (nLineHeight - nFontSize) / 2;
								break;
							case 2:	//居下
								nPicY -= (nSpanLines - nSkipedHalfLines - 1) * nFontSize;
								if (nLineHeight > nFontSize)
									nPicY += (nLineHeight - nFontSize) / 2;
								break;
							case 0:	//居中
							default:
								nPicY -= (nSpanLines - nSkipedHalfLines - 1) * nFontSize / 2;
								break;
						}
						if (cy > (nSpanLines - nSkipedHalfLines) * nFontSize)
							nPicY -= cy - (nSpanLines - nSkipedHalfLines) * nFontSize;
						else
							nPicY -= (int)(((cy - (nSpanLines - nSkipedHalfLines) * nFontSize)) * 1.5);
					}
					else
						nPicY -= (cy - nFontSize) / 2;
					if (SUCCEEDED(g_pIInlinePicSinkRP->DrawPic(TailCtrl.wParam, x, nPicY)))
					{
						x += cx;
					}
				}
			}
			break;
		}
	}while(TailCtrl.cCtrl != KTC_TAIL);
	if (x > pParam->nX)
		nLineDrawed += nSpanLines - (bFirstLine ? nSkipedHalfLines : 0);
	return nLineDrawed;
}


//--------------------------------------------------------------------------
//	功能：判断某一个坐标，在指定的位置输出的字符串中，所占的偏移量
//	返回：离指定坐标最近的字符偏移(字符偏移：有n个字符，那个就有0～n一共n+1个偏移)
//	注意：如果超出字符所占的矩形则返回的偏移为离指定坐标最近的一个字符的偏移
//		  本函数不能跟 DrawTextLine(..) 函数嵌套使用
//	[wxb 2003-7-21]
//--------------------------------------------------------------------------
int KTextProcess::TransXYPosToCharOffset(int nX, int nY, iFont* pFont, int nFontSize, KOutputTextParam* pParam)
{
	_ASSERT(pParam);
	if (pFont == NULL || pParam->nNumLine <= 0)
		return 0;

	KTP_CTRL	PreCtrl1, PreCtrl2, TailCtrl;
	int			nCount = 0, nPos = 0;
	int			x = pParam->nX, y = pParam->nY, nLineDrawed = 0;

	int			nHalfIndex = 0, nFontHalfWidth[2];
	int			nSkipedHalfLines = 0;	//因为嵌入式图片造成的跨半行现象
	nFontHalfWidth[0] = nFontSize / 2;
	nFontHalfWidth[1] = (nFontSize + 1) / 2;

	TP_COLOR	PreFontColor, CurFontColor;
	TP_COLOR	PreBorderColor, CurBorderColor;

	PreFontColor.Color_dw = CurFontColor.Color_dw = pParam->Color;
	PreBorderColor.Color_dw = CurBorderColor.Color_dw = 0xff000000;

#define	_SET_CURRENT_COLOR(c)				\
	{	PreFontColor = CurFontColor;				\
		CurFontColor.Color_b.r = c.cParam0;	\
		CurFontColor.Color_b.g = c.cParam1;	\
		CurFontColor.Color_b.b = c.cParam2;	}
#define	_SET_CURRENT_BORDER_COLOR(c)				\
	{	PreBorderColor = CurBorderColor;						\
		CurBorderColor.Color_b.r = c.cParam0;			\
		CurBorderColor.Color_b.g = c.cParam1;			\
		CurBorderColor.Color_b.b = c.cParam2;			\
		pFont->SetBorderColor(CurBorderColor.Color_dw);	}

	if (SeekToSpecialLine(pParam->nSkipLine, PreCtrl1, PreCtrl2, nSkipedHalfLines, nFontSize, pParam->bPicPackInSingleLine) < 0)
		return 0;
	
	if (PreCtrl1.cCtrl == KTC_COLOR)
		_SET_CURRENT_COLOR(PreCtrl1);
	if (PreCtrl2.cCtrl == KTC_BORDER_COLOR)
		_SET_CURRENT_BORDER_COLOR(PreCtrl2);

	int nSpanLines = 0, nLineHeight = 0;
	int bFirstLine = 1;			//true
	int nOffsetXThisLine = 0, nOldX = 0, nOldReadPos = 0;
	do
	{
		if (0 == nSpanLines || KTC_ENTER == TailCtrl.cCtrl)
		{
			//[wxb 2003-6-20][wxb 2003-7-10]
			GetCurLineHeight(nSpanLines, nLineHeight, nFontSize, pParam->bPicPackInSingleLine);
			if (nSpanLines > 1)
			{
				_ASSERT(nSpanLines * nFontSize - nLineHeight >= 0);
				switch (pParam->nVertAlign)
				{
				case 1:	//居上
					if (nLineHeight > nFontSize)
						y += (nLineHeight - nFontSize) / 2;
					break;
				case 2:	//居下
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize;
					if (nLineHeight > nFontSize)
						y -= (nLineHeight - nFontSize) / 2;
					break;
				case 0:	//居中
				default:
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize / 2;
					break;
				}
			}

			nOffsetXThisLine = -1;	//每一行开始的时候清空
		}
		nOldReadPos = m_nReadPos;
		nCount = GetSimplexText(nPos, PreCtrl1, TailCtrl, nFontSize);

		switch(PreCtrl1.cCtrl)
		{
		case KTC_COLOR:
			_SET_CURRENT_COLOR(PreCtrl1);
			break;
		case KTC_BORDER_COLOR:
			_SET_CURRENT_BORDER_COLOR(PreCtrl1);
			break;
		case KTC_COLOR_RESTORE:
			CurFontColor = PreFontColor;
			break;
		case KTC_BORDER_RESTORE:
			CurBorderColor = PreBorderColor;
			pFont->SetBorderColor(CurBorderColor.Color_dw);
			break;
		}

		if (nCount)
		{
			pFont->OutputText(m_pBuffer + nPos, nCount, x, y, CurFontColor.Color_dw, 0);
			nOldX = x;
			x += nCount / 2 * nFontSize;
			if (nCount % 2)
			{
				x += nFontHalfWidth[nHalfIndex];
				nHalfIndex ^= 1;
			}

			//计算点中的位置
			if (nOffsetXThisLine != -1)
			{
				if (nX <= nOldX)
					nOffsetXThisLine = nOldReadPos;
				else if (nX < x)
					nOffsetXThisLine = nOldReadPos + (nX - nOldX + nFontSize / 4) * 2 / nFontSize;
			}
		}

		switch(TailCtrl.cCtrl)
		{
		case KTC_ENTER:
			x = pParam->nX;
			y += nFontSize;

			if (nSpanLines > 1)
			{
				//[wxb 2003-6-20][wxb 2003-7-10]
				switch (pParam->nVertAlign)
				{
				case 1:	//居上
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize;
					if (nLineHeight > nFontSize)
						y -= (nLineHeight - nFontSize) / 2;
					break;
				case 2:	//居下
					if (nLineHeight > nFontSize)
						y += (nLineHeight - nFontSize) / 2;
					break;
				case 0:	//居中
				default:
					y += (nSpanLines - nSkipedHalfLines - 1) * nFontSize / 2;
					break;
				}
			}
			nLineDrawed += nSpanLines - (bFirstLine ? nSkipedHalfLines : 0);
			if (nLineDrawed >= pParam->nNumLine)
				TailCtrl.cCtrl = KTC_TAIL;	//为了中止循环
			else
				nHalfIndex = 0;

			bFirstLine = 0;			//false
			nSkipedHalfLines = 0;	//false
			break;
		case  KTC_COLOR:
			_SET_CURRENT_COLOR(TailCtrl);
			break;
		case KTC_BORDER_COLOR:
			_SET_CURRENT_BORDER_COLOR(TailCtrl);
			break;
		case KTC_COLOR_RESTORE:
			CurFontColor = PreFontColor;
			break;
		case KTC_BORDER_RESTORE:
			CurBorderColor = PreBorderColor;
			pFont->SetBorderColor(CurBorderColor.Color_dw);
			break;
		case KTC_INLINE_PIC: //[wxb 2003-6-20][wxb 2003-7-10]
			if (g_pIInlinePicSinkRP)
			{
				int cx, cy;
				if (SUCCEEDED(g_pIInlinePicSinkRP->GetPicSize(TailCtrl.wParam, cx, cy)))
				{
					int nPicY = y;
					if (nSkipedHalfLines && cy > nFontSize)
					{
						switch (pParam->nVertAlign)
						{
							case 1:	//居上
								if (nLineHeight > nFontSize)
									nPicY -= (nLineHeight - nFontSize) / 2;
								break;
							case 2:	//居下
								nPicY -= (nSpanLines - nSkipedHalfLines - 1) * nFontSize;
								if (nLineHeight > nFontSize)
									nPicY += (nLineHeight - nFontSize) / 2;
								break;
							case 0:	//居中
							default:
								nPicY -= (nSpanLines - nSkipedHalfLines - 1) * nFontSize / 2;
								break;
						}
						if (cy > (nSpanLines - nSkipedHalfLines) * nFontSize)
							nPicY -= cy - (nSpanLines - nSkipedHalfLines) * nFontSize;
						else
							nPicY -= (int)(((cy - (nSpanLines - nSkipedHalfLines) * nFontSize)) * 1.5);
					}
					else
						nPicY -= (cy - nFontSize) / 2;
					if (SUCCEEDED(g_pIInlinePicSinkRP->DrawPic(TailCtrl.wParam, x, nPicY)))
					{
						nOldX = x;
						x += cx;
						//计算点中的位置
						if (nOffsetXThisLine != -1)
						{
							if (nX <= nOldX)
								nOffsetXThisLine = nOldReadPos;
							else if (nX < x)
								nOffsetXThisLine = nOldReadPos + (nX - nOldX + cx / 2) / cx;
						}
					}
				}
			}
			break;
		}

		//判断是否就点在这一行
		if (KTC_ENTER == TailCtrl.cCtrl && y > nY || KTC_TAIL == TailCtrl.cCtrl)
		{
			if (nOffsetXThisLine != -1)
				return nOffsetXThisLine;
			else
				return m_nReadPos;
		}

	}while(TailCtrl.cCtrl != KTC_TAIL);
	if (x > pParam->nX)
		nLineDrawed += nSpanLines - (bFirstLine ? nSkipedHalfLines : 0);

	_ASSERT(0);
	return nOffsetXThisLine;
}


//[wxb 2003-6-20]
void KTextProcess::GetCurLineHeight(int& nSpanLines, int& nHeight, int nFontSize, int bPicPackInSingleLine/*=FALSE*/)
{
	unsigned short wIndex = 0;
	nHeight = nFontSize;
	//改为图片如何都只占一行高，所以注释下面代码
	if (g_pIInlinePicSinkRP && !bPicPackInSingleLine)
	{
		Backup();

		unsigned char cCode;
		while(m_nReadPos < m_nCount)
		{
			cCode = m_pBuffer[m_nReadPos];
			//判断头一个字符是否是控制符号
			if (cCode > 0x80)	//可能是中文字符
			{	//字符串先被处理过，不会出现单个单字节数值大于0x80的字符
				m_fCurrentLineLen += 2;
				m_nCurrentLineLen = (int)(m_fCurrentLineLen + 0.9999);
				if (m_nMaxLineLen == 0 || m_nCurrentLineLen < m_nMaxLineLen)
				{
					m_nReadPos += 2;
				}
				else if(m_nCurrentLineLen == m_nMaxLineLen || m_nCurrentLineLen == 0)
				{
					m_nReadPos += 2;
					break;
				}
				else
				{
					m_nCurrentLineLen -= 2;
					m_fCurrentLineLen -= 2;
					break;
				}
			}
			else if (cCode == KTC_INLINE_PIC)	//[wxb 2003-6-20]
			{
				wIndex	= *((unsigned short*)(m_pBuffer + m_nReadPos + 1));
				m_nReadPos += 1 + sizeof(unsigned short);
				if (g_pIInlinePicSinkRP)
				{
					int cx, cy;
					if (SUCCEEDED(g_pIInlinePicSinkRP->GetPicSize(wIndex, cx, cy)))
					{
						if (cy > nHeight)
							nHeight = cy;
						m_fCurrentLineLen += cx;
						m_nCurrentLineLen = (int)(m_fCurrentLineLen + 0.9999);
					}
				}
			}
			else if (cCode == KTC_COLOR || cCode == KTC_BORDER_COLOR)//颜色控制
				m_nReadPos += 4;
			else if (cCode == KTC_COLOR_RESTORE || cCode == KTC_BORDER_RESTORE)
				m_nReadPos += 1;
			else if (cCode == KTC_ENTER)
			{
				m_nReadPos ++;
				break;
			}
			else
			{
				m_nCurrentLineLen ++;
				m_fCurrentLineLen += 1;
				m_nReadPos ++;
			}
			if (m_nMaxLineLen)
			{
				if (m_nCurrentLineLen >= m_nMaxLineLen)
					break;
				else if (m_nCurrentLineLen && m_nCurrentLineLen + 3 >= m_nMaxLineLen)
				{
					const char* pNext = TGetSecondVisibleCharacterThisLine((const char*)m_pBuffer, m_nReadPos, m_nCount);
					if (pNext && TIsCharacterNotAlowAtLineHead(pNext))
						break;
				}
			}
		}

		Restore();
	}
	nSpanLines = (nHeight % nFontSize) ? nHeight / nFontSize + 1 : nHeight / nFontSize;
}
