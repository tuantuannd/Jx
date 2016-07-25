//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCodecLha.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	LHA Encode and Decode Algorithm
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "lhalib.h"
#include "KCodecLha.h"
//---------------------------------------------------------------------------
// 函数:	Encode
// 功能:	压缩
// 参数:	pCodeInfo	待压缩数据指针
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KCodecLha::Encode(TCodeInfo* pCodeInfo)
{
	lha_encode(pCodeInfo->lpData,
		pCodeInfo->dwDataLen, 
		pCodeInfo->lpPack,
		&pCodeInfo->dwPackLen);
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	解压缩
// 参数:	pCodeInfo	压缩数据指针
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KCodecLha::Decode(TCodeInfo* pCodeInfo)
{
	lha_decode(pCodeInfo->lpPack,
		pCodeInfo->dwPackLen,
		pCodeInfo->lpData,
		&pCodeInfo->dwDataLen);
	return TRUE;
}
//---------------------------------------------------------------------------
