//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDrawFade.cpp
// Date:	2000.08.08
// Code:	Zroc,Daphnis
// Desc:	Fade Drawing Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDDraw.h"
#include "KCanvas.h"
#include "KDrawFade.h"
//---------------------------------------------------------------------------
// 函数:	DrawFade
// 功能:	区域亮度衰减
// 参数:	node, canvas
// 返回:	void
//---------------------------------------------------------------------------
void g_DrawFade(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;

	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of sprite
	long nHeight = pNode->m_nHeight;// height of sprite
	long nAlpha = pNode->m_nAlpha;// aplha level

	// 检查ALPHA值
	if (nAlpha < 0 || nAlpha > 31)
		return;

	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
		return;

	int nPitch;
	void* lpBuffer = pCanvas->LockCanvas(nPitch);
	if (lpBuffer == NULL)
		return;

	// 计算屏幕下一行的偏移
	long ScreenOffset = nPitch - Clipper.width /2 * 4;//4的倍数

	// setup local var
	DWORD dwRGBMask1, dwRGBMask2;
	if (pCanvas->m_nMask16 == RGB_555)
	{
		dwRGBMask1 = 0x03e07c1f;
		dwRGBMask2 = 0xfc1f03e0;
	}
	else
	{
		dwRGBMask1 = 0x07e0f81f;
		dwRGBMask2 = 0xf81f07e0;
	}

	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = nPitch * Clipper.y + Clipper.x * 2 + lpBuffer
//---------------------------------------------------------------------------
		mov		eax, nPitch
		mov		ebx, Clipper.y
		mul		ebx
		mov     ebx, Clipper.x
		add		ebx, ebx
		add     eax, ebx
		mov		edi, lpBuffer
		add		edi, eax
//---------------------------------------------------------------------------
// 开始循环 
//---------------------------------------------------------------------------
		mov		ecx, Clipper.height

loc_DrawFade_Loop1:
		push	ecx
		// 取得宽度
		mov		ecx, Clipper.width
		// 一次计算两个点
		shr		ecx, 1

loc_DrawFade_Loop2:
		// 取操作数
		mov		eax, [edi]
		mov		ebx, eax
		// 留下 _g1_r2_b2
		and		eax, dwRGBMask1
		// 留下 r1_b1_g2_
		and		ebx, dwRGBMask2
		// _g1_r2_b2 * nAlpha
		imul	eax, nAlpha
		// _g1_r2_b2 / 32
		shr		eax, 5
		// r1_b1_g2_ / 32, 为防止溢出,先除再乘
		shr		ebx, 5
		// r1_b1_g2_ * nAlpha
		imul	ebx, nAlpha
		// 留下 _g1_r2_b2
		and		eax, dwRGBMask1
		// 留下 r1_b1_g2_
		and		ebx, dwRGBMask2
		// 得到最后结果
		or		eax, ebx
		// 赋值
		mov		[edi], eax
		// 指针后移
		add		edi, 4
		// 循环继续本行
		dec		ecx
		jnz		loc_DrawFade_Loop2
		// 循环下一行象素
		add		edi, ScreenOffset
		pop		ecx
		dec		ecx
		jnz		loc_DrawFade_Loop1
	}
	pCanvas->UnlockCanvas();
}
//---------------------------------------------------------------------------
