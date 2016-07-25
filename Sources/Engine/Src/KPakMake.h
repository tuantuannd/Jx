//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakMake.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#pragma warning(disable:4786)

#ifndef KPakMake_H
#define KPakMake_H
//---------------------------------------------------------------------------
#include "KPakData.h"
//---------------------------------------------------------------------------
#include<vector>
using namespace std;
class ENGINE_API KPakMake
{
private:
	KMemClass	m_MemFile;
	KMemClass	m_MemList;
	KMemClass	m_MemOffs;
	KFile		m_DiskFile;
	KFile		m_PackFile;
	KCodec*		m_pCodec;
	LPSTR		m_pRootPath;
	int			m_nFileNum;
	int			m_nCompressMethod;
	WORD		m_BlockSize[1024];
private:
	LPSTR		NextLine(LPSTR pList);
	BOOL		ReadFileList(LPSTR lpListFileName);
	BOOL		PackFileList(LPSTR lpPackFileName);
	BOOL ReadFileList(const vector<string>& FileList,const string& RootPath );

public:
	BOOL Pack(LPSTR lpPackFileName, const vector<string>& vFileList,const string& sRootPath,int nCompressMethod);
	KPakMake();
	~KPakMake();
	BOOL		Pack(LPSTR lpListFileName, LPSTR lpPackFileName, int nCompressMethod);
};
//---------------------------------------------------------------------------
#endif
