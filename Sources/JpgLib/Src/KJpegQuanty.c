//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KJpegQuanty.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Jpeg 读取 Quantize Table 
// From:	Cloud Wu's JPEG Decoder
//---------------------------------------------------------------------------
#include <windows.h>
#include "KJpegLib.h"
//---------------------------------------------------------------------------
// 函数:	read_DQT
// 功能:	读取 Quantize Table 
// 参数:	stream		Jpeg数据流
// 返回:	PBYTE		Jpeg数据流
//---------------------------------------------------------------------------
PBYTE jpeg_read_DQT(PBYTE stream)
{
	WORD seg_size;
	int i;
	LPBYTE stream_end;
	short *qtb;
	BYTE qtb_id;
	short tmp;

	READ_WORD(seg_size, stream);

	stream_end = stream + seg_size - 2;

	while (stream < stream_end)
	{
		qtb_id = READ_BYTE(stream);
		if (qtb_id & 0x10)
		{
			qtb_id &= 3;
			qtb = (short *) malloc(256);
			jpeg_qtable[qtb_id] = qtb;
			for (i = 0; i < 64; i++)
			{
				READ_WORD(tmp, stream);
				qtb[i] = tmp<<4;
			}
		}
		else
		{
			qtb_id &= 3;
			qtb = (short *) malloc(256);
			jpeg_qtable[qtb_id] = qtb;
			for (i = 0; i < 64; i++)
			{
				qtb[i] = (READ_BYTE(stream))<<4;
			}
		}
	}

	if (stream != stream_end)
		return NULL;

	return stream_end;
}
//---------------------------------------------------------------------------
