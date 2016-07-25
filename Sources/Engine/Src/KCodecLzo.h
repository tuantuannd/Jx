//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCodecLzo.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KCodecLzo_H
#define KCodecLzo_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KCodec.h"
//---------------------------------------------------------------------------
class ENGINE_API KCodecLzo : public KCodec
{
private:
	KMemClass	m_WorkMem;
	DWORD		Compress(PBYTE pIn, DWORD dwInLen, PBYTE pOut, PDWORD pOutLen);
public:
	BOOL		Encode(TCodeInfo* pCodeInfo);
	BOOL		Decode(TCodeInfo* pCodeInfo);
};
//---------------------------------------------------------------------------
#endif
