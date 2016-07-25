//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KTabFile.cpp
// Date:	2002.02.20
// Code:	Huyi(Spe)
// Desc:	Tab File Operation Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KStrBase.h"
#include "KFile.h"
#include "KFilePath.h"
#include "KPakFile.h"
#ifndef _SERVER
#include "KCodec.h"
#endif
#include "KTabFile.h"
#include <string.h>
//---------------------------------------------------------------------------
// 函数:	KTabFile
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KTabFile::KTabFile()
{
	m_Width		= 0;
	m_Height	= 0;
}
//---------------------------------------------------------------------------
// 函数:	~KTabFile
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KTabFile::~KTabFile()
{
	Clear();
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	加载一个Tab文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KTabFile::Load(LPSTR FileName)
{
	KPakFile	File;
	DWORD		dwSize;
	PVOID		Buffer;

	// check file name
	if (FileName[0] == 0)
		return FALSE;

	if (!File.Open(FileName))
	{
		g_DebugLog("Can't open tab file : %s", FileName);
		return FALSE;
	}

	dwSize = File.Size();

	Buffer = m_Memory.Alloc(dwSize);

	File.Read(Buffer, dwSize);

	CreateTabOffset();

	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	CreateTabOffset
// 功能:	建立制作表符分隔文件的偏移表
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KTabFile::CreateTabOffset()
{
	int		nWidth, nHeight, nOffset, nSize;
	BYTE	*Buffer;
	TABOFFSET *TabBuffer;

	nWidth	= 1;
	nHeight	= 1;
	nOffset = 0;

	Buffer	= (BYTE *)m_Memory.GetMemPtr();
	nSize	= m_Memory.GetMemLen();

	// 读第一行决定有多少列
	while (*Buffer != 0x0d && *Buffer != 0x0a)
	{
		if (*Buffer == 0x09)
		{
			nWidth++;
		}
		Buffer++;
		nOffset++;
	}
	if (*Buffer == 0x0d && *(Buffer + 1) == 0x0a)
	{
		Buffer += 2;	// 0x0a跳过		
		nOffset += 2;	// 0x0a跳过
	}
	else
	{
		Buffer += 1;	// 0x0a跳过		
		nOffset += 1;	// 0x0a跳过
	}
	while(nOffset < nSize)
	{
		while (*Buffer != 0x0d && *Buffer != 0x0a)
		{
			Buffer++;
			nOffset++;
			if (nOffset >= nSize)
				break;
		}
		nHeight++;
		if (*Buffer == 0x0d && *(Buffer + 1) == 0x0a)
		{
			Buffer += 2;	// 0x0a跳过		
			nOffset += 2;	// 0x0a跳过
		}
		else
		{
			Buffer += 1;	// 0x0a跳过		
			nOffset += 1;	// 0x0a跳过
		}
	}
	m_Width		= nWidth;
	m_Height	= nHeight;

	TabBuffer = (TABOFFSET *)m_OffsetTable.Alloc(m_Width * m_Height * sizeof (TABOFFSET));
	Buffer = (BYTE *)m_Memory.GetMemPtr();

	nOffset = 0;
	int nLength;
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			TabBuffer->dwOffset = nOffset;	
			nLength = 0;
			while(*Buffer != 0x09 && *Buffer != 0x0d && *Buffer != 0x0a && nOffset < nSize)
			{
				Buffer++;
				nOffset++;
				nLength++;
			}
			Buffer++;	// 0x09或0x0d或0x0a(linux)跳过
			nOffset++;
			TabBuffer->dwLength = nLength;
			TabBuffer++;
			if (*(Buffer - 1) == 0x0a || *(Buffer - 1) == 0x0d)	//	本行已经结束了，虽然可能没到nWidth //for linux modified [wxb 2003-7-29]
			{
				for (int k = j+1; k < nWidth; k++)
				{
					TabBuffer->dwOffset = nOffset;
					TabBuffer->dwLength = 0;
					TabBuffer++;					
				}
				break;
			}
		}

		//modified for linux [wxb 2003-7-29]
		if (*(Buffer - 1) == 0x0d && *Buffer == 0x0a)
		{
			Buffer++;				// 0x0a跳过	
			nOffset++;				// 0x0a跳过	
		}
	}
}

