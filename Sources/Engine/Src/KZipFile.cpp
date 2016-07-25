//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipFile.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	File In Zip Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KFilePath.h"
#include "KZipList.h"
#include "KZipFile.h"
//---------------------------------------------------------------------------
// 文件读取模式 0 = 优先从磁盘读取 1 = 优先从文件包读取
static int m_nZipFileMode = 0;
//---------------------------------------------------------------------------
// 函数:	SetFileMode
// 功能:	设置文件读取模式
// 参数:	int
// 返回:	void
//---------------------------------------------------------------------------
void g_SetZipFileMode(int nFileMode)
{
	m_nZipFileMode = nFileMode;
}
//---------------------------------------------------------------------------
// 函数:	KZipFile
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KZipFile::KZipFile()
{
	m_nZipFile = -1;
}
//---------------------------------------------------------------------------
// 函数:	~KZipFile
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KZipFile::~KZipFile()
{
	Close();
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开文件, 先寻找当前目录下是否有同名的单独文件,
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KZipFile::Open(LPSTR FileName)
{
	BOOL bRet = FALSE;
	
	Close();

	if (m_nZipFileMode == 0)
	{
		bRet = m_DiskFile.Open(FileName);
	}
	else if (g_pZipList)
	{
		g_pZipList->Lock();
		bRet = OpenZip(FileName);
		g_pZipList->Unlock();
	}
	return bRet;
}
//---------------------------------------------------------------------------
// 函数:	Read
// 功能:	从文件中读取数据
// 参数:	pBuffer		缓冲区指针
//			dwSize		要读取的长度
// 返回:	读到的字节长度
//---------------------------------------------------------------------------
DWORD KZipFile::Read(PVOID pBuffer, DWORD dwSize)
{
	if (m_nZipFile < 0)
	{
		return m_DiskFile.Read(pBuffer, dwSize);
	}
	else
	{
		g_pZipList->Lock();
		dwSize = ReadZip(pBuffer, dwSize);
		g_pZipList->Unlock();
	}
	return dwSize;
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	文件读指针定位
// 参数:	pOffset			偏移量
//			dwMethod		定位方法
// 返回:	文件的指针
//---------------------------------------------------------------------------
DWORD KZipFile::Seek(LONG lOffset,DWORD dwMethod)
{
	if (m_nZipFile < 0)
	{
		return m_DiskFile.Seek(lOffset, dwMethod);
	}
	else
	{
		g_pZipList->Lock();
		lOffset = SeekZip(lOffset, dwMethod);
		g_pZipList->Unlock();
	}
	return lOffset;
}
//---------------------------------------------------------------------------
// 函数:	Tell
// 功能:	返回文件的指针
// 参数:	void
// 返回:	文件的指针
//---------------------------------------------------------------------------
DWORD KZipFile::Tell()
{
	if (m_nZipFile < 0)
		return m_DiskFile.Tell();
	return m_FilePointer;
}
//---------------------------------------------------------------------------
// 函数:	Size
// 功能:	返回文件大小
// 参数:	void
// 返回:	文件的大小 in bytes
//---------------------------------------------------------------------------
DWORD KZipFile::Size()
{
	if (m_nZipFile < 0)
		return m_DiskFile.Size();
	return m_FileSize;
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭一个文件
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KZipFile::Close()
{
	m_FileBuff.Free();
	m_nZipFile = -1;
}
//---------------------------------------------------------------------------
// 函数:	OpenZip
// 功能:	打开文件, 先寻找当前目录下是否有同名的单独文件,
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KZipFile::OpenZip(LPSTR FileName)
{
	KMemClass Buffer;// 压缩源文件缓存

	// search file in zip pack
	m_nZipFile = g_pZipList->Search(FileName, &m_FileOffs, &m_FileSize);
	if (m_nZipFile < 0)
		return FALSE;

	// read pack file
	g_pZipList->Seek(m_FileOffs, FILE_BEGIN);
	LF	Header;
	g_pZipList->Read(&Header, sizeof(Header));
	m_PackSize = Header.lf_csize;
	g_pZipList->Seek(Header.lf_fn_len, FILE_CURRENT);
	g_pZipList->Seek(Header.lf_ef_len, FILE_CURRENT);

	// alloc pack file buffer memory
	if (!Buffer.Alloc(m_PackSize))
		return FALSE;
	PVOID pPackBuf = Buffer.GetMemPtr();

	// alloc unpack file buffer memory
	if (!m_FileBuff.Alloc(m_FileSize))
		return FALSE;
	PVOID pFileBuf = m_FileBuff.GetMemPtr();

	// read pack file data
	g_pZipList->Read(pPackBuf, m_PackSize);
	
	// decode zip file
	g_pZipList->Decode((PBYTE)pPackBuf, (PBYTE)pFileBuf, &Header);

	// reset file pointer
	m_FilePointer = 0;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	ReadZip
// 功能:	从文件中读取数据
// 参数:	pBuffer		缓冲区指针
//			dwSize		要读取的长度
// 返回:	读到的字节长度
//---------------------------------------------------------------------------
DWORD KZipFile::ReadZip(PVOID pBuffer, DWORD dwSize)
{
	if (dwSize + m_FilePointer > m_FileSize)
	{
		dwSize = m_FileSize - m_FilePointer;
	}

	if (dwSize == 0)
		return 0;

	PBYTE pFileBuf = (PBYTE)m_FileBuff.GetMemPtr();
	pFileBuf += m_FilePointer;
	g_MemCopy(pBuffer, pFileBuf, dwSize);
	m_FilePointer += dwSize;

	return dwSize;
}
//---------------------------------------------------------------------------
// 函数:	SeekZip
// 功能:	文件读指针定位
// 参数:	pOffset			偏移量
//			dwMethod		定位方法
// 返回:	文件的指针
//---------------------------------------------------------------------------
DWORD KZipFile::SeekZip(LONG lOffset,DWORD dwMethod)
{
	switch (dwMethod)
	{
	case FILE_BEGIN:
		m_FilePointer = lOffset;
		break;
		
	case FILE_END:
		m_FilePointer = m_FileSize + lOffset - 1;
		break;
		
	case FILE_CURRENT:
		m_FilePointer = m_FilePointer + lOffset;
		break;
	}
	
	if (m_FilePointer < 0)
	{
		m_FilePointer = 0;
	}
	if (m_FilePointer >= m_FileSize)
	{
		m_FilePointer = m_FileSize - 1;
	}
	
	return m_FilePointer;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KZipFile::Save(LPSTR FileName)
{
	if (m_nZipFile < 0)
		return TRUE;
	if (!m_DiskFile.Create(FileName))
		return FALSE;
	m_DiskFile.Write(m_FileBuff.GetMemPtr(), m_FileSize);
	m_DiskFile.Close();
	return TRUE;
}
//---------------------------------------------------------------------------
