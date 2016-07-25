// -------------------------------------------------------------------------
//	文件名		：	FilterText.h
//	创建者		：	谢茂培 (Hsie)
//	创建时间	：	2003-07-23 10:13:07
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __FILTERTEXT_H__
#define __FILTERTEXT_H__

#include "unknwn.h"


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FILTERTEXT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FILTERTEXT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef FILTERTEXT_STATICLIB
		#define FILTERTEXT_API
#else
	#ifdef FILTERTEXT_EXPORTS
		#define FILTERTEXT_API __declspec(dllexport)
	#else
		#define FILTERTEXT_API __declspec(dllimport)
	#endif
#endif


const TCHAR leadchar_common = '=';
const TCHAR leadchar_advance = '+';
const TCHAR leadchar_ignore = '-';
const TCHAR leadchar_insensitive = '@';


struct ITextFilter : IUnknown
{
	virtual BOOL AddExpression(LPCTSTR szExp) = 0;
	virtual BOOL Clearup() = 0;
	virtual BOOL IsTextPass(LPCTSTR text) = 0;
};

extern "C" 
FILTERTEXT_API HRESULT CreateTextFilter(ITextFilter** ppTextFilter);




#endif // __FILTERTEXT_H__
