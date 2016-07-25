// KTransferUnit.h: interface for the KTransferUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KTRANSFERUNIT_H__F9D3209E_872F_4C57_B780_6E4612D1B597__INCLUDED_)
#define AFX_KTRANSFERUNIT_H__F9D3209E_872F_4C57_B780_6E4612D1B597__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _STANDALONE
#include "Buffer.h"
using OnlineGameLib::Win32::CPackager;
#else 
#include "KWin32.h"
#include "package.h"
#endif


class KTransferUnit  
{
public:
	CPackager	m_RecvData;

private:
	DWORD		m_dwIp;
	DWORD		m_dwRelayID;
public:
	KTransferUnit(DWORD dwIp, DWORD dwRelayID);
	virtual ~KTransferUnit();
	DWORD		GetIp() { return m_dwIp; };
	DWORD		GetRelayID() { return m_dwRelayID; };
};

#endif // !defined(AFX_KTRANSFERUNIT_H__F9D3209E_872F_4C57_B780_6E4612D1B597__INCLUDED_)
