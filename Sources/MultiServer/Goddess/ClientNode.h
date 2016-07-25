/********************************************************************
	created:	2003/06/05
	file base:	ClientNode
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_CLIENTNODE_H__
#define __INCLUDE_CLIENTNODE_H__

#include "IServer.h"
#include "HeavenInterface.h"

#include "Buffer.h"
#include "CriticalSection.h"
#include "Event.h"

#include "kprotocoldef.h"

#define _NAME_LENGTH 64

#include <list>

using namespace std;

class CClientNode
{
public:
	
	explicit CClientNode( IServer *pServer, size_t id );

	virtual ~CClientNode();

	static bool Start( IServer *pServer );
	static void End();

	static CClientNode *AddNode( IServer *pServer, size_t id );
	static void DelNode( size_t id );

	void AppendData( const void *pData, size_t datalength );
	void Process();

protected:

	static unsigned int __stdcall ThreadFunction( void *pV );

	static HANDLE									m_hThread;
	static OnlineGameLib::Win32::CEvent				m_hQuitEvent;

	static OnlineGameLib::Win32::CCriticalSection	m_csCL;
	
	typedef map< size_t, CClientNode * >	stdMap;

	static stdMap					m_theClientMap;

	void LargePackProcess( const void *pData, size_t dataLength );
	void SmallPackProcess( const void *pData, size_t dataLength );

private:

	size_t		m_nIndentity;

	OnlineGameLib::Win32::CPackager m_theRecv;
	OnlineGameLib::Win32::CPackager m_theSend;

	OnlineGameLib::Win32::CBuffer::Allocator m_theAllocator;

	IServer		*m_pServer;

	typedef void ( CClientNode::*ProcessArray[c2s_end] )( const void *pData, size_t dataLength );

	ProcessArray m_theProcessArray;

	void _QueryRoleList( const void *pData, size_t dataLength );
	void _CreateRole( const void *pData, size_t dataLength );
	void _SaveRoleInfo( const void *pData, size_t dataLength );
	void _DelRole( const void *pData, size_t dataLength );
	void _GetRoleInfo( const void *pData, size_t dataLength );
	void _RelayExtend( const void *pData, size_t dataLength );
	void _GetGameStat( const void *pData, size_t dataLength );//发送游戏统计数据(By Fellow,2003.7.22)
	void _LockOrUnlockRole( const void *pData, size_t dataLength );

	/*
	 * class CDataQueue
	 */
	class CDataQueue
	{
	public:

		explicit CDataQueue( size_t bufferSize = 1024 * 64, size_t maxFreeBuffers = 160 );
		
		~CDataQueue();

		/*
		 * Function
		 */
		bool		AddData( const BYTE *pData, size_t datalength );

		OnlineGameLib::Win32::CBuffer		*Get();

	private:
		/*
		 * Data
		 */
		OnlineGameLib::Win32::CCriticalSection		m_csQueue;

		typedef list< OnlineGameLib::Win32::CBuffer * > stdDataList;

		stdDataList									m_theData;

		OnlineGameLib::Win32::CBuffer::Allocator	m_theDQAllocator;
	};

	CDataQueue	m_theDataQueue;

	static OnlineGameLib::Win32::CCriticalSection	m_csCR;

	typedef map< std::string, size_t >	stdRoleLockMap;

	static stdRoleLockMap m_csRoleLock;

	bool IsRoleLock(char* szRole);
	bool IsRoleLockBySelf(char* szRole);
	bool UnlockRoleSelf(char* szRole);

	void UnlockAllRole(size_t ID);
};

#endif // __INCLUDE_CLIENTNODE_H__