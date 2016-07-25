//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KFileDialog.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Win32 FileDialog Operation Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KWin32Wnd.h"
#include "KFileDialog.h"
//---------------------------------------------------------------------------
// 函数:	KFileDialog
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KFileDialog::KFileDialog()
{
	// init OpenFileName members
	m_OpenFileName.lStructSize		= sizeof(OPENFILENAME);
	m_OpenFileName.hwndOwner		= NULL;
	m_OpenFileName.hInstance		= NULL;
	m_OpenFileName.lpstrFilter		= m_szFilter;
	m_OpenFileName.lpstrCustomFilter= NULL;
	m_OpenFileName.nMaxCustFilter	= 0;
	m_OpenFileName.nFilterIndex		= 0;
	m_OpenFileName.lpstrFile		= m_szFileName;
	m_OpenFileName.nMaxFile			= sizeof(m_szFileName);
	m_OpenFileName.lpstrFileTitle	= m_szFileTitle;
	m_OpenFileName.nMaxFileTitle	= sizeof(m_szFileTitle);
	m_OpenFileName.lpstrInitialDir	= m_szFilePath;
	m_OpenFileName.lpstrTitle		= m_szTitle;
	m_OpenFileName.Flags			= 0;
	m_OpenFileName.nFileOffset		= 0;
	m_OpenFileName.nFileExtension	= 0;
	m_OpenFileName.lpstrDefExt		= m_szDefExt;
	m_OpenFileName.lCustData		= 0;

	// init FileDialog members
	m_szFileName[0]		= 0;
	m_szFilePath[0]		= 0;
	m_szFileTitle[0]	= 0;
	m_szTitle[0]		= 0;
	m_szFilter[0]		= 0;
	m_szDefExt[0]		= 0;
}
//---------------------------------------------------------------------------
// 函数:	SetTitle
// 功能:	设置文件对话窗的标题
// 参数:	szTitle		标题
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetTitle(LPSTR szTitle)
{
	g_StrCpy(m_szTitle, szTitle);
}
//---------------------------------------------------------------------------
// 函数:	SetFlags
// 功能:	设置文件对话窗的标志
// 参数:	Flags	标志
//				OFN_ALLOWMULTISELECT - 允许多选
//				OFN_OVERWRITEPROMPT - 文件重名时显示提示
//				OFN_EXPLORER - 浏览器风格
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetFlags(DWORD Flags)
{
	m_OpenFileName.Flags = Flags | OFN_EXPLORER;
}
//---------------------------------------------------------------------------
// 函数:	SetFilter
// 功能:	设置文件对话窗的文件扩展名过滤
// 参数:	szFilter	过滤器
//			例如"Text files (*.txt)|*.TXT";
//				"Text files (*.txt)|*.TXT|Pascal files (*.pas)|*.PAS";
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetFilter(LPSTR szFilter)
{
	int i;

	// copy filter string
	g_StrCpy(m_szFilter, szFilter);

	// convert '|' to '\0'
	for (i = 0; m_szFilter[i]; i++)
	{
		if (m_szFilter[i] == '|')
			m_szFilter[i] = 0;
	}

	// double null terminators
	m_szFilter[i + 1] = 0;
}
//---------------------------------------------------------------------------
// 函数:	SetDefExt
// 功能:	设置文件对话窗的缺省扩展名
// 参数:	szDefExt	缺省扩展名
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetDefExt(LPSTR szDefExt)
{
	g_StrCpy(m_szDefExt, szDefExt);
}
//---------------------------------------------------------------------------
// 函数:	SetFilePath
// 功能:	设置文件路径
// 参数:	lpstrPath	文件路径
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetFilePath(LPSTR lpstrPath)
{
	if (lpstrPath[0] == '\\')
	{
		g_SetFilePath(lpstrPath);
		g_GetFullPath(m_szFilePath, lpstrPath);
	}
	else
	{
		g_StrCpy(m_szFilePath, lpstrPath);
	}
}
//---------------------------------------------------------------------------
// 函数:	SetFileName
// 功能:	设置文件对话窗的文件名
// 参数:	lpstrFile	文件名
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetFileName(LPSTR lpstrFile)
{
	g_StrCpy(m_szFileName, lpstrFile);
}
//---------------------------------------------------------------------------
// 函数:	SetFileTitle
// 功能:	设置文件对话窗的文件名
// 参数:	lpstrFile	文件名
// 返回:	void
//---------------------------------------------------------------------------
void KFileDialog::SetFileTitle(LPSTR lpstrFile)
{
	g_StrCpy(m_szFileTitle, lpstrFile);
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	文件打开对话窗
// 参数:	void
// 返回:	成功返回TRUE，失败或者取消返回FALSE。
//---------------------------------------------------------------------------
BOOL KFileDialog::Open()
{
	m_OpenFileName.hwndOwner = g_GetMainHWnd();
	if (GetOpenFileName(&m_OpenFileName))
	{
		g_StrCpyLen(m_szFilePath, m_szFileName, m_OpenFileName.nFileOffset);
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	文件保存对话窗
// 参数:	void
// 返回:	成功返回TRUE，失败或者取消返回FALSE。
//---------------------------------------------------------------------------
BOOL KFileDialog::Save()
{
	m_OpenFileName.hwndOwner = g_GetMainHWnd();
	if (GetSaveFileName(&m_OpenFileName))
	{
		g_StrCpyLen(m_szFilePath, m_szFileName, m_OpenFileName.nFileOffset);
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------

