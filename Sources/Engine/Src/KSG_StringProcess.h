//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSG_StringProcess.h
//  Version     :   1.0
//  Creater     :   Freeeway Chen
//  Date        :   2003-8-1 12:43:32
//  Comment     :   Process String to Int, Skip Symbol
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _KSG_STRING_PROCESS_H   
#define _KSG_STRING_PROCESS_H   1

#include "KWin32.h"

ENGINE_API int KSG_StringGetInt(const char **ppcszString, int nDefaultValue);
ENGINE_API bool KSG_StringSkipSymbol(const char **ppcszString, int nSymbol);

inline int KSG_StringGetInt(char **ppszString, int nDefaultValue)
{
    return KSG_StringGetInt((const char **)ppszString, nDefaultValue);    
}

inline bool KSG_StringSkipSymbol(char **ppszString, int nSymbol)
{
    return KSG_StringSkipSymbol((const char **)ppszString, nSymbol);
}


#endif

