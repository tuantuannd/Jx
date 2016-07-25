//////////////////////////////////////////////////////////////////////////////////////
//	FileName		:	CFile.cpp
//	FileAuthor		:	zroc
//	FileCreateDate	:
//	FileDescription	:	File operation class
//
//////////////////////////////////////////////////////////////////////////////////////
#include <stdafx.h>
#include "KLCFile.h"
//---------------------------------------------------------------------------
//  Function:   Constructor
//	Comment:	构造函数
//---------------------------------------------------------------------------
KLCFile::KLCFile()
{
	hFile = INVALID_HANDLE_VALUE;
	nSize = 0;
	dwPos = 0;
}
//---------------------------------------------------------------------------
//  Function:   Destructor
//	Comment:	析构函数
//---------------------------------------------------------------------------
KLCFile::~KLCFile()
{
	Close();
}
//---------------------------------------------------------------------------
//  Function:   Open exist file
//  Commemt:	打开一个文件，准备读操作
//---------------------------------------------------------------------------
BOOL KLCFile::Open(LPCSTR FileName)
{
	char PathName[MAX_PATH];

	strcpy(PathName,FileName);
	// close prior file handle
	if (hFile != INVALID_HANDLE_VALUE)
		Close();
	// Open the file for read
	hFile = CreateFile(
					PathName,		// pointer to name of the file with path
					GENERIC_READ,	// access (read-write) mode
					FILE_SHARE_READ,// share mode
					NULL,			// pointer to security attributes
					OPEN_EXISTING,	// how to create
					FILE_ATTRIBUTE_NORMAL,// file attributes
					NULL);			// template file
	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	// get the open file size
	nSize = GetFileSize(hFile, NULL);
	// return true
	return TRUE;
}
//---------------------------------------------------------------------------
//  Function:   Create new file
//  Comment:	创建一个新文件，准备写操作
//---------------------------------------------------------------------------
BOOL KLCFile::Create(LPCSTR FileName)
{
	char PathName[MAX_PATH];

	strcpy(PathName,FileName);
	// close prior file handle
	if (hFile != INVALID_HANDLE_VALUE)
		Close();
	// create file for write
	hFile = CreateFile(
					PathName,		// pointer to name of the file with path
					GENERIC_WRITE,	// access (read-write) mode
					FILE_SHARE_READ,// share mode
					NULL,			// pointer to security attributes
					CREATE_ALWAYS,	// create or over write
					FILE_ATTRIBUTE_NORMAL, // file attributes
					NULL);			// template file
	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	// return true
	return TRUE;
}
/*******************************************************
// FUNCTION	: CzFileOperate::Eof
// PURPOSE	: 
// RETURN	: BOOL  TRUE End of File
// COMMENTS	:
*******************************************************/
BOOL KLCFile::Eof()
{
	if (dwPos >= nSize)
		return TRUE;
	return FALSE;
}
//---------------------------------------------------------------------------
//  Function:   Close file
//  Comment:	关闭已经打开的文件
//---------------------------------------------------------------------------
BOOL KLCFile::Close()
{
	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	// close file handle
	CloseHandle(hFile);
	// reset file param
	hFile = INVALID_HANDLE_VALUE;
	nSize = 0;
	dwPos = 0;
	// return ture
	return TRUE;
}
//---------------------------------------------------------------------------
//  Function:   Read data from file
//  Return:		success : number of bytes read from file
//              fail    : 0
//  Comment:	读取文件数据
//---------------------------------------------------------------------------
DWORD KLCFile::Read(LPVOID lpBuffer,DWORD nReadBytes)
{
	DWORD BytesRead;

	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;
	// read data form file
	ReadFile(hFile, lpBuffer, nReadBytes, &BytesRead, NULL);
	// move file pointer
	dwPos += BytesRead;
	// return bytes readed
	return BytesRead;
}
//---------------------------------------------------------------------------
//  Function:   Write data to file
//  Return:		success : number of bytes write to file
//              fail    : 0
//  Comment:	写入文件数据
//---------------------------------------------------------------------------
DWORD KLCFile::Write(LPVOID lpBuffer,DWORD nWriteBytes)
{
	DWORD BytesWrite;

	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;
	// write data to file
	WriteFile(hFile, lpBuffer, nWriteBytes, &BytesWrite, NULL);
	// move file pointer
	dwPos += BytesWrite;
	// return bytes writed
	return BytesWrite;
}
//---------------------------------------------------------------------------
//  Function:   Move file pointer
//  Params:		Distance = file offset
//              MoveMethod = FILE_BEGIN
//                           FILE_CURRENT
//                           FILE_END
//	Return:		success : pointer of file
//				fail    : 0xffffffff
//  Comment:	移动文件指针
//---------------------------------------------------------------------------
DWORD KLCFile::Seek(LONG Distance, DWORD MoveMethod)
{
	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return 0xffffffff;
	// move file pointer
	dwPos = SetFilePointer(hFile, Distance, NULL, MoveMethod);
	// return file pointer position
	return dwPos;
}
//---------------------------------------------------------------------------
//  Function:   Get file pointer pos
//	Return:		success : low dword of file size
//				fail    : 0xffffffff
//  Comment:	取得文件指针位置
//---------------------------------------------------------------------------
DWORD KLCFile::Tell()
{
	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return 0xffffffff;
	// return file size
	return dwPos;
}
//---------------------------------------------------------------------------
//  Function:   Get file size (number of bytes)
//	Return:		success : low dword of file size
//				fail    : 0xffffffff
//  Comment:	取得文件的大小
//---------------------------------------------------------------------------
DWORD KLCFile::Size()
{
	// check file handle
	if (hFile == INVALID_HANDLE_VALUE)
		return 0xffffffff;
	// return file size
	return nSize;
}
//---------------------------------------------------------------------------

