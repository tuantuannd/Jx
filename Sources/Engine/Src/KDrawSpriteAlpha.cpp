//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDrawSpriteAlpha.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis), Wooy(Wu yue)
// Desc:	Sprite Alpha Drawing Function
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KCanvas.h"
#include "KDrawSpriteAlpha.h"
#include "DrawSpriteMP.inc"

//---------------------------------------------------------------------------
// 函数:	Draw Sprite nAlpha
// 功能:	绘制256色Sprite位图(不带预渲染)
// 参数:	KDrawNode*, KCanvas* 
// 返回:	void
//---------------------------------------------------------------------------
void g_DrawSpriteAlpha(void* node, void* canvas)
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
	int	 nAlpha;

	__asm
	{
        mov     eax, pPalette
        movd    mm0, eax        // mm0: pPalette

        mov     eax, Clipper.width
        movd    mm1, eax        // mm1: Clipper.width

        mov     eax, nMask32
        movd    mm2, eax        // mm2: nMask32

        // mm3: nAlpha

        // mm4: temp use

        // mm7: push ecx, pop ecx
        // mm6: push edx, pop edx
        // mm5: push eax, pop eax


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
				movd	edx, mm1    // mm1: Clipper.width
				_DrawFullLineSection_LineLocal_:
				{
					read_alpha_2_ebx_run_length_2_eax

					or		ebx, ebx
					jnz		_DrawFullLineSection_LineLocal_Alpha_
                    lea     edi, [edi + eax * 2]
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

						cmp		ebx, 255
						jl		_DrawFullLineSection_LineLocal_HalfAlpha_

						//_DrawFullLineSection_LineLocal_DirectCopy_:
						{
							movd     ebx, mm0   // mm0: pPalette
                            
                            sub ecx, 4
                            jl  _DrawFullLineSection_CopyPixel_continue
							_DrawFullLineSection_CopyPixel4_:
							{
								copy_4pixel_use_eax
                                
                                sub ecx, 4
                                jg     _DrawFullLineSection_CopyPixel4_
							}
							_DrawFullLineSection_CopyPixel_continue:
                            add ecx, 4
                            jz _DrawFullLineSection_CopyPixel_End 

						    _DrawFullLineSection_CopyPixel_:
							{
								copy_pixel_use_eax
                                dec     ecx
                                jnz     _DrawFullLineSection_CopyPixel_
							}
                            _DrawFullLineSection_CopyPixel_End:

							or		edx, edx
							jnz		_DrawFullLineSection_LineLocal_
	
							add		edi, nBuffNextLine
							dec		Clipper.height
							jnz		_DrawFullLineSection_Line_
							jmp		_EXIT_WAY_
						}

						_DrawFullLineSection_LineLocal_HalfAlpha_:
						{
							movd    mm6, edx
							shr		ebx, 3
                            movd    mm3, ebx    // mm3: nAlpha
							_DrawFullLineSection_HalfAlphaPixel_:
							{
								mix_2_pixel_color_alpha_use_eabdx
								loop	_DrawFullLineSection_HalfAlphaPixel_
							}
							movd    edx, mm6
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
			movd	edx, mm1    // mm1: Clipper.width
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
				cmp		ebx, 255
				jl		_DrawPartLineSection_LineLocal_HalfAlpha_
						
				//_DrawPartLineSection_LineLocal_DirectCopy_:
				{
					movd     ebx, mm0 // mm0: pPalette
					_DrawPartLineSection_CopyPixel_:
					{
						copy_pixel_use_eax
						loop	_DrawPartLineSection_CopyPixel_
					}
					jmp		_DrawPartLineSection_LineLocal_
				}
				
				_DrawPartLineSection_LineLocal_HalfAlpha_:
				{
					movd    mm6, edx
					shr		ebx, 3
                    movd    mm3, ebx    // mm3: nAlpha
					_DrawPartLineSection_HalfAlphaPixel_:
					{
						mix_2_pixel_color_alpha_use_eabdx
						loop	_DrawPartLineSection_HalfAlphaPixel_
					}
					movd    edx, mm6
					jmp		_DrawPartLineSection_LineLocal_
				}
			}
			_DrawPartLineSection_LineLocal_Alpha_Part_:
			{
				add		eax, edx
				mov		ecx, eax
				cmp		ebx, 255
				jl		_DrawPartLineSection_LineLocal_HalfAlpha_Part_
					
				//_DrawPartLineSection_LineLocal_DirectCopy_Part_:
				{
					movd    ebx,  mm0   // mm0: pPalette
					_DrawPartLineSection_CopyPixel_Part_:
					{
						copy_pixel_use_eax
						loop	_DrawPartLineSection_CopyPixel_Part_
					}
			
					dec		Clipper.height
					jz		_EXIT_WAY_
					neg		edx
					mov		ebx, 255
					jmp		_DrawPartLineSection_LineSkip_
				}
				
				_DrawPartLineSection_LineLocal_HalfAlpha_Part_:
				{
					movd    mm6, edx
					shr		ebx, 3
                    movd    mm3, ebx    // mm3: nAlpha
					_DrawPartLineSection_HalfAlphaPixel_Part_:
					{
						mix_2_pixel_color_alpha_use_eabdx
						loop	_DrawPartLineSection_HalfAlphaPixel_Part_
					}
					movd    edx, mm6
					neg		edx
					mov		ebx, nAlpha
					shl		ebx, 3			//如果想要确切的原ebx(alpha)值可以在前头push ebx，此处pop获得
					add		ebx, 1
					dec		Clipper.height
					jg		_DrawPartLineSection_LineSkip_
					jmp		_EXIT_WAY_
				}
			}
		}

		_DrawPartLineSection_SkipLeft_Line_:
		{
			mov		eax, edx
			movd	edx, mm1    // mm1: Clipper.width
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
				cmp		ebx, 255
				jl		_DrawPartLineSection_SkipLeft_LineLocal_nAlpha_
						
				//_DrawPartLineSection_SkipLeft_LineLocal_DirectCopy_:
				{
					movd    ebx, mm0    // mm0: pPalette
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

				_DrawPartLineSection_SkipLeft_LineLocal_nAlpha_:
				{
					movd    mm6, edx
					shr		ebx, 3
                    movd    mm3, ebx    // mm3: nAlpha
					_DrawPartLineSection_SkipLeft_HalfAlphaPixel_:
					{
						mix_2_pixel_color_alpha_use_eabdx
						loop	_DrawPartLineSection_SkipLeft_HalfAlphaPixel_
					}
					movd    edx, mm6
					or		edx, edx
					jnz		_DrawPartLineSection_SkipLeft_LineLocal_
					dec		Clipper.height
					jg		_DrawPartLineSection_SkipLeft_LineSkip_
					jmp		_EXIT_WAY_
				}
			}
		}

		_DrawPartLineSection_SkipRight_Line_:
		{
			movd	edx, mm1    // mm1: Clipper.width
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
				cmp		ebx, 255
				jl		_DrawPartLineSection_SkipRight_LineLocal_HalfAlpha_
						
				//_DrawPartLineSection_SkipRight_LineLocal_DirectCopy_:
				{
					movd    ebx, mm0    // mm0: pPalette
					_DrawPartLineSection_SkipRight_CopyPixel_:
					{
						copy_pixel_use_eax
						loop	_DrawPartLineSection_SkipRight_CopyPixel_
					}
					jmp		_DrawPartLineSection_SkipRight_LineLocal_
				}
				
				_DrawPartLineSection_SkipRight_LineLocal_HalfAlpha_:
				{
					movd    mm6, edx
					shr		ebx, 3
                    movd    mm3, ebx    // mm3: nAlpha
					_DrawPartLineSection_SkipRight_HalfAlphaPixel_:
					{
						mix_2_pixel_color_alpha_use_eabdx
						loop	_DrawPartLineSection_SkipRight_HalfAlphaPixel_
					}
					movd	edx, mm6
					jmp		_DrawPartLineSection_SkipRight_LineLocal_
				}
			}
			_DrawPartLineSection_SkipRight_LineLocal_Alpha_Part_:
			{
				add		eax, edx
				mov		ecx, eax
				cmp		ebx, 255
				jl		_DrawPartLineSection_SkipRight_LineLocal_HalfAlpha_Part_
					
				//_DrawPartLineSection_SkipRight_LineLocal_DirectCopy_Part_:
				{
					movd    ebx, mm0 // mm0: pPalette
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
				
				_DrawPartLineSection_SkipRight_LineLocal_HalfAlpha_Part_:
				{
					movd    mm6, edx
					shr		ebx, 3
                    movd    mm3, ebx    // mm3: nAlpha
					_DrawPartLineSection_SkipRight_HalfAlphaPixel_Part_:
					{
						mix_2_pixel_color_alpha_use_eabdx
						loop	_DrawPartLineSection_SkipRight_HalfAlphaPixel_Part_
					}
					movd	edx, mm6
					neg		edx
					mov		ebx, 128
					dec		Clipper.height
					jg		_DrawPartLineSection_SkipRight_LineSkip_//如果想要确切的原ebx(alpha)值可以在前头push ebx，此处pop获得
					jmp		_EXIT_WAY_
				}
			}
		}
		_EXIT_WAY_:
        emms
	}
	pCanvas->UnlockCanvas();
}

