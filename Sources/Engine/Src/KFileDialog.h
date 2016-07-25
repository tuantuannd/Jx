//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KFileDialog.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef WsFileDialogH
#define WsFileDialogH
//---------------------------------------------------------------------------
#include <CommDlg.h>
#include "KFilePath.h"
//---------------------------------------------------------------------------
class ENGINE_API	KFileDialog
{
private:
	OPENFILENAME	m_OpenFileName;
	char			m_szFileName[4096];
	char			m_szFilePath[MAXPATH];
	char			m_szFileTitle[MAXFILE];
	char			m_szTitle[80];
	char			m_szFilter[80];
	char			m_szDefExt[80];
public:
	KFileDialog();
	BOOL			Open();
	BOOL			Save();
	void			SetTitle(LPSTR Title);
	void			SetFlags(DWORD Flags);
	void			SetFilter(LPSTR Filter);
	void			SetDefExt(LPSTR DefExt);
	void			SetFilePath(LPSTR lpstrPath);
	LPSTR			GetFilePath(){ return m_szFilePath; };
	void			SetFileName(LPSTR lpstrFile);
	LPSTR			GetFileName(){ return m_szFileName; };
	void			SetFileTitle(LPSTR lpstrFile);
	LPSTR			GetFileTitle(){ return m_szFileTitle; };
};
//---------------------------------------------------------------------------
#endif
