//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KFileCopy.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Win32 File Copy Operation Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"
#include "KFileCopy.h"
//---------------------------------------------------------------------------
// 函数:	CopyFiles
// 功能:	拷贝文件
// 参数:	lpSrcDir		源目录
//			lpDesDir		目标目录
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_DeleteFiles(LPSTR lpSrcDir, LPSTR lpDesDir)
{
	char			szSrcDir[MAXPATH];
	char			szDesDir[MAXPATH];
	WIN32_FIND_DATA FindData1;
	WIN32_FIND_DATA FindData2;
	HANDLE			hFind1;
	HANDLE			hFind2;
	int				nSrcLen;
	int				nDesLen;
	
	g_StrCpy(szSrcDir, lpSrcDir);
	g_StrCpy(szDesDir, lpDesDir);

	nSrcLen = g_StrLen(szSrcDir);
	nDesLen = g_StrLen(szDesDir);
	
	g_StrCpy(&szSrcDir[nSrcLen], "*.*");
	g_StrCpy(&szDesDir[nDesLen], "*.*");
	
	hFind1 = FindFirstFile(szSrcDir, &FindData1);
	if (hFind1 == INVALID_HANDLE_VALUE)
		return;
	
	while (FindNextFile(hFind1, &FindData1))
	{
		g_StrCpy(&szSrcDir[nSrcLen], FindData1.cFileName);
		g_StrCpy(&szDesDir[nDesLen], FindData1.cFileName);
		hFind2 = FindFirstFile(szDesDir, &FindData2);
		// 如果szDesDir没有同名文件就删除szSrcDir下的文件
		if (hFind2 == INVALID_HANDLE_VALUE)
		{
			SetFileAttributes(szSrcDir, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szSrcDir);
		}
		FindClose(hFind2);
	}
	FindClose(hFind1);
}
//---------------------------------------------------------------------------
// 函数:	UpdateFiles
// 功能:	拷贝文件
// 参数:	lpSrcDir		源目录
//			lpDesDir		目标目录
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_UpdateFiles(LPSTR lpSrcDir, LPSTR lpDesDir)
{
	char			szSrcDir[MAXPATH];
	char			szDesDir[MAXPATH];
	WIN32_FIND_DATA FindData1;
	WIN32_FIND_DATA FindData2;
	HANDLE			hFind1;
	HANDLE			hFind2;
	LONG			lResult;
	int				nSrcLen;
	int				nDesLen;
	
	g_StrCpy(szSrcDir, lpSrcDir);
	g_StrCpy(szDesDir, lpDesDir);

	nSrcLen = g_StrLen(szSrcDir);
	nDesLen = g_StrLen(szDesDir);
	
	g_StrCpy(&szSrcDir[nSrcLen], "*.*");
	g_StrCpy(&szDesDir[nDesLen], "*.*");
	
	hFind1 = FindFirstFile(szSrcDir, &FindData1);
	if (hFind1 == INVALID_HANDLE_VALUE)
		return;

	while (FindNextFile(hFind1, &FindData1))
	{
		g_StrCpy(&szSrcDir[nSrcLen], FindData1.cFileName);
		g_StrCpy(&szDesDir[nDesLen], FindData1.cFileName);
		hFind2 = FindFirstFile(szDesDir, &FindData2);
		if (hFind2 == INVALID_HANDLE_VALUE)
		{	// 没有同名文件
			CopyFile(szSrcDir, szDesDir, FALSE);
		}
		else
		{	// 有同名文件，就比较修时间
			lResult = CompareFileTime(&FindData1.ftLastWriteTime,
				&FindData2.ftLastWriteTime);
			if (lResult != 0)
			{	// 时间不同则拷贝覆盖，为确保拷贝成功，去掉目标文件的只读属性
				SetFileAttributes(szDesDir, FILE_ATTRIBUTE_NORMAL);
				CopyFile(szSrcDir, szDesDir, FALSE);
			}
		}
		// 去掉拷贝过来的文件的只读属性
		SetFileAttributes(szDesDir, FILE_ATTRIBUTE_NORMAL);
		FindClose(hFind2);
	}
	FindClose(hFind1);
}
//---------------------------------------------------------------------------
