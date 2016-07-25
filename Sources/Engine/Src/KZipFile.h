//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipFile.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KZipFile_H
#define KZipFile_H
//---------------------------------------------------------------------------
#include "KFile.h"
#include "KMemClass.h"
//---------------------------------------------------------------------------
class ENGINE_API KZipFile
{
private:
	KFile		m_DiskFile;		// 磁盘文件(不在包中)
	KMemClass	m_FileBuff;		// 解压后文件缓存
	int			m_nZipFile;		// 是否存在于包中
	DWORD		m_FileOffs;		// 在ZIP文件中的徧移
	DWORD		m_FileSize;		// 没压缩前的文件长度
	DWORD		m_PackSize;		// 压缩文件长度
	DWORD		m_FilePointer;	// 文件指针的位置(解码后)
	BOOL		OpenZip(LPSTR FileName);
	DWORD		ReadZip(PVOID pBuffer, DWORD dwSize);
	DWORD		SeekZip(LONG lOffset, DWORD dwMethod);

public:
	KZipFile();
	~KZipFile();
	BOOL		Open(LPSTR FileName);
	void		Close();
	DWORD		Read(PVOID pBuffer, DWORD dwSize);
	DWORD		Seek(LONG lOffset, DWORD dwMethod);
	DWORD		Tell();
	DWORD		Size();
	BOOL		Save(LPSTR FileName);
};
//---------------------------------------------------------------------------
ENGINE_API void g_SetZipFileMode(int nFileMode);
//---------------------------------------------------------------------------
#endif
