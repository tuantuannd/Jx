//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakFile.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	File In Dat Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KFilePath.h"
#include "KPakFile.h"
#ifndef _SERVER
	#include "KPakList.h"
#endif

//---------------------------------------------------------------------------
// 文件读取模式 0 = 优先从磁盘读取 1 = 优先从文件包读取
static int m_nPakFileMode = 0;

//---------------------------------------------------------------------------
// 函数:	SetFileMode
// 功能:	设置文件读取模式
// 参数:	int
// 返回:	void
//---------------------------------------------------------------------------
void g_SetPakFileMode(int nFileMode)
{
	m_nPakFileMode = nFileMode;
}

#define	PAK_INDEX_STORE_IN_RESERVED	0

#ifndef _SERVER

//----modify by Wooy to add Adjust color palette and to get rid of #@$%^& ----2003.8.19
SPRHEAD* SprGetHeader(const char* pszFileName, SPROFFS*& pOffsetTable)
{
	pOffsetTable = NULL;

	if(pszFileName == NULL || pszFileName[0] == 0)
		return NULL;

	KPakFile	File;
	if (!File.Open(pszFileName))
		return NULL;

	SPRHEAD*		pSpr = NULL;
	if (File.IsFileInPak())
	{
		//====到文件包内寻找读取图文件=====
		XPackElemFileRef	PakRef;
		//_ASSERT(g_pPakList);
		if (g_pPakList->FindElemFile(pszFileName, PakRef))
		{
			pSpr = g_pPakList->GetSprHeader(PakRef, pOffsetTable);
			if (pSpr)
				pSpr->Reserved[PAK_INDEX_STORE_IN_RESERVED] = (WORD)(short)PakRef.nPackIndex;
		}
	}
	else
	{
		bool			bOk = false;
		SPRHEAD			Header;
		//---读文件头，并判断是否为合法的spr图文件---
		while(File.Read(&Header, sizeof(SPRHEAD)) == sizeof(SPRHEAD))
		{
			if (*(int*)&Header.Comment[0] != SPR_COMMENT_FLAG || Header.Colors > 256)
				break;
			//---为输出缓冲区分配空间---
			unsigned int uEntireSize = File.Size();
			pSpr = (SPRHEAD*)malloc(uEntireSize);
			if (pSpr == NULL)
				break;

			uEntireSize -= sizeof(SPRHEAD);
			//---读取spr剩下的数据---
			if (File.Read(&pSpr[1], uEntireSize) == uEntireSize)
			{
				//----获得图形帧索引表的开始位置---
				pOffsetTable = (SPROFFS*)(((char*)(pSpr)) + sizeof(SPRHEAD) + Header.Colors * 3);
				Header.Reserved[PAK_INDEX_STORE_IN_RESERVED] = (WORD)(-1);
				memcpy(pSpr, &Header, sizeof(SPRHEAD));
				bOk = true;
			}
			break;
		};

		if (bOk == false && pSpr)
		{
			free (pSpr);
			pSpr = NULL;
		}
	}
	File.Close();
	return pSpr;
}

void SprReleaseHeader(SPRHEAD* pSprHeader)
{
    if (pSprHeader)
		free(pSprHeader);
}

SPRFRAME* SprGetFrame(SPRHEAD* pSprHeader, int nFrame)
{
	SPRFRAME*	pFrame = NULL;
	if (pSprHeader && g_pPakList)
	{
		int nPakIndex = (short)pSprHeader->Reserved[PAK_INDEX_STORE_IN_RESERVED];
		if (nPakIndex >= 0)
			pFrame = g_pPakList->GetSprFrame(nPakIndex, pSprHeader, nFrame);
	}
	return pFrame;
}

void SprReleaseFrame(SPRFRAME* pFrame)
{
    if (pFrame)
		free(pFrame);
}

#include "JpgLib.h"
#include "KDDraw.h"

