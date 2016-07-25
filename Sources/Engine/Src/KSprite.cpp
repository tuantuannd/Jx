//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KSprite.cpp
// Date:	2000.09.18
// Code:	WangWei(Daphnis),Wooy
// Desc:	Sprite Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KPakFile.h"
#include "KCanvas.h"
#include "KDrawSprite.h"
#include "KDrawSpriteAlpha.h"
#include "KSprite.h"
//---------------------------------------------------------------------------
// 函数:	KSprite
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KSprite::KSprite()
{
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nCenterX		= 0;
	m_nCenterY		= 0;
	m_nFrames		= 0;
	m_nColors		= 0;
	m_nDirections	= 1;
	m_nInterval		= 1;
	m_pPal24		= NULL;
	m_pPal16		= NULL;
	m_pOffset		= NULL;
	m_pSprite		= NULL;
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	载入
// 参数:	FileName 文件名
// 返回:	TRUE－成功
//			FALSE－失败
//---------------------------------------------------------------------------
BOOL KSprite::Load(LPSTR FileName)
{
	KPakFile	File;
	SPRHEAD*	pHeader;
	PBYTE		pTemp;

	// open the file
	if (!File.Open(FileName))
		return FALSE;

	// alloc memory
	if (!m_Buffer.Alloc(File.Size()))
		return FALSE;
	pTemp = (PBYTE)m_Buffer.GetMemPtr();

	// read data from file
	File.Read(pTemp, File.Size());

	// check file header setup sprite member
	pHeader = (SPRHEAD*)pTemp;
	if (!g_MemComp(pHeader->Comment, "SPR", 3))
		return FALSE;

	// get sprite info
	m_nWidth      = pHeader->Width;
	m_nHeight     = pHeader->Height;
	m_nCenterX    = pHeader->CenterX;
	m_nCenterY    = pHeader->CenterY;
	m_nFrames     = pHeader->Frames;
	m_nColors     = pHeader->Colors;
	m_nDirections = pHeader->Directions;
	m_nInterval   = pHeader->Interval;

	// setup palette pointer
	pTemp += sizeof(SPRHEAD);
	m_pPal24 = (KPAL24*)pTemp;

	// setup offset pointer
	pTemp += m_nColors * sizeof(KPAL24);
	m_pOffset = (SPROFFS*)pTemp;

	// setup sprite pointer
	pTemp += m_nFrames * sizeof(SPROFFS);
	m_pSprite = (LPBYTE)pTemp; // 相对偏移

	// make color table
	MakePalette();

	return TRUE;
}


//---------------------------------------------------------------------------
// 函数:	LoadFor3D
// 功能:	载入  为三维模式生成4444的调色版与二维调色版不同
// 参数:	FileName 文件名
// 返回:	TRUE－成功
//			FALSE－失败
//---------------------------------------------------------------------------
BOOL KSprite::LoadFor3D(LPSTR FileName)
{
	KPakFile	File;
	SPRHEAD*	pHeader;
	PBYTE		pTemp;

	// open the file
	if (!File.Open(FileName))
		return FALSE;

	// alloc memory
	if (!m_Buffer.Alloc(File.Size()))
		return FALSE;
	pTemp = (PBYTE)m_Buffer.GetMemPtr();

	// read data from file
	File.Read(pTemp, File.Size());

	// check file header setup sprite member
	pHeader = (SPRHEAD*)pTemp;
	if (!g_MemComp(pHeader->Comment, "SPR", 3))
		return FALSE;

	// get sprite info
	m_nWidth      = pHeader->Width;
	m_nHeight     = pHeader->Height;
	m_nCenterX    = pHeader->CenterX;
	m_nCenterY    = pHeader->CenterY;
	m_nFrames     = pHeader->Frames;
	m_nColors     = pHeader->Colors;
	m_nDirections = pHeader->Directions;
	m_nInterval   = pHeader->Interval;

	// setup palette pointer
	pTemp += sizeof(SPRHEAD);
	m_pPal24 = (KPAL24*)pTemp;

	// setup offset pointer
	pTemp += m_nColors * sizeof(KPAL24);
	m_pOffset = (SPROFFS*)pTemp;

	// setup sprite pointer
	pTemp += m_nFrames * sizeof(SPROFFS);
	m_pSprite = (LPBYTE)pTemp; // 相对偏移

	// make color table
	Make4444Palette();

	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	Free
// 功能:	释放
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::Free()
{
	m_Buffer.Free();
	m_Palette.Free();
	m_nFrames = 0;
	m_nColors = 0;
	m_pPal24  = NULL;
	m_pPal16  = NULL;
	m_pOffset = NULL;
	m_pSprite = NULL;
}
//---------------------------------------------------------------------------
// 函数:	Make4444Palette
// 功能:	生成4444色盘，其中alpha层为0
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::Make4444Palette()
{
	m_Palette.Alloc(m_nColors * sizeof(KPAL16));
	m_pPal16 = (KPAL16*)m_Palette.GetMemPtr();
	
	int nColors = m_nColors;
	KPAL24 * pPal24 = m_pPal24;
	KPAL16 * pPal16 = m_pPal16;
	
	while (nColors -- > 0)
	{
		//*pPal16 = pPal24->Red<<4 | pPal24->Green<<8 | pPal24->Blue << 12 | 0;
		WORD	Red = (pPal24->Red) >> 4;
		WORD	Green = (pPal24->Green) >> 4;
		WORD	Blue = (pPal24->Blue) >> 4;

		*pPal16 = (Red << 8) | (Green << 4) | Blue;

		pPal16++;
		pPal24++;
	}
	
}

//---------------------------------------------------------------------------
// 函数:	MakePalette
// 功能:	生成色盘
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::MakePalette()
{
	m_Palette.Alloc(m_nColors * sizeof(KPAL16));
	m_pPal16 = (KPAL16*)m_Palette.GetMemPtr();
	g_Pal24ToPal16(m_pPal24, m_pPal16, m_nColors);
}
//---------------------------------------------------------------------------
// 函数:	Draw
// 功能:	Draw Sprite Onto Screen
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::Draw(int nX, int nY, int nFrame)
{
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	
	// goto frame
	SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);

	// draw the frame
	g_pCanvas->DrawSprite(
		nX + pFrame->OffsetX,
		nY + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_pPal16);
}

//---------------------------------------------------------------------------
// 函数:	DrawCenter
// 功能:	Draw Sprite Onto Screen in a CenterPoint
// 参数:	CX		X坐标
//			CY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawCenter(int nX, int nY, int nFrame)
{
	int X0,Y0;
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	   X0=nX-m_nCenterX;
       Y0=nY-m_nCenterY;
	// goto frame
	SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);

	// draw the frame
	g_pCanvas->DrawSprite(
		X0 + pFrame->OffsetX,
		Y0 + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_pPal16);
}
//---------------------------------------------------------------------------
// 函数:	DrawMixColor
// 功能:	Draw Sprite Onto Screen
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawMixColor(int nX, int nY, int nFrame,int nColor,int nAlpha)
{
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	
	// goto frame
	SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);

	// draw the frame
	g_pCanvas->DrawSpriteMixColor(
		nX + pFrame->OffsetX,
		nY + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_pPal16,nColor,nAlpha);
}


