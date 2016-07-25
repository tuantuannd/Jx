//-----------------------------------------//
//                                         //
//  File		: S3PList.cpp			   //
//	Author		: Yang Xiaodong            //
//	Modified	: 2/9/2002		           //
//                                         //
//-----------------------------------------//
#include "S3PList.h"

S3PList::S3PList()
{
	Init();
}

S3PList::~S3PList()
{

}

void S3PList::Init()
{
	m_lpHead = NULL;
	m_lpTail = NULL;
	m_pFreeDataFun = NULL;
}

BOOL S3PList::FreeData( FREEDATAPROC pFreeDataFun, DWORD data )
{
	BOOL bRet = FALSE;
	if ( NULL == pFreeDataFun )
	{
		bRet = FALSE;
	}
	else
	{
		bRet = pFreeDataFun( data );
	}
	return bRet;
}

void S3PList::AddTail( _LPS3PLISTNODE lpNewTail )
{
	if ( NULL == lpNewTail )
	{
		return;
	}
	if ( NULL == m_lpTail )
	{
		m_lpTail = lpNewTail;
		m_lpHead = m_lpTail;
	}
	else
	{
		m_lpTail->lpNext = lpNewTail;
		lpNewTail->lpPrev = m_lpTail;

		m_lpTail = lpNewTail;
		m_lpTail->lpNext = NULL;
	}
}

void S3PList::AddHead( _LPS3PLISTNODE lpNewHead )
{
	if ( NULL == lpNewHead )
	{
		return;
	}
	if ( NULL == m_lpHead )
	{
		m_lpHead = lpNewHead;
		m_lpTail = m_lpHead;
	}
	else
	{
		m_lpHead->lpPrev = lpNewHead;
		lpNewHead->lpNext = m_lpHead;

		m_lpHead = lpNewHead;
		m_lpHead->lpPrev = NULL;
	}
}

void S3PList::NeedFreeData( FREEDATAPROC pFreeDataFun )
{
	m_pFreeDataFun = pFreeDataFun;
}

BOOL S3PList::GetTail( DWORD& data )
{
	BOOL bRet = FALSE;

	if ( NULL != m_lpTail )
	{
		bRet = TRUE;
		data = m_lpTail->data;
	}

	return bRet;
}

BOOL S3PList::GetHead( DWORD& data )
{
	BOOL bRet = FALSE;

	if ( NULL != m_lpHead )
	{
		bRet = TRUE;
		data = m_lpHead->data;
	}

	return bRet;
}

void S3PList::RemoveAll()
{
	_LPS3PLISTNODE lpAt = m_lpHead;
	while ( lpAt )
	{
		_LPS3PLISTNODE lpNext = lpAt->lpNext;
		FreeData( m_pFreeDataFun, lpAt->data );
		delete lpAt;
		lpAt = lpNext;
	}
	m_lpHead = NULL;
	m_lpTail = NULL;
}

void S3PList::RemoveHead()
{
	if ( NULL == m_lpHead )
	{
		return;
	}
	if ( m_lpHead == m_lpTail )
	{
		FreeData( m_pFreeDataFun, m_lpHead->data );
		delete m_lpHead;
		m_lpHead = NULL;
		m_lpTail = NULL;
	}
	else
	{
		_LPS3PLISTNODE lpNext = m_lpHead->lpNext;

		lpNext->lpPrev = NULL;
		FreeData( m_pFreeDataFun, m_lpHead->data );
		delete m_lpHead;
		m_lpHead = lpNext;
	}
}

void S3PList::RemoveTail()
{
	if ( NULL == m_lpTail )
	{
		return;
	}
	if ( m_lpTail == m_lpHead )
	{
		FreeData( m_pFreeDataFun, m_lpTail->data );
		delete m_lpTail;
		m_lpTail = NULL;
		m_lpHead = NULL;
	}
	else
	{
		_LPS3PLISTNODE lpPrev = m_lpTail->lpPrev;

		lpPrev->lpNext = NULL;
		FreeData( m_pFreeDataFun, m_lpTail->data );
		delete m_lpTail;
		m_lpTail = lpPrev;
	}
}

BOOL S3PList::AddHead( DWORD data )
{
	BOOL bRet = FALSE;
	_LPS3PLISTNODE lpNewHead = new _S3PLISTNODE;
	if ( NULL != lpNewHead )
	{
		memset( lpNewHead, 0, sizeof( _S3PLISTNODE ) );
		lpNewHead->data = data;
		AddHead( lpNewHead );
		bRet = TRUE;
	}
	return bRet;
}

BOOL S3PList::AddTail( DWORD data )
{
	BOOL bRet = FALSE;
	_LPS3PLISTNODE lpNewTail = new _S3PLISTNODE;
	if ( NULL != lpNewTail )
	{
		memset( lpNewTail, 0, sizeof( _S3PLISTNODE ) );
		lpNewTail->data = data;
		AddTail( lpNewTail );
		bRet = TRUE;
	}
	return bRet;
}

BOOL S3PList::IsEmpty()
{
	BOOL bRet = FALSE;
	if ( ( NULL == m_lpHead )
		&& ( NULL == m_lpTail ) )
	{
		bRet = TRUE;
	}
	return bRet;
}