KSGImageContent*	get_jpg_image(const char cszName[], unsigned uRGBMask16)
{
	KPakFile	File;
	unsigned char *pbyFileData = NULL;

	if (File.Open(cszName))
	{
		unsigned int uSize = File.Size();
		pbyFileData = (unsigned char *)malloc(uSize);
		if (pbyFileData)
		{
			if (File.Read(pbyFileData, uSize) != uSize)
			{
				free (pbyFileData);
				pbyFileData = NULL;
			}
		}
	}

	if (!pbyFileData)
        return NULL;

	int nResult = false;
    int nRetCode = false;
    KSGImageContent *pImageResult = NULL;

	BOOL		bRGB555;
	JPEG_INFO	JpegInfo;

    if (uRGBMask16 == ((unsigned)-1))
    {
    	bRGB555 = (g_pDirectDraw->GetRGBBitMask16() == RGB_555) ? TRUE : FALSE;
    }
    else
    {
        bRGB555 = (uRGBMask16 == RGB_555) ? TRUE : FALSE;
    }

    nRetCode = jpeg_decode_init(bRGB555, TRUE);
	if(!nRetCode)
        goto Exit0;
         
	nRetCode = jpeg_decode_info(pbyFileData, &JpegInfo);
    if (!nRetCode)
        goto Exit0;

	pImageResult = (KSGImageContent *)malloc(KSG_IMAGE_CONTENT_SIZE(JpegInfo.width, JpegInfo.height));
    if (!pImageResult)
        goto Exit0;

    pImageResult->nWidth = JpegInfo.width;
    pImageResult->nHeight = JpegInfo.height;

	nRetCode = jpeg_decode_data(pImageResult->Data, &JpegInfo);
    if (!nRetCode)
        goto Exit0;

    nResult = true;

Exit0:
	free (pbyFileData);
    if (!nResult && pImageResult)
	{
		free (pImageResult);
		pImageResult = NULL;
    }

	return pImageResult;
}


void release_image(KSGImageContent *pImage)
{
    if (pImage)
        free (pImage);
}

#endif

//---------------------------------------------------------------------------
// 功能:	购造函数
//---------------------------------------------------------------------------
KPakFile::KPakFile()
{
#ifndef _SERVER
	m_PackRef.nPackIndex = -1;
	m_PackRef.uId = 0;
#endif
}

//---------------------------------------------------------------------------
// 功能:	析造函数
//---------------------------------------------------------------------------
KPakFile::~KPakFile()
{
	Close();
}

//---------------------------------------------------------------------------
// 功能:	判断此文件是否从包中打开的
//---------------------------------------------------------------------------
bool KPakFile::IsFileInPak()
{
#ifndef _SERVER
	return (m_PackRef.nPackIndex >= 0 && m_PackRef.uId);
#else
	return false;
#endif
}

//---------------------------------------------------------------------------
// 功能:	打开一个文件, 先寻找当前目录下是否有同名的单独文件,
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPakFile::Open(const char* pszFileName)
{
	if (pszFileName == NULL || pszFileName[0] == 0)
		return false;

	bool bOk = false;
	Close();

#ifndef _SERVER
	if (m_nPakFileMode == 0)	//0=优先从磁盘读取
	{
#endif
		bOk = (m_File.Open((char*)pszFileName) != FALSE);
#ifndef _SERVER
		if (bOk == false && g_pPakList)
		{
			bOk = g_pPakList->FindElemFile(pszFileName, m_PackRef);
		}
	}
	else	//1=优先从文件包读取
	{
		if (g_pPakList)
			bOk = g_pPakList->FindElemFile(pszFileName, m_PackRef);
		if (bOk == false)
			bOk = (m_File.Open((char*)pszFileName) != FALSE);
	}
#endif
	return bOk;
}

//---------------------------------------------------------------------------
// 功能:	从文件中读取数据
// 参数:	pBuffer		缓冲区指针
//			dwSize		要读取的长度
// 返回:	读到的字节长度
//---------------------------------------------------------------------------
DWORD KPakFile::Read(void* pBuffer, unsigned int uSize)
{
#ifndef _SERVER
	if (m_PackRef.nPackIndex >= 0)
	{
		if (g_pPakList->ElemFileRead(m_PackRef, pBuffer, uSize) == false)
			uSize = 0;
	}
	else
#endif
	{
		uSize = m_File.Read(pBuffer, uSize);
	}
	return uSize;
}

