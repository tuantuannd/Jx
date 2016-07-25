//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSG_MD5_String.h
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   2003-7-29 17:53:17
//  Comment     :   Conver String to MD5 String
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef KSG_MD5_STRING_H
#define KSG_MD5_STRING_H    1

#include "KWin32.h"

ENGINE_API int KSG_StringToMD5String(char szDestMD5String[64], const char cszSrcString[]);

#endif // KSG_MD5_STRING_H