void g_DrawSpriteAlpha(void* node, void* canvas, int nExAlpha)
{
	if (nExAlpha == 0)
		return;

	if (nExAlpha >= 32)
	{
		g_DrawSpriteAlpha(node, canvas);
		return;
	}

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
	long nAlpha = pNode->m_nAlpha;// alpha value
	long nMask32 = pCanvas->m_nMask32;// rgb mask32
	WORD wAlpha = (WORD)nExAlpha;

	// 绘制函数的汇编代码
	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = lpBuffer + nPitch * Clipper.y + nX * 2;
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
		jz		loc_DrawSpriteAlpha_0011

loc_DrawSpriteAlpha_0008:

		mov		edx, nWidth

loc_DrawSpriteAlpha_0009:

//		movzx	eax, byte ptr[esi]
//		inc		esi
//		movzx	ebx, byte ptr[esi]
//		inc		esi
//		use uv, change to below
		xor		eax, eax
		xor		ebx, ebx
		mov		al,	 byte ptr[esi]
		inc		esi
		mov		bl,  byte ptr[esi]
		inc		esi
//		change	end
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0010
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0009
		dec     ecx
		jnz		loc_DrawSpriteAlpha_0008
		jmp		loc_DrawSpriteAlpha_0011

loc_DrawSpriteAlpha_0010:

		add		esi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0009
		dec     ecx
		jnz		loc_DrawSpriteAlpha_0008
//---------------------------------------------------------------------------
// 根据 Clipper.left, Clipper.right 分 4 种情况
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0011:

		mov		eax, Clipper.left
		or		eax, eax
		jnz		loc_DrawSpriteAlpha_0012
		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteAlpha_0013
		jmp		loc_DrawSpriteAlpha_0100

loc_DrawSpriteAlpha_0012:

		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteAlpha_0014
		jmp		loc_DrawSpriteAlpha_0200

loc_DrawSpriteAlpha_0013:

		jmp		loc_DrawSpriteAlpha_0300

loc_DrawSpriteAlpha_0014:

		jmp		loc_DrawSpriteAlpha_0400
//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0100:

		mov		edx, Clipper.width

loc_DrawSpriteAlpha_0101:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0102
		
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0100
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0102:
		push	eax
		push	edx
		mov		ax, wAlpha
		mul		bx
		shr		eax, 5
		mov		ebx, eax
		pop		edx
		pop		eax		
		jg		loc_lgzone
		mov		ebx, 0
loc_lgzone:
		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0110
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_0103:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0103

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0100
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0110:

		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0111:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0111

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0100
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0200:

		mov		edx, Clipper.left

loc_DrawSpriteAlpha_0201:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0202
//---------------------------------------------------------------------------
// 处理nAlpha == 0 的像素 (左边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0201
		jz		loc_DrawSpriteAlpha_0203
		neg		edx
		mov		eax, edx
		mov		edx, Clipper.width
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理nAlpha != 0 的像素 (左边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0202:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0201
		jz		loc_DrawSpriteAlpha_0203
//---------------------------------------------------------------------------
// 把多减的宽度补回来
//---------------------------------------------------------------------------
		neg		edx
		sub		esi, edx
		sub		edi, edx
		sub		edi, edx

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0210

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop20:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteAlpha_Loop20

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0210:

		push	eax
		push	edx
		mov		ecx, edx
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0211:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0211

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 已处理完剪裁区 下面的处理相对简单
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0203:

		mov		edx, Clipper.width

loc_DrawSpriteAlpha_0204:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0206
//---------------------------------------------------------------------------
// 处理nAlpha == 0的像素 (左边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理nAlpha != 0的像素 (左边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0206:

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0220

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop21:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg		loc_DrawSpriteAlpha_Loop21

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0220:

		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0221:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0221

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0300:

		mov		edx, Clipper.width

loc_DrawSpriteAlpha_0301:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0303
//---------------------------------------------------------------------------
// 处理 nAlpha == 0 的像素 (右边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0301
		neg		edx
		jmp		loc_DrawSpriteAlpha_0305
//---------------------------------------------------------------------------
// 处理 nAlpha != 0 的像素 (右边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0303:

		cmp		edx, eax
		jl		loc_DrawSpriteAlpha_0304

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0310
		
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop30:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteAlpha_Loop30

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0301
		neg		edx
		jmp		loc_DrawSpriteAlpha_0305

loc_DrawSpriteAlpha_0310:

		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0311:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0311

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0301
		neg		edx
		jmp		loc_DrawSpriteAlpha_0305

//---------------------------------------------------------------------------
// 连续点的个数 (eax) > 裁减后的宽度 (edx)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0304:

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0320

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop31:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteAlpha_Loop31

		pop		edx
		pop		eax
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
		jmp		loc_DrawSpriteAlpha_0305

loc_DrawSpriteAlpha_0320:

		push	eax
		push	edx
		mov		ecx, edx
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0321:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0321

		pop		edx
		pop		eax
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
		jmp		loc_DrawSpriteAlpha_0305

//---------------------------------------------------------------------------
// 处理超过了右边界的部分, edx = 超过右边界部分的长度
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0305:

		mov		eax, edx
		mov		edx, Clipper.right
		sub		edx, eax
		jle		loc_DrawSpriteAlpha_0308

loc_DrawSpriteAlpha_0306:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0307
//---------------------------------------------------------------------------
// 处理 nAlpha == 0 的像素 (右边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0306
		jmp		loc_DrawSpriteAlpha_0308
//---------------------------------------------------------------------------
// 处理 nAlpha != 0 的像素 (右边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0307:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0306

loc_DrawSpriteAlpha_0308:

		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0300
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0400:		// Line Begin

		mov		edx, Clipper.left

loc_Draw_GetLength:						// edx 记录该行压缩数据到裁剪左端的长度，可能是负值

		movzx	eax, byte ptr[esi]		// 取出压缩数据的长度
		inc		esi
		movzx	ebx, byte ptr[esi]		// 取出Alpha值
		inc		esi
		cmp		edx, eax
		jge		loc_Draw_AllLeft		// edx >= eax, 所有数据都在左边界外
		mov		ecx, Clipper.width		// ecx 得到Clipper宽度
		add		ecx, edx				// ecx = Clipper宽度 + 压缩数据左端被剪长度
		cmp		ecx, 0
		jle		loc_Draw_AllRight		// ecx <= 0，所有数据都在右边界外
		sub		ecx, eax				// 比较压缩数据长度和 ecx ，ecx小于0的话，ecx值为该段右端裁剪长度
		jge		loc_Draw_GetLength_0	// ecx >= eax 说明右端无裁剪
		cmp		edx, 0
		jl		loc_Draw_RightClip		// 左端有裁剪，右端也有
		jmp		loc_Draw_AllClip
loc_Draw_GetLength_0:
		cmp		edx, 0
		jl		loc_Draw_NoClip			// 左右都没裁剪
		jmp		loc_Draw_LeftClip
//---------------------------------------------------------------------------
// 全在左端外
//---------------------------------------------------------------------------
loc_Draw_AllLeft:
		or		ebx, ebx
		jnz		loc_Draw_AllLeft_1
//loc_Draw_AllLeft_0:	// alpha == 0
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
loc_Draw_AllLeft_1: // alpha != 0
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
//---------------------------------------------------------------------------
// 全在右端外
//---------------------------------------------------------------------------
loc_Draw_AllRight:
		or		ebx, ebx
		jnz		loc_Draw_AllRight_1
//loc_Draw_AllRight_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		mov		ebx, edx
		add		ebx, Clipper.width
		add		ebx, Clipper.right
		cmp		ebx, 0
		jl		loc_Draw_GetLength
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400	// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit		
loc_Draw_AllRight_1:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax
		mov		ebx, edx
		add		ebx, Clipper.width
		add		ebx, Clipper.right
		cmp		ebx, 0
		jl		loc_Draw_GetLength
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400	// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理左右端都不用裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_NoClip:
		or		ebx, ebx
		jnz		loc_Draw_NoClip_1
//loc_Draw_NoClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
loc_Draw_NoClip_1:
		cmp		ebx, 255
		jl		loc_Draw_NoClip_Alpha
		push	eax
		push	edx
		mov		ecx, eax
		mov		ebx, lpPalette

loc_Draw_NoClip_Copy:
		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_Draw_NoClip_Copy
		
		pop		edx
		pop		eax
		sub		edx, eax
		jmp		loc_Draw_GetLength

loc_Draw_NoClip_Alpha:
		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx
			
loc_Draw_NoClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_NoClip_Alpha_LOOP
		
		pop		edx
		pop		eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
//---------------------------------------------------------------------------
// 处理左右端同时要裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_AllClip:
		or		ebx, ebx				// 设置标志位
		jnz		loc_Draw_AllClip_1		// Alpha值不为零的处理
//loc_Draw_AllClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		neg		ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength		// Spr该行没完，接着处理
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
loc_Draw_AllClip_1:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax				// edx - eax < 0

		add		edi, edx				// 补回前面多减的部分
		add		edi, edx				// edi和esi指向实际要
		add		esi, edx				// 绘制的部分
		
		cmp		ebx, 255
		jl		loc_Draw_AllClip_Alpha
		push	eax
		push	edx
		push	ecx
		mov		ecx, Clipper.width		// 前后都被裁剪，所以绘制长度为Clipper.width
		mov		ebx, lpPalette
		
loc_Draw_AllClip_Copy:
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jnz		loc_Draw_AllClip_Copy
		
		pop		ecx
		pop		edx
		pop		eax
		jmp		loc_Draw_AllClip_End

loc_Draw_AllClip_Alpha:
		push	eax
		push	edx
		push	ecx
		mov		ecx, Clipper.width
		shr     ebx, 3
		mov		nAlpha, ebx
			
loc_Draw_AllClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_AllClip_Alpha_LOOP
		
		pop		ecx
		pop		edx
		pop		eax
loc_Draw_AllClip_End:
		neg		ecx
		add		edi, ecx				// 把edi、esi指针指向下一段
		add		edi, ecx
		add		esi, ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength		// Spr该行没完，接着处理
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理只有左端要裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_LeftClip:
		or		ebx, ebx
		jnz		loc_Draw_LeftClip_1

//loc_Draw_LeftClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
loc_Draw_LeftClip_1:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax
		add		edi, edx
		add		edi, edx
		add		esi, edx

		cmp		ebx, 255
		jl		loc_Draw_LeftClip_Alpha
		push	eax
		push	edx
		mov		ecx, edx
		neg		ecx
		mov     ebx, lpPalette
		
loc_Draw_LeftClip_Copy:
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_Draw_LeftClip_Copy
		
		pop		edx
		pop		eax
		jmp		loc_Draw_GetLength

loc_Draw_LeftClip_Alpha:
		push	eax
		push	edx
		mov		ecx, edx
		neg		ecx
		shr     ebx, 3
		mov		nAlpha, ebx
		
loc_Draw_LeftClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_LeftClip_Alpha_LOOP
		
		pop		edx
		pop		eax
		jmp		loc_Draw_GetLength
//---------------------------------------------------------------------------
// 处理只有右端要裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_RightClip:
		or		ebx, ebx
		jnz		loc_Draw_RightClip_1

//loc_Draw_RightClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		neg		ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400	// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit

loc_Draw_RightClip_1:
		sub		edx, eax
		cmp		ebx, 255
		jl		loc_Draw_RightClip_Alpha
		push	eax
		push	edx
		push	ecx
		add		ecx, eax					// 得到实际绘制的长度
		mov		ebx, lpPalette
		
loc_Draw_RightClip_Copy:
		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_Draw_RightClip_Copy
		
		pop		ecx
		pop		edx
		pop		eax
		jmp		loc_Draw_RightClip_End

loc_Draw_RightClip_Alpha:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		jmp		loc_Draw_RightClip_End
		push	eax
		push	edx
		push	ecx
		add		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx
			
loc_Draw_RightClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_RightClip_Alpha_LOOP

		pop		ecx
		pop		edx
		pop		eax
		
loc_Draw_RightClip_End:
		neg		ecx
		add		edi, ecx				// 把edi、esi指针指向下一段
		add		edi, ecx
		add		esi, ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength		// Spr该行没完，接着处理
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
			
loc_DrawSpriteAlpha_exit:
	}
	pCanvas->UnlockCanvas();
}


