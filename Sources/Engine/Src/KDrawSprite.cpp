//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDrawSprite.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis), Wooy(Wu yue)
// Desc:	Sprite Drawing Function
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KCanvas.h"
#include "KDrawSprite.h"
#include "DrawSpriteMP.inc"

//---------------------------------------------------------------------------
// 函数:	DrawSprite
// 功能:	绘制256色Sprite位图
// 参数:	KDrawNode*, KCanvas* 
// 返回:	void
//---------------------------------------------------------------------------
void g_DrawSprite(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;
	
	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (pCanvas->MakeClip(pNode->m_nX, pNode->m_nY, pNode->m_nWidth, pNode->m_nHeight, &Clipper) == 0)
		return;

	// pBuffer指向屏幕绘制行的头一个像点处 
	int nPitch;
	void* pBuffer = pCanvas->LockCanvas(nPitch);
	if (pBuffer == NULL)
		return;
	pBuffer = (char*)(pBuffer) + Clipper.y * nPitch;
	void* pPalette	= pNode->m_pPalette;// palette pointer
	void* pSprite = pNode->m_pBitmap;	// sprite pointer
	long nMask32 = pCanvas->m_nMask32;	// rgb mask32
	long nBuffNextLine = nPitch - Clipper.width * 2;// next line add
	long nSprSkip = pNode->m_nWidth * Clipper.top + Clipper.left;
	long nSprSkipPerLine = Clipper.left + Clipper.right;

	__asm
	{
		//使edi指向buffer绘制起点,	(以字节计)	
		mov		edi, pBuffer
		mov		eax, Clipper.x
		add		edi, eax
		add		edi, eax

		//使esi指向图块数据起点,(跳过nSprSkip个像点的图形数据)
		mov		esi, pSprite
		//_SkipSpriteAheadContent_:
		{
			mov		edx, nSprSkip
			or		edx, edx
			jz		_SkipSpriteAheadContentEnd_

			_SkipSpriteAheadContentLocalStart_:
			{
				read_alpha_2_ebx_run_length_2_eax
				or		ebx, ebx
				jnz		_SkipSpriteAheadContentLocalAlpha_
				sub		edx, eax
				jg		_SkipSpriteAheadContentLocalStart_
				neg		edx
				jmp		_SkipSpriteAheadContentEnd_

				_SkipSpriteAheadContentLocalAlpha_:
				{
					add		esi, eax
					sub		edx, eax
					jg		_SkipSpriteAheadContentLocalStart_
					add		esi, edx
					neg		edx
					jmp		_SkipSpriteAheadContentEnd_
				}
			}
		}
		_SkipSpriteAheadContentEnd_:

		mov		eax, nSprSkipPerLine
		or		eax, eax
		jnz		_DrawPartLineSection_	//if (nSprSkipPerLine) goto _DrawPartLineSection_

		//_DrawFullLineSection_:
		{
			//因为sprite不会跨行压缩，则运行到此处edx必为0，如sprite会跨行压缩则_DrawFullLineSection_需改			
			_DrawFullLineSection_Line_:
			{
				mov		edx, Clipper.width
				_DrawFullLineSection_LineLocal_:
				{
					read_alpha_2_ebx_run_length_2_eax

					or		ebx, ebx
					jnz		_DrawFullLineSection_LineLocal_Alpha_
					add		edi, eax
					add		edi, eax
					sub		edx, eax
					jg		_DrawFullLineSection_LineLocal_

					add		edi, nBuffNextLine
					dec		Clipper.height
					jnz		_DrawFullLineSection_Line_
					jmp		_EXIT_WAY_
				
					_DrawFullLineSection_LineLocal_Alpha_:
					{
						sub		edx, eax
						mov		ecx, eax
						mov     ebx, pPalette
						_DrawFullLineSection_CopyPixel_:
						{
							copy_pixel_use_eax
							loop	_DrawFullLineSection_CopyPixel_
						}
						or		edx, edx
						jnz		_DrawFullLineSection_LineLocal_

						add		edi, nBuffNextLine
						dec		Clipper.height
						jnz		_DrawFullLineSection_Line_
						jmp		_EXIT_WAY_
					}
				}
			}
		}

		_DrawPartLineSection_:
		{
			mov		eax, Clipper.left
			or		eax, eax
			jz		_DrawPartLineSection_SkipRight_Line_

			mov		eax, Clipper.right
			or		eax, eax
			jz		_DrawPartLineSection_SkipLeft_Line_
		}

		_DrawPartLineSection_Line_:
		{
			mov		eax, edx
			mov		edx, Clipper.width
			or		eax, eax
			jnz		_DrawPartLineSection_LineLocal_CheckAlpha_
			_DrawPartLineSection_LineLocal_:
			{
				read_alpha_2_ebx_run_length_2_eax
				_DrawPartLineSection_LineLocal_CheckAlpha_:
				or		ebx, ebx
				jnz		_DrawPartLineSection_LineLocal_Alpha_
				add		edi, eax
				add		edi, eax
				sub		edx, eax
				jg		_DrawPartLineSection_LineLocal_

				dec		Clipper.height
				jz		_EXIT_WAY_

				add		edi, edx
				add		edi, edx
				neg		edx
			}
			
			_DrawPartLineSection_LineSkip_:
			{
				add		edi, nBuffNextLine
				//跳过nSprSkipPerLine像素的sprite内容
				mov		eax, edx
				mov		edx, nSprSkipPerLine
				or		eax, eax
				jnz		_DrawPartLineSection_LineSkipLocal_CheckAlpha_
				_DrawPartLineSection_LineSkipLocal_:
				{
					read_alpha_2_ebx_run_length_2_eax
					
					_DrawPartLineSection_LineSkipLocal_CheckAlpha_:
					or		ebx, ebx
					jnz		_DrawPartLineSection_LineSkipLocal_Alpha_
					sub		edx, eax
					jg		_DrawPartLineSection_LineSkipLocal_
					neg		edx
					jmp		_DrawPartLineSection_Line_
					_DrawPartLineSection_LineSkipLocal_Alpha_:
					{
						add		esi, eax
						sub		edx, eax
						jg		_DrawPartLineSection_LineSkipLocal_
						add		esi, edx
						neg		edx
						jmp		_DrawPartLineSection_Line_
					}
				}
			}
			_DrawPartLineSection_LineLocal_Alpha_:
			{
				sub		edx, eax
				jle		_DrawPartLineSection_LineLocal_Alpha_Part_		//不能全画这eax个相同alpha值的像点，后面有点已经超出区域

				mov		ecx, eax
				mov     ebx, pPalette

				_DrawPartLineSection_CopyPixel_:
				{
					copy_pixel_use_eax
					loop	_DrawPartLineSection_CopyPixel_
				}
				jmp		_DrawPartLineSection_LineLocal_
			}
			_DrawPartLineSection_LineLocal_Alpha_Part_:
			{
				add		eax, edx
				mov		ecx, eax
				mov     ebx, pPalette
				_DrawPartLineSection_CopyPixel_Part_:
				{
					copy_pixel_use_eax
					loop	_DrawPartLineSection_CopyPixel_Part_
				}
			
				dec		Clipper.height
				jz		_EXIT_WAY_
				neg		edx
				mov		ebx, 255	//如果想要确切的原ebx(alpha)值可以在前头push ebx，此处pop获得
				jmp		_DrawPartLineSection_LineSkip_
			}
		}

		_DrawPartLineSection_SkipLeft_Line_:
		{
			mov		eax, edx
			mov		edx, Clipper.width
			or		eax, eax
			jnz		_DrawPartLineSection_SkipLeft_LineLocal_CheckAlpha_
			_DrawPartLineSection_SkipLeft_LineLocal_:
			{
				read_alpha_2_ebx_run_length_2_eax
				_DrawPartLineSection_SkipLeft_LineLocal_CheckAlpha_:
				or		ebx, ebx
				jnz		_DrawPartLineSection_SkipLeft_LineLocal_Alpha_
				add		edi, eax
				add		edi, eax
				sub		edx, eax
				jg		_DrawPartLineSection_SkipLeft_LineLocal_

				dec		Clipper.height
				jz		_EXIT_WAY_
			}
			
			_DrawPartLineSection_SkipLeft_LineSkip_:
			{
				add		edi, nBuffNextLine
				//跳过nSprSkipPerLine像素的sprite内容
				mov		edx, nSprSkipPerLine
				_DrawPartLineSection_SkipLeft_LineSkipLocal_:
				{
					read_alpha_2_ebx_run_length_2_eax
					or		ebx, ebx
					jnz		_DrawPartLineSection_SkipLeft_LineSkipLocal_Alpha_
					sub		edx, eax
					jg		_DrawPartLineSection_SkipLeft_LineSkipLocal_
					neg		edx
					jmp		_DrawPartLineSection_SkipLeft_Line_
					_DrawPartLineSection_SkipLeft_LineSkipLocal_Alpha_:
					{
						add		esi, eax
						sub		edx, eax
						jg		_DrawPartLineSection_SkipLeft_LineSkipLocal_
						add		esi, edx
						neg		edx
						jmp		_DrawPartLineSection_SkipLeft_Line_
					}
				}
			}
			_DrawPartLineSection_SkipLeft_LineLocal_Alpha_:
			{
				sub		edx, eax		;先把eax减了，这样後面就可以不需要保留eax了
				mov		ecx, eax						
				mov     ebx, pPalette
				_DrawPartLineSection_SkipLeft_CopyPixel_:
				{
					copy_pixel_use_eax
					loop	_DrawPartLineSection_SkipLeft_CopyPixel_
				}
				or		edx, edx
				jnz		_DrawPartLineSection_SkipLeft_LineLocal_
				dec		Clipper.height
				jg		_DrawPartLineSection_SkipLeft_LineSkip_
				jmp		_EXIT_WAY_
			}
		}

		_DrawPartLineSection_SkipRight_Line_:
		{
			mov		edx, Clipper.width
			_DrawPartLineSection_SkipRight_LineLocal_:
			{
				read_alpha_2_ebx_run_length_2_eax
				or		ebx, ebx
				jnz		_DrawPartLineSection_SkipRight_LineLocal_Alpha_
				add		edi, eax
				add		edi, eax
				sub		edx, eax
				jg		_DrawPartLineSection_SkipRight_LineLocal_

				dec		Clipper.height
				jz		_EXIT_WAY_

				add		edi, edx
				add		edi, edx
				neg		edx
			}
			
			_DrawPartLineSection_SkipRight_LineSkip_:
			{
				add		edi, nBuffNextLine
				//跳过nSprSkipPerLine像素的sprite内容
				mov		eax, edx
				mov		edx, nSprSkipPerLine
				or		eax, eax
				jnz		_DrawPartLineSection_SkipRight_LineSkipLocal_CheckAlpha_
				_DrawPartLineSection_SkipRight_LineSkipLocal_:
				{
					read_alpha_2_ebx_run_length_2_eax
					
					_DrawPartLineSection_SkipRight_LineSkipLocal_CheckAlpha_:
					or		ebx, ebx
					jnz		_DrawPartLineSection_SkipRight_LineSkipLocal_Alpha_
					sub		edx, eax
					jg		_DrawPartLineSection_SkipRight_LineSkipLocal_
					jmp		_DrawPartLineSection_SkipRight_Line_
					_DrawPartLineSection_SkipRight_LineSkipLocal_Alpha_:
					{
						add		esi, eax
						sub		edx, eax
						jg		_DrawPartLineSection_SkipRight_LineSkipLocal_
						jmp		_DrawPartLineSection_SkipRight_Line_
					}
				}
			}
			_DrawPartLineSection_SkipRight_LineLocal_Alpha_:
			{
				sub		edx, eax
				jle		_DrawPartLineSection_SkipRight_LineLocal_Alpha_Part_		//不能全画这eax个相同alpha值的像点，后面有点已经超出区域

				mov		ecx, eax				
				mov     ebx, pPalette
				_DrawPartLineSection_SkipRight_CopyPixel_:
				{
					copy_pixel_use_eax
					loop	_DrawPartLineSection_SkipRight_CopyPixel_
				}
				jmp		_DrawPartLineSection_SkipRight_LineLocal_
			}
			_DrawPartLineSection_SkipRight_LineLocal_Alpha_Part_:
			{
				add		eax, edx
				mov		ecx, eax
				mov     ebx, pPalette
				_DrawPartLineSection_SkipRight_CopyPixel_Part_:
				{
					copy_pixel_use_eax
					loop	_DrawPartLineSection_SkipRight_CopyPixel_Part_
				}
				neg		edx
				mov		ebx, 255	//如果想要确切的原ebx(alpha)值可以在前头push ebx，此处pop获得
				dec		Clipper.height
				jg		_DrawPartLineSection_SkipRight_LineSkip_
				jmp		_EXIT_WAY_
			}
		}
		_EXIT_WAY_:
	}
	pCanvas->UnlockCanvas();
}