//---------------------------------------------------------------------------
// 函数:	DrawWithColor
// 功能:	Draw Sprite Onto Screen
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawWithColor(int nX, int nY, int nFrame,int nColor,int nAlpha)
{
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	
	// goto frame
	SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);

	// draw the frame
	g_pCanvas->DrawSpriteWithColor(
		nX + pFrame->OffsetX,
		nY + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_pPal16,nColor,nAlpha);
}

//---------------------------------------------------------------------------
// 函数:	DrawAlpha
// 功能:	绘制带Alpha的精灵图形
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawAlpha(int nX, int nY, int nFrame, int nExAlpha)
{
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	
	// goto frame
	SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);
	
	// draw the frame
	g_pCanvas->DrawSpriteAlpha(
		nX + pFrame->OffsetX,
		nY + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_pPal16, nExAlpha);
}
//---------------------------------------------------------------------------
// 函数:	DrawAlphaCenter
// 功能:	以nX,nY为中心绘制带Alpha的精灵图形
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawAlphaCenter(int nX, int nY, int nFrame, int nExAlpha)
{
	int X0,Y0;
	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	   X0=nX-m_nCenterX;
       Y0=nY-m_nCenterY;
	
	// goto frame
	SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);
	
	// draw the frame
	g_pCanvas->DrawSpriteAlpha(
		X0 + pFrame->OffsetX,
		Y0 + pFrame->OffsetY,
		pFrame->Width,
		pFrame->Height,
		pFrame->Sprite,
		m_pPal16, nExAlpha);
}

