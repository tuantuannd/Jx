//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegLib.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg Decode Library
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
/****************************************************************************
附:JPEG 文件格式
~~~~~~~~~~~~~~~~
  - 文件头 (2 bytes):  $ff, $d8 (SOI) (Start Of Image)
  - 任意数量的段 , 见后面
  - 文件结束 (2 bytes): $ff, $d9 (EOI) (End Of Image)
  
	段的格式:
	~~~~~~~~~
	  - header (4 bytes):
	  $ff     段标识
	  n       段的类型 (1 byte)
	  sh, sl  该段长度, 包括这两个字节, 但是不包括前面的 $ff 和 n.
	  注意: 长度不是 intel 次序, 而是 Motorola 的, 高字节在前,
	  低字节在后!
	  - 该段的内容, 最多 65533 字节
****************************************************************************/
PBYTE		jpeg_stream = NULL;
short		jpeg_ybuf[256];
short		jpeg_cbbuf[64];
short		jpeg_crbuf[64];
short		jpeg_DC[3];
short*		jpeg_qtable[4];
JPEG_HTABLE	jpeg_htable[8];
BYTE		jpeg_bit;
JPEG_SOF0	jpeg_head;
void		(*jpeg_IDCT)(short* buf) = NULL;
void		(*jpeg_Y2RGB)(WORD *bmppixel, int pitch) = NULL;
void		(*jpeg_YCbCr411)(WORD *bmppixel, int pitch) = NULL;
void		(*jpeg_YCbCr111)(WORD *bmppixel, int pitch) = NULL;
//---------------------------------------------------------------------------
// 函数:	jpeg_decode_info
// 功能:	
// 参数:	pJpgBuf		输入指针
//			pInfo		格式信息
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL jpeg_decode_info(PBYTE pJpgBuf, JPEG_INFO* pInfo)
{
	WORD head;
	BYTE sign;
	int	 HY, VY;
	int  HCb, VCb, HCr, VCr;

	jpeg_stream = (PBYTE)pJpgBuf;
	
	// is it a JPEG file ?
	READ_WORD(head, jpeg_stream);
	// SOI = start of image
	if (head != 0xffd8)
		return FALSE;

	// init huffman table
	jpeg_init_table();

	// read jpeg header data
	do
	{
		// find first 0xff
		while ((sign = READ_BYTE(jpeg_stream)) != 0xff);

		// skip other 0xff
		while ((sign = READ_BYTE(jpeg_stream)) == 0xff);
		
		switch (sign)
		{
		case 0x01: // TEM = 可以忽略
			break;
			
		case 0xc0: // SOF0 = Start Of Frame 0
			jpeg_stream = jpeg_read_SOF(jpeg_stream);
			if (jpeg_stream == NULL)
				return FALSE;
			break;
			
		case 0xc4: // DHT = Define Huffman Table
			jpeg_stream = jpeg_read_DHT(jpeg_stream);
			if (jpeg_stream == NULL)
				return FALSE;
			break;
			
		case 0xda: // SOS = Start Of Scan
			jpeg_stream = jpeg_read_SOS(jpeg_stream);
			if (jpeg_stream == NULL)
				return FALSE;
			break;
			
		case 0xdb: // DQT = Define Quantization Table
			jpeg_stream = jpeg_read_DQT(jpeg_stream);
			if (jpeg_stream == NULL)
				return FALSE;
			break;
			
		case 0xd9: // EOI = End of Image
			return FALSE;
			
		case 0xdd: // DRI = Define Reset interval
			// Not supported
			return FALSE;
			
		default: // 忽略其他的 Segment
			jpeg_stream = jpeg_skip_SEG(jpeg_stream);
			break;
		}

	} while (sign != 0xda); // SOS 扫描行开始
	
	// 初始化
	HY = jpeg_head.component[0].h;
	VY = jpeg_head.component[0].v;
	if (HY != VY || HY > 2)
	{
		// 不支持的 MCU (只支持 YDU, YDU*4 CbDU CrDU, YDU CbDU CrDU 三种模式)
		return FALSE;
	}
	
	// 确定JPEG文件的色彩模式
	if (jpeg_head.components == 1)
	{
		pInfo->mode = 0;
	}
	else
	{
		HCb = jpeg_head.component[1].h;
		VCb = jpeg_head.component[1].v;
		HCr = jpeg_head.component[2].h;
		VCr = jpeg_head.component[2].v;
		
		if ((HCb | VCb | HCr | VCr) != 1)
		{
			// 不支持的 MCU (只支持 YDU, YDU*4 CbDU CrDU, YDU CbDU CrDU 三种模式)
			return FALSE;
		}
		
		if (HY == 2)
		{
			pInfo->mode = 1;
		}
		else
		{
			pInfo->mode = 2;
		}
	}
	
	// HDU = horizion data unit (width)
	pInfo->width = (jpeg_head.width + HY * 8 - 1) & (-1 << (HY + 2));
	
	// VDU = vertical data unit (height)
	pInfo->height = (jpeg_head.height + VY * 8 - 1) & (-1 << (VY + 2));
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	jpeg_decode_data
// 功能:	
// 参数:	pBitmap		输出指针
//			pInfo		格式信息
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL jpeg_decode_data(PWORD pBitmap, JPEG_INFO* pInfo)
{
	int	  x, y;
	int   width = pInfo->width;
	int   height = pInfo->height;
	int   pitch = pInfo->width * 2;
	PWORD bitmap = pBitmap;

	if (jpeg_stream == NULL)
		return FALSE;

	jpeg_DC[0] = 0;
	jpeg_DC[1] = 0;
	jpeg_DC[2] = 0;

	jpeg_preprocess(jpeg_stream);

	jpeg_bit = 0;
	
	switch (pInfo->mode)
	{
	case 0: // Gray Jpeg
		for (y = 0; y < height; y += 8)
		{
			for (x = 0; x < width; x += 8)
			{
				jpeg_decode_DU(jpeg_ybuf, 0);
				jpeg_Y2RGB(bitmap + width * y + x, pitch);
			}
		}
		break;
			
	case 1: // Y4CbCr Jpeg
		for (y = 0; y < height; y += 16)
		{
			for (x = 0; x < width; x += 16)
			{
				jpeg_decode_DU(jpeg_ybuf, 0);
				jpeg_decode_DU(jpeg_ybuf + 64, 0);
				jpeg_decode_DU(jpeg_ybuf + 128, 0);
				jpeg_decode_DU(jpeg_ybuf + 192, 0);
				jpeg_decode_DU(jpeg_cbbuf, 1);
				jpeg_decode_DU(jpeg_crbuf, 2);
				jpeg_YCbCr411(bitmap + width * y + x, pitch);
			}
		}
		break;
			
	case 2: // YCbCr Jpeg
		for (y = 0; y < height; y += 8)
		{
			for (x = 0; x < width; x += 8)
			{
				jpeg_decode_DU(jpeg_ybuf, 0);
				jpeg_decode_DU(jpeg_cbbuf, 1);
				jpeg_decode_DU(jpeg_crbuf, 2);
				jpeg_YCbCr111(bitmap + width * y + x, pitch);
			}
		}
		break;
	}		

	jpeg_free_table();

	jpeg_stream	= NULL;

	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	jpeg_decode_data
// 功能:	
// 参数:	pBitmap		输出指针
//			pitch		数据行跨度
//			pInfo		格式信息
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL jpeg_decode_dataEx(PWORD pBitmap, int nPitch, JPEG_INFO* pInfo)
{
	int	  x, y;
	int   width = pInfo->width;
	int   height = pInfo->height;
	int   pitch = nPitch;
	PWORD bitmap = pBitmap;

	if (jpeg_stream == NULL)
		return FALSE;

	jpeg_DC[0] = 0;
	jpeg_DC[1] = 0;
	jpeg_DC[2] = 0;

	jpeg_preprocess(jpeg_stream);

	jpeg_bit = 0;
	
	switch (pInfo->mode)
	{
	case 0: // Gray Jpeg
		for (y = 0; y < height; y += 8)
		{
			for (x = 0; x < width; x += 8)
			{
				jpeg_decode_DU(jpeg_ybuf, 0);
				jpeg_Y2RGB(bitmap + pitch * y / 2 + x, pitch);
			}
		}
		break;
			
	case 1: // Y4CbCr Jpeg
		for (y = 0; y < height; y += 16)
		{
			for (x = 0; x < width; x += 16)
			{
				jpeg_decode_DU(jpeg_ybuf, 0);
				jpeg_decode_DU(jpeg_ybuf + 64, 0);
				jpeg_decode_DU(jpeg_ybuf + 128, 0);
				jpeg_decode_DU(jpeg_ybuf + 192, 0);
				jpeg_decode_DU(jpeg_cbbuf, 1);
				jpeg_decode_DU(jpeg_crbuf, 2);
				jpeg_YCbCr411(bitmap + pitch * y / 2 + x, pitch);
			}
		}
		break;
			
	case 2: // YCbCr Jpeg
		for (y = 0; y < height; y += 8)
		{
			for (x = 0; x < width; x += 8)
			{
				jpeg_decode_DU(jpeg_ybuf, 0);
				jpeg_decode_DU(jpeg_cbbuf, 1);
				jpeg_decode_DU(jpeg_crbuf, 2);
				jpeg_YCbCr111(bitmap + pitch * y / 2 + x, pitch);
			}
		}
		break;
	}		

	jpeg_free_table();

	jpeg_stream	= NULL;

	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	jpeg_init_table
// 功能:	初始化表格
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void jpeg_init_table()
{
	memset(jpeg_htable, 0, 8 * sizeof(JPEG_HTABLE));
	memset(jpeg_qtable, 0, 4 * sizeof(short *));
}
//---------------------------------------------------------------------------
// 函数:	jpeg_free_table
// 功能:	释放表格
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void jpeg_free_table()
{
	int  i;
	for (i = 0; i < 8; i++)
	{
		if (jpeg_htable[i].htb)
		{
			free(jpeg_htable[i].htb);
			jpeg_htable[i].htb = NULL;
		}
	}
	for (i = 0 ; i < 4; i++)
	{
		if (jpeg_qtable[i])
		{
			free(jpeg_qtable[i]);
			jpeg_qtable[i] = NULL;
		}
	}
}
//---------------------------------------------------------------------------
