//-----------------------------------------//
//                                         //
//  File		: S3PRelockAccount.h	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/28/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PRELOCKACCOUNT_H_
#define _S3PRELOCKACCOUNT_H_

#include "KStdAfx.h"

typedef struct tag_RELOCKERPARAM
{
	DWORD dwCycle;
	HANDLE hEnable;
}_RELOCKERPARAM, *_LPRELOCKERPARAM;

class S3PRelockAccount  
{
public:
	static S3PRelockAccount* Instance();
	static void ReleaseInstance();

protected:
	static S3PRelockAccount* m_pInstance;
	static DWORD m_dwCycle;		// Relock cycle
	static HANDLE m_hEnable;	// Thread running signal
	
	static DWORD WINAPI Relock( LPVOID lpParam );
public:
	virtual HANDLE Start();
	virtual BOOL Stop();
protected:
	virtual void Init();
	S3PRelockAccount();
	virtual ~S3PRelockAccount();

	HANDLE m_hRelocker;		// Thread handle
	DWORD m_dwRelockerID;	// Thread ID
	_RELOCKERPARAM m_param;
};

#endif	// _S3PRELOCKACCOUNT_H_
