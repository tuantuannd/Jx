//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegSegment.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg 读取 Segment
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
//---------------------------------------------------------------------------
// 函数:	skip_SEG
// 功能:	跳过一个有内容的段
// 参数:	stream		Jpeg数据流
// 返回:	PBYTE		Jpeg数据流
//---------------------------------------------------------------------------
PBYTE jpeg_skip_SEG(PBYTE stream)
{
	PBYTE stream_end;
	WORD  seg_size;

	READ_WORD(seg_size, stream);
	stream_end = stream + seg_size - 2;
	return stream_end;
}
/****************************************************************************
SOF0: Start Of Frame 0:
~~~~~~~~~~~~~~~~~~~~~~~
  - $ff, $c0 (SOF0)
  - 长度 (高字节, 低字节), 8+components*3
  - 数据精度 (1 byte) 每个样本位数, 通常是 8 (大多数软件不支持 12 和 16)
  - 图片高度 (高字节, 低字节), 如果不支持 DNL 就必须 >0
  - 图片宽度 (高字节, 低字节), 如果不支持 DNL 就必须 >0
  - components 数量(1 byte), 灰度图是 1, YCbCr/YIQ 彩色图是 3, CMYK 彩色图
    是 4
  - 每个 component: 3 bytes
     - component id (1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q)
     - 采样系数 (bit 0-3 vert., 4-7 hor.)
     - quantization table 号
****************************************************************************/
//---------------------------------------------------------------------------
// 函数:	read_SOF
// 功能:	read start of frame
// 参数:	stream		Jpeg数据流
// 返回:	PBYTE		Jpeg数据流
//---------------------------------------------------------------------------
PBYTE jpeg_read_SOF(PBYTE stream)
{
	PBYTE stream_end;
	WORD seg_size;
	BYTE id,q,i;
	WORD w;

	READ_WORD(seg_size, stream);
	stream_end = stream + seg_size - 2;

	jpeg_head.colordepth = READ_BYTE(stream);
	READ_WORD(w, stream);
	jpeg_head.height = w;
	READ_WORD(w, stream);
	jpeg_head.width = w;

	if ((jpeg_head.components = READ_BYTE(stream)) > 3) 
		return NULL; // 不支持 3 个组件以上

	for (i = 0; i < jpeg_head.components; i++)
	{
		if ((id=READ_BYTE(stream)) > 3)
			return NULL; //不支持的模式
		if (--id < 0)
			return NULL;
		q = READ_BYTE(stream);
		if ((jpeg_head.component[id].v = q & 0xf ) > 2) 
			return NULL; // 不支持超过 2 的采样率
		if ((jpeg_head.component[id].h = q >> 4) > 2)
			return NULL; // 不支持超过 2 的采样率
		jpeg_head.component[id].qtb = READ_BYTE(stream) & 3;
	}
	return stream_end;
}

/****************************************************************************
SOS: Start Of Scan:
~~~~~~~~~~~~~~~~~~~
  - $ff, $da (SOS)
  - 长度 (高字节, 低字节), 必须是 6+2*(扫描行内组件的数量)
  - 扫描行内组件的数量 (1 byte), 必须 >= 1 , <=4 (否则是错的) 通常是 3
  - 每个组件: 2 bytes
     - component id (1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q), 见 SOF0
     - 使用的 Huffman 表:
	- bit 0..3: AC table (0..3)
	- bit 4..7: DC table (0..3)
  - 忽略 3 bytes (???)
****************************************************************************/
//---------------------------------------------------------------------------
// 函数:	read_SOS
// 功能:	read start of scan
// 参数:	stream		Jpeg数据流
// 返回:	PBYTE		Jpeg数据流
//---------------------------------------------------------------------------
PBYTE jpeg_read_SOS(PBYTE stream)
{
	PBYTE stream_end;
	WORD seg_size;
	BYTE i,id,com,q;

	READ_WORD(seg_size, stream);
	stream_end = stream + seg_size - 2;

	if ((com = READ_BYTE(stream)) != jpeg_head.components)
		return NULL; // 组件数目不对

	for (i = 0; i < com; i++)
	{
		if ((id = READ_BYTE(stream)) > 3)
			return NULL; //不支持的模式
		if (--id < 0)
			return NULL;
		q = READ_BYTE(stream);
		jpeg_head.component[id].act = (q & 3) | 4;
		jpeg_head.component[id].dct = (q >> 4) & 3;
	}
	return stream_end;
}
//---------------------------------------------------------------------------
