/*******************************************************************************
// FileName			:	KFont2.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2002-9-5
// FileDescription	:	字体类
// Revision Count	:	
*******************************************************************************/
#include "KFont2.h"
#include "string.h"
#include "../KRepresentUnit.h"
#include "../../../Engine/Src/KEngine.h"
#include "../../../Engine/Src/Text.h"
#include "../../../Engine/Src/KColors.h"

/*!*****************************************************************************
// Purpose		:  构造函数
*****************************************************************************/
KFont2::KFont2()
{
	m_nFontWidth	= 16;
	m_nFontHalfWidth[0] = m_nFontHalfWidth[1] = 8;
	m_nFontHeight	= 16;
	m_nLastPosH		= 0;
	m_nLastPosV		= 0;
	m_pDevice		= 0;
	m_bLoaded		= false;
	m_nRefCount		= 1;
//	m_nDrawBorderWithDeffColor = true;
	m_nBorderColor = 0;
	m_nOutputWidth = m_nFontWidth;
	m_nOutputHeight= m_nFontHeight;
}

/*!*****************************************************************************
// Purpose		:  析构函数
*****************************************************************************/
KFont2::~KFont2()
{
	Terminate();
}

/*!*****************************************************************************
// Function		: KFont2::Init
// Purpose		: 初始化
// Return		: bool 是否成功
// Argumant		: void* pDrawDevice -> 接口的实例的指针
*****************************************************************************/
bool KFont2::Init(void* pDrawDevice)
{
	Terminate();
	if ((m_pDevice = pDrawDevice) == 0)
		return false;
	return true;
}

/*!*****************************************************************************
// Function		: KFont2::Terminate
// Purpose		: 结束，清除操作
*****************************************************************************/
void KFont2::Terminate()
{
	m_pDevice = 0;
	m_bLoaded = false;
	m_Resources.Terminate();
}

/*!*****************************************************************************
// Function		: KFont2::Load
// Purpose		: 载入字库
// Return		: bool 是否成功
// Argumant		: cosnt char *pszFontFile 字库文件名
*****************************************************************************/
bool KFont2::Load(const char* pszFontFile)
{
	m_Resources.Terminate();
	m_bLoaded = false;
	if (m_pDevice)
	{
		//初始化字体字库资源
		if (m_Resources.Load(pszFontFile))
		{
			m_Resources.GetInfo(m_nFontWidth, m_nFontHeight);
			m_bLoaded = true;
			return true;
		}
	}
	return false;
}

/*!*****************************************************************************
// Function		: KFont2::TextOut
// Purpose		: 显示字符串
// Argumant		: cosnt char *pszText 字符串
// Argumant		: int nCount  字符串的长度(BYTE)，默认值为-1，表示此字符串是以'\0'结尾。
// Argumant		: int nX	  字符串显示起点坐标X，如果传入值为KF_FOLLOW，
//							  则此字符串紧接在上次字符串的输出位置之后。
// Argumant		: int nY      字符串显示起点坐标Y, 如果传入值为KF_FOLLOW，
//							  此字符串与前一次输出字符串在同一行的位置。
// Argumant		: uint nColor 字符串显示颜色，默认为黑色，用32bit数以ARGB的格
//							  式表示颜色，每个分量8bit。
// Argumant     : nLineWidth  自动换行的行宽限制，如果其值小于一个全角字符宽度
//							  则不做自动换行处理。默认值为0，既不做自动换行处理。
*****************************************************************************/
void KFont2::OutputText(const char* pszText, int nCount/*= KF_ZERO_END*/,
					int nX/*=KF_FOLLOW*/, int nY/*=KF_FOLLOW*/,
					unsigned int nColor/*=0xff000000*/, int nLineWidth/*=0*/)
{
	if (!pszText || !m_bLoaded)
		return;

	unsigned char*	lpByte = (unsigned char*) pszText;
	int				nL, h;
	int				nPos = 0;
	int				nHalfIndex = 0;

	if (nCount == KRF_ZERO_END)
		nCount = strlen(pszText);
	if (nX == KRF_FOLLOW)
		nX = m_nLastPosH;
	if (nY == KRF_FOLLOW)
		nY = m_nLastPosV;

	h = 0;

	KRColor		c;
	c.Color_dw = nColor;
	int			sColor = g_RGB(c.Color_b.r, c.Color_b.g, c.Color_b.b);

	if (nLineWidth < m_nOutputWidth + m_nOutputWidth)
		nLineWidth = 0; //不做自动换行处理

	while (nPos < nCount)
	{
		//*********字符的判断与处理*********
		if (lpByte[nPos] > 0x80 && nPos + 1 < nCount)
		{
			DrawCharacter(nX + h, nY, lpByte[nPos], lpByte[nPos + 1], sColor);
			nPos += 2;
			h += m_nOutputWidth;
		}
		else	//西文字符
		{
			nL = lpByte[nPos++];
			if (nL > 0x20 && nL < 0x5F)
				DrawCharacter(nX + h, nY, 0xa3, (0x20 + nL), sColor);
			else if (nL >= 0x5F && nL < 0x7F)
				DrawCharacter(nX + h, nY, 0xa3, (0x21 + nL), sColor);
			
			if (nL != 0x0a)
			{
				h += m_nFontHalfWidth[nHalfIndex];
				nHalfIndex ^= 1;
			}
			else
			{
				h = 0;
				nY += m_nOutputHeight;
			}
		}
		if (nLineWidth)
		{
			if (h + m_nOutputWidth > nLineWidth)
			{
				if (nPos < nCount && lpByte[nPos] == 0x0a)	//处理自动换行位置后恰好紧接一个换行符号
					nPos++;
				h = 0;
				nY += m_nOutputHeight;
				nHalfIndex = 0;
			}
			else if (h && nPos + 3 < nCount &&
				h + m_nOutputWidth + m_nFontHalfWidth[nHalfIndex] >= nLineWidth)
			{
				const char* pNext = TGetSecondVisibleCharacterThisLine((const char*)lpByte, nPos, nCount);
				if (pNext && TIsCharacterNotAlowAtLineHead(pNext))
				{
					h = 0;
					nY += m_nOutputHeight;
					nHalfIndex = 0;
				}
			}
		}
	}
	m_nLastPosH = nX + h;
	m_nLastPosV = nY;
}

