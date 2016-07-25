//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipData.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Zip File Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"
#include "KZipData.h"
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开一个ZIP文件
// 参数:	lpZipFileName	Zip文件名
// 返回:	TRUE			Success
//			FALSE			Fail
//---------------------------------------------------------------------------
BOOL KZipData::Open(LPSTR lpZipFileName)
{
	ZIPHeader		Head;
	ZIPEnd			End;
	ZIPCtrlHeader	FileInZip;
	ZIPFileInfo		FileInfo;
	char			FileName[MAX_PATH];
	DWORD			dwHashCode;
	int				i, n;

	// Close opened zip file
	Close();
	
	// Open zip file
	if (!m_ZipFile.Open(lpZipFileName))
		return FALSE;

	// Read the file header
	m_ZipFile.Read(&Head, sizeof(Head));
	if (Head.Signature != ZIPHeaderSig)
		return FALSE;

	// Read the final header
	m_ZipFile.Seek(-22, FILE_END);
	m_ZipFile.Read(&End, sizeof(End));
	if (End.Signature != ZIPEndSig)
		return FALSE;

	// Init zip file list
	if (!m_FileList.Init(sizeof(ZIPFileInfo), End.FilesOnDisk))
		return FALSE;
	
	// Read the header of each file
	m_ZipFile.Seek(End.Offset, FILE_BEGIN);
	for (n = 0; n < End.FilesOnDisk; n++)
	{
		m_ZipFile.Read(&FileInZip, sizeof(FileInZip));
		if (FileInZip.Signature != ZIPCtrlHeaderSig)
			return FALSE;
		FileInfo.UnpackSize = FileInZip.UnCompressedSize;
		FileInfo.OffsetInZip = FileInZip.Offset;
		// Read filename
		m_ZipFile.Read(FileName, FileInZip.FileNameLength);
		// Set filename tailer
		FileName[FileInZip.FileNameLength] = 0;
		// Convert all '/' to '\'
		for (i = 0; FileName[i]; i++)
		{
			if (FileName[i] == '/')
				FileName[i] = '\\';
		}
		g_StrLower(FileName);
		//g_StrCpy(FileInfo.FileName, FileName);
		dwHashCode = g_FileName2Id(FileName);
		m_FileList.Insert(dwHashCode, &FileInfo);
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭打开的ZIP文件
// 参数:	void
// 返回:	TRUE			Success
//			FALSE			Fail
//---------------------------------------------------------------------------
void KZipData::Close()
{
	m_FileList.Free();
	m_ZipFile.Close();
}
//---------------------------------------------------------------------------
// 函数:	Read
// 功能:	读文件数据
// 参数:	lpBuffer		读取的数据缓存
//			dwLength		读取的字节长度
// 返回:	实际读取的字节数
//---------------------------------------------------------------------------
DWORD KZipData::Read(LPVOID lpBuffer, DWORD dwLength)
{
	return m_ZipFile.Read(lpBuffer, dwLength);
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	移动文件指针位置
// 参数:	Offset			偏移量
//			Method			基准位置
// 返回:	实际移动到的位置
//---------------------------------------------------------------------------
DWORD KZipData::Seek(LONG lOffset,DWORD dwMethod)
{
	return m_ZipFile.Seek(lOffset, dwMethod);
}
//---------------------------------------------------------------------------
// 函数:	Tell
// 功能:	取得文件指针位置
// 参数:	void
// 返回:	实际文件指针的位置
//---------------------------------------------------------------------------
DWORD KZipData::Tell()
{
	return m_ZipFile.Tell();
}
//---------------------------------------------------------------------------
// 函数:	Search
// 功能:	在包中寻找一个文件
// 参数:	pFileName	要找的文件名
//			pOffset		找到文件的偏移
//			pLen		找到文件的长度
// 返回:	TRUE		找到了
//			FALSE		没找到
//---------------------------------------------------------------------------
BOOL KZipData::Search(LPSTR pFileName, PDWORD pOffset, PDWORD pLen)
{
	ZIPFileInfo FileInfo;
	DWORD dwHashCode;
	char szPathName[MAXPATH];

	g_GetPackPath(szPathName, pFileName);
	dwHashCode = g_FileName2Id(szPathName);
	if (!m_FileList.Search(dwHashCode, &FileInfo))
		return FALSE;
	*pOffset = FileInfo.OffsetInZip;
	*pLen = FileInfo.UnpackSize;
	return TRUE;
}
//---------------------------------------------------------------------------
