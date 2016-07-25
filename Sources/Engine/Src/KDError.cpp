//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDError.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	DirectX Errors Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KDError.h"
//---------------------------------------------------------------------------
typedef struct {
	int		nCode;
	char*	szMsg;
} ERRORS;
//---------------------------------------------------------------------------
// 函数:	DDError
// 功能:	DirectDraw错误信息
// 参数:	nErrorCode	错误代号
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_DDError(int nErrorCode)
{
	static ERRORS Errors[] = {
		{ DDERR_DIRECTDRAWALREADYCREATED, "DDERR_DIRECTDRAWALREADYCREATED" },
		{ DDERR_GENERIC, "DDERR_GENERIC" },
		{ DDERR_INCOMPATIBLEPRIMARY, "DDERR_INCOMPATIBLEPRIMARY" },
		{ DDERR_INVALIDDIRECTDRAWGUID, "DDERR_INVALIDDIRECTDRAWGUID" },
		{ DDERR_INVALIDMODE, "DDERR_INVALIDMODE" },
		{ DDERR_INVALIDCAPS, "DDERR_INVALIDCAPS" },
		{ DDERR_INVALIDPIXELFORMAT, "DDERR_INVALIDPIXELFORMAT" },
		{ DDERR_INVALIDOBJECT, "DDERR_INVALIDOBJECT" },
		{ DDERR_INVALIDPARAMS, "DDERR_INVALIDPARAMS" },
		{ DDERR_LOCKEDSURFACES, "DDERR_LOCKEDSURFACES" },
		{ DDERR_NOALPHAHW, "DDERR_NOALPHAHW" },
		{ DDERR_NOCOOPERATIVELEVELSET, "DDERR_NOCOOPERATIVELEVELSET" },
		{ DDERR_NODIRECTDRAWHW, "DDERR_NODIRECTDRAWHW" },
		{ DDERR_NOEMULATION, "DDERR_NOEMULATION" },
		{ DDERR_NOEXCLUSIVEMODE, "DDERR_NOEXCLUSIVEMODE" },
		{ DDERR_NOFLIPHW, "DDERR_NOFLIPHW" },
		{ DDERR_NOMIPMAPHW, "DDERR_NOMIPMAPHW" },
		{ DDERR_NOOVERLAYHW, "DDERR_NOOVERLAYHW" },
		{ DDERR_NOZBUFFERHW, "DDERR_NOZBUFFERHW" },
		{ DDERR_OUTOFMEMORY, "DDERR_OUTOFMEMORY" },
		{ DDERR_OUTOFVIDEOMEMORY, "DDERR_OUTOFVIDEOMEMORY" },
		{ DDERR_PRIMARYSURFACEALREADYEXISTS, "DDERR_PRIMARYSURFACEALREADYEXISTS" },
		{ DDERR_SURFACEBUSY, "DDERR_SURFACEBUSY" },
		{ DDERR_UNSUPPORTED, "DDERR_UNSUPPORTED" },
	};

	int  nErrNum = sizeof(Errors) / sizeof(ERRORS);
	int  i;
	for (i = 0; i < nErrNum; i++)
	{
		if (nErrorCode == Errors[i].nCode)
		{
			g_MessageBox(Errors[i].szMsg);
			return;
		}
	}
	g_MessageBox("Unknown DirectDraw Errors !");
}
//---------------------------------------------------------------------------
// 函数:	DSError
// 功能:	DirectSound错误信息
// 参数:	nErrorCode	错误代号
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_DSError(int nErrorCode)
{
	static ERRORS Errors[] = {
		{ DSERR_ALLOCATED, "DSERR_ALLOCATED" },
		{ DSERR_ALREADYINITIALIZED, "DSERR_ALREADYINITIALIZED" },
		{ DSERR_BADFORMAT, "DSERR_BADFORMAT" },
		{ DSERR_BUFFERLOST, "DSERR_BUFFERLOST" },
		{ DSERR_CONTROLUNAVAIL, "DSERR_CONTROLUNAVAIL" },
		{ DSERR_GENERIC, "DSERR_GENERIC" },
		{ DSERR_INVALIDCALL, "DSERR_INVALIDCALL" },
		{ DSERR_INVALIDPARAM, "DSERR_INVALIDPARAM" },
		{ DSERR_NOAGGREGATION, "DSERR_NOAGGREGATION" },
		{ DSERR_NODRIVER, "DSERR_NODRIVER" },
		{ DSERR_NOINTERFACE, "DSERR_NOINTERFACE" },
		{ DSERR_OTHERAPPHASPRIO, "DSERR_OTHERAPPHASPRIO" },
		{ DSERR_OUTOFMEMORY, "DSERR_OUTOFMEMORY" },
		{ DSERR_PRIOLEVELNEEDED, "DSERR_PRIOLEVELNEEDED" },
		{ DSERR_UNINITIALIZED, "DSERR_UNINITIALIZED" },
		{ DSERR_UNSUPPORTED, "DSERR_UNSUPPORTED" },
	};

	int  nErrNum = sizeof(Errors) / sizeof(ERRORS);
	int  i;
	for (i = 0; i < nErrNum; i++)
	{
		if (nErrorCode == Errors[i].nCode)
		{
			g_MessageBox(Errors[i].szMsg);
			return;
		}
	}
	g_MessageBox("Unknown DirectSound Errors !");
}
//---------------------------------------------------------------------------
// 函数:	DIError
// 功能:	DirectInput出错信息
// 参数:	nErrorCode	错误代号
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_DIError(int nErrorCode)
{
	static ERRORS Errors[] = {
		{ DIERR_ACQUIRED, "DIERR_ACQUIRED" },
		{ DIERR_ALREADYINITIALIZED, "DIERR_ALREADYINITIALIZED" },
		{ DIERR_BADDRIVERVER, "DIERR_BADDRIVERVER" },
		{ DIERR_BETADIRECTINPUTVERSION, "DIERR_BETADIRECTINPUTVERSION" },
		{ DIERR_DEVICEFULL, "DIERR_DEVICEFULL" },
		{ DIERR_DEVICENOTREG, "DIERR_DEVICENOTREG" },
		{ DIERR_EFFECTPLAYING, "DIERR_EFFECTPLAYING" },
		{ DIERR_HASEFFECTS, "DIERR_HASEFFECTS" },
		{ DIERR_GENERIC, "DIERR_GENERIC" },
		{ DIERR_HANDLEEXISTS, "DIERR_HANDLEEXISTS" },
		{ DIERR_INCOMPLETEEFFECT, "DIERR_INCOMPLETEEFFECT" },
		{ DIERR_INPUTLOST, "DIERR_INPUTLOST" },
		{ DIERR_INVALIDPARAM, "DIERR_INVALIDPARAM" },
		{ DIERR_MOREDATA, "DIERR_MOREDATA" },
		{ DIERR_NOAGGREGATION, "DIERR_NOAGGREGATION" },
		{ DIERR_NOINTERFACE, "DIERR_NOINTERFACE" },
		{ DIERR_NOTACQUIRED, "DIERR_NOTACQUIRED" },
		{ DIERR_NOTBUFFERED, "DIERR_NOTBUFFERED" },
		{ DIERR_NOTDOWNLOADED, "DIERR_NOTDOWNLOADED" },
		{ DIERR_NOTEXCLUSIVEACQUIRED, "DIERR_NOTEXCLUSIVEACQUIRED" },
		{ DIERR_NOTFOUND, "DIERR_NOTFOUND" },
		{ DIERR_NOTINITIALIZED, "DIERR_NOTINITIALIZED" },
		{ DIERR_OLDDIRECTINPUTVERSION, "DIERR_OLDDIRECTINPUTVERSION" },
		{ DIERR_OUTOFMEMORY, "DIERR_OUTOFMEMORY" },
		{ DIERR_UNSUPPORTED, "DIERR_UNSUPPORTED" },
	};

	int  nErrNum = sizeof(Errors) / sizeof(ERRORS);
	int  i;
	for (i = 0; i < nErrNum; i++)
	{
		if (nErrorCode == Errors[i].nCode)
		{
			g_MessageBox(Errors[i].szMsg);
			return;
		}
	}
	g_MessageBox("Unknown DirectInput Errors !");
}
//---------------------------------------------------------------------------