//---------------------------------------------------------------------------
// 功能:	文件读指针定位
// 参数:	lOffset			偏移量
//			dwMethod		定位方法
// 返回:	文件的指针
//---------------------------------------------------------------------------
DWORD KPakFile::Seek(int nOffset, unsigned int uMethod)
{
#ifndef _SERVER
	if (m_PackRef.nPackIndex >= 0)
	{
		if (uMethod == FILE_BEGIN)
			m_PackRef.nOffset = nOffset;
		else if (uMethod == FILE_END)
			m_PackRef.nOffset = m_PackRef.nSize + nOffset;
		else
			m_PackRef.nOffset += nOffset;
		if (m_PackRef.nOffset > m_PackRef.nSize)
			m_PackRef.nOffset =  m_PackRef.nSize;
		else if (m_PackRef.nOffset < 0)
			m_PackRef.nOffset = 0;
		nOffset = m_PackRef.nOffset;
	}
	else
#endif
	{
		nOffset = m_File.Seek(nOffset, uMethod);
	}
	return nOffset;
}

//---------------------------------------------------------------------------
// 功能:	返回文件的指针
// 返回:	文件的指针
//---------------------------------------------------------------------------
DWORD KPakFile::Tell()
{
	int nOffset;
#ifndef _SERVER
	if (m_PackRef.nPackIndex >= 0)
		nOffset = m_PackRef.nOffset;
	else
#endif
		nOffset = m_File.Tell();
	return nOffset;
}

//---------------------------------------------------------------------------
// 功能:	返回文件大小
// 返回:	文件的大小 in bytes
//---------------------------------------------------------------------------
DWORD KPakFile::Size()
{
	unsigned int uSize;
#ifndef _SERVER
	if (m_PackRef.nPackIndex >= 0)
		uSize = m_PackRef.nSize;
	else
#endif
		uSize = m_File.Size();
	return uSize;
}
//---------------------------------------------------------------------------
// 功能:	关闭一个文件
//---------------------------------------------------------------------------
void KPakFile::Close()
{
#ifndef _SERVER
	if (m_PackRef.nPackIndex >= 0)
	{
		m_PackRef.nPackIndex = -1;
		m_PackRef.uId = 0;
	}
	else
#endif
	{
		m_File.Close();
	}
}


//---------------------------------------------------------------------------
// 每次读取数据块的大小
#define BLOCK_SIZE	(0x10000L)

