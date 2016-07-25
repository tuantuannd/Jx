//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KZipList.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KZipList_H
#define KZipList_H
//---------------------------------------------------------------------------
#include "KZipData.h"
#include "KZipCodec.h"
#include "KMutex.h"
//---------------------------------------------------------------------------
#define MAX_PAK		32
//---------------------------------------------------------------------------
class ENGINE_API KZipList
{
private:
	KZipCodec	m_ZipCodec;	// ZIP—πÀı/Ω‚—π
	KZipData	m_ZipFile[MAX_PAK];
	int			m_nNumber;
	int			m_nActive;
	KMutex		m_Mutex;
public:
	KZipList();
	~KZipList();
	BOOL		Open(LPSTR FileName);
	void		Close();
	void		SetActive(int nActive){m_nActive = nActive;};
	int			GetActive(){return m_nActive;};
	DWORD		Read(LPVOID lpBuffer, DWORD dwLen);
	DWORD		Seek(LONG lOffset, DWORD dwMethod);
	DWORD		Tell();
	int			Search(LPSTR pFileName, PDWORD pOffset, PDWORD pLen);
	BOOL		Decode(PBYTE pIn, PBYTE pOut, LF* pLf);
	void		Lock(){m_Mutex.Lock();};
	void		Unlock(){m_Mutex.Unlock();};
};
extern ENGINE_API KZipList* g_pZipList;
//---------------------------------------------------------------------------
#endif
