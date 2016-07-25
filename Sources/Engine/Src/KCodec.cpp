//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCodec.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Data Compress / Decompress Base Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KCodec.h"
//---------------------------------------------------------------------------
//#include "KCodecLha.h"
#include "KCodecLzo.h"
//---------------------------------------------------------------------------
// 函数:	GetPackLen
// 功能:	取得压缩输出缓存的最小长度
// 参数:	dwDataLen	待压缩数据长度
// 返回:	压缩输出缓存的最小长度(in Bytes)
// 注释:	有可能压缩后的数据比源数据多, 所以需要得到输出缓冲区的最小长度
//---------------------------------------------------------------------------
DWORD KCodec::GetPackLen(DWORD dwDataLen)
{
	return (dwDataLen + dwDataLen / 10 + 1024);
}
//---------------------------------------------------------------------------
// 函数:	Encode
// 功能:	压缩
// 参数:	pCodeInfo	压缩数据结构
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KCodec::Encode(TCodeInfo* pCodeInfo)
{
	g_MemCopyMmx(pCodeInfo->lpPack, pCodeInfo->lpData, pCodeInfo->dwDataLen);
	pCodeInfo->dwPackLen = pCodeInfo->dwDataLen;
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	解压缩
// 参数:	pCodeInfo	压缩数据结构
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KCodec::Decode(TCodeInfo* pCodeInfo)
{
	g_MemCopyMmx(pCodeInfo->lpData, pCodeInfo->lpPack, pCodeInfo->dwPackLen);
	pCodeInfo->dwDataLen = pCodeInfo->dwPackLen;
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	g_InitCodec
// 功能:	初始化编码、解码器
// 参数:	ppCodec				编码、解码器指针
//			nCompressMethod		压缩方法
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_InitCodec(KCodec** ppCodec, int nCompressMethod)
{
	*ppCodec = NULL;

	switch (nCompressMethod)
	{
	case CODEC_NONE:
		*ppCodec = new KCodec;
		break;
//
//	case CODEC_LHA:
//		*ppCodec = new KCodecLha;
//		break;

	case CODEC_LZO:
		*ppCodec = new KCodecLzo;
		break;
		
	}
}
//---------------------------------------------------------------------------
// 函数:	g_FreeCodec
// 功能:	释放编码、解码器
// 参数:	ppCodec				编码、解码器指针
//			nCompressMethod		压缩方法
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_FreeCodec(KCodec** ppCodec, int nCompressMethod)
{
	if (*ppCodec == NULL)
		return;

	switch (nCompressMethod)
	{
	case CODEC_NONE:
		delete (KCodec *)*ppCodec;
		break;
//
//	case CODEC_LHA:
//		delete (KCodecLha *)*ppCodec;
//		break;

	case CODEC_LZO:
		delete (KCodecLzo *)*ppCodec;
		break;

	}
	*ppCodec = NULL;
}
//---------------------------------------------------------------------------

