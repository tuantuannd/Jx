// Sample.h: interface for the CSample class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAMPLE_H__AEAA425F_A0DA_4AC2_9A33_1C60E6BC9321__INCLUDED_)
#define AFX_SAMPLE_H__AEAA425F_A0DA_4AC2_9A33_1C60E6BC9321__INCLUDED_

#include <winsock2.h>
#include "ESClient\Exception.h"
#include "ESClient\Utils.h"
#include "ESClient\ManualResetEvent.h"
#include "GameClient.h"
//[ Include in .\..\Protocol
#include "Protocol\Protocol.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSample  
{
public:
	static CSample* Instance();
	static void ReleaseInstance();
	BOOL Start();
	BOOL Connect( LPCTSTR lpszAddr, short siPort );
	void Logout( DWORD dwGamerID, DWORD dwServerID );
	void DecAccountDeposit( DWORD dwGamerID, DWORD dwServerID, DWORD dwDecSeconds );
	void QueryGSList();
	void Login( const char* lpszAccName, const char* lpszPassword, const DWORD dwServerID );
	virtual ~CSample();
protected:
	CSample();
	BOOL CreateClientSocket();
	void ReleaseClientSocket();
	static CSample* m_pInstance;
	CGameClient* m_pS;
};

#endif // !defined(AFX_SAMPLE_H__AEAA425F_A0DA_4AC2_9A33_1C60E6BC9321__INCLUDED_)