//---------------------------------------------------------------------------
// 函数:	Str2Column
// 功能:	取得某行某列字符串的值
// 参数:	szColumn
// 返回:	第几列
//---------------------------------------------------------------------------
int KTabFile::Str2Col(LPSTR szColumn)
{
	int	nStrLen = g_StrLen(szColumn);
	char	szTemp[4];

	g_StrCpy(szTemp, szColumn);
	g_StrUpper(szTemp);
	if (nStrLen == 1)
	{
		return (szTemp[0] - 'A');
	}
	return ((szTemp[0] - 'A' + 1) * 26 + szTemp[1] - 'A') + 1;
}

//---------------------------------------------------------------------------
// 函数:	GetString
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行
//			nColomn			列
//			lpDefault		缺省值
//			lpRString		返回值
//			dwSize			返回字符串的最大长度
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetString(int nRow, LPSTR szColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize, BOOL bColumnLab)
{
	int nColumn;
	if (bColumnLab)
		nColumn = FindColumn(szColumn);
	else
		nColumn = Str2Col(szColumn);
	if (GetValue(nRow - 1, nColumn - 1, lpRString, dwSize))
		return TRUE;
	g_StrCpyLen(lpRString, lpDefault, dwSize);
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	GetString
// 功能:	取得某行某列字符串的值
// 参数:	szRow			行	（关键字）
//			szColomn		列	（关键字）
//			lpDefault		缺省值
//			lpRString		返回值
//			dwSize			返回字符串的最大长度
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetString(LPSTR szRow, LPSTR szColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize)
{
	int nRow, nColumn;

	nRow = FindRow(szRow);
	nColumn = FindColumn(szColumn);
	if (GetValue(nRow - 1, nColumn - 1, lpRString, dwSize))
		return TRUE;
	g_StrCpyLen(lpRString, lpDefault, dwSize);
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	GetString
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行		从1开始
//			nColomn			列		从1开始
//			lpDefault		缺省值
//			lpRString		返回值
//			dwSize			返回字符串的最大长度
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetString(int nRow, int nColumn, LPSTR lpDefault, LPSTR lpRString, DWORD dwSize)
{
	if (GetValue(nRow - 1, nColumn - 1,  lpRString, dwSize))
		return TRUE;
	g_StrCpyLen(lpRString, lpDefault, dwSize);
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	GetInteger
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行
//			szColomn		列
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetInteger(int nRow, LPSTR szColumn, int nDefault, int *pnValue, BOOL bColumnLab)
{
	char	Buffer[32];
	int		nColumn;
	if (bColumnLab)
		nColumn = FindColumn(szColumn);
	else
		nColumn = Str2Col(szColumn);
	if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer)))
	{
		*pnValue = atoi(Buffer);
		return TRUE;
	}
	else
	{
		*pnValue = nDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetInteger
// 功能:	取得某行某列字符串的值
// 参数:	szRow			行
//			szColomn		列
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetInteger(LPSTR szRow, LPSTR szColumn, int nDefault, int *pnValue)
{
	int		nRow, nColumn;
	char	Buffer[32];

	nRow = FindRow(szRow);
	nColumn = FindColumn(szColumn);
	if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer)))
	{
		*pnValue = atoi(Buffer);
		return TRUE;
	}
	else
	{
		*pnValue = nDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetInteger
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行		从1开始
//			nColomn			列		从1开始
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetInteger(int nRow, int nColumn, int nDefault, int *pnValue)
{
	char	Buffer[32];

	if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer)))
	{
		*pnValue = atoi(Buffer);
		return TRUE;
	}
	else
	{
		*pnValue = nDefault;
		return TRUE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetFloat
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行
//			szColomn		列
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetFloat(int nRow, LPSTR szColumn, float fDefault, float *pfValue, BOOL bColumnLab)
{
	char	Buffer[32];
	int		nColumn;
	if (bColumnLab)
		nColumn = FindColumn(szColumn);
	else
		nColumn = Str2Col(szColumn);
	if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer)))
	{
		*pfValue = (float)atof(Buffer);
		return TRUE;
	}
	else
	{
		*pfValue = fDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetFloat
// 功能:	取得某行某列字符串的值
// 参数:	szRow			行
//			szColomn		列
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetFloat(LPSTR szRow, LPSTR szColumn, float fDefault, float *pfValue)
{
	int		nRow, nColumn;
	char	Buffer[32];

	nRow = FindRow(szRow);
	nColumn = FindColumn(szColumn);
	if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer)))
	{
		*pfValue = (float)atof(Buffer);
		return TRUE;
	}
	else
	{
		*pfValue = fDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetFloat
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行		从1开始
//			nColomn			列		从1开始
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetFloat(int nRow, int nColumn, float fDefault, float *pfValue)
{
	char	Buffer[32];
	
	if (GetValue(nRow - 1, nColumn - 1, Buffer, sizeof(Buffer)))
	{
		*pfValue = (float)atof(Buffer);
		return TRUE;
	}
	else
	{
		*pfValue = fDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetValue
// 功能:	取得某行某列字符串的值
// 参数:	nRow			行
//			nColomn			列
//			lpDefault		缺省值
//			lpRString		返回值
//			dwSize			返回字符串的最大长度
// 返回:	是否成功
//---------------------------------------------------------------------------
BOOL KTabFile::GetValue(int nRow, int nColumn, LPSTR lpRString, DWORD dwSize)
{
	if (nRow >= m_Height || nColumn >= m_Width || nRow < 0 || nColumn < 0)
		return FALSE;

	TABOFFSET	*TempOffset;
	LPSTR		Buffer;

	Buffer = (LPSTR)m_Memory.GetMemPtr();
	TempOffset = (TABOFFSET *)m_OffsetTable.GetMemPtr();
	TempOffset += nRow * m_Width + nColumn;

	ZeroMemory(lpRString, dwSize);
	Buffer += TempOffset->dwOffset;
	if (TempOffset->dwLength == 0)
	{
		return FALSE;
	}
	if (dwSize > TempOffset->dwLength)
	{
		memcpy(lpRString, Buffer, TempOffset->dwLength);
		lpRString[TempOffset->dwLength] = 0;
	}
	else
	{
		memcpy(lpRString, Buffer, dwSize);
		lpRString[dwSize] = 0;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Clear
// 功能:	清除TAB文件的内容
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KTabFile::Clear()
{
	m_Memory.Free();
	m_OffsetTable.Free();
}
//---------------------------------------------------------------------------
// 函数:	FindRow
// 功能:	查找行关键字
// 参数:	szRow（行关键字）
// 返回:	int
//---------------------------------------------------------------------------
int KTabFile::FindRow(LPSTR szRow)
{
	char	szTemp[128];
	for (int i = 0; i < m_Height; i++)	// 从1开始，跳过第一行的字段行
	{
		GetValue(i, 0, szTemp, g_StrLen(szRow));
		if (g_StrCmp(szTemp, szRow))
			return i + 1; //改动此处为加一 by Romandou,即返回以1为起点的标号
	}
	return -1;
}
//---------------------------------------------------------------------------
// 函数:	FindColumn
// 功能:	查找列关键字
// 参数:	szColumn（行关键字）
// 返回:	int
//---------------------------------------------------------------------------
int KTabFile::FindColumn(LPSTR szColumn)
{
	char	szTemp[128];
	for (int i = 0; i < m_Width; i++)	// 从1开始，跳过第一列的字段行
	{
		GetValue(0, i, szTemp, g_StrLen(szColumn));
		if (g_StrCmp(szTemp, szColumn))
			return i + 1;//改动此处为加一 by Romandou,即返回以1为起点的标号
	}
	return -1;
}

//---------------------------------------------------------------------------
// 函数:	Col2Str
// 功能:	把整数转成字符串
// 参数:	szColumn
// 返回:	第几列
//---------------------------------------------------------------------------
void KTabFile::Col2Str(int nCol, LPSTR szColumn)
{

	if (nCol < 26)
	{
		szColumn[0] = 'A' + nCol;
		szColumn[1]	= 0;
	}
	else
	{
		szColumn[0] = 'A' + (nCol / 26 - 1);
		szColumn[1] = 'A' + nCol % 26;
		szColumn[2] = 0;
	}
}
