/********************************************************************
	created:	2003/05/05
	file base:	backup
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_BACKUP_H__
#define __INCLUDE_BACKUP_H__

#ifdef USE_THIS


enum enumBaseValue
{
	_enumLoginState = 0x10,
};

enum enumLoginState
{
	enumLoginBase = _enumLoginState,

	enumLoginEnter,
	enumLoginLeave
};

/*
 * Log on to system
 */
bool LoginSystem( HINSTANCE hInstance, UINT nState /* enumLoginState */ )
{
	static HWND hLoginDlg = NULL;

	if ( enumLoginEnter == nState )
	{
		if ( !hLoginDlg || !::IsWindow( hLoginDlg ) )
		{
			hLoginDlg = ::CreateDialog( hInstance, 
				MAKEINTRESOURCE( IDD_DLG_LOGIN ),
				NULL,
				( DLGPROC )LoginDlgProc );
			
			if ( !hLoginDlg || !::IsWindow( hLoginDlg ) )
			{
				return false;
			}
			
			::ShowWindow( hLoginDlg, SW_SHOW );
			::UpdateWindow( hLoginDlg );
		}
	}
	else if ( enumLoginLeave == nState )
	{
		if ( hLoginDlg && ::IsWindow( hLoginDlg ) )
		{
			::PostMessage( hLoginDlg, WM_COMMAND, MAKEWORD( IDCANCEL, 0 ), 0L );
		}
	}
	else
	{
		ASSERT( FALSE );

		return false;
	}

	return true;
}

BOOL CALLBACK LoginDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch ( message )
    {
        case WM_INITDIALOG:
			::MessageBox( hwndDlg, "ENTER", "INFO", MB_OK );
            return TRUE; 
 
        case WM_COMMAND: 
            switch ( LOWORD( wParam ) ) 
            { 
                case IDOK: 
                    /*
					 * Notify the owner window to carry out the task. 
					 */
 
                    return TRUE; 
 
                case IDCANCEL: 
					::MessageBox( hwndDlg, "LEAVE", "INFO", MB_OK ); 

                    ::DestroyWindow( hwndDlg ); 
                    return TRUE; 
            } 
    } 

	return FALSE;
}

#endif // USE_THIS

#endif // __INCLUDE_BACKUP_H__