//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakData.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KPakData_H
#define KPakData_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KFile.h"
#include "KCodec.h"
//---------------------------------------------------------------------------
// pack file header
typedef struct {
	BYTE		Signature[8];		// 文件标识 ("PACKAGE")
	DWORD		FilesInPack;		// 包中的文件总数
	DWORD		CompressMethod;		// 使用的压缩算法
} TPakFileHeader;

// pack file index 
typedef struct {
	DWORD		FileId;				// 32bit Id值
	DWORD		FileOffset;			// 文件在包中的偏移
	DWORD		FileLength;			// 文件的原始长度
} TPakFileIndex;
//---------------------------------------------------------------------------
class ENGINE_API KPakData
{
private:
	KFile		m_File;				// PAK文件
	KMemClass	m_MemIndex;			// 索引内存
	KCodec*		m_pCodec;			// 解码器
	DWORD		m_nFilesInPack;		// 包中的文件总数
	DWORD		m_nCompressMethod;	// 压缩算法

public:
	KPakData();
	~KPakData();
	BOOL		Open(LPSTR FileName);
	DWORD		Read(LPVOID lpBuffer, DWORD dwLength);
	DWORD		Seek(LONG lOffset, DWORD dwMethod);
	DWORD		Tell();
	void		Close();
	BOOL		Search(LPSTR pFileName, PDWORD pOffset, PDWORD pLen);
	BOOL		Decode(TCodeInfo* pCodeInfo);
};
//---------------------------------------------------------------------------
#endif