//设置绘制时字符边缘的颜色，如alpha为0表示字符边缘不单独处理
void KFont2::SetBorderColor(unsigned int uColor)
{
//	if (uColor & 0xff000000)
//	{		
		KRColor		c;
		c.Color_dw = uColor;
		m_nBorderColor = g_RGB(c.Color_b.r, c.Color_b.g, c.Color_b.b);
//		m_nDrawBorderWithDeffColor = true;
//	}
//	else
//	{
//		m_nDrawBorderWithDeffColor = false;
//	}
}

/*!*****************************************************************************
// Function		: KFont2::TextOut
// Purpose		: 绘制单个字符
// Argumant		: unsigned char cFirst 字符编码的前个字节
// Argumant		: unsigned char cNext  字符编码的后个字节
*****************************************************************************/
void KFont2::DrawCharacter(int x, int y, unsigned char cFirst, unsigned char cNext, int nColor) const
{
	if (m_pDevice && m_bLoaded)
	{
		//取得字符在字库里的数据区指针
		unsigned char* pCharacterData = m_Resources.GetCharacterData(cFirst, cNext);		
		if (pCharacterData)
		{
//			if (m_nDrawBorderWithDeffColor == false)
//				((KCanvas*)m_pDevice)->DrawFont(x, y, m_nFontWidth, m_nFontHeight, nColor, 31, pCharacterData);
//			else
				((KCanvas*)m_pDevice)->DrawFontWithBorder(x, y, m_nFontWidth, m_nFontHeight, nColor, 31, pCharacterData, m_nBorderColor);
		}
	}
}

void KFont2::GetFontSize(int* pWidth, int* pHeight)
{
	if (pWidth)
		*pWidth = m_nFontWidth;
	if (pHeight)
		*pHeight = m_nFontHeight;
}

//设置字符绘制缩进
void KFont2::SetOutputSize(int nOutputWith, int nOutputHeight)
{
	if (nOutputWith > 0)
		m_nOutputWidth = nOutputWith;
	else
		m_nOutputWidth = m_nFontWidth;
	if (nOutputHeight > 0)
		m_nOutputHeight = nOutputHeight;
	else
		m_nOutputHeight = m_nFontHeight;
	m_nFontHalfWidth[0] = m_nOutputWidth / 2;
	m_nFontHalfWidth[1] = (m_nOutputWidth + 1) / 2;
}

//克隆一个接口对象指针
iFont* KFont2::Clone()
{
	if (m_nRefCount < 0xffff)
	{
		m_nRefCount ++;
		return ((iFont*)this);
	}
	return NULL;
}

//释放接口对象
void KFont2::Release()
{
	if ((--m_nRefCount) == 0)
		delete (this);
}
