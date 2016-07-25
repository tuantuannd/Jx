//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KStrBase.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KStrBase_H
#define KStrBase_H
//---------------------------------------------------------------------------
ENGINE_API	int		g_StrLen(LPCSTR lpStr);
ENGINE_API	LPSTR	g_StrEnd(LPCSTR lpStr);
ENGINE_API	void	g_StrCpy(LPSTR lpDest, LPCSTR lpSrc);
ENGINE_API	void	g_StrCpyLen(LPSTR lpDest, LPCSTR lpSrc, int nMaxLen);
ENGINE_API	void	g_StrCat(LPSTR lpDest, LPCSTR lpSrc);
ENGINE_API	void	g_StrCatLen(LPSTR lpDest, LPCSTR lpSrc, int nMaxLen);
ENGINE_API	BOOL	g_StrCmp(LPCSTR lpDest, LPCSTR lpSrc);
ENGINE_API	BOOL	g_StrCmpLen(LPCSTR lpDest, LPCSTR lpSrc, int nMaxLen);
ENGINE_API	void	g_StrUpper(LPSTR lpDest);
ENGINE_API	void	g_StrLower(LPSTR lpDest);
ENGINE_API	void	g_StrRep(LPSTR lpDest, LPSTR lpSrc, LPSTR lpRep);

//---------------------------------------------------------------------------
#endif
