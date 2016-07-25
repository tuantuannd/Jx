//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPath.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	File Path Utility
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"

#ifndef _SERVER
#include "KPakList.h"
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <string.h>
//---------------------------------------------------------------------------
#ifdef WIN32
static char szRootPath[MAXPATH] = "C:";		// 启动路径
static char szCurrPath[MAXPATH] = "\\";		// 当前路径
#else
static char szRootPath[MAXPATH] = "/";		// 启动路径
static char szCurrPath[MAXPATH] = "/";		// 当前路径
#endif

int RemoveTwoPointPath(LPTSTR szPath, int nLength)
{
	int nRemove = 0;
	KASSERT(szPath);
#ifdef WIN32
	LPCTSTR lpszOld = "\\..\\";
#else
	LPCTSTR lpszOld = "/../";
#endif
	LPTSTR lpszTarget = strstr(szPath, lpszOld);
	if (lpszTarget)
	{
		LPTSTR lpszAfter = lpszTarget + 3;
		while(lpszTarget > szPath)
		{
			lpszTarget--;
			if ((*lpszTarget) == '\\' ||(*lpszTarget) == '/')
				break;
		}
		memmove(lpszTarget, lpszAfter, (nLength - (lpszAfter - szPath) + 1) * sizeof(char));
		nRemove = (lpszAfter - lpszTarget);
		return RemoveTwoPointPath(szPath, nLength - nRemove);
	}

	return nLength - nRemove;
}

int RemoveOnePointPath(LPTSTR szPath, int nLength)
{
	int nRemove = 0;
	KASSERT(szPath);
#ifdef WIN32
	LPCTSTR lpszOld = "\\.\\";
#else
	LPCTSTR lpszOld = "/./";
#endif
	LPTSTR lpszTarget = strstr(szPath, lpszOld);
	if (lpszTarget)
	{
		LPTSTR lpszAfter = lpszTarget + 2;
		memmove(lpszTarget, lpszAfter, (nLength - (lpszAfter - szPath) + 1) * sizeof(char));
		nRemove = (lpszAfter - lpszTarget);
		return RemoveOnePointPath(szPath, nLength - nRemove);
	}

	return nLength - nRemove;
}

int RemoveAllPointPath(LPTSTR szPath, int nLength)
{
	return RemoveOnePointPath(szPath, RemoveTwoPointPath(szPath, nLength));
}

