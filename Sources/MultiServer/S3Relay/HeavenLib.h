// HeavenLib.h: interface for the CHeavenLib class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEAVENLIB_H__A8F1A748_6FAA_4711_A71D_204D4BB2C5D7__INCLUDED_)
#define AFX_HEAVENLIB_H__A8F1A748_6FAA_4711_A71D_204D4BB2C5D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../Multiserver/Heaven/Interface/IServer.h"

class CHeavenLib  
{
public:
	CHeavenLib();
	virtual ~CHeavenLib();

public:
	BOOL Initialize();
	BOOL Uninitialize();

private:
	HMODULE m_hHeaven;

	typedef HRESULT ( __stdcall * LPFNCREATEINTERFACE )(REFIID, void**);
	LPFNCREATEINTERFACE m_lpfnCreateInterface;

public:
	HRESULT CreateInterface(REFIID riid, void** pp);
};

#endif // !defined(AFX_HEAVENLIB_H__A8F1A748_6FAA_4711_A71D_204D4BB2C5D7__INCLUDED_)
