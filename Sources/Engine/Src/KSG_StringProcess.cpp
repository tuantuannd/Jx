//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSG_StringProcess.cpp
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   2003-8-1 12:45:11
//  Comment     :   Process String to Int, Skip Symbol
//
//////////////////////////////////////////////////////////////////////////////////////

#include "KWin32.h"
#include <ctype.h>

#include "KSG_StringProcess.h"


int KSG_StringGetInt(const char **ppcszString, int nDefaultValue)
{
    int nResult = false;
    int nRetValue = 0;
    int nNegSignFlag = false;
    int nRetValueValidFlag = false;
    const char *pcszString = NULL;

    if (!ppcszString)
        goto Exit0;
    
    pcszString = *ppcszString;

    if (!pcszString)
        goto Exit0;

    while (isspace(*pcszString))
        pcszString++;
    
    if ((*pcszString) == '\0')
        goto Exit0;

    if ((*pcszString) == '-')
    {
        nNegSignFlag = true;
        pcszString++;

        // Skip Prev Space
        while (isspace(*pcszString))
            pcszString++;

        if ((*pcszString) == '\0')
            goto Exit0;
    }

    
    while (isdigit(*pcszString))
    {
        nRetValueValidFlag = true;

        nRetValue = nRetValue * 10 +  ((int)(*pcszString - '0'));

        pcszString++;
    }

    nResult = true;
Exit0:

    if (pcszString)
    {
        if (ppcszString)
            *ppcszString = pcszString;
    }

    if (nNegSignFlag)
        nRetValue = -nRetValue;

    if (
        (!nResult) ||
        (!nRetValueValidFlag)
    )
        nRetValue = nDefaultValue; 


    return nRetValue;
}


bool KSG_StringSkipSymbol(const char **ppcszString, int nSymbol)
{
    bool bResult = false;
    const char *pcszString = NULL;

    if (!ppcszString)
        goto Exit0;
    
    pcszString = *ppcszString;

    if (!pcszString)
        goto Exit0;

    while (isspace(*pcszString))
        pcszString++;
    
    if (((unsigned)(*pcszString)) != (unsigned)nSymbol)
        goto Exit0;

    pcszString++;   // Skip Symbol

    bResult = true;
Exit0:

    if (pcszString)
    {
        if (ppcszString)
            *ppcszString = pcszString;
    }

    return bResult;
}

