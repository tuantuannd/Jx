/********************************************************************
	created:	2003/05/30
	file base:	Application
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_APPLICATION_H__
#define __INCLUDE_APPLICATION_H__

#include "mutex.h"

#include "Network.h"
#include "Intercessor.h"

class CBishopApp
{
public:

	CBishopApp();
	CBishopApp( HINSTANCE hInstance );

	virtual ~CBishopApp();
	
	int Run();

protected:
	
	/*
	 * Login dialog
	 */
	bool LoginSystem();

	static BOOL CALLBACK LoginDlgProc( HWND hwndDlg,
		UINT message, 
		WPARAM wParam, 
		LPARAM lParam );

	static void EnterToAffirm( HWND hwndDlg );
	static void LeaveToAffirm( HWND hwndDlg );
	static bool CheckUserInfo( HWND hwndDlg );

	static DWORD WINAPI ServerLoginRoutine( HWND hwndDlg );
	
	static void SendAnnounceText( HWND hwndDlg, const char *pText, UINT uLength, UINT uOption, BOOL bAllGS );
	static void EnableGameSvrCtrl( HWND hwndDlg, BOOL nEnable );
	static void EnableCtrl( HWND hwndDlg, UINT nCtrlID, BOOL nEnable );
	
	/*
	 * Main dialog
	 */
	BOOL RegisterMainWndClass();
	UINT MainDialog();

	static BOOL CALLBACK MainWndProc( HWND hwnd, 
                           UINT msg,
                           WPARAM wParam,
                           LPARAM lParam );

	static void InitMainDlg( HWND hDlg );
	static void CloseMainDlg( HWND hDlg );
	static void UpdateVariable( HWND hDlg );

private:
	
	static void AddGameServerInfo( HWND hDlg, UINT nID, const char *pInfo );
	static void DelGameServerInfo( HWND hDlg, UINT nID );
	
	enum enumServerLoginErrorCode
	{
		enumConnectFailed	= 0xA1,
		enumUsrNamePswdErr,
		enumIPPortErr,
		enumException
	};

	OnlineGameLib::Win32::CMutex m_theOnlyOneInstance;
	static HINSTANCE			 m_hInst;

	static CNetwork				 m_theNetwork;

	static CIntercessor			 *m_pIntercessor;
	

};

#endif // __INCLUDE_APPLICATION_H__