//---------------------------------------------------------------------------
// 函数:	SetRootPath
// 功能:	设置程序的根路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_SetRootPath(LPSTR lpPathName)
{
	if (lpPathName)
	{
		g_DebugLog("set path %s\n", lpPathName);
		g_StrCpy(szRootPath, lpPathName);
	}
	else
	{
//#ifdef WIN32
//		GetCurrentDirectory(MAXPATH, szRootPath);
//#else
		g_DebugLog("set path NULL\n");
		getcwd(szRootPath, MAXPATH);
//#endif
	}

	// 去掉路径末尾的 '\'
	int len = g_StrLen(szRootPath);
	g_DebugLog("set path %s(%d)\n", szRootPath, len);
	if (szRootPath[len - 1] == '\\' || szRootPath[len - 1] == '/')
	{
		szRootPath[len - 1] = 0;
	}
	g_DebugLog("RootPath = %s", szRootPath);
}
//---------------------------------------------------------------------------
// 函数:	GetRootPath
// 功能:	取得程序的根路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_GetRootPath(LPSTR lpPathName)
{
	g_StrCpy(lpPathName, szRootPath);
}
//---------------------------------------------------------------------------
// 函数:	SetFilePath
// 功能:	设置当前文件路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_SetFilePath(LPSTR lpPathName)
{
	// 去掉前面的 "\\"
	if (lpPathName[0] == '\\' ||lpPathName[0] == '/')
	{
		g_StrCpy(szCurrPath, lpPathName + 1);
	}
	else
	{
		g_StrCpy(szCurrPath, lpPathName);
	}

	// 末尾加上 "\\"
	int len = g_StrLen(szCurrPath);
	if (len > 0 && szCurrPath[len - 1] != '\\' && szCurrPath[len - 1] != '/')
	{
#ifdef WIN32
		szCurrPath[len] = '\\';
#else
		szCurrPath[len] = '/';
#endif
		szCurrPath[len + 1] = 0;
	}
	RemoveAllPointPath(szCurrPath, len + 1);
#ifndef WIN32
	//'\\' -> '/' [wxb 2003-7-29]
	for (len = 0; szCurrPath[len]; len++)
	{
		if (szCurrPath[len] == '\\')
			szCurrPath[len] = '/';
	}
#endif
}
//---------------------------------------------------------------------------
// 函数:	GetFilePath
// 功能:	取得当前文件路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API	void g_GetFilePath(LPSTR lpPathName)
{
	g_StrCpy(lpPathName, szCurrPath);
}
//---------------------------------------------------------------------------
// 函数:	GetFullPath
// 功能:	取得文件的全路径名
// 参数:	lpPathName	路径名
//			lpFileName	文件名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_GetFullPath(LPSTR lpPathName, LPSTR lpFileName)
{
	// 文件带有全路径
	if (lpFileName[1] == ':')
	{
		g_StrCpy(lpPathName, lpFileName);
		return;
	}

	// 文件带有部分路径
	if (lpFileName[0] == '\\' || lpFileName[0] == '/')
	{
		g_StrCpy(lpPathName, szRootPath);
		g_StrCat(lpPathName, lpFileName);
		return;
	}
	
	// 当前路径为全路径
#ifdef WIN32
	if (szCurrPath[1] == ':')
	{
		g_StrCpy(lpPathName, szCurrPath);
		g_StrCat(lpPathName, lpFileName);
		return;
	}
#endif
	// 当前路径为部分路径
	g_StrCpy(lpPathName, szRootPath);
        if(szCurrPath[0] != '\\' && szCurrPath[0] != '/') {
#ifdef WIN32
	g_StrCat(lpPathName, "\\");
#else
	g_StrCat(lpPathName, "/");
#endif
      }
	g_StrCat(lpPathName, szCurrPath);

	if (lpFileName[0] == '.' && (lpFileName[1] == '\\'||lpFileName[1] == '/') )
		g_StrCat(lpPathName, lpFileName + 2);
	else
		g_StrCat(lpPathName, lpFileName);
}
//---------------------------------------------------------------------------
// 函数:	GetHalfPath
// 功能:	取得文件的半路径名，不带根路径
// 参数:	lpPathName	路径名
//			lpFileName	文件名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_GetHalfPath(LPSTR lpPathName, LPSTR lpFileName)
{
	// 文件带有部分路径
	if (lpFileName[0] == '\\' || lpFileName[0] == '/')
	{
		g_StrCpy(lpPathName, lpFileName);
	}
	else
	{
#ifdef WIN32
		g_StrCpy(lpPathName, "\\");
#else
		g_StrCpy(lpPathName, "/");
#endif
		g_StrCat(lpPathName, szCurrPath);
		g_StrCat(lpPathName, lpFileName);
	}
}
//---------------------------------------------------------------------------
// 函数:	GetPackPath
// 功能:	取得文件在压缩包中的路径名
// 参数:	lpPathName	路径名
//			lpFileName	文件名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_GetPackPath(LPSTR lpPathName, LPSTR lpFileName)
{
	// 文件带有部分路径
	if (lpFileName[0] == '\\' || lpFileName[0] == '/')
	{
		g_StrCpy(lpPathName, lpFileName + 1);
	}
	else
	{
		g_StrCpy(lpPathName, szCurrPath);
		g_StrCat(lpPathName, lpFileName);
	}
	int len = g_StrLen(lpPathName);
	RemoveAllPointPath(lpPathName, len + 1);
	// 全部转换为小写字母
	g_StrLower(lpPathName);
}
//---------------------------------------------------------------------------
// 函数:	GetDiskPath
// 功能:	取得CDROM对应的文件路径名
// 参数:	lpPathName	路径名
//			lpFileName	文件名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_GetDiskPath(LPSTR lpPathName, LPSTR lpFileName)
{
	g_StrCpy(lpPathName, "C:");
	for (int i = 0; i < 24; lpPathName[0]++, i++)
	{
//		if (GetDriveType(lpPathName) == DRIVE_CDROM)
//			break;
	}
	if (lpFileName[0] == '\\' || lpPathName[0] == '/')
	{
		g_StrCat(lpPathName, lpFileName);
	}
	else
	{
#ifdef WIN32
		g_StrCat(lpPathName, "\\");
#else
		g_StrCat(lpPathName, "/");
#endif
		g_StrCat(lpPathName, szCurrPath);
		g_StrCat(lpPathName, lpFileName);
	}
}