//---------------------------------------------------------------------------
// 函数:	DrawTrans
// 功能:	绘制带Alpha的精灵图形
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawTrans(int nX, int nY, int nFrame)
{
}
//---------------------------------------------------------------------------
// 函数:	DrawBorder
// 功能:	Draw Sprite Onto Screen
// 参数:	nX		X坐标
//			nY		Y坐标
//			nFrame	要绘制的帧
// 返回:	void
//---------------------------------------------------------------------------
void KSprite::DrawBorder(int nX, int nY, int nFrame, int nColor)
{
	LPBYTE	pSprite = m_pSprite;

	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return;
	
	// goto frame
	pSprite += m_pOffset[nFrame].Offset;

	// draw the frame
	g_pCanvas->DrawSpriteBorder(nX, nY, m_nWidth, m_nHeight, nColor, pSprite);
	//g_DrawSpriteBorder(nX, nY, m_nWidth, m_nHeight, nColor, pSprite);
}
//---------------------------------------------------------------------------
// 函数:	NexFrame
// 功能:	取得下一帧
// 参数:	nFrame		当前帧
// 返回:	下一帧
//---------------------------------------------------------------------------
int KSprite::NextFrame(int nFrame)
{
	nFrame++;
	if (nFrame >= m_nFrames)
		nFrame = 0;
	return nFrame;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// 函数:	GetPixelAlpha
// 功能:	取得某一点的alpha值
// 参数:	nFrame		当前帧
//---------------------------------------------------------------------------
int	KSprite::GetPixelAlpha(int nFrame, int x, int y)
{
	int	nAlpha = 0;
	if (nFrame >= 0 && nFrame < m_nFrames)
	{
		SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset[nFrame].Offset);
		x -= pFrame->OffsetX;
		y -= pFrame->OffsetY;
		if (x >= 0  && x < pFrame->Width && y >= 0 && y < pFrame->Height)
		{

			int	nNumPixels = pFrame->Width;
			void*	pSprite = pFrame->Sprite;
			y++;
			_asm
			{
				//使SDI指向sprite中的图形数据位置
				mov		esi, pSprite
			dec_line:
				dec		y				//减掉一行
				jz		last_line

				mov		edx, nNumPixels
			skip_line:
				movzx	eax, byte ptr[esi]
				inc		esi
				movzx	ebx, byte ptr[esi]
				inc		esi
				or		ebx, ebx
				jz		skip_line_continue
				add		esi, eax
			skip_line_continue:
				sub		edx, eax
				jg		skip_line
				jmp		dec_line

			last_line:
				mov		edx, x
			last_line_alpha_block:
				movzx	eax, byte ptr[esi]
				inc		esi
				movzx	ebx, byte ptr[esi]
				inc		esi
				or		ebx, ebx
				jz		last_line_continue
				add		esi, eax
			last_line_continue:
				sub		edx, eax
				jg		last_line_alpha_block
				mov		nAlpha, ebx
			}
		}
	}
	return nAlpha;
}

//---------------------------------------------------------------------------
// 函数:	GetFrame
// 功能:	Get Sprite Frame
// 参数:	nFrame	帧
// 返回:	void
//---------------------------------------------------------------------------
LPVOID KSprite::GetFrame(int nFrame)
{
	LPBYTE	pSprite = m_pSprite;

	// check frame range
	if (nFrame < 0 || nFrame >= m_nFrames)
		return NULL;
	
	// goto frame
	pSprite += m_pOffset[nFrame].Offset;
	
	return (LPVOID)pSprite;
}
