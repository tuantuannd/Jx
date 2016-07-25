//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakTool.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Pack one file Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KTimer.h"
#include "KFilePath.h"
#include "KPakTool.h"
//---------------------------------------------------------------------------
// pack file header
typedef struct {
	DWORD		Signature;			// 文件标识 = 0x4b434150 ("PACK")
	DWORD		FileLen;			// 文件原始的长度
	DWORD		PackLen;			// 文件压缩后长度
	DWORD		CompressMethod;		// 使用的压缩算法
} PACKFILEHEADER;
//---------------------------------------------------------------------------
// 函数:	KPakTool()
// 功能:	构造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KPakTool::KPakTool()
{
	m_pCodec = NULL;
	m_nCompressMethod = CODEC_NONE;
}
//---------------------------------------------------------------------------
// 函数:	~KPakTool()
// 功能:	析构函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KPakTool::~KPakTool()
{
	g_FreeCodec(&m_pCodec, m_nCompressMethod);
}
//---------------------------------------------------------------------------
// 函数:	Pack
// 功能:	压缩一个文件
// 参数:	pInFile		文件名
//			pOutFile	文件名
//			nMethod		压缩方法
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPakTool::Pack(LPSTR pInFile, LPSTR pOutFile, int nMethod)
{
	KFile		File;
	KMemClass	DataBuf;
	KMemClass	PackBuf;
	TCodeInfo	CodeInfo;
	KTimer		Timer;

	// open input file
	if (!File.Open(pInFile))
		return FALSE;

	// if the file too small ?
	if (File.Size() <= sizeof(PACKFILEHEADER))
		return FALSE;

	// get codec
	m_nCompressMethod = nMethod;
	g_InitCodec(&m_pCodec, m_nCompressMethod);
	if (m_pCodec == NULL)
		return FALSE;

	// get in & out len
	CodeInfo.dwDataLen = File.Size();
	CodeInfo.dwPackLen = m_pCodec->GetPackLen(CodeInfo.dwDataLen);

	// alloc in buffer
	if (!DataBuf.Alloc(CodeInfo.dwDataLen))
		return FALSE;

	// alloc out buffer
	if (!PackBuf.Alloc(CodeInfo.dwPackLen))
		return FALSE;

	// read input file 
	File.Read(DataBuf.GetMemPtr(), CodeInfo.dwDataLen);

	// is it a packed file ?
	if (g_MemComp(DataBuf.GetMemPtr(), "PACK", 4))
		return FALSE;

	// close input file
	File.Close();

	// create output file
	if (!File.Create(pOutFile))
		return FALSE;

	// encode file data
	CodeInfo.lpData = (PBYTE)DataBuf.GetMemPtr();
	CodeInfo.lpPack = (PBYTE)PackBuf.GetMemPtr();
	Timer.Start();
	m_pCodec->Encode(&CodeInfo);
	Timer.Stop();
	g_DebugLog("Encode time = %d ms", Timer.GetInterval());

	// delete encoder
	g_FreeCodec(&m_pCodec, m_nCompressMethod);

	// write file header
	PACKFILEHEADER Header;
	Header.Signature = 0x4b434150;
	Header.FileLen = CodeInfo.dwDataLen;
	Header.PackLen = CodeInfo.dwPackLen;
	Header.CompressMethod = m_nCompressMethod;
	File.Write(&Header, sizeof(Header));

	// write file data
	File.Write(PackBuf.GetMemPtr(), CodeInfo.dwPackLen);

	// close out file
	File.Close();

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	UnPack
// 功能:	解压一个文件
// 参数:	pInFile		文件名
//			pOutFile	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KPakTool::UnPack(LPSTR pInFile, LPSTR pOutFile)
{
	KFile		File;
	KMemClass	PackBuf;
	KMemClass	DataBuf;
	TCodeInfo	CodeInfo;
	KTimer		Timer;

	// open input file
	if (!File.Open(pInFile))
		return FALSE;

	// if the file too small ?
	if (File.Size() <= sizeof(PACKFILEHEADER))
		return FALSE;

	// read file header
	PACKFILEHEADER Header;
	if (File.Read(&Header, sizeof(Header)) != sizeof(Header))
		return FALSE;
	
	// check signature
	if (Header.Signature != 0x4b434150)
		return FALSE;

	// get in & out len
	CodeInfo.dwPackLen = Header.PackLen;
	CodeInfo.dwDataLen = Header.FileLen;
	m_nCompressMethod = Header.CompressMethod;

	// get codec
	g_InitCodec(&m_pCodec, m_nCompressMethod);
	if (m_pCodec == NULL)
		return FALSE;

	// alloc pack buffer
	if (!PackBuf.Alloc(CodeInfo.dwPackLen))
		return FALSE;

	// alloc data buffer
	if (!DataBuf.Alloc(CodeInfo.dwDataLen))
		return FALSE;

	// read input file 
	File.Read(PackBuf.GetMemPtr(), CodeInfo.dwPackLen);

	// close input file
	File.Close();

	// create output file
	if (!File.Create(pOutFile))
		return FALSE;

	// decode file data
	CodeInfo.lpData = (PBYTE)DataBuf.GetMemPtr();
	CodeInfo.lpPack = (PBYTE)PackBuf.GetMemPtr();
	Timer.Start();
	m_pCodec->Decode(&CodeInfo);
	Timer.Stop();
	g_DebugLog("Decode time = %d ms", Timer.GetInterval());

	// delete encoder
	g_FreeCodec(&m_pCodec, m_nCompressMethod);

	// write file data
	File.Write(DataBuf.GetMemPtr(), CodeInfo.dwDataLen);

	// close out file
	File.Close();

	return TRUE;
}
//---------------------------------------------------------------------------
