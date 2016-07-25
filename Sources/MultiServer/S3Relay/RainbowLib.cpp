// RainbowLib.cpp: implementation of the CRainbowLib class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RainbowLib.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRainbowLib::CRainbowLib()
	: m_hRainbow(NULL), m_lpfnCreateInterface(NULL)
{

}

CRainbowLib::~CRainbowLib()
{

}


BOOL CRainbowLib::Initialize()
{
	if (m_hRainbow)
		return FALSE;

	if (!(m_hRainbow = ::LoadLibrary(_T("rainbow.dll"))))
			return FALSE;

	if (!(m_lpfnCreateInterface = (LPFNCREATEINTERFACE)GetProcAddress(m_hRainbow, "CreateInterface")))
		return FALSE;

	return TRUE;
}

BOOL CRainbowLib::Uninitialize()
{
	m_lpfnCreateInterface = NULL;

	if (m_hRainbow)
	{
		::FreeLibrary(m_hRainbow);
		m_hRainbow = NULL;
	}

	return TRUE;
}


HRESULT CRainbowLib::CreateInterface(REFIID riid, void** pp)
{
	if (!m_lpfnCreateInterface)
		return -1;

	return m_lpfnCreateInterface(riid, pp);
}
