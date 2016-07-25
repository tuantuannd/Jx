/***
*resetstk.c - Recover from Stack overflow.
*
*       Copyright (c) 1989-2001, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the _resetstkoflw() function.
*
*******************************************************************************/

#if defined(_MSC_VER) & _MSC_VER < 1300
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>

#define MIN_STACK_REQ_WIN9X 0x11000
#define MIN_STACK_REQ_WINNT 0x2000

#ifdef _WIN64
typedef unsigned __int64 REGEX_DWORD_PTR;
#else
typedef unsigned __int32 REGEX_DWORD_PTR;
#endif

struct osplatform_getter
{
    int m_osplatform;

    osplatform_getter() : m_osplatform( 0 )
    {
        OSVERSIONINFOA osvi;
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        if( GetVersionExA( & osvi ) )
            m_osplatform = osvi.dwPlatformId;
    }
};

inline int get_osplatform()
{
    static osplatform_getter const s_osplatform_getter;
    return s_osplatform_getter.m_osplatform;
};

/***
* void _resetstkoflw(void) - Recovers from Stack Overflow
*
* Purpose:
*       Sets the guard page to its position before the stack overflow.
*
* Exit:
*       Returns nonzero on success, zero on failure
*
*******************************************************************************/

extern "C" int __cdecl _resetstkoflw(void)
{
    LPBYTE pStack, pGuard, pStackBase, pMinGuard;
    MEMORY_BASIC_INFORMATION mbi;
    SYSTEM_INFO si;
    DWORD PageSize;
    DWORD flNewProtect;
    DWORD flOldProtect;

    // Use _alloca() to get the current stack pointer

    pStack = static_cast<LPBYTE>( _alloca(1) );

    // Find the base of the stack.

    if (VirtualQuery(pStack, &mbi, sizeof mbi) == 0)
        return 0;
    pStackBase = static_cast<LPBYTE>( mbi.AllocationBase );

    // Find the page just below where the stack pointer currently points.
    // This is the new guard page.

    GetSystemInfo(&si);
    PageSize = si.dwPageSize;

    pGuard = (LPBYTE) (((REGEX_DWORD_PTR)pStack & ~(REGEX_DWORD_PTR)(PageSize - 1))
                       - PageSize);

    // If the potential guard page is too close to the start of the stack
    // region, abandon the reset effort for lack of space.  Win9x has a
    // larger reserved stack requirement.

    pMinGuard = pStackBase + ((get_osplatform() == VER_PLATFORM_WIN32_WINDOWS)
                              ? MIN_STACK_REQ_WIN9X
                              : MIN_STACK_REQ_WINNT);

    if (pGuard < pMinGuard)
        return 0;

    // On a non-Win9x system, release the stack region below the new guard
    // page.  This can't be done for Win9x because of OS limitations.

    if (get_osplatform() != VER_PLATFORM_WIN32_WINDOWS) {
        if (pGuard > pStackBase)
            VirtualFree(pStackBase, pGuard - pStackBase, MEM_DECOMMIT);

        VirtualAlloc(pGuard, PageSize, MEM_COMMIT, PAGE_READWRITE);
    }

    // Enable the new guard page.

    flNewProtect = get_osplatform() == VER_PLATFORM_WIN32_WINDOWS
                   ? PAGE_NOACCESS
                   : PAGE_READWRITE | PAGE_GUARD;

    return VirtualProtect(pGuard, PageSize, flNewProtect, &flOldProtect);
}

#endif
