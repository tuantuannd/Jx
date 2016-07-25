// Memory.h: interface for the CMemory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORY_H__0907B6C3_6BEC_4108_BDD7_283C7779A3B8__INCLUDED_)
#define AFX_MEMORY_H__0907B6C3_6BEC_4108_BDD7_283C7779A3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMemory  
{
public:
	CMemory();
	virtual ~CMemory();

public:
	BOOL Initialize();
	BOOL Uninitialize();

public:
	void* Alloc(unsigned len);
	void Reuse(void* p);
	void Free(void* p);
	unsigned Size(void* p);
};

#endif // !defined(AFX_MEMORY_H__0907B6C3_6BEC_4108_BDD7_283C7779A3B8__INCLUDED_)
