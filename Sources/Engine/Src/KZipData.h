//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipData.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KZipData_H
#define KZipData_H
//---------------------------------------------------------------------------
#include "KFile.h"
#include "KMemClass.h"
#include "KSortList.h"
//---------------------------------------------------------------------------
#pragma pack(push, before_ZIPStructs)
#pragma pack(1)

#define ZIPHeaderSig		0x04034b50
#define ZIPCtrlHeaderSig	0x02014b50
#define ZIPEndSig			0x06054b50

typedef struct  {
	LONG	Signature;			// (0x04034b50)
	WORD	Version;
	WORD	Flags;
	WORD	Method;
	LONG	LastMod;
	LONG	CRC32;
	LONG	CompressedSize;
	LONG	UnCompressedSize;
	WORD	FileNameLength;
	WORD	ExtraLength;
} ZIPHeader;

typedef struct  {
	LONG	Signature;			// (0x02014b50)
	WORD	VersionMade;
	WORD	VersionNeeded;
	WORD	Flags;
	WORD	Method;
	LONG	LastMod;
	LONG	CRC32;
	LONG	CompressedSize;
	LONG	UnCompressedSize;
	WORD	FileNameLength;
	WORD	ExtraLength;
	WORD	CommentLength;
	WORD	StartDisk;
	WORD	IniternalAttribs;
	LONG	ExternalAttribs;
	LONG	Offset;
} ZIPCtrlHeader;

typedef struct  {
	LONG	Signature;			// (0x06054b50)
	WORD	DiskNumber;			// number of this disk
	WORD	StartDiskNumber;	// start of the central directory
	WORD	FilesOnDisk;		// total number of files in central dir
	WORD	Files;				// total number of entries in the central dir
	LONG	Size;				// size of the central directory
	LONG	Offset;				// offset of start of central directory
	WORD	CommentLength;		// zipfile comment length
} ZIPEnd;

typedef struct {
	ULONG	UnpackSize;
	ULONG	OffsetInZip;
	//char	FileName[128];
} ZIPFileInfo;

#pragma pack(pop, before_ZIPStructs)
//---------------------------------------------------------------------------
class ENGINE_API KZipData
{
private:
	KFile		m_ZipFile;
	KSortList	m_FileList;
	int			m_nFileNum;

public:
	BOOL		Open(LPSTR lpZipFileName);
	void		Close();
	DWORD		Read(LPVOID lpBuffer, DWORD dwLength);
	DWORD		Seek(LONG lOffset, DWORD dwMethod);
	DWORD		Tell();
	BOOL		Search(LPSTR pFileName,PDWORD pOffset,PDWORD pLen);
};
//---------------------------------------------------------------------------
#endif // KZipData_H
