//-----------------------------------------//
//                                         //
//  File		: S3PList.h				   //
//	Author		: Yang Xiaodong            //
//	Modified	: 2/9/2002		           //
//                                         //
//-----------------------------------------//
#if !defined(AFX_S3PLIST_H__61BA3EE5_6CC2_4231_BEA3_147F54A54458__INCLUDED_)
#define AFX_S3PLIST_H__61BA3EE5_6CC2_4231_BEA3_147F54A54458__INCLUDED_

#include "KStdAfx.h"
typedef BOOL ( CALLBACK *FREEDATAPROC )( DWORD );

class S3PList  
{
	typedef struct tag_S3PLISTNODE
	{
		DWORD data;
		struct tag_S3PLISTNODE* lpPrev;
		struct tag_S3PLISTNODE* lpNext;
	}_S3PLISTNODE, *_LPS3PLISTNODE;
public:
	S3PList();
	virtual ~S3PList();

	virtual void NeedFreeData( FREEDATAPROC pFreeDataFun );
	virtual BOOL GetTail( DWORD& data );
	virtual BOOL GetHead( DWORD& data );
	virtual void RemoveAll();
	virtual void RemoveHead();
	virtual void RemoveTail();
	virtual BOOL AddHead( DWORD data );
	virtual BOOL AddTail( DWORD data );
	virtual BOOL IsEmpty();

protected:
	virtual void Init();
	virtual BOOL FreeData( FREEDATAPROC pFreeDataFun, DWORD data );
	virtual void AddTail( _LPS3PLISTNODE lpNewTail );
	virtual void AddHead( _LPS3PLISTNODE lpNewHead );

	FREEDATAPROC m_pFreeDataFun;
	_LPS3PLISTNODE m_lpHead;
	_LPS3PLISTNODE m_lpTail;
};

#endif // !defined(AFX_S3PLIST_H__61BA3EE5_6CC2_4231_BEA3_147F54A54458__INCLUDED_)