//三级alpha绘制
void g_DrawSprite3LevelAlpha(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;

	void* pSprite = pNode->m_pBitmap;	// sprite pointer
	void* pPalette	= pNode->m_pPalette;// palette pointer

	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(pNode->m_nX, pNode->m_nY, pNode->m_nWidth, pNode->m_nHeight, &Clipper))
		return;

	int nPitch;
	void* pBuffer = pCanvas->LockCanvas(nPitch);
	if (pBuffer == NULL)
		return;

	long nMask32 = pCanvas->m_nMask32;	// rgb mask32

	// pBuffer指向屏幕起点的偏移位置 (以字节计)
	pBuffer = (char*)pBuffer + Clipper.y * nPitch + Clipper.x * 2;
	long nBuffNextLine = nPitch - Clipper.width * 2;// next line add
	long nSprSkip = pNode->m_nWidth * Clipper.top + Clipper.left;
	long nSprSkipPerLine = Clipper.left + Clipper.right;

	__asm
	{
        mov     eax, pPalette
        movd    mm0, eax        // mm0: pPalette

        mov     eax, Clipper.width
        movd    mm1, eax        // mm1: Clipper.width

        mov     eax, nMask32
        movd    mm2, eax        // mm2: nMask32

        // mm3: nAlpha
        // mm4: 32 - nAlpha

        // mm7: push ecx, pop ecx
        // mm6: push edx, pop edx
        // mm5: push eax, pop eax

		//使edi指向canvas绘制起点,使esi指向图块数据起点,(跳过nSprSkip个像点的图形数据)
		mov		edi, pBuffer
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
				movd	edx, mm1    // mm1: Clipper.width
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
						movd    mm5, eax
						mov		ecx, eax

						cmp		ebx, 200
						jl		_DrawFullLineSection_LineLocal_HalfAlpha_

						//_DrawFullLineSection_LineLocal_DirectCopy_:
						{
							movd    ebx, mm0    // mm0: pPalette
							_DrawFullLineSection_CopyPixel_:
							{
								copy_pixel_use_eax
								loop	_DrawFullLineSection_CopyPixel_
							}

							movd    eax, mm5
							sub		edx, eax
							jg		_DrawFullLineSection_LineLocal_
	
							add		edi, nBuffNextLine
							dec		Clipper.height
							jnz		_DrawFullLineSection_Line_
							jmp		_EXIT_WAY_
						}

						_DrawFullLineSection_LineLocal_HalfAlpha_:
						{
        					movd    mm6, edx
							_DrawFullLineSection_HalfAlphaPixel_:
							{
								mix_2_pixel_color_use_eabdx
								loop	_DrawFullLineSection_HalfAlphaPixel_
							}
        					movd	edx, mm6
							movd    eax, mm5
							sub		edx, eax
							jg		_DrawFullLineSection_LineLocal_

							add		edi, nBuffNextLine
							dec		Clipper.height
							jnz		_DrawFullLineSection_Line_
							jmp		_EXIT_WAY_
						}
					}
				}
			}
		}

		_DrawPartLineSection_:
		{
			_DrawPartLineSection_Line_:
			{
				mov		eax, edx
				movd	edx, mm1    // mm1: Clipper.width
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
					cmp		eax, edx
					jnl		_DrawPartLineSection_LineLocal_Alpha_Part_		//不能全画这eax个相同alpha值的像点，后面有点已经超出区域

					movd	mm5, eax
					mov		ecx, eax
					cmp		ebx, 200
					jl		_DrawPartLineSection_LineLocal_HalfAlpha_
						
					//_DrawPartLineSection_LineLocal_DirectCopy_:
					{
						movd    ebx, mm0    // mm0: pPalette
						_DrawPartLineSection_CopyPixel_:
						{
							copy_pixel_use_eax
							loop	_DrawPartLineSection_CopyPixel_
						}						
						movd    eax, mm5
						sub		edx, eax
						jmp		_DrawPartLineSection_LineLocal_
					}
					
					_DrawPartLineSection_LineLocal_HalfAlpha_:
					{
    					movd    mm6, edx
						_DrawPartLineSection_HalfAlphaPixel_:
						{
							mix_2_pixel_color_use_eabdx
							loop	_DrawPartLineSection_HalfAlphaPixel_
						}
       					movd	edx, mm6
						movd    eax, mm5
						sub		edx, eax
						jmp		_DrawPartLineSection_LineLocal_
					}
				}

				_DrawPartLineSection_LineLocal_Alpha_Part_:
				{
					movd    mm5, eax
					mov		ecx, edx
					cmp		ebx, 200
					jl		_DrawPartLineSection_LineLocal_HalfAlpha_Part_
						
					//_DrawPartLineSection_LineLocal_DirectCopy_Part_:
					{
						movd    ebx, mm0    // mm0: pPalette
						_DrawPartLineSection_CopyPixel_Part_:
						{
							copy_pixel_use_eax
							loop	_DrawPartLineSection_CopyPixel_Part_
						}						
						movd    eax, mm5
				
						dec		Clipper.height
						jz		_EXIT_WAY_

						sub		eax, edx
						mov		edx, eax
						mov		ebx, 255	//如果想要确切的原ebx(alpha)值可以在前头push ebx，此处pop获得
						jmp		_DrawPartLineSection_LineSkip_
					}
					
					_DrawPartLineSection_LineLocal_HalfAlpha_Part_:
					{
    					movd    mm6, edx
						_DrawPartLineSection_HalfAlphaPixel_Part_:
						{
							mix_2_pixel_color_use_eabdx
							loop	_DrawPartLineSection_HalfAlphaPixel_Part_
						}
       					movd	edx, mm6
						movd    eax, mm5
						dec		Clipper.height
						jz		_EXIT_WAY_
						sub		eax, edx
						mov		edx, eax
						mov		ebx, 128	//如果想要确切的原ebx(alpha)值可以在前头push ebx，此处pop获得
						jmp		_DrawPartLineSection_LineSkip_
					}
				}
			}
		}
		_EXIT_WAY_:
        emms
	}
	pCanvas->UnlockCanvas();
}

