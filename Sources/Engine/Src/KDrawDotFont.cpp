//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDrawDotFont.cpp
// Date:	2002.07.19
// Code:	QIUXIN
// Desc:	Dot Font Drawing Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KCanvas.h"
#include "KDrawDotFont.h"
//---------------------------------------------------------------------------
// 函数:	DrawDotFont
// 功能:	绘制点阵的字体
// 参数:	KDrawNode*, KCanvas* 
// 返回:	void
//---------------------------------------------------------------------------
void g_DrawDotFont(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;
	
	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of font
	long nHeight = pNode->m_nHeight;// height of font
	long nColor = pNode->m_nColor;// color of font
	long nAlpha = 32-(pNode->m_nAlpha&0x001f);// nAlpha值1
	long nTmpAlpha = 0;// nAlpha值2
	void* lpFont = pNode->m_pBitmap;// font pointer
	void* lpBuffer = pCanvas->m_pCanvas;// canvas buffer
	long nPitch = pCanvas->m_nPitch;// canvas pitch
	long nMask32 = pCanvas->m_nMask32;// RGB mask 32bit

	
	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
		return;
	
	// 计算屏幕下一行的偏移
	long nNextLine = nPitch - Clipper.width * 2;
	
	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = nPitch * Clipper.y + nX * 2 + lpBuffer
//---------------------------------------------------------------------------
		mov		eax, nPitch
		mov		ebx, Clipper.y
		mul		ebx
		mov     ebx, Clipper.x
		add		ebx, ebx
		add     eax, ebx
		mov 	edi, lpBuffer
		add		edi, eax
//---------------------------------------------------------------------------
// 初始化 ESI 指向图块数据起点 (跳过 Clipper.top 行图形数据)
//---------------------------------------------------------------------------
		mov		esi, lpFont
		mov		ecx, Clipper.top
		or		ecx, ecx
		jz		loc_DrawDotFont_0011
		xor		eax, eax

loc_DrawDotFont_0008:

		mov		edx, nWidth

loc_DrawDotFont_0009:

		mov     al, [esi]
		inc     esi
		and		al, 0x1f
		sub		edx, eax
		jg		loc_DrawDotFont_0009
		dec     ecx
		jg  	loc_DrawDotFont_0008
//---------------------------------------------------------------------------
// jump acorrd Clipper.left, Clipper.right
//---------------------------------------------------------------------------
loc_DrawDotFont_0011:

		mov		eax, Clipper.left
		or		eax, eax
		jz		loc_DrawDotFont_0012
		jmp		loc_DrawDotFont_exit

loc_DrawDotFont_0012:

		mov		eax, Clipper.right
		or		eax, eax
		jz		loc_DrawDotFont_0100
		jmp		loc_DrawDotFont_exit
//---------------------------------------------------------------------------
// Clipper.left  == 0
// Clipper.right == 0
//---------------------------------------------------------------------------
loc_DrawDotFont_0100:

		mov		edx, Clipper.width

loc_DrawDotFont_0101:

		xor		eax, eax
		mov     al, [esi]
		inc     esi
		mov		ebx, eax
		shr		ebx, 5
		or		ebx, ebx
		jnz		loc_DrawDotFont_0102

		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawDotFont_0101
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawDotFont_0100
		jmp		loc_DrawDotFont_exit

loc_DrawDotFont_0102:

        shl     ebx, 2    //Alpha1 
		
		push    eax
		push    edx 
		mov     eax,nAlpha
		mul     ebx
        shr     eax,5 
        mov		nTmpAlpha, eax
		pop     edx  
		pop     eax 
   
		and		eax, 0x1f
		mov     ecx, eax
		push	eax
		push    edx

loc_DrawDotFont_Loop1:

		push	ecx
		mov     ecx, nColor				// ecx = ...rgb
		mov		ax, cx					// eax = ...rgb
		sal		eax, 16					// eax = rgb...
		mov		ax, cx					// eax = rgbrgb
		and		eax, nMask32			// eax = .g.r.b

		mov		cx, [edi]				// ecx = ...rgb
		mov		bx, cx					// ebx = ...rgb
		sal		ebx, 16					// ebx = rgb...
		mov		bx, cx					// ebx = rgbrgb
		and		ebx, nMask32			// ebx = .g.r.b

		mov		ecx, nTmpAlpha 			// ecx = nAlpha
		mul		ecx						// eax = eax*ecx
		neg		ecx						// ecx = -nAlpha
		add		ecx, 0x20				// ecx = 32-nAlpha
		xchg	eax, ebx				// exchange eax,ebx
		mul		ecx						// eax = eax*ecx
		add		eax, ebx				// eax = eax + ebx
		sar		eax, 5					// eax = eax/32
		and     eax, nMask32			// eax = .g.r.b

		mov     cx, ax					// ecx = ...r.b
		sar     eax, 16					// eax = ....g.
		or      ax, cx					// eax = ...rgb
        stosw                       

		pop		ecx
        loop    loc_DrawDotFont_Loop1

        pop     edx
        pop     eax

		sub		edx, eax
		jg		loc_DrawDotFont_0101
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawDotFont_0100
		jmp		loc_DrawDotFont_exit

loc_DrawDotFont_exit:
	}
}
