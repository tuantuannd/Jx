//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegIdct.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg 解码 MMX 版本 8x8 AAN IDCT
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
/****************************************************************************
  本函数摘取自 developer.intel.com

  ; esi - input and output data pointer
  ; the input data is tranposed and each 16 bit element in the 8x8 matrix 
  ;	is left aligned:
  ; for example in 11...1110000 format
  ; If the iDCT is of I macroblock then 0.5 needs to be added to the 
  ;	DC Component 
  ; (element[0][0] of the matrix)
****************************************************************************/
static __int64 x0005000200010001 = 0x0005000200010001;
static __int64 x0040000000000000 = 0x40000000000000;
static __int64 x5a825a825a825a82 = 0x5a825a825a825a82; // 23170
static __int64 x539f539f539f539f = 0x539f539f539f539f; // 21407
static __int64 x4546454645464546 = 0x4546454645464546; // 17734
static __int64 x61f861f861f861f8 = 0x61f861f861f861f8; // 25080
//---------------------------------------------------------------------------
static __int64 scratch1 = 0;
static __int64 scratch3 = 0;
static __int64 scratch5 = 0;
static __int64 scratch7 = 0;
//---------------------------------------------------------------------------
static WORD preSC[64] = {
		16384, 22725, 21407, 19266,  16384, 12873, 8867,  4520,
		22725, 31521, 29692, 26722,  22725, 17855, 12299, 6270,
		21407, 29692, 27969, 25172,  21407, 16819, 11585, 5906,
		19266, 26722, 25172, 22654,  19266, 15137, 10426, 5315,
		16384, 22725, 21407, 19266,  16384, 12873, 8867,  4520,
		12873, 17855, 16819, 15137,  25746, 20228, 13933, 7103,
		17734, 24598, 23170, 20853,  17734, 13933, 9597,  4892,
		18081, 25080, 23624, 21261,  18081, 14206, 9785,  4988
};
//---------------------------------------------------------------------------
// 函数:	jpeg_idct_mmx
// 功能:	MMX 版本 8x8 AAN IDCT
// 参数:	buf		解码后的缓存
// 返回:	void
//---------------------------------------------------------------------------
void jpeg_idct_mmx(short* buf)
{
	__asm
	{
		lea	ecx, [preSC];
		mov	esi, buf	; source
			
		;slot
		
		; column 0: even part
		; use V4, V12, V0, V8 to produce V22..V25
		movq mm0, dword ptr [ecx+8*12]		; maybe the first mul can be done together
		; with the dequantization in iHuff module ?
		;slot
		
		pmulhw mm0, dword ptr [esi+8*12]	; V12
		;slot
		
		movq mm1, dword ptr [ecx+8*4]
		;slot
		
		pmulhw mm1, dword ptr [esi+8*4]	; V4
		;slot
		
		movq mm3, dword ptr [ecx+8*0]
		psraw mm0, 1				; t64=t66
		
		pmulhw mm3, dword ptr [esi+8*0]	; V0
		;slot
		
		movq mm5, dword ptr [ecx+8*8]		; duplicate V4
		movq mm2, mm1                           ; added 11/1/96
		
		pmulhw mm5, dword ptr [esi+8*8]	; V8
		psubsw mm1, mm0				; V16
		
		
		pmulhw mm1, dword ptr x5a825a825a825a82 ; 23170 ->V18
		paddsw mm2, mm0                          ; V17
		
		movq mm0, mm2				; duplicate V17
		psraw mm2, 1				; t75=t82
		
		psraw mm0, 2  				; t72
		movq mm4, mm3				; duplicate V0
		
		paddsw mm3, mm5				; V19
		psubsw mm4, mm5				; V20 ;mm5 free
		
		;moved from the block below
		movq mm7, dword ptr [ecx+8*10]
		psraw mm3, 1				; t74=t81
		
		movq mm6, mm3				; duplicate t74=t81
		psraw mm4, 2				; t77=t79
		
		psubsw mm1, mm0				; V21 ; mm0 free
		paddsw mm3, mm2				; V22
		
		movq mm5, mm1				; duplicate V21
		paddsw mm1, mm4				; V23
		
		movq dword ptr [esi+8*4], mm3		; V22
		psubsw mm4, mm5				; V24; mm5 free
		
		movq dword ptr [esi+8*12], mm1		; V23
		psubsw mm6, mm2				; V25; mm2 free
		
		movq dword ptr [esi+8*0], mm4		; V24
		;slot
		
		; keep mm6 alive all along the next block
		;movq dword ptr [esi+8*8], mm6		; V25
		
		; column 0: odd part
		; use V2, V6, V10, V14 to produce V31, V39, V40, V41
		
		;moved above
		;movq mm7, dword ptr [ecx+8*10]
		
		pmulhw mm7, dword ptr [esi+8*10]		; V10
		;slot
		
		movq mm0, dword ptr [ecx+8*6]
		;slot
		
		pmulhw mm0, dword ptr [esi+8*6]		; V6
		;slot
		
		movq mm5, dword ptr [ecx+8*2]
		movq mm3, mm7					; duplicate V10
		
		pmulhw mm5, dword ptr [esi+8*2]		; V2
		;slot
		
		movq mm4, dword ptr [ecx+8*14]
		psubsw mm7, mm0					; V26
		
		pmulhw mm4, dword ptr [esi+8*14]		; V14
		paddsw mm3, mm0					; V29 ; free mm0
		
		movq mm1, mm7					; duplicate V26
		psraw mm3, 1					; t91=t94
		
		pmulhw mm7, dword ptr x539f539f539f539f 	; V33
		psraw mm1, 1					; t96
		
		movq mm0, mm5					; duplicate V2
		psraw mm4, 2					; t85=t87
		
		paddsw mm5, mm4					; V27
		psubsw mm0, mm4					; V28 ; free mm4
		
		movq mm2, mm0					; duplicate V28
		psraw mm5, 1					; t90=t93
		
		pmulhw mm0, dword ptr x4546454645464546 	; V35
		psraw mm2, 1					; t97
		
		movq mm4, mm5					; duplicate t90=t93
		psubsw mm1, mm2					; V32 ; free mm2
		
		pmulhw mm1, dword ptr x61f861f861f861f8 	; V36
		psllw mm7, 1					; t107
		
		paddsw mm5, mm3					; V31
		psubsw mm4, mm3					; V30 ; free mm3
		
		pmulhw mm4, dword ptr x5a825a825a825a82 	; V34
		nop ;slot
		
		psubsw mm0, mm1					; V38
		psubsw mm1, mm7					; V37 ; free mm7
		
		psllw mm1, 1					; t114
		;move from the next block
		movq mm3, mm6		; duplicate V25
		
		;move from the next block
		movq mm7, dword ptr [esi+8*4]			; V22
		psllw mm0, 1					; t110
		
		psubsw mm0, mm5					; V39 (mm5 still needed for next block)
		psllw mm4, 2					; t112
		
		;move from the next block
		movq mm2, dword ptr [esi+8*12]	; V23
		psubsw mm4, mm0					; V40
		
		paddsw mm1, mm4					; V41; free mm0
		;move from the next block
		psllw mm2, 1					; t117=t125
		
		; column 0: output butterfly
		;move above
		;movq mm3, mm6		; duplicate V25
		;movq mm7, dword ptr [esi+8*4]			; V22
		;movq mm2, dword ptr [esi+8*12]    		; V23
		;psllw mm2, 1					; t117=t125
		
		psubsw mm6, mm1					; tm6
		paddsw mm3, mm1					; tm8; free mm1
		
		movq mm1, mm7					; duplicate V22
		paddsw mm7, mm5					; tm0
		
		movq dword ptr [esi+8*8], mm3     		; tm8; free mm3
		psubsw mm1, mm5					; tm14; free mm5
		
		movq dword ptr [esi+8*6], mm6			; tm6; free mm6
		movq mm3, mm2					; duplicate t117=t125
		
		movq mm6, dword ptr [esi+8*0]			; V24
		paddsw mm2, mm0					; tm2
		
		movq dword ptr [esi+8*0], mm7      		; tm0; free mm7
		psubsw mm3, mm0					; tm12; free mm0
		
		movq dword ptr [esi+8*14], mm1			; tm14; free mm1
		psllw mm6, 1					; t119=t123
		
		movq dword ptr [esi+8*2], mm2			; tm2; free mm2
		movq mm0, mm6					; duplicate t119=t123
		
		movq dword ptr [esi+8*12], mm3      		; tm12; free mm3
		paddsw mm6, mm4					; tm4
		
		;moved from next block
		movq mm1, dword ptr [ecx+8*5]
		psubsw mm0, mm4					; tm10; free mm4
		
		;moved from next block
		pmulhw mm1, dword ptr [esi+8*5]		; V5
		;slot
		
		movq dword ptr [esi+8*4], mm6			; tm4; free mm6
		;slot
		
		movq dword ptr [esi+8*10], mm0     		; tm10; free mm0
		;slot
		
		; column 1: even part
		; use V5, V13, V1, V9 to produce V56..V59
		;moved to prev block
		;movq mm1, dword ptr [ecx+8*5]
		;pmulhw mm1, dword ptr [esi+8*5]		; V5
		
		movq mm7, dword ptr [ecx+8*13]
		psllw mm1, 1					; t128=t130
		
		pmulhw mm7, dword ptr [esi+8*13]		; V13
		movq mm2, mm1					; duplicate t128=t130
		
		movq mm3, dword ptr [ecx+8*1]
		;slot
		
		pmulhw mm3, dword ptr [esi+8*1]		; V1
		;slot
		
		movq mm5, dword ptr [ecx+8*9]
		psubsw mm1, mm7					; V50
		
		pmulhw mm5, dword ptr [esi+8*9]		; V9
		paddsw mm2, mm7					; V51
		
		pmulhw mm1, dword ptr x5a825a825a825a82	; 23170 ->V52
		movq mm6, mm2					; duplicate V51
		
		psraw mm2, 1					; t138=t144
		movq mm4, mm3					; duplicate V1
		
		psraw mm6, 2  					; t136
		paddsw mm3, mm5					; V53
		
		psubsw mm4, mm5					; V54 ;mm5 free
		movq mm7, mm3					; duplicate V53
		
		;moved from next block
		movq mm0, dword ptr [ecx+8*11]
		psraw mm4, 1					; t140=t142
		
		psubsw mm1, mm6					; V55 ; mm6 free
		paddsw mm3, mm2					; V56
		
		movq mm5, mm4					; duplicate t140=t142
		paddsw mm4, mm1					; V57
		
		movq dword ptr [esi+8*5], mm3			; V56
		psubsw mm5, mm1					; V58; mm1 free
		
		movq dword ptr [esi+8*13], mm4			; V57
		psubsw mm7, mm2					; V59; mm2 free
		
		movq dword ptr [esi+8*9], mm5			; V58
		;slot
		
		; keep mm7 alive all along the next block
		;movq dword ptr [esi+8*1], mm7			; V59
		
		;moved above
		;movq mm0, dword ptr [ecx+8*11]
		
		pmulhw mm0, dword ptr [esi+8*11]		; V11
		;slot
		
		movq mm6, dword ptr [ecx+8*7]
		;slot
		
		pmulhw mm6, dword ptr [esi+8*7]		; V7
		;slot
		
		movq mm4, dword ptr [ecx+8*15]
		movq mm3, mm0					; duplicate V11
		
		pmulhw mm4, dword ptr [esi+8*15]		; V15
		;slot
		
		movq mm5, dword ptr [ecx+8*3]
		psllw mm6,1					; t146=t152
		
		pmulhw mm5, dword ptr [esi+8*3]		; V3
		paddsw mm0, mm6					; V63
		
		; note that V15 computation has a correction step:
		; this is a 'magic' constant that rebiases the results to be closer to the expected result
		; this magic constant can be refined to reduce the error even more
		; by doing the correction step in a later stage when the number is actually multiplied by 16
		
		paddw mm4, dword ptr x0005000200010001;
		psubsw mm3, mm6					; V60 ; free mm6
		
		psraw mm0, 1					; t154=t156
		movq mm1, mm3					; duplicate V60
		
		pmulhw mm1, dword ptr x539f539f539f539f 	; V67
		movq mm6, mm5					; duplicate V3
		
		psraw mm4, 2					; t148=t150
		;slot
		
		paddsw mm5, mm4					; V61
		psubsw mm6, mm4					; V62 ; free mm4
		
		movq mm4, mm5					; duplicate V61
		psllw mm1, 1					; t169
		
		paddsw mm5, mm0					; V65 -> result
		psubsw mm4, mm0					; V64 ; free mm0
		
		pmulhw mm4, dword ptr x5a825a825a825a82 	; V68
		psraw mm3, 1					; t158
		
		psubsw mm3, mm6					; V66
		movq mm2, mm5					; duplicate V65
		
		pmulhw mm3, dword ptr x61f861f861f861f8 	; V70
		psllw mm6, 1					; t165
		
		pmulhw mm6, dword ptr x4546454645464546 	; V69
		psraw mm2, 1					; t172
		
		;moved from next block
		movq mm0, dword ptr [esi+8*5]			; V56
		psllw mm4, 1					; t174
		
		;moved from next block
		psraw mm0, 1					; t177=t188
		nop ; slot
		
		psubsw mm6, mm3					; V72
		psubsw mm3, mm1					; V71 ; free mm1
		
		psubsw mm6, mm2					; V73 ; free mm2
		;moved from next block
		psraw mm5, 1					; t178=t189
		
		psubsw mm4, mm6					; V74
		;moved from next block
		movq mm1, mm0					; duplicate t177=t188
		
		paddsw mm3, mm4					; V75
		;moved from next block
		paddsw mm0, mm5					; tm1
		
		;location 
		;  5 - V56
		; 13 - V57
		;  9 - V58
		;  X - V59, mm7
		;  X - V65, mm5
		;  X - V73, mm6
		;  X - V74, mm4
		;  X - V75, mm3
		; free mm0, mm1 & mm2
		;move above
		;movq mm0, dword ptr [esi+8*5]			; V56
		;psllw mm0, 1					; t177=t188 ! new !!
		;psllw mm5, 1					; t178=t189 ! new !!
		;movq mm1, mm0					; duplicate t177=t188
		;paddsw mm0, mm5					; tm1
		
		movq mm2, dword ptr [esi+8*13]     		; V57
		psubsw mm1, mm5					; tm15; free mm5
		
		movq dword ptr [esi+8*1], mm0      		; tm1; free mm0
		psraw mm7, 1					; t182=t184 ! new !!
		
		;save the store as used directly in the transpose
		;movq dword ptr [esi+8*15], mm1		; tm15; free mm1
		movq mm5, mm7                   		; duplicate t182=t184
		psubsw mm7, mm3					; tm7
		
		paddsw mm5, mm3					; tm9; free mm3
		;slot
		
		movq mm0, dword ptr [esi+8*9]			; V58
		movq mm3, mm2					; duplicate V57
		
		movq dword ptr [esi+8*7], mm7			; tm7; free mm7
		psubsw mm3, mm6					; tm13
		
		paddsw mm2, mm6					; tm3 ; free mm6
		; moved up from the transpose 
		movq mm7, mm3                  
		
		; moved up from the transpose 
		punpcklwd mm3, mm1
		movq mm6, mm0					; duplicate V58
		
		movq dword ptr [esi+8*3], mm2			; tm3; free mm2
		paddsw mm0, mm4					; tm5
		
		psubsw mm6, mm4					; tm11; free mm4
		; moved up from the transpose 
		punpckhwd mm7, mm1
		
		movq dword ptr [esi+8*5], mm0			; tm5; free mm0
		; moved up from the transpose 
		movq mm2, mm5 
		
		; transpose - M4 part
		;  ---------       ---------
		; | M1 | M2 |     | M1'| M3'|
		;  ---------  -->  ---------
		; | M3 | M4 |     | M2'| M4'|
		;  ---------       ---------
		; Two alternatives: use full dword approach so the following code can be 
		; scheduled before the transpose is done without stores, or use the faster
		; half dword stores (when possible)
		
		movd dword ptr [esi+8*9+4], mm3 	; MS part of tmt9
		punpcklwd mm5, mm6
		
		movd dword ptr [esi+8*13+4], mm7	; MS part of tmt13 
		punpckhwd mm2, mm6
		
		movd dword ptr [esi+8*9], mm5		; LS part of tmt9 
		punpckhdq mm5, mm3				; free mm3
		
		movd dword ptr [esi+8*13], mm2		; LS part of tmt13
		punpckhdq mm2, mm7				; free mm7
		
		; moved up from the M3 transpose 
		movq mm0, dword ptr [esi+8*8] 
		;slot
		
		; moved up from the M3 transpose 
		movq mm1, dword ptr [esi+8*10] 
		; moved up from the M3 transpose 
		movq mm3, mm0 
		
		; shuffle the rest of the data, and write it with 2 dword writes
		movq dword ptr [esi+8*11], mm5 	; tmt11
		; moved up from the M3 transpose 
		punpcklwd mm0, mm1
		
		movq dword ptr [esi+8*15], mm2		; tmt15
		; moved up from the M3 transpose 
		punpckhwd mm3, mm1
		
		; transpose - M3 part
		
		; moved up to previous code section
		;movq mm0, dword ptr [esi+8*8] 
		;movq mm1, dword ptr [esi+8*10] 
		;movq mm3, mm0 
		;punpcklwd mm0, mm1
		;punpckhwd mm3, mm1
		
		movq mm6, dword ptr [esi+8*12] 
		;slot
		
		movq mm4, dword ptr [esi+8*14] 
		movq mm2, mm6 
		
		; shuffle the data and write out the lower parts of the transposed in 4 dwords
		punpcklwd mm6, mm4
		movq mm1, mm0
		
		punpckhdq mm1, mm6
		movq mm7, mm3
		
		punpckhwd mm2, mm4				; free mm4
		;slot
		
		punpckldq mm0, mm6				; free mm6
		;slot
		
		;moved from next block
		movq mm4, dword ptr [esi+8*13]			; tmt13
		punpckldq mm3, mm2
		
		punpckhdq mm7, mm2				; free mm2
		;moved from next block
		movq mm5, mm3					; duplicate tmt5
		
		; column 1: even part (after transpose)
		
		;moved above
		;movq mm5, mm3					; duplicate tmt5
		;movq mm4, dword ptr [esi+8*13]		; tmt13
		
		psubsw mm3, mm4					; V134
		;slot
		
		pmulhw mm3, dword ptr x5a825a825a825a82	; 23170 ->V136
		;slot
		
		movq mm6, dword ptr [esi+8*9]	 		; tmt9
		paddsw mm5, mm4					; V135 ; mm4 free
		
		movq mm4, mm0					; duplicate tmt1
		paddsw mm0, mm6					; V137
		
		psubsw mm4, mm6					; V138 ; mm6 free
		psllw mm3, 2					; t290
		psubsw mm3, mm5					; V139
		movq mm6, mm0					; duplicate V137
		
		paddsw mm0, mm5					; V140
		movq mm2, mm4					; duplicate V138
		
		paddsw mm2, mm3					; V141
		psubsw mm4, mm3					; V142 ; mm3 free
		
		movq dword ptr [esi+8*9], mm0 			; V140
		psubsw mm6, mm5					; V143 ; mm5 free
		
		;moved from next block
		movq mm0, dword ptr[esi+8*11]			; tmt11
		;slot
		
		movq dword ptr [esi+8*13], mm2 		; V141
		;moved from next block
		movq mm2, mm0					; duplicate tmt11
		
		; column 1: odd part (after transpose)
		
		;moved up to the prev block
		;movq mm0, dword ptr[esi+8*11]			; tmt11
		;movq mm2, mm0					; duplicate tmt11
		
		movq mm5, dword ptr[esi+8*15]			; tmt15
		psubsw mm0, mm7					; V144
		
		movq mm3, mm0					; duplicate V144
		paddsw mm2, mm7					; V147 ; free mm7
		
		pmulhw mm0, dword ptr x539f539f539f539f	; 21407-> V151
		movq mm7, mm1					; duplicate tmt3
		
		paddsw mm7, mm5					; V145
		psubsw mm1, mm5					; V146 ; free mm5
		
		psubsw mm3, mm1					; V150
		movq mm5, mm7					; duplicate V145
		
		pmulhw mm1, dword ptr x4546454645464546	; 17734-> V153
		psubsw mm5, mm2					; V148
		
		pmulhw mm3, dword ptr x61f861f861f861f8	; 25080-> V154
		psllw mm0, 2					; t311
		
		pmulhw mm5, dword ptr x5a825a825a825a82	; 23170-> V152
		paddsw mm7, mm2					; V149 ; free mm2
		
		psllw mm1, 1					; t313
		nop ; slot
		
		;without the nop above - freeze here for one clock
		;the nop cleans the mess a little bit
		movq mm2, mm3					; duplicate V154
		psubsw mm3, mm0					; V155 ; free mm0
		
		psubsw mm1, mm2					; V156 ; free mm2
		;moved from the next block
		movq mm2, mm6					; duplicate V143
		
		;moved from the next block
		movq mm0, dword ptr[esi+8*13]	; V141
		psllw mm1, 1					; t315
		
		psubsw mm1, mm7					; V157 (keep V149)
		psllw mm5, 2					; t317
		
		psubsw mm5, mm1					; V158
		psllw mm3, 1					; t319
		
		paddsw mm3, mm5					; V159
		;slot
		
		; column 1: output butterfly (after transform)
		;moved to the prev block
		;movq mm2, mm6					; duplicate V143
		;movq mm0, dword ptr[esi+8*13]	; V141
		
		psubsw mm2, mm3					; V163
		paddsw mm6, mm3					; V164 ; free mm3
		
		movq mm3, mm4					; duplicate V142
		psubsw mm4, mm5					; V165 ; free mm5
		
		movq dword ptr scratch7, mm2			; out7
		psraw mm6, 4
		
		psraw mm4, 4
		paddsw mm3, mm5					; V162
		
		movq mm2, dword ptr[esi+8*9]			; V140
		movq mm5, mm0					; duplicate V141
		
		;in order not to perculate this line up, we read [esi+8*9] very near to this location
		// (9)
		movq dword ptr [esi+8*9], mm6			; out9
		paddsw mm0, mm1					; V161
		
		movq dword ptr scratch5, mm3			; out5
		psubsw mm5, mm1					; V166 ; free mm1
		
		// (11)
		movq dword ptr[esi+8*11], mm4			; out11
		psraw mm5, 4
		
		movq dword ptr scratch3, mm0			; out3
		movq mm4, mm2					; duplicate V140
		
		// (13)
		movq dword ptr[esi+8*13], mm5			; out13
		paddsw mm2, mm7					; V160
		
		;moved from the next block
		movq mm0, dword ptr [esi+8*1] 
		psubsw mm4, mm7					; V167 ; free mm7
		
		;moved from the next block
		movq mm7, dword ptr [esi+8*3] 
		psraw mm4, 4
		
		movq dword ptr scratch1, mm2			; out1
		;moved from the next block
		movq mm1, mm0 
		
		// (15)
		movq dword ptr[esi+8*15], mm4			; out15
		;moved from the next block
		punpcklwd mm0, mm7
		
		; transpose - M2 parts
		;moved up to the prev block
		;movq mm0, dword ptr [esi+8*1] 
		;movq mm7, dword ptr [esi+8*3] 
		;movq mm1, mm0 
		;punpcklwd mm0, mm7
		
		movq mm5, dword ptr [esi+8*5] 
		punpckhwd mm1, mm7
		
		movq mm4, dword ptr [esi+8*7] 
		movq mm3, mm5 
		
		; shuffle the data and write out the lower parts of the trasposed in 4 dwords
		movd dword ptr [esi+8*8], mm0 		; LS part of tmt8
		punpcklwd mm5, mm4
		
		movd dword ptr [esi+8*12], mm1 		; LS part of tmt12
		punpckhwd mm3, mm4
		
		movd dword ptr [esi+8*8+4], mm5 		; MS part of tmt8
		punpckhdq mm0, mm5				; tmt10
		
		movd dword ptr [esi+8*12+4], mm3 		; MS part of tmt12
		punpckhdq mm1, mm3				; tmt14
		
		
		; transpose - M1 parts
		movq mm7, dword ptr [esi] 
		;slot
		
		movq mm2, dword ptr [esi+8*2] 
		movq mm6, mm7 
		
		movq mm5, dword ptr [esi+8*4] 
		punpcklwd mm7, mm2
		
		movq mm4, dword ptr [esi+8*6] 
		punpckhwd mm6, mm2 ; free mm2
		
		movq mm3, mm5 
		punpcklwd mm5, mm4
		
		punpckhwd mm3, mm4				; free mm4
		movq mm2, mm7
		
		movq mm4, mm6
		punpckldq mm7, mm5				; tmt0
		
		punpckhdq mm2, mm5				; tmt2 ; free mm5
		;slot
		
		; shuffle the rest of the data, and write it with 2 dword writes
		punpckldq mm6, mm3				; tmt4
		;move from next block
		movq mm5, mm2					; duplicate tmt2
		
		punpckhdq mm4, mm3				; tmt6 ; free mm3
		;move from next block
		movq mm3, mm0					; duplicate tmt10
		
		; column 0: odd part (after transpose)
		;moved up to prev block
		;movq mm3, mm0					; duplicate tmt10
		;movq mm5, mm2					; duplicate tmt2
		
		psubsw mm0, mm4					; V110
		paddsw mm3, mm4					; V113 ; free mm4
		
		movq mm4, mm0					; duplicate V110
		paddsw mm2, mm1					; V111
		
		pmulhw mm0, dword ptr x539f539f539f539f	; 21407-> V117
		psubsw mm5, mm1					; V112 ; free mm1
		
		psubsw mm4, mm5					; V116
		movq mm1, mm2					; duplicate V111
		
		pmulhw mm5, dword ptr x4546454645464546	; 17734-> V119
		psubsw mm2, mm3					; V114
		
		pmulhw mm4, dword ptr x61f861f861f861f8	; 25080-> V120
		paddsw mm1, mm3					; V115 ; free mm3
		
		pmulhw mm2, dword ptr x5a825a825a825a82	; 23170-> V118
		psllw mm0, 2					; t266
		
		movq dword ptr[esi+8*0], mm1			; save V115
		psllw mm5, 1					; t268
		
		psubsw mm5, mm4					; V122
		psubsw mm4, mm0					; V121 ; free mm0
		
		psllw mm5, 1					; t270
		;slot
		
		psubsw mm5, mm1					; V123 ; free mm1
		psllw mm2, 2					; t272
		
		psubsw mm2, mm5					; V124 (keep V123)
		psllw mm4, 1					; t274
		
		
		movq dword ptr[esi+8*2], mm5			; save V123 ; free mm5
		paddsw mm4, mm2					; V125 (keep V124)
		
		; column 0: even part (after transpose)
		movq mm0, dword ptr[esi+8*12]			; tmt12
		movq mm3, mm6					; duplicate tmt4
		
		psubsw mm6, mm0					; V100
		paddsw mm3, mm0					; V101 ; free mm0
		
		pmulhw mm6, dword ptr	x5a825a825a825a82 	; 23170 ->V102
		movq mm5, mm7					; duplicate tmt0
		
		movq mm1, dword ptr[esi+8*8]			; tmt8
		;slot
		
		paddsw mm7, mm1					; V103
		psubsw mm5, mm1					; V104 ; free mm1
		
		movq mm0, mm7					; duplicate V103
		psllw mm6, 2					; t245
		
		paddsw mm7, mm3					; V106
		movq mm1, mm5					; duplicate V104
		
		psubsw mm6, mm3					; V105
		psubsw mm0, mm3					; V109; free mm3   
		
		paddsw mm5, mm6					; V107
		psubsw mm1, mm6					; V108 ; free mm6
		
		; column 0: output butterfly (after transform)
		movq mm3, mm1					; duplicate V108
		paddsw mm1, mm2					; out4
		
		psraw mm1, 4
		psubsw mm3, mm2					; out10 ; free mm2
		
		psraw mm3, 4
		movq mm6, mm0					; duplicate V109
		
		// (4)
		movq dword ptr[esi+8*4], mm1			; out4 ; free mm1
		psubsw mm0, mm4					; out6
		
		// (10)
		movq dword ptr[esi+8*10], mm3			; out10 ; free mm3
		psraw mm0, 4
		
		paddsw mm6, mm4					; out8 ; free mm4
		movq mm1, mm7					; duplicate V106
		
		// (6)
		movq dword ptr[esi+8*6], mm0			; out6 ; free mm0
		psraw mm6, 4
		
		movq mm4, dword ptr[esi+8*0] 			; V115
		;slot
		
		// (8)
		movq dword ptr[esi+8*8], mm6			; out8 ; free mm6
		movq mm2, mm5	; duplicate V107
		
		movq mm3, dword ptr[esi+8*2] 			; V123
		paddsw mm7, mm4					; out0
		
		;moved up from next block
		movq mm0, dword ptr scratch3
		psraw mm7, 4
		
		;moved up from next block
		movq mm6, dword ptr scratch5
		psubsw mm1, mm4					; out14 ; free mm4
		
		paddsw mm5, mm3					; out2
		psraw mm1, 4
		
		// (0)
		// movq dword ptr[esi], mm7			; out0 ; free mm7
		psraw mm5, 4
		
		// (14)
		movq dword ptr[esi+8*14], mm1			; out14 ; free mm1
		psubsw mm2, mm3					; out12 ; free mm3
		
		// (2)
		// movq dword ptr[esi+8*2], mm5			; out2 ; free mm5
		psraw mm2, 4
		
		;moved up to the prev block
		movq mm4, dword ptr scratch7
		;moved up to the prev block
		psraw mm0, 4
		
		// (12)
		//movq dword ptr[esi+8*12], mm2			; out12 ; free mm2
		
		;moved up to the prev block
		movq mm1, dword ptr scratch1
		psraw mm6, 4
		
		packsswb mm7,mm7 	
		
		psraw mm1, 4
		
		movd [esi],mm7;		//out0
		
		psraw mm4, 4
			
		packsswb mm1,mm1;
		
		
		// (3)
		//movq dword ptr [esi+8*3], mm0		; out3
		
		// (5)
		//movq dword ptr [esi+8*5], mm6		; out5
		
		// (7)
		//movq dword ptr [esi+8*7], mm4		; out7
		
		// (1)
		//movq dword ptr [esi+8*1], mm1		; out1
		
		// 将 16bit 变成 8bit
		// mm1=1,mm4=7,mm6=5,mm0=3,mm2=12,mm5=2,mm7=0
		
		packsswb mm7,[esi+8*4]; //mm7=4
		packsswb mm6,mm6;
		packsswb mm5,mm5;
		punpckhdq mm7,mm6;
		movd [esi+4*1],mm1;		//out1   free mm1
		movd [esi+4*2],mm5;		//out2   free mm5
		packsswb mm0,mm0;
		packsswb mm1,[esi+8*6]; //mm1=6
		packsswb mm4,mm4;
		movd [esi+4*3],mm0;		//out3   free mm0
		punpckhdq mm1,mm4;
		packsswb mm3,[esi+8*8]; //mm3=8
		movq [esi+4*4],mm7;		//out4,5 free mm6,mm7
		packsswb mm0,[esi+8*9]; //mm0=9
		packsswb mm6,[esi+8*10];//mm6=10
		movq [esi+4*6],mm1;     //out6,7 free mm1,mm4
		packsswb mm1,[esi+8*11];//mm1=11
		punpckhdq mm3,mm0;
		punpckhdq mm6,mm1;
		packsswb mm2,mm2;
		packsswb mm4,[esi+8*13];//mm4=13
		movq [esi+4*8],mm3;     //out8,9 free mm0,mm3
		punpckhdq mm2,mm4;
		packsswb mm0,[esi+8*14];//mm3=14
		packsswb mm5,[esi+8*15];//mm5=15
		movq [esi+4*10],mm6;	//out10,11  free mm1,mm6
		punpckhdq mm0,mm5;
		movq [esi+4*12],mm2;	//out12,13  free mm3,mm4
		movq [esi+4*14],mm0;
		
		emms;
	}
}
//---------------------------------------------------------------------------
