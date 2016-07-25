//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KFile.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KFile_H
#define KFile_H
//---------------------------------------------------------------------------
#define SEEK_ERROR		0xFFFFFFFF
//---------------------------------------------------------------------------
#ifndef __linux
class ENGINE_API KFile
#else
class KFile
#endif
{
private:
//#ifndef __linux
//	HANDLE		m_hFile;	// File Handle
//#else
	FILE *		m_hFile;	// File Handle
//#endif
	DWORD		m_dwLen;	// File Size
	DWORD		m_dwPos;	// File Pointer
public:
	KFile();
	~KFile();
	BOOL		Open(LPSTR FileName);
	BOOL		Create(LPSTR FileName);
	BOOL		Append(LPSTR FileName);
	void		Close();
	DWORD		Read(LPVOID lpBuffer, DWORD dwReadBytes);
	DWORD		Write(LPVOID lpBuffer, DWORD dwWriteBytes);
	DWORD		Seek(LONG lDistance, DWORD dwMoveMethod);
	DWORD		Tell();
	DWORD		Size();
};
//---------------------------------------------------------------------------
#endif