//---------------------------------------------------------------------------
// 函数:	CreatePath
// 功能:	在游戏跟目录下建立一条路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API	void	g_CreatePath(LPSTR lpPathName)
{
	if (!lpPathName || !lpPathName[0])
		return;

	char	szFullPath[MAXPATH];
	int		i;
	// 文件带有全路径
	if (lpPathName[1] == ':')
	{
		if (g_StrLen(lpPathName) < 4)
			return;
		g_StrCpy(szFullPath, lpPathName);
		i = 4;
	}
	else if (lpPathName[0] == '\\' || lpPathName[0] == '/')
	{
		g_StrCpy(szFullPath, szRootPath);
		g_StrCat(szFullPath, lpPathName);
		i = g_StrLen(szRootPath) + 1;
	}
	else
	{
		g_StrCpy(szFullPath, szRootPath);
#ifdef WIN32
		g_StrCat(szFullPath, "\\");
#else
		g_StrCat(szFullPath, "/");
#endif
		g_StrCat(szFullPath, lpPathName);
                
		i = g_StrLen(szRootPath) + 1;
	}

	for (; i < g_StrLen(szFullPath); i++)
	{
#ifdef WIN32
		if (szFullPath[i] == '\\') {
			szFullPath[i] = 0;
			CreateDirectory(szFullPath, NULL);
			szFullPath[i] = '\\';
		}
#else
		if (szFullPath[i] == '/') {
			szFullPath[i] = 0;
			szFullPath[i] = '/';
		}
#endif
	}
#ifdef WIN32
	CreateDirectory(szFullPath, NULL);
#else
	// flying comment
	// 朱传靖实现这个调用
	//mkdir();
#endif
}

//---------------------------------------------------------------------------
// 函数:	g_UnitePathAndName
// 功能:	一个路径和一个文件名，合并到lpGet中形成一个完整的路径文件名
// 参数:	lpPath 传入路径名 lpFile 传入文件名 lpGet 获得的最终完整文件名
// 返回:	void
// 注意：   这里没有考虑字符串的长度，使用的时候要保证字符串的长度足够
//---------------------------------------------------------------------------
ENGINE_API	void	g_UnitePathAndName(char *lpPath, char *lpFile, char *lpGet)
{
	if (!lpPath || !lpFile || !lpGet)
		return;
	strcpy(lpGet, lpPath);
	int	nSize = strlen(lpGet);
	if (lpGet[nSize] - 1 != '\\')
	{
		lpGet[nSize] = '\\';
		lpGet[nSize + 1] = 0;
	}
	if (lpFile[0] != '\\')
	{
		strcat(lpGet, lpFile);
	}
	else
	{
		strcat(lpGet, &lpFile[1]);
	}
}


