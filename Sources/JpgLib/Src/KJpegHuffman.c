//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegDeocode.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg Huffman Table 的解码
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
//---------------------------------------------------------------------------
// 函数:	read_DHT
// 功能:	read defined huffman table
// 参数:	stream		Jpeg数据流
// 返回:	void*		Jpeg数据流
//---------------------------------------------------------------------------
PBYTE jpeg_read_DHT(PBYTE stream)
{
	JPEG_HCODE* htb;
	WORD	seg_size;
	int		i,j,p,code,si;
	PBYTE	stream_end;
	PBYTE	code_len;
	BYTE	htb_id;

	READ_WORD(seg_size, stream);
	stream_end = stream + seg_size - 2;
	
	while (stream < stream_end)
	{
		// 取信息码
		htb_id = READ_BYTE(stream);
		htb_id = (htb_id & 0x10)? (htb_id & 3 | 4) : (htb_id & 3);
		
		// 统计HTB长度,分配内存
		code_len = (BYTE *)stream;
		for (i = 0; i < 16; i++)
		{
			jpeg_htable[htb_id].num += code_len[i];
		}
		stream = stream + 16;
		htb = (JPEG_HCODE*) malloc(jpeg_htable[htb_id].num * sizeof(JPEG_HCODE));
		jpeg_htable[htb_id].htb = htb;

		// 计算代码表
		for (i = 0, p = 0; i < 16; i++)
		{
			for (j = 0; j < code_len[i]; j++)
			{
				htb[p].num = READ_BYTE(stream);
				htb[p++].len = i + 1;
			}
		}
		for (si = htb[0].len, i = code = 0; i < p; code <<= 1, si++)
		{
			while (htb[i].len == si) 
			{
				htb[i++].code = code++;
			}
		}
	}

	if (stream != stream_end)
		return NULL;
	
	return stream_end;
}
//---------------------------------------------------------------------------
