// Memory.cpp: implementation of the CMemory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Memory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemory::CMemory()
{

}

CMemory::~CMemory()
{

}


BOOL CMemory::Initialize()
{
	return TRUE;
}

BOOL CMemory::Uninitialize()
{
	return TRUE;
}



void* CMemory::Alloc(unsigned len)
{
	assert(len > 0);

	if (len > 0)
	{
		BYTE* pRef = (BYTE*)malloc(len + 1);
		*pRef = 1;
		return pRef + 1;
	}

	return NULL;
}

void CMemory::Reuse(void* p)
{
	assert(p);

	if (p)
	{
		BYTE* pRef = (BYTE*)p - 1;
		(*pRef) ++;
	}
}


void CMemory::Free(void* p)
{
	assert(p);

	if (p)
	{
		BYTE* pRef = (BYTE*)p - 1;
		if (-- (*pRef) == 0)
			free(pRef);
	}
}

unsigned CMemory::Size(void* p)
{
	assert(p);

	if (p)
	{
		BYTE* pRef = (BYTE*)p - 1;
		return _msize(p) - 1;
	}

	return 0;
}
