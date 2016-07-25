// S3DBClient.h : main header file for the S3DBCLIENT application
//

#if !defined(AFX_S3DBCLIENT_H__FE1AC36E_2AE6_404C_BC43_7415CFD68B01__INCLUDED_)
#define AFX_S3DBCLIENT_H__FE1AC36E_2AE6_404C_BC43_7415CFD68B01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "ClientSocket.h"

/////////////////////////////////////////////////////////////////////////////
// CS3DBClientApp:
// See S3DBClient.cpp for the implementation of this class
//

typedef struct tag_SETUPINFO
{
	short siLocalPort;
	char szDBServerIP[16];
	short siDBServerPort;
	DWORD dwWaitTime;
}_SETUPINFO, *_PSETUPINFO;

class CS3DBClientApp : public CWinApp
{
public:
	CS3DBClientApp();

	BOOL m_bConnected;
	_SETUPINFO m_serverInfo;
	CClientSocket* m_pClientSocket;
	CString m_cstrCurUsername;
	CString m_cstrCurPassword;
	TCHAR m_szDefaultAccPassword[LOGIN_PASSWORD_MAX_LEN+2];
	TCHAR m_szDefaultAccRealName[LOGIN_REALNAME_MAX_LEN+2];
	TCHAR m_szLogPath[MAX_PATH+1];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS3DBClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CS3DBClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	void InitAccOperation();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S3DBCLIENT_H__FE1AC36E_2AE6_404C_BC43_7415CFD68B01__INCLUDED_)
