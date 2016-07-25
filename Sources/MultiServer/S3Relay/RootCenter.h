// RootCenter.h: interface for the CRootCenter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROOTCENTER_H__36B8EBBF_6ECB_4A6C_9B43_8D6A5AA1C31D__INCLUDED_)
#define AFX_ROOTCENTER_H__36B8EBBF_6ECB_4A6C_9B43_8D6A5AA1C31D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetCenter.h"

class CRootCenter : public CNetCenter  
{
public:
	CRootCenter();
	virtual ~CRootCenter();

public:
	BOOL TraceInfo();
};

#endif // !defined(AFX_ROOTCENTER_H__36B8EBBF_6ECB_4A6C_9B43_8D6A5AA1C31D__INCLUDED_)
