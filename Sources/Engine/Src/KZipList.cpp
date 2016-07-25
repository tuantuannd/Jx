//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipList.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Pack Data List Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"
#include "KIniFile.h"
#include "KZipList.h"
//---------------------------------------------------------------------------
ENGINE_API KZipList* g_pZipList = NULL;
//---------------------------------------------------------------------------
// 函数:	KZipList
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KZipList::KZipList()
{
	g_pZipList = this;
	m_nNumber  = 0;
	m_nActive  = 0;
}
//---------------------------------------------------------------------------
// 函数:	~KZipList
// 功能:	分造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KZipList::~KZipList()
{
	m_nNumber = 0;
	m_nActive = 0;
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开一个文件包
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KZipList::Open(LPSTR FileName)
{
	KIniFile IniFile;
	char Section[16] = "Package";
	char Key[16];
	char PakPath[32];
	char PakFile[32];
	
	if (!IniFile.Load(FileName))
		return FALSE;
	if (!IniFile.GetString(Section, "Path", "", PakPath, 32))
		return FALSE;
	g_SetFilePath(PakPath);
	Close();
	while (m_nNumber < MAX_PAK)
	{
		sprintf(Key, "%d", m_nNumber);
		if (!IniFile.GetString(Section, Key, "", PakFile, 32))
			break;
		if (!m_ZipFile[m_nNumber].Open(PakFile))
			g_DebugLog("PakList Open : %s ... Fail", PakFile);
		m_nNumber++;
		g_DebugLog("PakList Open : %s ... Ok", PakFile);
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭所有文件
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipList::Close()
{
	for (int i = 0; i < m_nNumber; i++)
		m_ZipFile[i].Close();
	m_nNumber = 0;
	m_nActive = 0;
}
//---------------------------------------------------------------------------
// 函数:	Read
// 功能:	读取文件数据
// 参数:	pBuffer		缓存
//			dwLen		长度
// 返回:	读取长度
//---------------------------------------------------------------------------
DWORD KZipList::Read(PVOID pBuffer, DWORD dwLen)
{
	return m_ZipFile[m_nActive].Read(pBuffer, dwLen);
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	文件指针定位
// 参数:	Offset		偏移
//			Method		方法
// 返回:	文件指针位置
//---------------------------------------------------------------------------
DWORD KZipList::Seek(LONG lOffset, DWORD Method)
{
	return m_ZipFile[m_nActive].Seek(lOffset, Method);
}
//---------------------------------------------------------------------------
// 函数:	Tell
// 功能:	取得文件指针位置
// 参数:	void
// 返回:	文件指针位置
//---------------------------------------------------------------------------
DWORD KZipList::Tell()
{
	return m_ZipFile[m_nActive].Tell();
}
//---------------------------------------------------------------------------
// 函数:	Search
// 功能:	在所有包中扫描制定文件
// 参数:	FileName	文件名
//			pOffset		文件偏移
//			pLen		文件大小
// 返回:	int			zipfile index
//---------------------------------------------------------------------------
int KZipList::Search(LPSTR FileName, PDWORD pOffset, PDWORD pLen)
{
	// if no pack file in list
	if (m_nNumber == 0)
		return -1;

	// search in all package files
	for (int i = 0; i < m_nNumber; i++)
	{
		if (m_ZipFile[i].Search(FileName, pOffset, pLen))
		{
			m_nActive = i;
			return m_nActive;
		}
	}
	return -1;
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	解压缩
// 参数:	pIn			压缩数据指针
//			pOut		解压数据指针
//			pLf			Local File header
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KZipList::Decode(
	PBYTE	pIn,		// 压缩数据指针
	PBYTE	pOut,		// 解压数据指针
	LF*		pLf			// Local Header ptr
	)
{
	return m_ZipCodec.Decode(pIn, pOut, pLf);
}
//---------------------------------------------------------------------------

