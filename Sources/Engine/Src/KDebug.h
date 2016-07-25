//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KDebug.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KDebug_H
#define KDebug_H
//---------------------------------------------------------------------------
ENGINE_API HWND g_FindDebugWindow(char* lpClassName, char* lpWindowName);
ENGINE_API void g_DebugLog(LPSTR Fmt, ...);
ENGINE_API void g_MessageBox(LPSTR lpMsg, ...);
ENGINE_API void g_AssertFailed(char* FileName, int LineNum);
//---------------------------------------------------------------------------
#define SAFE_FREE(a)	if (a) {g_MemFree(a); (a)=NULL;}
#define SAFE_RELEASE(a)	if (a) {(a)->Release(); (a)=NULL;}
//---------------------------------------------------------------------------
#ifdef _DEBUG
	#define KASSERT(x)	if (!(x)) g_AssertFailed(__FILE__, __LINE__)
#else
	#define KASSERT(x)	NULL
#endif
//---------------------------------------------------------------------------
#endif
