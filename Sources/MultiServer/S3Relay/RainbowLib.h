// RainbowLib.h: interface for the CRainbowLib class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAINBOWLIB_H__847B72C3_1033_46D5_8A09_BA841AB31E14__INCLUDED_)
#define AFX_RAINBOWLIB_H__847B72C3_1033_46D5_8A09_BA841AB31E14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../Multiserver/Rainbow/Interface/IClient.h"

class CRainbowLib  
{
public:
	CRainbowLib();
	virtual ~CRainbowLib();

public:
	BOOL Initialize();
	BOOL Uninitialize();

private:
	HMODULE m_hRainbow;

	typedef HRESULT ( __stdcall * LPFNCREATEINTERFACE )(REFIID, void**);
	LPFNCREATEINTERFACE m_lpfnCreateInterface;

public:
	HRESULT CreateInterface(REFIID riid, void** pp);
};

#endif // !defined(AFX_RAINBOWLIB_H__847B72C3_1033_46D5_8A09_BA841AB31E14__INCLUDED_)
