// FilterTextLib.cpp: implementation of the CFilterTextLib class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilterTextLib.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFilterTextLib::CFilterTextLib()
	: m_hFilterText(NULL), m_lpfnCreateTextFilter(NULL)
{

}

CFilterTextLib::~CFilterTextLib()
{

}


BOOL CFilterTextLib::Initialize()
{
	if (m_hFilterText)
		return FALSE;

	if (!(m_hFilterText = ::LoadLibrary("FilterText.dll")))
		return FALSE;

	if (!(m_lpfnCreateTextFilter = (LPFNCREATETEXTFILTER)GetProcAddress(m_hFilterText, "CreateTextFilter")))
		return FALSE;

	return TRUE;
}

BOOL CFilterTextLib::Uninitialize()
{
	m_lpfnCreateTextFilter = NULL;

	if (m_hFilterText)
	{
		::FreeLibrary(m_hFilterText);
		m_hFilterText = NULL;
	}

	return TRUE;
}


HRESULT CFilterTextLib::CreateTextFilter(ITextFilter** ppTextFilter)
{
	if (!m_lpfnCreateTextFilter)
		return -1;

	return m_lpfnCreateTextFilter(ppTextFilter);
}
