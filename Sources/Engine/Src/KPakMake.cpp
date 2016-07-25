//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakMake.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Pack data file make class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"
#include "KPakMake.h"
#include<string>
#include<strstream>
using namespace std;
//---------------------------------------------------------------------------
#define BLOCK_SIZE	(0x10000L)
#define COMBK_SIZE	(BLOCK_SIZE + BLOCK_SIZE / 64 + 19)
//---------------------------------------------------------------------------
// file list
typedef struct
{
	LPSTR		FileName;
	DWORD		FileId;
} FILELIST;
//---------------------------------------------------------------------------
// file offs
typedef	struct
{
	DWORD		FileId;
	DWORD		FileOffset;
	DWORD		FileLength;
} OFFSLIST;
//---------------------------------------------------------------------------
// 函数:	KPakMake
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KPakMake::KPakMake()
{
	m_pCodec = NULL;
	m_pRootPath = NULL;
	m_nFileNum = 0;
	m_nCompressMethod = CODEC_NONE;
}
//---------------------------------------------------------------------------
// 函数:	~KPakMake
// 功能:	分造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KPakMake::~KPakMake()
{
	g_FreeCodec(&m_pCodec, m_nCompressMethod);
}
//---------------------------------------------------------------------------
// 函数:	Pack
// 功能:	打包
// 参数:	lpListFileName		List文件名
//			lpPackFileName		Pack文件名
//			nCompressMethod		压缩方法
// 返回:	TRUE				成功
//			FALSE				失败
//---------------------------------------------------------------------------
BOOL KPakMake::Pack(LPSTR lpListFileName, LPSTR lpPackFileName, int nCompressMethod)
{
	m_nCompressMethod = nCompressMethod;

	g_InitCodec(&m_pCodec, m_nCompressMethod);
	if (m_pCodec == NULL)
		return FALSE;
	
	if (!ReadFileList(lpListFileName))
		return FALSE;
	
	if (!PackFileList(lpPackFileName))
		return FALSE;
	
	g_FreeCodec(&m_pCodec, m_nCompressMethod);
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	PackFileList
// 功能:	打包的文件
// 参数:	lpPackFileName	文件名
// 返回:	TRUE			成功
//			FALSE			失败
//---------------------------------------------------------------------------
BOOL KPakMake::PackFileList(LPSTR lpPackFileName)
{
	FILELIST*	pFileList;
	OFFSLIST*	pOffsList;
	DWORD		dwOffset;
	DWORD		dwLength;
	PBYTE		pDataBuf;
	PBYTE		pPackBuf;
	TCodeInfo	CodeInfo;
	int			nBlock;
	int			i, j;

	// create data file
	g_SetFilePath("\\");
	if (!m_PackFile.Create(lpPackFileName))
		return FALSE;

	// write file header
	TPakFileHeader Header;
	g_MemZero(&Header, sizeof(Header));
	g_MemCopy(Header.Signature, "PACKAGE", 7);
	Header.FilesInPack = m_nFileNum;
	Header.CompressMethod = m_nCompressMethod;
	m_PackFile.Write(&Header, sizeof(Header));

	// set list pointer
	pFileList = (FILELIST *)m_MemList.GetMemPtr();
	pOffsList = (OFFSLIST *)m_MemOffs.GetMemPtr();

	// write offset table
	m_PackFile.Write(pOffsList, sizeof(OFFSLIST) * m_nFileNum);

	// get file pointer
	dwOffset = m_PackFile.Tell();

	// alloc memory
	pDataBuf = (PBYTE)g_MemAlloc(BLOCK_SIZE);
	pPackBuf = (PBYTE)g_MemAlloc(COMBK_SIZE);
	CodeInfo.lpData = pDataBuf;
	CodeInfo.lpPack = pPackBuf;

	// write data file
	g_SetFilePath(m_pRootPath);
	for (i = 0; i < m_nFileNum; i++)
	{
		// open the file to pack
		if (!m_DiskFile.Open(pFileList[i].FileName))
		{
			g_DebugLog("Can't open file : %s\n", pFileList[i].FileName);
			return FALSE;
		}


		printf("packing file %s...\n", pFileList[i].FileName);

		string FileName(pFileList[i].FileName); 
		// setup offset list
		dwLength = m_DiskFile.Size();
		pOffsList[i].FileLength = dwLength;
		pOffsList[i].FileOffset = dwOffset;

		// seek to file begin
		m_PackFile.Seek(dwOffset, FILE_BEGIN);

		// write block size table
		nBlock = (dwLength + 0xffff) >> 16;
		if (nBlock >= 1024)
		{
			string Warning = FileName + " is Too big file to pack";

			throw exception(Warning.c_str() );
			break;
		}
		m_PackFile.Write(m_BlockSize, sizeof(WORD) * nBlock);

		// write compressed file block
		for (j = 0; dwLength >= BLOCK_SIZE; dwLength -= BLOCK_SIZE, j++)
		{
			m_DiskFile.Read(pDataBuf, BLOCK_SIZE);
			CodeInfo.dwDataLen = BLOCK_SIZE;
			CodeInfo.dwPackLen = 0;
			m_pCodec->Encode(&CodeInfo);
			if (CodeInfo.dwPackLen >= BLOCK_SIZE)
			{
				m_BlockSize[j] = 0;
				m_PackFile.Write(pDataBuf, BLOCK_SIZE);
			}
			else
			{
				m_BlockSize[j] = (WORD)CodeInfo.dwPackLen;
				m_PackFile.Write(pPackBuf, CodeInfo.dwPackLen);
			}
		}

		// 如果最后一块数据压缩后超过0x10000, 就保存不压缩的0x10000字节
		if (dwLength > 0)
		{
			m_DiskFile.Read(pDataBuf, dwLength);
			CodeInfo.dwDataLen = dwLength;
			CodeInfo.dwPackLen = 0;
			m_pCodec->Encode(&CodeInfo);
			if (CodeInfo.dwPackLen >= BLOCK_SIZE)
			{
				m_BlockSize[j] = 0;
				m_PackFile.Write(pDataBuf, BLOCK_SIZE);
			}
			else
			{
				m_BlockSize[j] = (WORD)CodeInfo.dwPackLen;
				m_PackFile.Write(pPackBuf, CodeInfo.dwPackLen);
			}
		}

		// save file pointer
		dwOffset = m_PackFile.Tell();

		// seek to file begin
		m_PackFile.Seek(pOffsList[i].FileOffset, FILE_BEGIN);

		// write block size table
		m_PackFile.Write(m_BlockSize, sizeof(WORD) * nBlock);

		// close file
		m_DiskFile.Close();
	}

	// free memory
	g_MemFree(pDataBuf);
	g_MemFree(pPackBuf);

	// seek to begin of offset table
	m_PackFile.Seek(sizeof(TPakFileHeader), FILE_BEGIN);

	// write offset table
	m_PackFile.Write(pOffsList, sizeof(OFFSLIST) * m_nFileNum);

	// close pack file
	m_PackFile.Close();

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	ReadFileList
// 功能:	读取要打包的文件列表
// 参数:	lpListFileName	文件名
// 返回:	TRUE			成功
//			FALSE			失败
//---------------------------------------------------------------------------
BOOL KPakMake::ReadFileList(LPSTR lpListFileName)
{
	FILELIST*	pFileList;
	OFFSLIST*	pOffsList;
	FILELIST	TempList;
	KFile		File;
	LPSTR		pList;
	LPSTR		pEnd;
	int			i, j;

	// open list file
	g_SetFilePath("\\");
	if (!File.Open(lpListFileName))
		return FALSE;

	// alloc memory
	m_MemFile.Alloc(File.Size() + 2);

	// read list file
	pList = (LPSTR)m_MemFile.GetMemPtr();
	File.Read(pList, File.Size());
	
	// add file end
	pEnd = pList;
	pEnd += File.Size();
	pEnd[0] = 0x0d;
	pEnd[1] = 0x0a;

	// get file path
	m_pRootPath = pList;
	pList = NextLine(pList);

	// get file number in list
	m_nFileNum = 0;
	while (pList < pEnd)
	{
		pList = NextLine(pList);
		m_nFileNum++;
	}
	if (m_nFileNum == 0)
		return FALSE;

	// allocate memory
	m_MemList.Alloc(sizeof(FILELIST) * m_nFileNum);
	m_MemOffs.Alloc(sizeof(OFFSLIST) * m_nFileNum);
	pFileList = (FILELIST *)m_MemList.GetMemPtr();
	pOffsList = (OFFSLIST *)m_MemOffs.GetMemPtr();

	// setup file list
	pList = (LPSTR)m_MemFile.GetMemPtr();
	pList = NextLine(pList);
	for (i = 0; i < m_nFileNum; i++)
	{
		pFileList[i].FileName = pList;
		g_StrLower(pFileList[i].FileName);
		pFileList[i].FileId = g_FileName2Id(pFileList[i].FileName);
		pList = NextLine(pList);
	}

	// sort the file list
	for (i = 0; i < m_nFileNum - 1; i++)
	{
		for (j = i + 1; j < m_nFileNum; j++)
		{
			if (pFileList[i].FileId > pFileList[j].FileId)
			{
				TempList = pFileList[i];
				pFileList[i] = pFileList[j];
				pFileList[j] = TempList;
			}
			else if (pFileList[i].FileId == pFileList[j].FileId)
			{
				g_DebugLog("There are files have the same id");

				char WarnChar[200];

				sprintf(WarnChar,"There are files have the same id: %d  File1 is  %s  File2 is %s ",pFileList[i].FileId,pFileList[i].FileName,pFileList[j].FileName);
                
				 //warn<<"There are files have the same id:" << pFileList[i].FileId <<"  File1 is " << pFileList[i].FileName  << " File2 is " << pFileList[j].FileName;

				throw exception(WarnChar );
				return FALSE;
			}
		}
	}

	// create offset table
	for (i = 0; i < m_nFileNum; i++)
	{
		pOffsList[i].FileId = pFileList[i].FileId;
		pOffsList[i].FileOffset = 0;
		pOffsList[i].FileLength = 0;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	NextLine
// 功能:	移动到下一行数据
// 参数:	pList		数据指针
// 返回:	下一行的数据指针
//---------------------------------------------------------------------------
LPSTR KPakMake::NextLine(LPSTR pList)
{
	// search 0x0d ('\r') 0x00 ('\0')
	while (*pList != 0x0d && *pList != 0x00)
		pList++;

	// remove 0x0d ('\r')
	*pList = 0;
	pList++;
	
	// remove 0x0a ('\n')
	*pList = 0;
	pList++;
	
	return pList;
}
//---------------------------------------------------------------------------

BOOL KPakMake::ReadFileList(const vector<string> &FileList, const string &RootPath)
{
	FILELIST*	pFileList;
	OFFSLIST*	pOffsList;
	FILELIST	TempList;
	

	int			i, j;

	// get file path
	m_pRootPath =(char *) RootPath.c_str()  ;

	m_nFileNum  = FileList.size();
	
	// allocate memory
	m_MemList.Alloc(sizeof(FILELIST) * m_nFileNum);
	m_MemOffs.Alloc(sizeof(OFFSLIST) * m_nFileNum);
	pFileList = (FILELIST *)m_MemList.GetMemPtr();
	pOffsList = (OFFSLIST *)m_MemOffs.GetMemPtr();

	// setup file list
	
	for (i = 0; i < m_nFileNum; i++)
	{
		pFileList[i].FileName = (char *)FileList[i].c_str();
		g_StrLower(pFileList[i].FileName);
		pFileList[i].FileId = g_FileName2Id(pFileList[i].FileName);
		
	}

	// sort the file list
	for (i = 0; i < m_nFileNum - 1; i++)
	{
		for (j = i + 1; j < m_nFileNum; j++)
		{
			if (pFileList[i].FileId > pFileList[j].FileId)
			{
				TempList = pFileList[i];
				pFileList[i] = pFileList[j];
				pFileList[j] = TempList;
			}
			else if (pFileList[i].FileId == pFileList[j].FileId)
			{
				g_DebugLog("There are files have the same id");

				char WarnChar[200];

				sprintf(WarnChar,"There are files have the same id: %d  File1 is  %s  File2 is %s ",pFileList[i].FileId,pFileList[i].FileName,pFileList[j].FileName);
                
				 //warn<<"There are files have the same id:" << pFileList[i].FileId <<"  File1 is " << pFileList[i].FileName  << " File2 is " << pFileList[j].FileName;

				throw exception(WarnChar );
				return FALSE;
			}
		}
	}

	// create offset table
	for (i = 0; i < m_nFileNum; i++)
	{
		pOffsList[i].FileId = pFileList[i].FileId;
		pOffsList[i].FileOffset = 0;
		pOffsList[i].FileLength = 0;
	}

	return TRUE;

}

#include "ZPackFile.h"
#define MAX_FILE					2004800			//最多20万个文件
#define COMPRESS_BUF_SIZE	10240000
#define FRAME_SIZE			800 * 1024			//800K以上的SPR文件使用分帧的压缩
#include <ucl/ucl.h>
typedef struct {
	BYTE	Comment[4];	// 注释文字(SPR\0)
	WORD	Width;		// 图片宽度
	WORD	Height;		// 图片高度
	WORD	CenterX;	// 重心的水平位移
	WORD	CenterY;	// 重心的垂直位移
	WORD	Frames;		// 总帧数
	WORD	Colors;		// 颜色数
	WORD	Directions;	// 方向数
	WORD	Interval;	// 每帧间隔（以游戏帧为单位）
	WORD	Reserved[6];// 保留字段（到以后使用）
} SPRHEAD;

typedef struct
{
	DWORD	Offset;		// 每一帧的偏移
	DWORD	Length;		// 每一帧的长度
} SPROFFS;

typedef struct {
	long compress_size;
	long size;
} frame_info;

typedef struct {
	unsigned long	id;
	int				frame;
} frame_index_info;

BOOL KPakMake::Pack(LPSTR lpPackFileName,const vector<string>& vFileList,const string& sRootPath,int nCompressMethod)
{
	char compress_buffer[COMPRESS_BUF_SIZE];			//10M的压缩缓冲区，存放所有的帧，一次写
	index_info index_list[MAX_FILE];
	char *temp_buffer[MAX_FILE * sizeof(index_info)];
	FILE *output = fopen(lpPackFileName, "wb");
	z_pack_header header;
	unsigned long offset = 0;
	int file_count = 0;
	memset(&header, 0, sizeof(header));
	fwrite(&header, 1, sizeof(header), output);
	offset += sizeof(header);
	memset(&header, 0, sizeof(header));
	memcpy(header.signature, "PACK", 4);
	header.index_offset = offset;
	header.data_offset = sizeof(header);
	header.count = file_count;

	char full_name[MAX_PATH];
	for(int index = 0; index < vFileList.size(); index++) {
		strcpy(full_name, sRootPath.c_str());
		int root_length = strlen(sRootPath.c_str());
//	getcwd(full_name, MAX_PATH);
		strcpy(full_name, "\\");
		strcat(full_name, vFileList[index].c_str());
		char *ptr = full_name;
		while(*ptr) {
			if(*ptr >= 'A' && *ptr <= 'Z') *ptr += 'a' - 'A';
			ptr++;
		}

		unsigned long id = hash(full_name + root_length);
		int index;
		for(index = 0; index < file_count; index++) {
			if(id < index_list[index].id) {
				memmove(temp_buffer, (char *)&index_list[index], (file_count - index) * sizeof(index_info));
				memmove((char *)&index_list[index + 1], temp_buffer, (file_count - index) * sizeof(index_info));
				break;
			}
		}	
		file_count++;

		ZMapFile map(full_name);
		int compress_size = 0;
	
		unsigned long compress_type = TYPE_UCL;				//使用UCL压缩
	
		bool bSPR = false;									//是否为SPR文件
		const char *ext = full_name + strlen(full_name) - 3;
		if(*ext == 's' && *(ext + 1) == 'p' && *(ext + 2) == 'r') bSPR = true;
	
		int r;
		unsigned int size = 0;
		map.map();
		if(map.m_Ptr) {
			index_list[index].id = id;
			index_list[index].offset = offset;
			index_list[index].size = map.m_Size;
			ptr = compress_buffer;
			if(!bSPR || map.m_Size < FRAME_SIZE) {
				if(compress_type == TYPE_UCL) {
					r = ucl_nrv2b_99_compress((BYTE *)map.m_Ptr, map.m_Size, (BYTE *)ptr, (unsigned int *)&index_list[index].compress_size, NULL, 5, NULL, NULL);
				}
				else if(compress_type == TYPE_BZIP2) {
//					index_list[index].compress_size = COMPRESS_BUF_SIZE;
	//				r = BZ2_bzBuffToBuffCompress(ptr, (unsigned int *)&index_list[index].compress_size, map.m_Ptr, map.m_Size, 9, 0, 30);
				}
				if(r) return false;
				fwrite(compress_buffer, 1, index_list[index].compress_size, output);
				offset += index_list[index].compress_size;
				printf("%s [%d]->[%d]\n", full_name + root_length, map.m_Size, index_list[index].compress_size);
				index_list[index].compress_size |= (compress_type << 24);
			}
			else {								//每帧独立压缩
				SPRHEAD *head;
				head = (SPRHEAD *)map.m_Ptr;
				memmove(ptr, head, sizeof(SPRHEAD) + head->Colors * 3);			//前面的数据不压缩
				ptr += sizeof(SPRHEAD) + head->Colors * 3;
				frame_info *compress_frame_info = (frame_info *)ptr;					//压缩后每一帧的数据
				ptr += head->Frames * sizeof(SPROFFS);
				SPROFFS *frame_info = (SPROFFS *)(map.m_Ptr + sizeof(SPRHEAD) + head->Colors * 3);		//原来每一帧的数据
				char *frame_data = (char *)frame_info + head->Frames * sizeof(SPROFFS);
				int frame_index;
	
				int frame_offset = 0;
				for(frame_index = 0; frame_index < head->Frames; frame_index++) {
//压缩每一帧的内容
					if(frame_info[frame_index].Length >= 256) {				//小于256字节的不压缩
						if(compress_type == TYPE_UCL) {
							r = ucl_nrv2b_99_compress((BYTE *)frame_data + frame_info[frame_index].Offset, frame_info[frame_index].Length, (BYTE *)ptr, &size, NULL, 10, NULL, NULL);
						}	
						else if(compress_type == TYPE_BZIP2) {
	//						size = COMPRESS_BUF_SIZE;
//							r = BZ2_bzBuffToBuffCompress(ptr, &size, frame_data + frame_info[frame_index].Offset, frame_info[frame_index].Length, 9, 0, 30);
						}
						if(r) return false;
						compress_frame_info[frame_index].size = frame_info[frame_index].Length;		//记录原来的大小
					}
					else {
						size = frame_info[frame_index].Length;
						memmove(ptr, (BYTE *)frame_data + frame_info[frame_index].Offset, size);
						compress_frame_info[frame_index].size = -(long)frame_info[frame_index].Length;		//记录原来的大小
					}
					compress_size += size;
					compress_frame_info[frame_index].compress_size = size;
					frame_offset += size;
					ptr += size;		
				}
				fwrite(compress_buffer, 1, ptr - compress_buffer, output);
				offset += ptr - compress_buffer;
				printf("[frame] %s old size = %d, compressed size = %d\n", full_name + root_length, map.m_Size, compress_size);
				index_list[index].compress_size = (ptr - compress_buffer) | ((compress_type | TYPE_FRAME) << 24);
			}
		}
	}
	int result = fwrite(&index_list[0], 1, file_count * sizeof(index_info), output);
	fseek(output, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(header), output);
	fclose(output);

    return true;

}
