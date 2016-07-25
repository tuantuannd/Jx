// NetRelay.h: interface for the CNetRelay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETRELAY_H__CDD2BB6B_7697_4606_8A4A_548E153CC60E__INCLUDED_)
#define AFX_NETRELAY_H__CDD2BB6B_7697_4606_8A4A_548E153CC60E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <deque>
#include "KThread.h"

class CNetRelay : public KThread_CS 
{
public:
	CNetRelay();
	virtual ~CNetRelay();

public:
	virtual BOOL Stop();
	virtual HANDLE Start();

private:
	virtual DWORD Main(LPVOID lpParam);
	virtual void Breathe();
	void Clearup();

private:
	HANDLE m_hDataExist;

private:
	struct PACKAGE
	{
		unsigned long idFrom;
		DWORD ipTo;
		DWORD portTo;
		void* pData;
		unsigned size;
	};
	typedef std::deque<PACKAGE>	PACKAGEQUEUE;
	PACKAGEQUEUE m_queuePckg;

public:
	int RelayPackage(unsigned long idFrom, DWORD ipTo, unsigned short portTo, void* pData, unsigned size);
};

#endif // !defined(AFX_NETRELAY_H__CDD2BB6B_7697_4606_8A4A_548E153CC60E__INCLUDED_)
