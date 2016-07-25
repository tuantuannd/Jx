// RoleNameFilter.h: interface for the CRoleNameFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROLENAMEFILTER_H__4D8459D3_39B0_454E_95B2_7B1D91C10B36__INCLUDED_)
#define AFX_ROLENAMEFILTER_H__4D8459D3_39B0_454E_95B2_7B1D91C10B36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FilterTextLib.h"

class CRoleNameFilter  
{
public:
	CRoleNameFilter();
	virtual ~CRoleNameFilter();

public:
	BOOL Initialize();
	BOOL Uninitialize();

	BOOL IsTextPass(LPCTSTR text);

private:
	ITextFilter* m_pTextFilter;
};

#endif // !defined(AFX_ROLENAMEFILTER_H__4D8459D3_39B0_454E_95B2_7B1D91C10B36__INCLUDED_)