//---------------------------------------------------------------------------
// 函数:	find if file exists in pak or in hard disk
// 功能:	返回指定的文件是否存在
// 参数:	lpPathName	路径名＋文件名
// 返回:	TRUE－成功，FALSE－失败。
//---------------------------------------------------------------------------
ENGINE_API BOOL g_FileExists(LPSTR FileName)
{
	BOOL	bExist = FALSE;
	char	szFullName[MAX_PATH];
	if (FileName && FileName[0])
	{
#ifndef _SERVER
		//先查是是否在打包文件中
		if (g_pPakList)
		{
			XPackElemFileRef	PackRef;
			bExist = g_pPakList->FindElemFile(FileName, PackRef);
		}
#endif
		//在检查是否单独存在文件系统里
		if (bExist == FALSE)
		{
			#ifdef	WIN32
				g_GetFullPath(szFullName, FileName);
				bExist = !(GetFileAttributes(szFullName) & FILE_ATTRIBUTE_DIRECTORY);// || dword == INVALID_FILE_ATTRIBUTES)
			#endif
		}
	}
	return bExist;
}
//---------------------------------------------------------------------------
// 函数:	File Name to 32bit Id
// 功能:	文件名转换成 Hash 32bit ID
// 参数:	lpFileName	文件名
// 返回:	FileName Hash 32bit ID
// 
// 注意:	游戏世界和主网关交互数据所用的哈希查找索引也是用
//			的这个函数，所以请修改这个函数时也对应修改主网管
//			中相对应的那个函数。这个函数存在于Common.lib工程的Utils.h
//			中，函数声明为 DWORD HashStr2ID( const char * const pStr );
//---------------------------------------------------------------------------
ENGINE_API DWORD g_FileName2Id(LPSTR lpFileName)
{
	DWORD Id = 0;
	char c = 0;
	for (int i = 0; lpFileName[i]; i++)
	{
		c = lpFileName[i];
#ifndef WIN32
		//for linux path looking up
		if ('/' == c)
			c = '\\';
#endif
		Id = (Id + (i + 1) * c) % 0x8000000b * 0xffffffef;
	}
	return (Id ^ 0x12345678);
}
//---------------------------------------------------------------------------
// 函数:	change file extention
// 功能:	改变文件的扩展名
// 参数:	lpFileName	文件名
//			lpNewExt	新扩展名，不能有'.'
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_ChangeFileExt(LPSTR lpFileName, LPSTR lpNewExt)
{
	int  i;

	for (i = 0; lpFileName[i]; i++)
	{
		if (lpFileName[i] == '.')
			break;
	}
	
	if (lpFileName[i] == '.')
	{
		g_StrCpy(&lpFileName[i + 1], lpNewExt);
	}
	else
	{
		g_StrCat(lpFileName, ".");
		g_StrCat(lpFileName, lpNewExt);
	}
}
//---------------------------------------------------------------------------
// 函数:	Extract File Name from path name
// 功能:	取得文件名（不包含路径）
// 参数:	lpFileName	文件名（不包含路径）
//			lpFilePath	文件名（包含路径）
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_ExtractFileName(LPSTR lpFileName, LPSTR lpFilePath)
{
	int nLen = g_StrLen(lpFilePath);
	if (nLen < 5)
		return;
	int nPos = nLen;
	while (--nPos > 0)
	{
		if (lpFilePath[nPos] == '\\'||lpFilePath[nPos] == '/')
			break;
	}
	g_StrCpy(lpFileName, &lpFilePath[nPos + 1]);
}
//---------------------------------------------------------------------------
// 函数:	Extract File Path from path name
// 功能:	取得路径名
// 参数:	lpFileName	路径名
//			lpFilePath	路径名＋文件名
// 返回:	void
//---------------------------------------------------------------------------
ENGINE_API void g_ExtractFilePath(LPSTR lpPathName, LPSTR lpFilePath)
{
	int nLen = g_StrLen(lpFilePath);
	if (nLen < 5)
		return;
	int nPos = nLen;
	while (--nPos > 0)
	{
		if (lpFilePath[nPos] == '\\' ||lpFilePath[nPos] == '/')
			break;
	}
	g_StrCpyLen(lpPathName, lpFilePath, nPos);
}
//---------------------------------------------------------------------------
