// HeavenLib.cpp: implementation of the CHeavenLib class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HeavenLib.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHeavenLib::CHeavenLib()
	: m_hHeaven(NULL), m_lpfnCreateInterface(NULL)
{

}

CHeavenLib::~CHeavenLib()
{

}


BOOL CHeavenLib::Initialize()
{
	if (m_hHeaven)
		return FALSE;

	if (!(m_hHeaven = ::LoadLibrary(_T("heaven.dll"))))
		return FALSE;

	if (!(m_lpfnCreateInterface = (LPFNCREATEINTERFACE)GetProcAddress(m_hHeaven, "CreateInterface")))
		return FALSE;

	return TRUE;
}

BOOL CHeavenLib::Uninitialize()
{
	m_lpfnCreateInterface = NULL;

	if (m_hHeaven)
	{
		::FreeLibrary(m_hHeaven);
		m_hHeaven = NULL;
	}

	return TRUE;
}


HRESULT CHeavenLib::CreateInterface(REFIID riid, void** pp)
{
	if (!m_lpfnCreateInterface)
		return -1;

	return m_lpfnCreateInterface(riid, pp);
}