/*void g_DrawSpriteAlpha_OLD(void* node, void* canvas)
{
	KDrawNode* pNode = (KDrawNode *)node;
	KCanvas* pCanvas = (KCanvas *)canvas;
	
	long nX = pNode->m_nX;// x coord
	long nY = pNode->m_nY;// y coord
	long nWidth = pNode->m_nWidth;// width of sprite
	long nHeight = pNode->m_nHeight;// height of sprite
	void* lpSprite = pNode->m_pBitmap;// sprite pointer
	void* lpPalette	= pNode->m_pPalette;// palette pointer
	void* lpBuffer = pCanvas->m_pCanvas;// canvas buffer
	long nPitch = pCanvas->m_nPitch;// canvas pitch
	long nNextLine = nPitch - nWidth * 2;// next line add
	long nAlpha = pNode->m_nAlpha;// alpha value
	long nMask32 = pCanvas->m_nMask32;// rgb mask32

	// 对绘制区域进行裁剪
	KClipper Clipper;
	if (!pCanvas->MakeClip(nX, nY, nWidth, nHeight, &Clipper))
		return;
	//当前代码图形左右同时被裁减时有误
	if (Clipper.left && Clipper.right)
		return;
	
	// 绘制函数的汇编代码
	__asm
	{
//---------------------------------------------------------------------------
// 计算 EDI 指向屏幕起点的偏移量 (以字节计)
// edi = lpBuffer + nPitch * Clipper.y + nX * 2;
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
		jz		loc_DrawSpriteAlpha_0011

loc_DrawSpriteAlpha_0008:

		mov		edx, nWidth

loc_DrawSpriteAlpha_0009:

//		movzx	eax, byte ptr[esi]
//		inc		esi
//		movzx	ebx, byte ptr[esi]
//		inc		esi
//		use uv, change to below
		xor		eax, eax
		xor		ebx, ebx
		mov		al,	 byte ptr[esi]
		inc		esi
		mov		bl,  byte ptr[esi]
		inc		esi
//		change	end
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0010
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0009
		dec     ecx
		jnz		loc_DrawSpriteAlpha_0008
		jmp		loc_DrawSpriteAlpha_0011

loc_DrawSpriteAlpha_0010:

		add		esi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0009
		dec     ecx
		jnz		loc_DrawSpriteAlpha_0008
//---------------------------------------------------------------------------
// 根据 Clipper.left, Clipper.right 分 4 种情况
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0011:

		mov		eax, Clipper.left
		or		eax, eax
		jnz		loc_DrawSpriteAlpha_0012
		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteAlpha_0013
		jmp		loc_DrawSpriteAlpha_0100

loc_DrawSpriteAlpha_0012:

		mov		eax, Clipper.right
		or		eax, eax
		jnz		loc_DrawSpriteAlpha_0014
		jmp		loc_DrawSpriteAlpha_0200

loc_DrawSpriteAlpha_0013:

		jmp		loc_DrawSpriteAlpha_0300

loc_DrawSpriteAlpha_0014:

		jmp		loc_DrawSpriteAlpha_0400
//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0100:

		mov		edx, Clipper.width

loc_DrawSpriteAlpha_0101:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0102
		
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0100
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0102:
		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0110
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_0103:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0103

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0100
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0110:

		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0111:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0111

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0101
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0100
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 == 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0200:

		mov		edx, Clipper.left

loc_DrawSpriteAlpha_0201:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0202
//---------------------------------------------------------------------------
// 处理nAlpha == 0 的像素 (左边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0201
		jz		loc_DrawSpriteAlpha_0203
		neg		edx
		mov		eax, edx
		mov		edx, Clipper.width
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理nAlpha != 0 的像素 (左边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0202:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0201
		jz		loc_DrawSpriteAlpha_0203
//---------------------------------------------------------------------------
// 把多减的宽度补回来
//---------------------------------------------------------------------------
		neg		edx
		sub		esi, edx
		sub		edi, edx
		sub		edi, edx

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0210

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop20:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteAlpha_Loop20

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0210:

		push	eax
		push	edx
		mov		ecx, edx
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0211:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0211

		pop		edx
		pop		eax
		mov		ecx, edx
		mov		edx, Clipper.width
		sub		edx, ecx
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 已处理完剪裁区 下面的处理相对简单
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0203:

		mov		edx, Clipper.width

loc_DrawSpriteAlpha_0204:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0206
//---------------------------------------------------------------------------
// 处理nAlpha == 0的像素 (左边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理nAlpha != 0的像素 (左边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0206:

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0220

		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop21:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg		loc_DrawSpriteAlpha_Loop21

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jg		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

loc_DrawSpriteAlpha_0220:

		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0221:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0221

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0204
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0200
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 == 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0300:

		mov		edx, Clipper.width

loc_DrawSpriteAlpha_0301:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0303
//---------------------------------------------------------------------------
// 处理 nAlpha == 0 的像素 (右边界内)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0301
		neg		edx
		jmp		loc_DrawSpriteAlpha_0305
//---------------------------------------------------------------------------
// 处理 nAlpha != 0 的像素 (右边界内)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0303:

		cmp		edx, eax
		jl		loc_DrawSpriteAlpha_0304

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0310
		
		push	eax
		push	edx
		mov		ecx, eax
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop30:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteAlpha_Loop30

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0301
		neg		edx
		jmp		loc_DrawSpriteAlpha_0305

loc_DrawSpriteAlpha_0310:

		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0311:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0311

		pop		edx
		pop		eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0301
		neg		edx
		jmp		loc_DrawSpriteAlpha_0305

//---------------------------------------------------------------------------
// 连续点的个数 (eax) > 裁减后的宽度 (edx)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0304:

		cmp		ebx, 255
		jl		loc_DrawSpriteAlpha_0320

		push	eax
		push	edx
		mov		ecx, edx
		mov     ebx, lpPalette

loc_DrawSpriteAlpha_Loop31:

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_DrawSpriteAlpha_Loop31

		pop		edx
		pop		eax
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
		jmp		loc_DrawSpriteAlpha_0305

loc_DrawSpriteAlpha_0320:

		push	eax
		push	edx
		mov		ecx, edx
		shr     ebx, 3
		mov		nAlpha, ebx

loc_DrawSpriteAlpha_0321:

		push	ecx
		mov     ebx, lpPalette

		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb

		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_DrawSpriteAlpha_0321

		pop		edx
		pop		eax
		sub		eax, edx
		mov		edx, eax
		add		esi, eax
		add		edi, eax
		add		edi, eax
		jmp		loc_DrawSpriteAlpha_0305

//---------------------------------------------------------------------------
// 处理超过了右边界的部分, edx = 超过右边界部分的长度
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0305:

		mov		eax, edx
		mov		edx, Clipper.right
		sub		edx, eax
		jle		loc_DrawSpriteAlpha_0308

loc_DrawSpriteAlpha_0306:

		movzx	eax, byte ptr[esi]
		inc		esi
		movzx	ebx, byte ptr[esi]
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSpriteAlpha_0307
//---------------------------------------------------------------------------
// 处理 nAlpha == 0 的像素 (右边界外)
//---------------------------------------------------------------------------
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0306
		jmp		loc_DrawSpriteAlpha_0308
//---------------------------------------------------------------------------
// 处理 nAlpha != 0 的像素 (右边界外)
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0307:

		add		esi, eax
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jg		loc_DrawSpriteAlpha_0306

loc_DrawSpriteAlpha_0308:

		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0300
		jmp		loc_DrawSpriteAlpha_exit

//---------------------------------------------------------------------------
// 左边界裁剪量 != 0
// 右边界裁剪量 != 0
//---------------------------------------------------------------------------
loc_DrawSpriteAlpha_0400:		// Line Begin

		mov		edx, Clipper.left

loc_Draw_GetLength:						// edx 记录该行压缩数据到裁剪左端的长度，可能是负值

		movzx	eax, byte ptr[esi]		// 取出压缩数据的长度
		inc		esi
		movzx	ebx, byte ptr[esi]		// 取出Alpha值
		inc		esi
		cmp		edx, eax
		jge		loc_Draw_AllLeft		// edx >= eax, 所有数据都在左边界外
		mov		ecx, Clipper.width		// ecx 得到Clipper宽度
		add		ecx, edx				// ecx = Clipper宽度 + 压缩数据左端被剪长度
		cmp		ecx, 0
		jle		loc_Draw_AllRight		// ecx <= 0，所有数据都在右边界外
		sub		ecx, eax				// 比较压缩数据长度和 ecx ，ecx小于0的话，ecx值为该段右端裁剪长度
		jge		loc_Draw_GetLength_0	// ecx >= eax 说明右端无裁剪
		cmp		edx, 0
		jl		loc_Draw_RightClip		// 左端有裁剪，右端也有
		jmp		loc_Draw_AllClip
loc_Draw_GetLength_0:
		cmp		edx, 0
		jl		loc_Draw_NoClip			// 左右都没裁剪
		jmp		loc_Draw_LeftClip
//---------------------------------------------------------------------------
// 全在左端外
//---------------------------------------------------------------------------
loc_Draw_AllLeft:
		or		ebx, ebx
		jnz		loc_Draw_AllLeft_1
//loc_Draw_AllLeft_0:	// alpha == 0
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
loc_Draw_AllLeft_1: // alpha != 0
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
//---------------------------------------------------------------------------
// 全在右端外
//---------------------------------------------------------------------------
loc_Draw_AllRight:
		or		ebx, ebx
		jnz		loc_Draw_AllRight_1
//loc_Draw_AllRight_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		mov		ebx, edx
		add		ebx, Clipper.width
		add		ebx, Clipper.right
		cmp		ebx, 0
		jl		loc_Draw_GetLength
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400	// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit		
loc_Draw_AllRight_1:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax
		mov		ebx, edx
		add		ebx, Clipper.width
		add		ebx, Clipper.right
		cmp		ebx, 0
		jl		loc_Draw_GetLength
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400	// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理左右端都不用裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_NoClip:
		or		ebx, ebx
		jnz		loc_Draw_NoClip_1
//loc_Draw_NoClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
loc_Draw_NoClip_1:
		cmp		ebx, 255
		jl		loc_Draw_NoClip_Alpha
		push	eax
		push	edx
		mov		ecx, eax
		mov		ebx, lpPalette

loc_Draw_NoClip_Copy:
		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_Draw_NoClip_Copy
		
		pop		edx
		pop		eax
		sub		edx, eax
		jmp		loc_Draw_GetLength

loc_Draw_NoClip_Alpha:
		push	eax
		push	edx
		mov		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx
			
loc_Draw_NoClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_NoClip_Alpha_LOOP
		
		pop		edx
		pop		eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
//---------------------------------------------------------------------------
// 处理左右端同时要裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_AllClip:
		or		ebx, ebx				// 设置标志位
		jnz		loc_Draw_AllClip_1		// Alpha值不为零的处理
//loc_Draw_AllClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		neg		ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength		// Spr该行没完，接着处理
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
loc_Draw_AllClip_1:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax				// edx - eax < 0

		add		edi, edx				// 补回前面多减的部分
		add		edi, edx				// edi和esi指向实际要
		add		esi, edx				// 绘制的部分
		
		cmp		ebx, 255
		jl		loc_Draw_AllClip_Alpha
		push	eax
		push	edx
		push	ecx
		mov		ecx, Clipper.width		// 前后都被裁剪，所以绘制长度为Clipper.width
		mov		ebx, lpPalette
		
loc_Draw_AllClip_Copy:
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jnz		loc_Draw_AllClip_Copy
		
		pop		ecx
		pop		edx
		pop		eax
		jmp		loc_Draw_AllClip_End

loc_Draw_AllClip_Alpha:
		push	eax
		push	edx
		push	ecx
		mov		ecx, Clipper.width
		shr     ebx, 3
		mov		nAlpha, ebx
			
loc_Draw_AllClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_AllClip_Alpha_LOOP
		
		pop		ecx
		pop		edx
		pop		eax
loc_Draw_AllClip_End:
		neg		ecx
		add		edi, ecx				// 把edi、esi指针指向下一段
		add		edi, ecx
		add		esi, ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength		// Spr该行没完，接着处理
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
//---------------------------------------------------------------------------
// 处理只有左端要裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_LeftClip:
		or		ebx, ebx
		jnz		loc_Draw_LeftClip_1

//loc_Draw_LeftClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		jmp		loc_Draw_GetLength
loc_Draw_LeftClip_1:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		sub		edx, eax
		add		edi, edx
		add		edi, edx
		add		esi, edx

		cmp		ebx, 255
		jl		loc_Draw_LeftClip_Alpha
		push	eax
		push	edx
		mov		ecx, edx
		neg		ecx
		mov     ebx, lpPalette
		
loc_Draw_LeftClip_Copy:
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec     ecx
		jg      loc_Draw_LeftClip_Copy
		
		pop		edx
		pop		eax
		jmp		loc_Draw_GetLength

loc_Draw_LeftClip_Alpha:
		push	eax
		push	edx
		mov		ecx, edx
		neg		ecx
		shr     ebx, 3
		mov		nAlpha, ebx
		
loc_Draw_LeftClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_LeftClip_Alpha_LOOP
		
		pop		edx
		pop		eax
		jmp		loc_Draw_GetLength
//---------------------------------------------------------------------------
// 处理只有右端要裁剪的压缩段
//---------------------------------------------------------------------------
loc_Draw_RightClip:
		or		ebx, ebx
		jnz		loc_Draw_RightClip_1

//loc_Draw_RightClip_0:
		add		edi, eax
		add		edi, eax
		sub		edx, eax
		neg		ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400	// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit

loc_Draw_RightClip_1:
		sub		edx, eax
		cmp		ebx, 255
		jl		loc_Draw_RightClip_Alpha
		push	eax
		push	edx
		push	ecx
		add		ecx, eax					// 得到实际绘制的长度
		mov		ebx, lpPalette
		
loc_Draw_RightClip_Copy:
		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_Draw_RightClip_Copy
		
		pop		ecx
		pop		edx
		pop		eax
		jmp		loc_Draw_RightClip_End

loc_Draw_RightClip_Alpha:
		add		edi, eax
		add		edi, eax
		add		esi, eax
		jmp		loc_Draw_RightClip_End
		push	eax
		push	edx
		push	ecx
		add		ecx, eax
		shr     ebx, 3
		mov		nAlpha, ebx
			
loc_Draw_RightClip_Alpha_LOOP:
		
		push	ecx
		mov     ebx, lpPalette
		
		movzx	eax, byte ptr[esi]
		inc		esi
		mov     cx, [ebx + eax * 2]    // ecx = ...rgb
		mov		ax, cx                 // eax = ...rgb
		shl		eax, 16                // eax = rgb...
		mov		ax, cx                 // eax = rgbrgb
		and		eax, nMask32           // eax = .g.r.b
		mov		cx, [edi]              // ecx = ...rgb
		mov		bx, cx                 // ebx = ...rgb
		shl		ebx, 16                // ebx = rgb...
		mov		bx, cx                 // ebx = rgbrgb
		and		ebx, nMask32           // ebx = .g.r.b
		mov		ecx, nAlpha            // ecx = alpha
		mul		ecx                    // eax:edx = eax*ecx
		neg		ecx                    // ecx = -alpha
		add		ecx, 32                // ecx = 32 - alpha
		xchg	eax, ebx               // exchange eax,ebx
		mul		ecx                    // eax = eax * (32 - alpha)
		add		eax, ebx               // eax = eax + ebx
		shr		eax, 5                 // c = (c1 * alpha + c2 * (32 - alpha)) / 32
		and     eax, nMask32           // eax = .g.r.b
		mov     cx, ax                 // ecx = ...r.b
		shr     eax, 16                // eax = ....g.
		or      ax, cx                 // eax = ...rgb
		
		mov		[edi], ax
		inc		edi
		inc		edi
		pop		ecx
		dec		ecx
		jnz		loc_Draw_RightClip_Alpha_LOOP

		pop		ecx
		pop		edx
		pop		eax
		
loc_Draw_RightClip_End:
		neg		ecx
		add		edi, ecx				// 把edi、esi指针指向下一段
		add		edi, ecx
		add		esi, ecx
		cmp		ecx, Clipper.right
		jl		loc_Draw_GetLength		// Spr该行没完，接着处理
		add		edi, nNextLine
		dec		Clipper.height
		jnz		loc_DrawSpriteAlpha_0400// 行结束，下一行开始
		jmp		loc_DrawSpriteAlpha_exit
			
loc_DrawSpriteAlpha_exit:
	}
}*/