//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegColor.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg 解码 Color 的转换
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
//---------------------------------------------------------------------------
static WORD jpeg_Y[64];
static char Cr1_402[256];
static char Cb0_34414[256];
static char Cr0_71414[256];
static char Cb1_772[256];
//---------------------------------------------------------------------------
static __int64 x8080808080808080 = 0x8080808080808080;
static __int64 x7f7f7f7f7f7f7f7f = 0x7f7f7f7f7f7f7f7f;
static __int64 x03e003e003e003e0 = 0x03e003e003e003e0;
static __int64 mmx_mask_red;
static __int64 mmx_mask_green;
//---------------------------------------------------------------------------
static void Y2RGB(WORD *bmppixel, int pitch);
static void YCbCr111(WORD *bmppixel,int pitch);
static void YCbCr411_565(WORD *bmppixel, int pitch);
static void YCbCr411_555(WORD *bmppixel, int pitch);
static void YCbCr411_nommx(WORD *bmppixel, int pitch);
//---------------------------------------------------------------------------
static WORD (* RGB16)(signed char Y,signed char r,signed char g,signed char b);
static WORD RGB16_555(signed char Y,signed char r,signed char g,signed char b);
static WORD RGB16_565(signed char Y,signed char r,signed char g,signed char b);
//---------------------------------------------------------------------------
// 函数:	jpeg_decode_init
// 功能:	Init Jpeg Decoder
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL jpeg_decode_init(BOOL bRGB555, BOOL bMMXCPU)
{
	int i;

	jpeg_Y2RGB = Y2RGB;
	jpeg_YCbCr111 = YCbCr111;

	if (bRGB555)
	{
		RGB16 = RGB16_555;
		mmx_mask_red = 0xfc00fc00fc00fc00;
		mmx_mask_green = 0x03e003e003e003e0;
		for (i = 0; i < 32; i++)
			jpeg_Y[i * 2] = jpeg_Y[i * 2 + 1] = i<<10 | i<<5 | i;
	}
	else
	{
		RGB16 = RGB16_565;
		mmx_mask_red = 0xf800f800f800f800;
		mmx_mask_green = 0x07e007e007e007e0;
		for (i = 0; i < 64; i++)
			jpeg_Y[i] = (i>>1)<<11 | i<<5 | (i>>1);
	}

	if (bMMXCPU)
	{
		jpeg_IDCT = jpeg_idct_mmx;
		if (bRGB555)
			jpeg_YCbCr411 = YCbCr411_555;
		else
			jpeg_YCbCr411 = YCbCr411_565;
	}
	else
	{
		jpeg_IDCT = jpeg_idct_2d;
		jpeg_YCbCr411 = YCbCr411_nommx;
	}

	for (i = 0; i < 256; i++)
	{
		Cr1_402[i]   = (int)(1.402    * (char)i + 1) >> 1;
		Cb0_34414[i] = (int)(-0.34414 * (char)i + 1) >> 1;
		Cr0_71414[i] = (int)(-0.71414 * (char)i + 1) >> 1;
		Cb1_772[i]   = (int)(1.772    * (char)i + 1) >> 1;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Y2RGB
// 功能:	亮度值转化为16位RGB值
// 参数:	bmppixel
//			pitch
// 返回:	void
//---------------------------------------------------------------------------
void Y2RGB(WORD *bmppixel, int pitch)
{
	__asm
	{
		xor eax,eax;
		mov ebx,pitch;
		lea esi,jpeg_ybuf;
		sub ebx,16;
		mov edx,8;
		mov edi,bmppixel;
		mov ecx,edx;
		ALIGN 4
_loop:
		lodsb;
		add al,0x80;
		movzx eax,al;
		shr eax,2;
		mov ax,[jpeg_Y+eax*2];
		stosw;
		dec ecx;
		jnz _loop;
		mov ecx,8;
		add edi,ebx;
		dec edx;
		jnz _loop;
	}
}
//---------------------------------------------------------------------------
// 函数:	YCbCr411_565
// 功能:	YCbCr411格式转化位RGB(565)格式
// 参数:	bmppixel
//			pitch
// 返回:	void
//---------------------------------------------------------------------------
/*********************************************
R = Y + 1.402  *(Cr-128)
G = Y - 0.34414*(Cb-128) - 0.71414*(Cr-128)
B = Y + 1.772  *(Cb-128)
**********************************************/
void YCbCr411_565(WORD *bmppixel,int pitch)
{
	signed char *CrBuf,*CbBuf,*YBuf;
	int n = 4;

	__asm
	{
		mov edi,bmppixel;
		mov edx,pitch;
		cld;
		lea eax,jpeg_crbuf;
		lea ebx,jpeg_cbbuf;
		lea ecx,jpeg_ybuf;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
_loop1:
		call _YCbCr8x2;
		add edi,16;
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		call _YCbCr8x2;
		lea edi,[edi+2*edx-16];
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,16-128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		dec n;
		jnz _loop1;

		mov eax,YBuf;
		mov ebx,4;
		add eax,128+64;
		mov n,ebx;
		mov YBuf,eax;

_loop2:
		call _YCbCr8x2;
		add edi,16;
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		call _YCbCr8x2;
		lea edi,[edi+2*edx-16];
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,16-128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		dec n;
		jnz _loop2;

		jmp _end;
_YCbCr8x2:
		mov esi,CrBuf;
		lea ebx,Cr1_402;
		mov eax,[esi];
		mov ecx,4;
		ALIGN 4
_loop_cr_0:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cr_0;
		movd mm5,eax;
// eax 里是 4 个 Cr*1.402

		lea ebx,Cr0_71414;
		punpcklbw mm5,mm5;

		mov eax,[esi];
		mov ecx,4;
		ALIGN 4
_loop_cr_1:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cr_1;
		movd mm7,eax;
// eax 里是 4 个 Cr* (- 0.71414)

		mov esi,CbBuf;
		punpcklbw mm7,mm7;

		lea ebx,Cb0_34414;
		mov eax,[esi];
		mov ecx,4;
		ALIGN 4
_loop_cb_0:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cb_0;
		movd mm6,eax;
// eax 里是 4 个 Cb* (- 0.34414)

		lea ebx,Cb1_772;
		punpcklbw mm6,mm6;
		mov eax,[esi];
		paddsb mm6,mm7;
// mm6 里是 8 个 - 0.34414* Cb - 0.71414*Cr
		mov ecx,4;
		ALIGN 4
_loop_cb_1:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cb_1;
		movd mm7,eax;
// eax 里是 4 个 Cb* 1.772
		mov esi,YBuf;
		punpcklbw mm7,mm7;

		movq mm0,[esi];

		pxor mm3,mm3;

		paddb mm0,x8080808080808080; // Y + 128
		movq mm1,mm6;
		psrlq mm0,1;
		movq mm2,mm7;
		pand mm0,x7f7f7f7f7f7f7f7f;  // mmx 不支持 byte 移位
		pxor mm4,mm4;
		paddsb mm1,mm0;	// G
		paddsb mm2,mm0; // B
		paddsb mm0,mm5; // R

// 将所有的负数都设置为 0
		pcmpgtb mm4,mm1;
		pcmpgtb mm3,mm2;
		psubusb mm1,mm4;
		pxor mm4,mm4;
		psubusb mm2,mm3;
		pcmpgtb mm4,mm0;
		pxor mm3,mm3;
		psubusb mm0,mm4;

// 下面 555

		punpckhbw mm3,mm2;
		punpckhbw mm4,mm1;
		psrlw mm3,10;	// B 入 mm3
		psrlw mm4,4;	// G 入 mm4
		punpcklbw mm2,mm2; // B 的前 4 个
		pand mm4,mmx_mask_green;
		punpcklbw mm1,mm1; // G 的前 4 个

		por mm3,mm4;

		punpckhbw mm4,mm0;
		psllw mm4,1;
		pand mm4,mmx_mask_red;
		punpcklbw mm0,mm0; // R 的前 4 个

		por mm3,mm4;	// 合成 hRGB

		psllw mm0,1;

		pand mm0,mmx_mask_red;
		psrlw mm1,4;
		psrlw mm2,10;
		pand mm1,mmx_mask_green;

		por mm0,mm2;
		por mm0,mm1;	// 合成 lRGB
		movq [edi+8],mm3;
		movq [edi],mm0;
// ------------- 下 8 个 ------------
		movq mm0,[esi+8];

		pxor mm3,mm3;

		paddb mm0,x8080808080808080; // Y + 128
		movq mm1,mm6;
		psrlq mm0,1;
		movq mm2,mm7;
		pand mm0,x7f7f7f7f7f7f7f7f;  // mmx 不支持 byte 移位
		pxor mm4,mm4;
		paddsb mm1,mm0;	// G
		paddsb mm2,mm0; // B
		paddsb mm0,mm5; // R

// 将所有的负数都设置为 0
		pcmpgtb mm4,mm1;
		pcmpgtb mm3,mm2;
		psubusb mm1,mm4;
		pxor mm4,mm4;
		psubusb mm2,mm3;
		pcmpgtb mm4,mm0;
		pxor mm3,mm3;
		psubusb mm0,mm4;

// 下面 555

		punpckhbw mm3,mm2;
		punpckhbw mm4,mm1;
		psrlw mm3,10;	// B 入 mm3
		psrlw mm4,4;	// G 入 mm4
		punpcklbw mm2,mm2; // B 的前 4 个
		pand mm4,mmx_mask_green;
		punpcklbw mm1,mm1; // G 的前 4 个

		por mm3,mm4;

		punpckhbw mm4,mm0;
		psllw mm4,1;
		pand mm4,mmx_mask_red;
		punpcklbw mm0,mm0; // R 的前 4 个

		por mm3,mm4;	// 合成 hRGB
		psllw mm0,1;

		pand mm0,mmx_mask_red;
		psrlw mm1,4;
		psrlw mm2,10;
		pand mm1,mmx_mask_green;

		por mm0,mm2;
		por mm0,mm1;	// 合成 lRGB

		movq [edi+edx+8],mm3;
		movq [edi+edx],mm0;

		ret;

_end:
		emms;
	}
}
//---------------------------------------------------------------------------
// 函数:	YCbCr411_555
// 功能:	YCbCr411格式转化位RGB(555)格式
// 参数:	bmppixel
//			pitch
// 返回:	void
//---------------------------------------------------------------------------
void YCbCr411_555(WORD *bmppixel,int pitch)
{
	signed char *CrBuf,*CbBuf,*YBuf;
	int n=4;

	__asm
	{
		mov edi,bmppixel;
		mov edx,pitch;
		cld;
		lea eax,jpeg_crbuf;
		lea ebx,jpeg_cbbuf;
		lea ecx,jpeg_ybuf;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
_loop1:
		call _YCbCr8x2;
		add edi,16;
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		call _YCbCr8x2;
		lea edi,[edi+2*edx-16];
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,16-128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		dec n;
		jnz _loop1;

		mov eax,YBuf;
		mov ebx,4;
		add eax,128+64;
		mov n,ebx;
		mov YBuf,eax;

_loop2:
		call _YCbCr8x2;
		add edi,16;
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		call _YCbCr8x2;
		lea edi,[edi+2*edx-16];
		mov eax,CrBuf;
		mov ebx,CbBuf;
		mov ecx,YBuf;
		add eax,4;
		add ebx,4;
		add ecx,16-128;
		mov CrBuf,eax;
		mov CbBuf,ebx;
		mov YBuf,ecx;
		dec n;
		jnz _loop2;

		jmp _end;
_YCbCr8x2:
		mov esi,CrBuf;
		lea ebx,Cr1_402;
		mov eax,[esi];
		mov ecx,4;
		ALIGN 4
_loop_cr_0:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cr_0;
		movd mm5,eax;
// eax 里是 4 个 Cr*1.402

		lea ebx,Cr0_71414;
		punpcklbw mm5,mm5;

		mov eax,[esi];
		mov ecx,4;
		ALIGN 4
_loop_cr_1:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cr_1;
		movd mm7,eax;
// eax 里是 4 个 Cr* (- 0.71414)

		mov esi,CbBuf;
		punpcklbw mm7,mm7;

		lea ebx,Cb0_34414;
		mov eax,[esi];
		mov ecx,4;
		ALIGN 4
_loop_cb_0:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cb_0;
		movd mm6,eax;
// eax 里是 4 个 Cb* (- 0.34414)

		lea ebx,Cb1_772;
		punpcklbw mm6,mm6;
		mov eax,[esi];
		paddsb mm6,mm7;
// mm6 里是 8 个 - 0.34414* Cb - 0.71414*Cr
		mov ecx,4;
		ALIGN 4
_loop_cb_1:
		xlat;
		ror eax,8;
		dec ecx;
		jnz _loop_cb_1;
		movd mm7,eax;
// eax 里是 4 个 Cb* 1.772
		mov esi,YBuf;
		punpcklbw mm7,mm7;

		movq mm0,[esi];

		pxor mm3,mm3;

		paddb mm0,x8080808080808080; // Y + 128
		movq mm1,mm6;
		psrlq mm0,1;
		movq mm2,mm7;
		pand mm0,x7f7f7f7f7f7f7f7f;  // mmx 不支持 byte 移位
		pxor mm4,mm4;
		paddsb mm1,mm0;	// G
		paddsb mm2,mm0; // B
		paddsb mm0,mm5; // R

// 将所有的负数都设置为 0
		pcmpgtb mm4,mm1;
		pcmpgtb mm3,mm2;
		psubusb mm1,mm4;
		pxor mm4,mm4;
		psubusb mm2,mm3;
		pcmpgtb mm4,mm0;
		pxor mm3,mm3;
		psubusb mm0,mm4;

// 下面 555

		punpckhbw mm3,mm2;
		punpckhbw mm4,mm1;
		psrlw mm3,10;	// B 入 mm3
		psrlw mm4,5;	// G 入 mm4
		punpcklbw mm2,mm2; // B 的前 4 个
		pand mm4,mmx_mask_green;
		punpcklbw mm1,mm1; // G 的前 4 个

		por mm3,mm4;

		punpckhbw mm4,mm0;
		pand mm4,mmx_mask_red;
		punpcklbw mm0,mm0; // R 的前 4 个

		por mm3,mm4;	// 合成 hRGB

		pand mm0,mmx_mask_red;
		psrlw mm1,5;
		psrlw mm2,10;
		pand mm1,mmx_mask_green;

		por mm0,mm2;
		por mm0,mm1;	// 合成 lRGB
		movq [edi+8],mm3;
		movq [edi],mm0;
// ------------- 下 8 个 ------------
		movq mm0,[esi+8];

		pxor mm3,mm3;

		paddb mm0,x8080808080808080; // Y + 128
		movq mm1,mm6;
		psrlq mm0,1;
		movq mm2,mm7;
		pand mm0,x7f7f7f7f7f7f7f7f;  // mmx 不支持 byte 移位
		pxor mm4,mm4;
		paddsb mm1,mm0;	// G
		paddsb mm2,mm0; // B
		paddsb mm0,mm5; // R

// 将所有的负数都设置为 0
		pcmpgtb mm4,mm1;
		pcmpgtb mm3,mm2;
		psubusb mm1,mm4;
		pxor mm4,mm4;
		psubusb mm2,mm3;
		pcmpgtb mm4,mm0;
		pxor mm3,mm3;
		psubusb mm0,mm4;

// 下面 555

		punpckhbw mm3,mm2;
		punpckhbw mm4,mm1;
		psrlw mm3,10;	// B 入 mm3
		psrlw mm4,5;	// G 入 mm4
		punpcklbw mm2,mm2; // B 的前 4 个
		pand mm4,mmx_mask_green;
		punpcklbw mm1,mm1; // G 的前 4 个

		por mm3,mm4;

		punpckhbw mm4,mm0;
		pand mm4,mmx_mask_red;
		punpcklbw mm0,mm0; // R 的前 4 个

		por mm3,mm4;	// 合成 hRGB

		pand mm0,mmx_mask_red;
		psrlw mm1,5;
		psrlw mm2,10;
		pand mm1,mmx_mask_green;

		por mm0,mm2;
		por mm0,mm1;	// 合成 lRGB

		movq [edi+edx+8],mm3;
		movq [edi+edx],mm0;

		ret;

_end:
		emms;
	}
}
//---------------------------------------------------------------------------
// 函数:	RGB16_555
// 功能:	RGB16格式转化位RGB(555)格式
// 参数:	Y,r,g,b
// 返回:	void
//---------------------------------------------------------------------------
WORD RGB16_555(signed char Y,signed char r,signed char g,signed char b)
{
	unsigned short result;
	__asm
	{
		movsx dx,byte ptr Y;
		movsx ax,byte ptr r;
		add dx,128;
		movsx bx,byte ptr g;
		sar dx,1;
		movsx cx,byte ptr b;
		add ax,dx;
		jns _ax_n0;
		xor ax,ax;
_ax_n0:
		test ax,0xff80;
		jz _ax_7f;
		mov ax,0x7f;
_ax_7f:
		add bx,dx;
		jns _bx_n0;
		xor bx,bx;
_bx_n0:
		test bx,0xff80;
		jz _bx_7f;
		mov bx,0x7f;
_bx_7f:
		add cx,dx;
		jns _cx_n0;
		xor cx,cx;
_cx_n0:
		test cx,0xff80;
		jz _cx_7f;
		mov cx,0x7f;
_cx_7f:
		shl ax,8;
		shl bx,3;
		and ax,0x7c00;
		and bx,0x3e0;
		shr cx,2;
		or ax,bx;
		or ax,cx;
		mov result,ax;
	}
	return result;
}
//---------------------------------------------------------------------------
// 函数:	RGB16_565
// 功能:	RGB16格式转化位RGB(565)格式
// 参数:	Y,r,g,b
// 返回:	void
//---------------------------------------------------------------------------
WORD RGB16_565(signed char Y,signed char r,signed char g,signed char b)
{
	unsigned short result;
	__asm {
		movsx dx,byte ptr Y;
		movsx ax,byte ptr r;
		add dx,128;
		movsx bx,byte ptr g;
		sar dx,1;
		movsx cx,byte ptr b;
		add ax,dx;
		jns _ax_n0;
		xor ax,ax;
_ax_n0:
		test ax,0xff80;
		jz _ax_7f;
		mov ax,0x7f;
_ax_7f:
		add bx,dx;
		jns _bx_n0;
		xor bx,bx;
_bx_n0:
		test bx,0xff80;
		jz _bx_7f;
		mov bx,0x7f;
_bx_7f:
		add cx,dx;
		jns _cx_n0;
		xor cx,cx;
_cx_n0:
		test cx,0xff80;
		jz _cx_7f;
		mov cx,0x7f;
_cx_7f:
		shl ax,9;
		shl bx,4;
		and ax,0xf800;
		and bx,0x7e0;
		shr cx,2;
		or ax,bx;
		or ax,cx;
		mov result,ax;
		}
	return result;
}
//---------------------------------------------------------------------------
// 函数:	YCbCr411_nommx
// 功能:	YCbCr411
// 参数:	bmppixel
//			pitch
// 返回:	void
//---------------------------------------------------------------------------
void YCbCr411_nommx(WORD *bmppixel, int pitch)
{
	int i,j,Cb,Cr,m;
	signed char r,g,b;
	signed char *CrBuf = (signed char *)jpeg_crbuf;
	signed char *CbBuf = (signed char *)jpeg_cbbuf;
	signed char *YBuf = (signed char *)jpeg_ybuf;

	for (m = 0; m < 4; ++m, bmppixel -= (m&1)*pitch*4-8+16*(m==2))
	for (i = 0; i < 8; i += 2)
	{
		for (j = 0; j < 8; j += 2)
		{
			Cb = CbBuf[(i/2+4*(m/2))*8+j/2+4*(m&1)];
			Cr = CrBuf[(i/2+4*(m/2))*8+j/2+4*(m&1)];

			r = Cr1_402[(BYTE)Cr];
			g = Cb0_34414[(BYTE)Cb]+Cr0_71414[(BYTE)Cr];
			b = Cb1_772[(BYTE)Cb];

			*bmppixel = RGB16(YBuf[i*8+j+128*m],r,g,b),++bmppixel;
			*bmppixel = RGB16(YBuf[i*8+j+1+128*m],r,g,b),++bmppixel;
			*(bmppixel+pitch/2-2) = RGB16(YBuf[i*8+j+8+128*m],r,g,b);
			*(bmppixel+pitch/2-1) = RGB16(YBuf[i*8+j+9+128*m],r,g,b);
		}
		bmppixel=(unsigned short*)((unsigned)bmppixel+pitch*2-16);
	}
}
//---------------------------------------------------------------------------
// 函数:	YCbCr111
// 功能:	YCbCr111
// 参数:	bmppixel
//			pitch
// 返回:	void
//---------------------------------------------------------------------------
void YCbCr111(WORD *bmppixel,int pitch)
{
	int i,j,Cb,Cr;
	signed char *CrBuf = (signed char *)jpeg_crbuf;
	signed char *CbBuf = (signed char *)jpeg_cbbuf;
	signed char *YBuf = (signed char *)jpeg_ybuf;
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			Cb=CbBuf[i*8+j];
			Cr=CrBuf[i*8+j];
			*bmppixel = RGB16(
				YBuf[i*8+j],
				Cr1_402[(BYTE)Cr],
				(BYTE)(Cb0_34414[(BYTE)Cb]+Cr0_71414[(BYTE)Cr]),
				Cb1_772[(BYTE)Cb]);
			bmppixel++;
		}
		bmppixel = (unsigned short*)((unsigned)bmppixel + pitch - 16);
	}
}
//---------------------------------------------------------------------------
