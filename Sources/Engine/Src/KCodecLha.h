//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KCodecLha.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KCodecLha_H
#define KCodecLha_H
//---------------------------------------------------------------------------
#include "KCodec.h"
//---------------------------------------------------------------------------
class ENGINE_API KCodecLha : public KCodec
{
public:
	virtual BOOL	Encode(TCodeInfo* pCodeInfo);
	virtual BOOL	Decode(TCodeInfo* pCodeInfo);
};
//---------------------------------------------------------------------------
#endif
