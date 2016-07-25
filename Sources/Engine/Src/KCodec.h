//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCodec.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KCodec_H
#define KCodec_H
//---------------------------------------------------------------------------
#define CODEC_NONE		0
//#define CODEC_LHA		1
#define CODEC_LZO		2
//---------------------------------------------------------------------------
typedef struct {
	PBYTE		lpData;			// 待压缩&压缩前 数据指针
	DWORD		dwDataLen;		// 待压缩&压缩前 数据长度
	PBYTE		lpPack;			// 压缩后&压缩后 数据指针
	DWORD		dwPackLen;		// 压缩后&压缩后 数据长度
} TCodeInfo;
//---------------------------------------------------------------------------
class ENGINE_API KCodec
{
public:
	virtual DWORD	GetPackLen(DWORD dwDataLen);
	virtual BOOL	Encode(TCodeInfo* pCodeInfo);
	virtual BOOL	Decode(TCodeInfo* pCodeInfo);
	//lpData;dwDataLen;lpPack;必须正确设置，dwPackLen不为0即可	
};
//---------------------------------------------------------------------------
ENGINE_API void	g_InitCodec(KCodec** ppCodec, int nCompressMethod);
ENGINE_API void	g_FreeCodec(KCodec** ppCodec, int nCompressMethod);
//---------------------------------------------------------------------------
#endif