//---------------------------------------------------------------------------
// 函数:	DrawSprite
// 功能:	绘制256色Sprite位图
// 参数:	KDrawNode*, KCanvas* 
// 返回:	void
//---------------------------------------------------------------------------
/*void g_DrawSprite_OLD(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;

	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of sprite
	long nHeight = pNode->m_nHeight;// height of sprite
	void* lpSprite = pNode->m_pBitmap;// sprite pointer
	void* lpPalette	= pNode->m_pPalette;// palette pointer

	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
	 	return;
	//当前代码图形左右同时被裁减时有误
	if (Clipper.left && Clipper.right)
		return;

	int nPitch;
	void* lpBuffer = pCanvas->LockCanvas(nPitch);
	if (lpBuffer == NULL)
		return;

	long nNextLine = nPitch - nWidth * 2;// next line add

	// 绘制函数的汇编代码
	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = lpBuffer + dwPitch * Clipper.y + nX * 2;
//---------------------------------------------------------------------------
		mov		eax, nPitch
		mov		ebx, Clipper.y
		mul		ebx
		mov     ebx, nX
		add		ebx, ebx
		add		eax, ebx
		mov		edi, lpBuffer
		add		edi, eax
//---------------------------------------------------------------------------
// 初始化 ESI 指向图块数据起点 
// (跳过Clipper.top行压缩图形数据)
//---------------------------------------------------------------------------
		mov		esi, lpSprite
		mov		ecx, Clipper.top
		or		ecx, ecx
		jz		loc_DrawSprite_0011

loc_DrawSprite_0008:

		mov		edx, nWidth

loc_DrawSprite_0009:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0010
		sub		edx, eax
		jg		loc_DrawSprite_0009
		dec     ecx
		jnz		loc_DrawSprite_0008
		jmp		loc_DrawSprite_0011

loc_DrawSprite_0010:

		add		esi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0009
		dec     ecx
		jnz		loc_DrawSprite_0008
//---------------------------------------------------------------------------
// 根据 Clipper.left, Clipper.right 分 4 种情况
//---------------------------------------------------------------------------
loc_DrawSprite_0011:

		mov		eax, Clipper.left
		or		eax, eax
		jnz		loc_DrawSprite_0012
		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSprite_0013
		jmp		loc_DrawSprite_0100

loc_DrawSprite_0012:

		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSprite_0014
		jmp		loc_DrawSprite_0200

loc_DrawSprite_0013:

		jmp		loc_DrawSprite_0300

loc_DrawSprite_0014:

		jmp		loc_DrawSprite_exit
//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSprite_0100:

		mov		edx, Clipper.width

loc_DrawSprite_0101:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0102

		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSprite_0100
		jmp		loc_DrawSprite_exit

loc_DrawSprite_0102:

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSprite_0103:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_DrawSprite_0103

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSprite_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSprite_0100
		jmp		loc_DrawSprite_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSprite_0200:

		mov		edx, Clipper.left

loc_DrawSprite_0201:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0202
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (左边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0201
		jz		loc_DrawSprite_0203
		neg		edx
		mov		eax, edx
		mov		edx, Clipper.width
		sub		edx, eax
		jg		loc_DrawSprite_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSprite_0200
		jmp		loc_DrawSprite_exit
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (左边界外)
//---------------------------------------------------------------------------
loc_DrawSprite_0202:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0201
		jz		loc_DrawSprite_0203
//---------------------------------------------------------------------------
// 把多减的宽度补回来
//---------------------------------------------------------------------------
		neg		edx
		sub		esi, edx
		sub		edi, edx
		sub		edi, edx

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSprite_Loop20:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSprite_Loop20

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSprite_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSprite_0200
		jmp		loc_DrawSprite_exit
//---------------------------------------------------------------------------
// 已处理完剪裁区 下面的处理相对简单
//---------------------------------------------------------------------------
loc_DrawSprite_0203:

		mov		edx, Clipper.width

loc_DrawSprite_0204:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0206
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (左边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSprite_0200
		jmp		loc_DrawSprite_exit
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (左边界内)
//---------------------------------------------------------------------------
loc_DrawSprite_0206:

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSprite_Loop21:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg		loc_DrawSprite_Loop21

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSprite_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSprite_0200
		jmp		loc_DrawSprite_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSprite_0300:

		mov		edx, Clipper.width

loc_DrawSprite_0301:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0303
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (右边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0301
		neg		edx
		jmp		loc_DrawSprite_0305
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (右边界内)
//---------------------------------------------------------------------------
loc_DrawSprite_0303:

		cmp		edx, eax
		jl		loc_DrawSprite_0304
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSprite_Loop30:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSprite_Loop30

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSprite_0301
		neg		edx
		jmp		loc_DrawSprite_0305

//---------------------------------------------------------------------------
// 连续点的个数 (eax) > 裁减后的宽度 (edx)
//---------------------------------------------------------------------------
loc_DrawSprite_0304:

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSprite_Loop31:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSprite_Loop31

		pop		edx
		pop		eax
//---------------------------------------------------------------------------
// 计算超过了剪裁区长度 => edx
//---------------------------------------------------------------------------
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
//---------------------------------------------------------------------------
// 处理超过了右边界的部分, edx = 超过右边界部分的长度
//---------------------------------------------------------------------------
loc_DrawSprite_0305:

		mov		eax, edx
		mov		edx, Clipper.right
		sub		edx, eax
		jle		loc_DrawSprite_0308

loc_DrawSprite_0306:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0307
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (右边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0306
		jmp		loc_DrawSprite_0308
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (右边界外)
//---------------------------------------------------------------------------
loc_DrawSprite_0307:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSprite_0306

loc_DrawSprite_0308:

		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSprite_0300
		jmp		loc_DrawSprite_exit

loc_DrawSprite_exit:
	}
	pCanvas->UnlockCanvas();
}*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// 函数:	DrawSpriteMixColor
// 功能:	绘制与某一颜色混合的256色Sprite位图
// 参数:	KDrawNode*, KCanvas* 
// 返回:	void
//---------------------------------------------------------------------------
void g_DrawSpriteMixColor(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;

	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of sprite
	long nHeight = pNode->m_nHeight;// height of sprite
	void* lpSprite = pNode->m_pBitmap;// sprite pointer
	void* lpPalette	= pNode->m_pPalette;// palette pointer

	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
		return;
	//当前代码图形左右同时被裁减时有误
	if (Clipper.left && Clipper.right)
		return;

	int nPitch;
	void* lpBuffer = pCanvas->LockCanvas(nPitch);
	if (lpBuffer == NULL)
		return;

	long nNextLine = nPitch - nWidth * 2;// next line add
    long nColor=pNode->m_nColor;
    long nAlpha=pNode->m_nAlpha;
	long nMask32 = pCanvas->m_nMask32;

	// 绘制函数的汇编代码
	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = lpBuffer + dwPitch * Clipper.y + nX * 2;
//---------------------------------------------------------------------------
		mov		eax, nPitch
		mov		ebx, Clipper.y
		mul		ebx
		mov     ebx, nX
		add		ebx, ebx
		add		eax, ebx
		mov		edi, lpBuffer
		add		edi, eax
//---------------------------------------------------------------------------
// 初始化 ESI 指向图块数据起点 
// (跳过Clipper.top行压缩图形数据)
//---------------------------------------------------------------------------
		mov		esi, lpSprite
		mov		ecx, Clipper.top
		or		ecx, ecx
		jz		loc_DrawSpriteMixColor_0011

loc_DrawSpriteMixColor_0008:

		mov		edx, nWidth

loc_DrawSpriteMixColor_0009:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteMixColor_0010
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0009
		dec     ecx
		jnz		loc_DrawSpriteMixColor_0008
		jmp		loc_DrawSpriteMixColor_0011

loc_DrawSpriteMixColor_0010:

		add		esi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0009
		dec     ecx
		jnz		loc_DrawSpriteMixColor_0008
//---------------------------------------------------------------------------
// 根据 Clipper.left, Clipper.right 分 4 种情况
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0011:

		mov		eax, Clipper.left
		or		eax, eax
		jnz		loc_DrawSpriteMixColor_0012
		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteMixColor_0013
		jmp		loc_DrawSpriteMixColor_0100

loc_DrawSpriteMixColor_0012:

		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteMixColor_0014
		jmp		loc_DrawSpriteMixColor_0200

loc_DrawSpriteMixColor_0013:

		jmp		loc_DrawSpriteMixColor_0300

loc_DrawSpriteMixColor_0014:

		jmp		loc_DrawSpriteMixColor_exit
//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0100:

		mov		edx, Clipper.width

loc_DrawSpriteMixColor_0101:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteMixColor_0102

		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteMixColor_0100
		jmp		loc_DrawSpriteMixColor_exit

loc_DrawSpriteMixColor_0102:

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteMixColor_0103:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		
		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   
		
		inc		edi
		inc		edi





		dec		ecx
		jnz		loc_DrawSpriteMixColor_0103

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteMixColor_0100
		jmp		loc_DrawSpriteMixColor_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0200:

		mov		edx, Clipper.left

loc_DrawSpriteMixColor_0201:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteMixColor_0202
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (左边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0201
		jz		loc_DrawSpriteMixColor_0203
		neg		edx
		mov		eax, edx
		mov		edx, Clipper.width
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteMixColor_0200
		jmp		loc_DrawSpriteMixColor_exit
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (左边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0202:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0201
		jz		loc_DrawSpriteMixColor_0203
//---------------------------------------------------------------------------
// 把多减的宽度补回来
//---------------------------------------------------------------------------
		neg		edx
		sub		esi, edx
		sub		edi, edx
		sub		edi, edx

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteMixColor_Loop20:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]

		
		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteMixColor_Loop20

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSpriteMixColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteMixColor_0200
		jmp		loc_DrawSpriteMixColor_exit
//---------------------------------------------------------------------------
// 已处理完剪裁区 下面的处理相对简单
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0203:

		mov		edx, Clipper.width

loc_DrawSpriteMixColor_0204:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteMixColor_0206
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (左边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteMixColor_0200
		jmp		loc_DrawSpriteMixColor_exit
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (左边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0206:

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteMixColor_Loop21:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]

		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg		loc_DrawSpriteMixColor_Loop21

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteMixColor_0200
		jmp		loc_DrawSpriteMixColor_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0300:

		mov		edx, Clipper.width

loc_DrawSpriteMixColor_0301:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteMixColor_0303
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (右边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0301
		neg		edx
		jmp		loc_DrawSpriteMixColor_0305
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (右边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0303:

		cmp		edx, eax
		jl		loc_DrawSpriteMixColor_0304
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteMixColor_Loop30:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]


		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteMixColor_Loop30

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0301
		neg		edx
		jmp		loc_DrawSpriteMixColor_0305

//---------------------------------------------------------------------------
// 连续点的个数 (eax) > 裁减后的宽度 (edx)
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0304:

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteMixColor_Loop31:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteMixColor_Loop31

		pop		edx
		pop		eax
//---------------------------------------------------------------------------
// 计算超过了剪裁区长度 => edx
//---------------------------------------------------------------------------
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
//---------------------------------------------------------------------------
// 处理超过了右边界的部分, edx = 超过右边界部分的长度
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0305:

		mov		eax, edx
		mov		edx, Clipper.right
		sub		edx, eax
		jle		loc_DrawSpriteMixColor_0308

loc_DrawSpriteMixColor_0306:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteMixColor_0307
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (右边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0306
		jmp		loc_DrawSpriteMixColor_0308
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (右边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteMixColor_0307:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteMixColor_0306

loc_DrawSpriteMixColor_0308:

		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteMixColor_0300
		jmp		loc_DrawSpriteMixColor_exit

loc_DrawSpriteMixColor_exit:

	}
	pCanvas->UnlockCanvas();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// 函数:	DrawSpriteWithColor
// 功能:	绘制与某一颜色混合的256色Sprite位图
// 参数:	KDrawNode*, KCanvas* 
// 返回:	void
//---------------------------------------------------------------------------
void g_DrawSpriteWithColor(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;

	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of sprite
	long nHeight = pNode->m_nHeight;// height of sprite
	void* lpSprite = pNode->m_pBitmap;// sprite pointer
	void* lpPalette	= pNode->m_pPalette;// palette pointer
	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
		return;
	//当前代码图形左右同时被裁减时有误
	if (Clipper.left && Clipper.right)
		return;

	int nPitch;
	void* lpBuffer = pCanvas->LockCanvas(nPitch);
	if (lpBuffer == NULL)
		return;

	long nNextLine = nPitch - nWidth * 2;// next line add
    long nColor=pNode->m_nColor;
    long nAlpha=pNode->m_nAlpha;
	long nMask32 = pCanvas->m_nMask32;
	// 绘制函数的汇编代码
	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = lpBuffer + dwPitch * Clipper.y + nX * 2;
//---------------------------------------------------------------------------
		mov		eax, nPitch
		mov		ebx, Clipper.y
		mul		ebx
		mov     ebx, nX
		add		ebx, ebx
		add		eax, ebx
		mov		edi, lpBuffer
		add		edi, eax
//---------------------------------------------------------------------------
// 初始化 ESI 指向图块数据起点 
// (跳过Clipper.top行压缩图形数据)
//---------------------------------------------------------------------------
		mov		esi, lpSprite
		mov		ecx, Clipper.top
		or		ecx, ecx
		jz		loc_DrawSpriteWithColor_0011

loc_DrawSpriteWithColor_0008:

		mov		edx, nWidth

loc_DrawSpriteWithColor_0009:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteWithColor_0010
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0009
		dec     ecx
		jnz		loc_DrawSpriteWithColor_0008
		jmp		loc_DrawSpriteWithColor_0011

loc_DrawSpriteWithColor_0010:

		add		esi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0009
		dec     ecx
		jnz		loc_DrawSpriteWithColor_0008
//---------------------------------------------------------------------------
// 根据 Clipper.left, Clipper.right 分 4 种情况
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0011:

		mov		eax, Clipper.left
		or		eax, eax
		jnz		loc_DrawSpriteWithColor_0012
		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteWithColor_0013
		jmp		loc_DrawSpriteWithColor_0100

loc_DrawSpriteWithColor_0012:

		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteWithColor_0014
		jmp		loc_DrawSpriteWithColor_0200

loc_DrawSpriteWithColor_0013:

		jmp		loc_DrawSpriteWithColor_0300

loc_DrawSpriteWithColor_0014:

		jmp		loc_DrawSpriteWithColor_exit
//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0100:

		mov		edx, Clipper.width

loc_DrawSpriteWithColor_0101:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteWithColor_0102

		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteWithColor_0100
		jmp		loc_DrawSpriteWithColor_exit

loc_DrawSpriteWithColor_0102:

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteWithColor_0103:

		movzx	eax, byte ptr[esi]
		inc		esi
//		mov		dx, [ebx + eax * 2]
        xor     edx,edx 
		
		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   
		
		inc		edi
		inc		edi





		dec		ecx
		jnz		loc_DrawSpriteWithColor_0103

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteWithColor_0100
		jmp		loc_DrawSpriteWithColor_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0200:

		mov		edx, Clipper.left

loc_DrawSpriteWithColor_0201:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteWithColor_0202
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (左边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0201
		jz		loc_DrawSpriteWithColor_0203
		neg		edx
		mov		eax, edx
		mov		edx, Clipper.width
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteWithColor_0200
		jmp		loc_DrawSpriteWithColor_exit
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (左边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0202:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0201
		jz		loc_DrawSpriteWithColor_0203
//---------------------------------------------------------------------------
// 把多减的宽度补回来
//---------------------------------------------------------------------------
		neg		edx
		sub		esi, edx
		sub		edi, edx
		sub		edi, edx

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteWithColor_Loop20:

		movzx	eax, byte ptr[esi]
		inc		esi
//		mov     dx, [ebx + eax * 2]
        xor     edx,edx 

		
		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteWithColor_Loop20

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSpriteWithColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteWithColor_0200
		jmp		loc_DrawSpriteWithColor_exit
//---------------------------------------------------------------------------
// 已处理完剪裁区 下面的处理相对简单
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0203:

		mov		edx, Clipper.width

loc_DrawSpriteWithColor_0204:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteWithColor_0206
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (左边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteWithColor_0200
		jmp		loc_DrawSpriteWithColor_exit
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (左边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0206:

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteWithColor_Loop21:

		movzx	eax, byte ptr[esi]
		inc		esi
//		mov     dx, [ebx + eax * 2]
        xor     edx,edx

		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg		loc_DrawSpriteWithColor_Loop21

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteWithColor_0200
		jmp		loc_DrawSpriteWithColor_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0300:

		mov		edx, Clipper.width

loc_DrawSpriteWithColor_0301:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteWithColor_0303
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (右边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0301
		neg		edx
		jmp		loc_DrawSpriteWithColor_0305
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (右边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0303:

		cmp		edx, eax
		jl		loc_DrawSpriteWithColor_0304
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteWithColor_Loop30:

		movzx	eax, byte ptr[esi]
		inc		esi
//		mov     dx, [ebx + eax * 2]
        xor     edx,edx 


		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteWithColor_Loop30

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0301
		neg		edx
		jmp		loc_DrawSpriteWithColor_0305

//---------------------------------------------------------------------------
// 连续点的个数 (eax) > 裁减后的宽度 (edx)
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0304:

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteWithColor_Loop31:

		movzx	eax, byte ptr[esi]
		inc		esi
//		mov     dx, [ebx + eax * 2]
        xor     edx, edx
		push    ebx
        push    ecx
		
        mov		ax, dx
		mov		bx, ax
		sal		eax, 16
		mov		ax, bx
		and		eax, nMask32

		mov		ecx, nColor
		mov		bx, cx
		sal		ecx, 16
		mov		cx, bx
		and		ecx, nMask32

		mov		ebx, nAlpha
		mul		ebx
		neg		ebx
		add		ebx, 0x20
		xchg	eax, ecx
		mul		ebx
		add		eax, ecx
		sar		eax, 5

		and		eax, nMask32
		mov		bx, ax
		sar		eax, 16
		or		ax, bx
		mov     [edi], ax
        
		pop     ecx
		pop     ebx   

		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteWithColor_Loop31

		pop		edx
		pop		eax
//---------------------------------------------------------------------------
// 计算超过了剪裁区长度 => edx
//---------------------------------------------------------------------------
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
//---------------------------------------------------------------------------
// 处理超过了右边界的部分, edx = 超过右边界部分的长度
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0305:

		mov		eax, edx
		mov		edx, Clipper.right
		sub		edx, eax
		jle		loc_DrawSpriteWithColor_0308

loc_DrawSpriteWithColor_0306:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteWithColor_0307
//---------------------------------------------------------------------------
// 处理 Alpha == 0 的像素 (右边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0306
		jmp		loc_DrawSpriteWithColor_0308
//---------------------------------------------------------------------------
// 处理 Alpha != 0 的像素 (右边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteWithColor_0307:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteWithColor_0306

loc_DrawSpriteWithColor_0308:

		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteWithColor_0300
		jmp		loc_DrawSpriteWithColor_exit

loc_DrawSpriteWithColor_exit:

	}
	pCanvas->UnlockCanvas();
}

static 	KMemClass	Buffer;
//---------------------------------------------------------------------------
void g_DrawSpriteBorder(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;

	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of sprite
	long nHeight = pNode->m_nHeight;// height of sprite
	BYTE* lpSprite = (PBYTE)pNode->m_pBitmap;// sprite pointer
//	long nPitch = pCanvas->m_nPitch;// canvas pitch
//	long nNextLine = nPitch - nWidth * 2;// next line add
    long nColor=pNode->m_nColor;
    long nAlpha=pNode->m_nAlpha;
	long nMask32 = pCanvas->m_nMask32;
	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
		return;
	//当前代码图形左右同时被裁减时有误
	if (Clipper.left && Clipper.right)
		return;

	PBYTE	lpBitmap;
	BYTE	byPixel;
	BYTE	byAlpha;
	BYTE	byFlags;
	int		nTop = 0;
	int		nLeft = nWidth;
	int		nRight = 0;
	int		nBottom = nHeight;
	int		dx = 0;
	int		dy = 0;
	
	// alloc bitmap memory
	if (!Buffer.Alloc(nWidth * nHeight))
		return;
	
	// bitmap pointer
	lpBitmap = (PBYTE)Buffer.GetMemPtr();
	
	// draw sprite
	while (dy < nHeight)
	{
		while (dx < nWidth)
		{
			byPixel = *lpSprite++;
			byAlpha = *lpSprite++;
			if (byAlpha < 200)
			{
				memset(lpBitmap, 0, byPixel);
			}
			else
			{
				memset(lpBitmap, 1, byPixel);
			}
			if (byAlpha != 0)
				lpSprite += byPixel;
			lpBitmap += byPixel;
			dx += byPixel;
		}
		dx = 0;
		dy++;
	}

	// draw border line horizontal
	for (dy = 0; dy < nHeight; dy++)
	{
		lpBitmap = (PBYTE)Buffer.GetMemPtr();
		lpBitmap += dy * nWidth;
		byFlags = 0;

		for (dx = 0; dx < nWidth; dx++)
		{
			if (*lpBitmap != byFlags)
			{
				if (byFlags == 0)
				{
					pCanvas->DrawPixel(nX + dx - 1, nY + dy, nColor);
					byFlags = 1;
				}
				else
				{
					pCanvas->DrawPixel(nX + dx, nY + dy, nColor);
					byFlags = 0;
				}
			}
			lpBitmap++;
		}
	}

	// draw border line vertical
	for (dx = 0; dx < nWidth; dx++)
	{
		lpBitmap = (PBYTE)Buffer.GetMemPtr();
		lpBitmap += dx;
		byFlags = 0;

		for (dy = 0; dy < nHeight; dy++)
		{
			if (*lpBitmap != byFlags)
			{
				if (byFlags == 0)
				{
					pCanvas->DrawPixel(nX + dx, nY + dy - 1, nColor);
					byFlags = 1;
				}
				else
				{
					pCanvas->DrawPixel(nX + dx, nY + dy, nColor);
					byFlags = 0;
				}
			}
			lpBitmap += nWidth;
		}
	}
}