//---------------------------------------------------------------------------
// 功能:	打开一个包中的文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
/*BOOL KPakFile::OpenPak(LPSTR FileName)
{
	if (g_pPakList == NULL)
		return FALSE;

	KAutoMutex	AutoMutex(g_pPakList->GetMutexPtr());

	// 在所有文件包中查找要打开的文件是否存在
	m_nPackage = g_pPakList->Search(FileName, &m_dwFileOfs, &m_dwFileLen);
	if (m_nPackage < 0)
		return FALSE;
	
	// m_nBlocks 即块的个数, 源文件每64K打为一个包
	// m_nBlocks * 2 为块长度表的大小(每块的大小用一个WORD记录)
	m_nBlocks = (m_dwFileLen + 0xffff) >> 16;
	
	// 给 block buffer 分配内存
	if (!m_MemBlock.Alloc(m_nBlocks * 2))
		return FALSE;
	
	// 给 file buffer 分配内存64K, 为解压做准备
	if (!m_MemFile.Alloc(BLOCK_SIZE))
		return FALSE;
	
	// 给 read buffer 分配内存64K, 为解压做准备
	if (!m_MemRead.Alloc(BLOCK_SIZE))
		return FALSE;
	
	// 文件缓冲区指针
	m_pBuffer = (PBYTE)m_MemFile.GetMemPtr();
	
	// 每块的长度表
	m_pBlocks = (PWORD)m_MemBlock.GetMemPtr();
	
	// 移动到文件开始
	g_pPakList->Seek(m_dwFileOfs, FILE_BEGIN);
	
	// 读入每块的大小
	g_pPakList->Read(m_pBlocks, m_nBlocks * 2);
	
	// 第一块压缩数据的偏移量
	m_dwFileOfs = m_dwFileOfs + m_nBlocks * 2;
	
	// 读取压缩数据起始位置
	m_dwDataPtr = m_dwFileOfs;
	
	// 读指针的位置(解码后的位置) = 0;
	m_dwFilePtr = 0;
	
	// 成功打开文件
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	ReadPak
// 功能:	从文件中读取数据
// 参数:	pBuffer		缓冲区指针
//			dwSize		要读取的长度
// 返回:	读到的字节长度
//---------------------------------------------------------------------------
DWORD KPakFile::ReadPak(PVOID pBuffer, DWORD dwSize)
{
	KAutoMutex AutoMutex(g_pPakList->GetMutexPtr());

	// 如果是包中文件就从包中读
	UINT	nBlock = 0;
	DWORD	dwReadSize = 0;
	DWORD	dwBlockPos = 0;
	PBYTE	pOutBuf = (PBYTE)pBuffer;
	
	// 如果读取长度大于剩余文件长度
	if (m_dwFilePtr + dwSize > m_dwFileLen)
	{
		dwSize =  m_dwFileLen - m_dwFilePtr;
		dwReadSize = dwSize;
	}
	else
	{
		dwReadSize = dwSize;
	}

	// 已经读入的块数
	nBlock = m_dwFilePtr >> 16;

	// 已经读入小于64K块的长度
	dwBlockPos = m_dwFilePtr & 0xffff;

	// 以前读过小于64K的数据
	if (dwBlockPos)
	{
		// 要读的数据长度小于64K
		if (dwBlockPos + dwSize <= BLOCK_SIZE)
		{
			// m_pBuffer为64K大小, 上次已读入了
			g_MemCopyMmx(pOutBuf, m_pBuffer + dwBlockPos, dwSize);
			m_dwFilePtr += dwSize;

			// 如果此时 m_dwFilePtr 为64K 的倍数
			if ((m_dwFilePtr & 0xffff) == 0)
				m_dwDataPtr += (m_pBlocks[nBlock] == 0)? BLOCK_SIZE : m_pBlocks[nBlock];

			return dwSize;
		}
		
		// 要读的数据长度大于64K
		g_MemCopyMmx(pOutBuf, m_pBuffer + dwBlockPos, BLOCK_SIZE - dwBlockPos);
		pOutBuf += BLOCK_SIZE - dwBlockPos;
		m_dwDataPtr += (m_pBlocks[nBlock] == 0)? BLOCK_SIZE : m_pBlocks[nBlock];
		m_dwFilePtr = (++nBlock) << 16;
		dwSize -= (BLOCK_SIZE - dwBlockPos);
	}

	// 读入其余部分
	while (dwSize > 0xffff) // 大于64K
	{
		ReadBlock(pOutBuf, nBlock);
		pOutBuf += BLOCK_SIZE;
		m_dwDataPtr += (m_pBlocks[nBlock] == 0)? BLOCK_SIZE : m_pBlocks[nBlock];
		m_dwFilePtr = (++nBlock) << 16;
		dwSize -= BLOCK_SIZE;
	}

	// 刚好读完则返回
	if (dwSize == 0)
	{
		return dwReadSize;
	}

	// 读一个64K数据块到缓冲区
	ReadBlock(m_pBuffer, nBlock);

	// 拷贝缓冲区数据到目标地址
	g_MemCopyMmx(pOutBuf, m_pBuffer, dwSize);

	// 调整文件指针
	m_dwFilePtr += dwSize;

	// 返回读取的字节长度
	return dwReadSize;
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	文件读指针定位
// 参数:	lOffset			偏移量
//			dwMethod		定位方法
// 返回:	文件的指针
//---------------------------------------------------------------------------
DWORD KPakFile::SeekPak(long lOffset, DWORD dwMethod)
{
	KAutoMutex AutoMutex(g_pPakList->GetMutexPtr());

	if (m_nPackage < 0)
	{
		return m_File.Seek(lOffset, dwMethod);
	}

	int	nFilePtr = m_dwFilePtr;

	switch (dwMethod)
	{
	case FILE_BEGIN:
		nFilePtr = lOffset;
		break;

	case FILE_END:
		nFilePtr = m_dwFileLen + lOffset;
		break;

	case FILE_CURRENT:
		nFilePtr = m_dwFilePtr + lOffset;
		break;
	}

	if (nFilePtr < 0)
	{
		nFilePtr = 0;
	}
	else if (nFilePtr > (int)m_dwFileLen)
	{
		nFilePtr = m_dwFileLen;
	}

	m_dwFilePtr = nFilePtr;
	m_dwDataPtr = m_dwFileOfs;

	int nBlocks = nFilePtr >> 16;
	for (int i = 0; i < nBlocks; i++)
	{
		m_dwDataPtr += (m_pBlocks[i] == 0)? BLOCK_SIZE : m_pBlocks[i];
	}
	if (nFilePtr & 0xffff)
	{
		ReadBlock(m_pBuffer, nBlocks);
	}

	return m_dwFilePtr;
}

//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
*/
BOOL KPakFile::Save(const char* pszFileName)
{
/*	if (m_nPackage < 0)
		return TRUE;

	if (!m_File.Create(pszFileName))
		return FALSE;

	DWORD dwSize = m_dwFileLen;
	int nBlock = 0;

	// set data ptr
	m_dwDataPtr = m_dwFileOfs;

	// read blocks and write to file
	while (dwSize > BLOCK_SIZE)
	{
		ReadBlock(m_pBuffer, nBlock);
		dwSize -= BLOCK_SIZE;
		m_File.Write(m_pBuffer, BLOCK_SIZE);
		m_dwDataPtr += (m_pBlocks[nBlock] == 0)? BLOCK_SIZE : m_pBlocks[nBlock];
		nBlock++;
	}

	// read last block and write to file
	ReadBlock(m_pBuffer, nBlock);
	m_File.Write(m_pBuffer, dwSize);*/

	return FALSE;
}

