//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakTool.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KPakTool_H
#define KPakTool_H
//---------------------------------------------------------------------------
#include "KPakData.h"
//---------------------------------------------------------------------------
class ENGINE_API KPakTool
{
private:
	KCodec*		m_pCodec;
	int			m_nCompressMethod;
public:
	KPakTool();
	~KPakTool();
	BOOL		Pack(LPSTR InFile, LPSTR OutFile, int nMethod);
	BOOL		UnPack(LPSTR InFile, LPSTR OutFile);
};
//---------------------------------------------------------------------------
#endif
