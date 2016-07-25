//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPakList.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
// Modify:	Wooy(2003-9-17)
//---------------------------------------------------------------------------
#ifndef KPakList_H
#define KPakList_H

#include "XPackFile.h"
//---------------------------------------------------------------------------


class ENGINE_API KPakList
{
public:
	KPakList();
	~KPakList();
	bool		Open(const char* pPakListFile);
	void		Close();
	//查找包内的子文件
	bool		FindElemFile(unsigned long uId, XPackElemFileRef& ElemRef);
	//查找包内的子文件
	bool		FindElemFile(const char* pszFileName, XPackElemFileRef& ElemRef);
	//读取包内的子文件
	int			ElemFileRead(XPackElemFileRef& ElemRef, void* pBuffer, unsigned uSize);
	//读取spr文件头部或整个spr
	SPRHEAD*	GetSprHeader(XPackElemFileRef& ElemRef, SPROFFS*& pOffsetTable);
	//读取按帧压缩的spr的一帧的数据
	SPRFRAME*	GetSprFrame(int nPackIndex, SPRHEAD* pSprHeader, int nFrame);
private:
	unsigned long FileNameToId(const char* pszFileName);
private:
	#define MAX_PAK		32
	XPackFile*			m_PakFilePtrList[MAX_PAK];
	long				m_nPakNumber;
};

extern ENGINE_API KPakList* g_pPakList;

#endif
