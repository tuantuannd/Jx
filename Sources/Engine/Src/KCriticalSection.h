//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KCriticalSection.h
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   2003-9-2 17:17:16
//  Comment     :   参考了刘鹏的的实现 
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _KCRITICALSECTION_H_
#define _KCRITICALSECTION_H_    1

#include "windows.h"

class KCriticalSection
{

private:
	CRITICAL_SECTION	m_CriticalSection;

public:
    KCriticalSection()
    {
        InitializeCriticalSection(&m_CriticalSection);
    }

    ~KCriticalSection()
    {
        DeleteCriticalSection(&m_CriticalSection);
    }

    int Lock()
    {
        EnterCriticalSection(&m_CriticalSection);
        return true;
    }

    int UnLock()
    {
        LeaveCriticalSection(&m_CriticalSection);

        return false;
    }
};

class KAutoCriticalSection
{
    KCriticalSection &m_CriticalSection;

public:
    KAutoCriticalSection(KCriticalSection &CriticalSection)
        : m_CriticalSection(CriticalSection)
    {
        m_CriticalSection.Lock();
    }

    ~KAutoCriticalSection()
    {
        m_CriticalSection.UnLock();
    }
};

#endif

