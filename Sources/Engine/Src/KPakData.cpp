//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakData.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	DataFile class used in package
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KFilePath.h"
#include "KPakData.h"
//---------------------------------------------------------------------------
// 函数:	KPakData()
// 功能:	构造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KPakData::KPakData()
{
	m_pCodec = NULL;
	m_nFilesInPack = 0;
	m_nCompressMethod = CODEC_NONE;
}
//---------------------------------------------------------------------------
// 函数:	~KPakData()
// 功能:	析构函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KPakData::~KPakData()
{
	Close();
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开一个Pak文件包
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPakData::Open(LPSTR FileName)
{
	TPakFileHeader	Header;

	// colse opened file
	if (m_nFilesInPack)
		Close();

	// open data file
	if (!m_File.Open(FileName))
		return FALSE;

	// read file header
	m_File.Read(&Header, sizeof(Header));

	// check file signature
	if (!g_MemComp(Header.Signature, "PACKAGE", 7))
		return FALSE;

	// get file number in pack
	m_nFilesInPack = Header.FilesInPack;

	// get compress method
	m_nCompressMethod = Header.CompressMethod;

	// alloc index table memory
	if (!m_MemIndex.Alloc(m_nFilesInPack * sizeof(TPakFileIndex)))
		return FALSE;

	// read index table
	m_File.Read(m_MemIndex.GetMemPtr(),
		m_nFilesInPack * sizeof(TPakFileIndex));
	
	// init decoder
	g_InitCodec(&m_pCodec, m_nCompressMethod);

	// return true
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Read
// 功能:	读文件数据
// 参数:	lpBuffer		读取的数据缓存
//			dwLength		读取的字节长度
// 返回:	实际读取的字节数
//---------------------------------------------------------------------------
DWORD KPakData::Read(LPVOID lpBuffer, DWORD dwLength)
{
	return m_File.Read(lpBuffer, dwLength);
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	移动文件指针位置
// 参数:	pOffset			偏移量
//			dwMethod		基准位置
// 返回:	实际移动到的位置
//---------------------------------------------------------------------------
DWORD KPakData::Seek(LONG lOffset, DWORD dwMethod)
{
	return m_File.Seek(lOffset, dwMethod);
}
//---------------------------------------------------------------------------
// 函数:	Tell
// 功能:	取得文件指针位置
// 参数:	空
// 返回:	实际文件指针的位置
//---------------------------------------------------------------------------
DWORD KPakData::Tell()
{
	return m_File.Tell();
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭一个数据文件
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KPakData::Close()
{
	g_FreeCodec(&m_pCodec, m_nCompressMethod);
	m_File.Close();
	m_MemIndex.Free();
	m_nFilesInPack = 0;
	m_nCompressMethod = 0;
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
BOOL KPakData::Search(LPSTR pFileName, PDWORD pOffset, PDWORD pLen)
{
	char	szPathName[MAXPATH];
	DWORD	dwFileId;
	int		nBegin;
	int		nEnd;
	int		nMid;

	// check file number
	if (m_nFilesInPack <= 0)
		return FALSE;

	// get file index table pointer
	TPakFileIndex*	pIndex;
	pIndex = (TPakFileIndex*)m_MemIndex.GetMemPtr();

	// get pack file name with path
	g_GetPackPath(szPathName, pFileName);

	// get file id 
	dwFileId = g_FileName2Id(szPathName);

	// 用二分法查找匹配的文件
	nBegin = 0;
	nEnd = m_nFilesInPack - 1;
	while (nBegin <= nEnd)
	{
		nMid = (nBegin + nEnd) / 2;
		if (dwFileId == pIndex[nMid].FileId)
		{
			// 找到了
			*pOffset = pIndex[nMid].FileOffset;
			*pLen = pIndex[nMid].FileLength;
			return TRUE;
		}
		if (dwFileId < pIndex[nMid].FileId)
			nEnd = nMid - 1;
		else
			nBegin = nMid + 1;
	}

	// 没找到
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	Decode
// 功能:	解压缩
// 参数:	pCodeInfo	压缩数据指针
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPakData::Decode(TCodeInfo* pCodeInfo)
{
	return m_pCodec->Decode(pCodeInfo);
}
//---------------------------------------------------------------------------

