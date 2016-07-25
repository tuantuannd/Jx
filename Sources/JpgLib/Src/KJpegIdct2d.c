//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegDeocode.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg 解码 C 版 2D IDCT
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
/****************************************************************************
 2D IDCT 公式

          { 1//8 当 u=0 时
  a(u) =  {
          { 1/2 其他情况

           7   7                         2*x+1                2*y+1
f(x,y) =  sum sum a(u)*a(v)*F(u,v)*cos (------- *u*PI)* cos (------ *v*PI)
          u=0 v=0                         16                   16

 x, y = 0 , 1...7
****************************************************************************/
#define FIX(a) ((long)((a)*256+0.5))

#define	c0  FIX(0.7071068)
#define	c1  FIX(0.4903926)
#define	c2  FIX(0.4619398)
#define	c3  FIX(0.4157348)
#define	c4  FIX(0.3535534)
#define	c5  FIX(0.2777851)
#define	c6  FIX(0.1913417)
#define	c7  FIX(0.0975452)

#define FIXMUL(a,b) (((a)>>8)*(b))
//---------------------------------------------------------------------------
// 函数:	jpeg_idct_1d
// 功能:	C 版本 1D IDCT
// 参数:	coef	
//			result	
// 返回:	void
//---------------------------------------------------------------------------
void jpeg_idct_1d(long *coef, long *result)
{
	int  j, j1;
	long buf2[8];
	long flt1 = FIXMUL(coef[1],c7) - FIXMUL(coef[7],c1);
	long flt2 = FIXMUL(coef[7],c7) + FIXMUL(coef[1],c1);
	long flt3 = FIXMUL(coef[5],c3) - FIXMUL(coef[3],c5);
	long flt4 = FIXMUL(coef[3],c3) + FIXMUL(coef[5],c5);

	buf2[0] = FIXMUL((coef[0] + coef[4]),c4);
	buf2[1] = FIXMUL((coef[0] - coef[4]),c4);
	buf2[2] = FIXMUL(coef[2],c6) - FIXMUL(coef[6],c2);
	buf2[3] = FIXMUL(coef[6],c6) + FIXMUL(coef[2],c2);
	coef[4] = flt1 + flt3;
	buf2[5] = flt1 - flt3;
	buf2[6] = flt2 - flt4;
	coef[7] = flt2 + flt4;
	coef[5] = FIXMUL((buf2[6] - buf2[5]),c0);
	coef[6] = FIXMUL((buf2[6] + buf2[5]),c0);
	coef[0] = buf2[0] + buf2[3];
	coef[1] = buf2[1] + buf2[2];
	coef[2] = buf2[1] - buf2[2];
	coef[3] = buf2[0] - buf2[3];

	for (j = 0; j < 4; j++)
	{
		j1 = 7 - j;
		result[j] = coef[j] + coef[j1];
		result[j1] = coef[j] - coef[j1];
	}
}
//---------------------------------------------------------------------------
// 函数:	jpeg_idct_2d
// 功能:	C 版本 2D IDCT
// 参数:	buf		解码后的缓存
// 返回:	void
//---------------------------------------------------------------------------
void jpeg_idct_2d(short *buf)
{
	int  i, j;
	long buf1[8];
	long buf2[8];
	long coef[64];
	signed char *result;
	short tmp;

	for (i = 0; i < 64; i++)
	{
		coef[i] = (long)buf[i]<<12;
	}

	result = (signed char *)buf;

	for(i = 0; i < 8; i++)
	{
		jpeg_idct_1d(coef + 8 * i, buf1);
		for(j = 0; j < 8; j++)
			coef[i * 8 + j] = buf1[j];
	}

	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 8; j++)
		{
			buf1[j] = coef[j * 8 + i];
		}

		jpeg_idct_1d(buf1, buf2);

		for (j = 0; j < 8; j++)
		{
			tmp = (buf2[j] + 0x8000)>>16;
			if (tmp < -128)
				tmp = -128;
			else if (tmp > 127)
					tmp=127;
			result[i * 8 + j] = (signed char)tmp;
		}
	}
}
//---------------------------------------------------------------------------