/*
//---------------------------------------------------------------------------
// 函数:	ReadBlock
// 功能:	读一个压缩数据块
// 参数:	pBuffer		缓冲区指针
//			nBlock		块索引
// 返回:	void
//---------------------------------------------------------------------------
void KPakFile::ReadBlock(PBYTE pBuffer, int nBlock)
{
	TCodeInfo	CodeInfo;
	
	// 设置当前使用的文件包
	g_pPakList->SetActivePak(m_nPackage);
	
	// 填充解压缩接口结构
	CodeInfo.lpPack = (PBYTE)m_MemRead.GetMemPtr();
	CodeInfo.dwPackLen = m_pBlocks[nBlock];
	CodeInfo.lpData = pBuffer;
	CodeInfo.dwDataLen = BLOCK_SIZE;
	
	// 检查是否压缩过
	if (CodeInfo.dwPackLen == 0) // 没有压缩
	{
		g_pPakList->Seek(m_dwDataPtr, FILE_BEGIN);
		g_pPakList->Read(CodeInfo.lpData, CodeInfo.dwDataLen);
		return;
	}
	
	// 最后一块的实际长度（只有LHA用）
	if (nBlock == (m_nBlocks - 1))
	{
		CodeInfo.dwDataLen = m_dwFileLen - nBlock * BLOCK_SIZE;
	}
	
	// 移动指针，读取压缩数据，再解压缩
	g_pPakList->Seek(m_dwDataPtr, FILE_BEGIN);
	g_pPakList->Read(CodeInfo.lpPack, CodeInfo.dwPackLen);
	g_pPakList->Decode(&CodeInfo);
}
//---------------------------------------------------------------------------
*/
