/********************************************************************
	created:	2003/04/08
	file base:	GamePlayer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_GAMEPLAYER_H__
#define __INCLUDE_GAMEPLAYER_H__

#include "IPlayer.h"
#include "ICommand.h"

#pragma warning(disable : 4786)  // identifier was truncated to '255' characters 
                                 // in the debug information

#include "CriticalSection.h"
#include "tstring.h"
#include "Buffer.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include <vector>
#include <map>

using namespace std;

/*
 * class CGamePlayer
 */
class CGamePlayer : public IPlayer
{
public:

	CGamePlayer( IClient *pAccSvrClient, 
		IServer *pPlayerServer, 
		IClient	*pDBRoleClient, 
		UINT nIdentityID = ( UINT )( -1 ) );

	virtual ~CGamePlayer();

	/*
	 * Task option
	 */
	enum
	{		
		/*
		 * Task command
		 */
		enumLogin = 0,
		enumLogOut,
		enumExchangeRegin,

		/*
		 * Task status
		 */
		enumToNextTask,
		enumCompleted,
		enumRepeat,
		enumError,
		enumNone
	};
	
	enum enumCDataQueueOwner
	{
		enumOwnerAccSvr = 0,
		enumOwnerRoleSvr,
		enumOwnerPlayer,

		enumOwnerTotal
	};

	virtual bool DispatchTask( UINT nTaskID );
	virtual bool IsWorking();
	virtual int  Run();
	virtual bool AppendData( UINT nOwner, const void *pData, size_t dataLength );
	
	virtual bool Create( const char * const pName, const char * const pPassword );
	virtual bool Destroy( const char * const /*pPassword*/ );

	static bool SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers );

private:

	static OnlineGameLib::Win32::CBuffer::Allocator	m_theGlobalAllocator;

	static LONG m_slnIdentityCounts;
	LONG		m_lnIdentityID;

	/*
	 * Task system
	 */
	class CTask
	{
	public:

		explicit CTask( CGamePlayer *pReceiver, UINT nTaskID );
		~CTask();

		typedef UINT ( CGamePlayer::* Action )();

		size_t	AddCmd( Action pFun );
		
		UINT	Execute();
		void	Reset() { m_indexCmd = 0; }

	protected:

		CTask();

		OnlineGameLib::Win32::CCriticalSection	m_csTask;

	private:

		CGamePlayer *m_pReceiver;

		typedef vector< ICommand * > stdVector;

		stdVector	m_stdCommand;
		size_t		m_indexCmd;

		UINT		m_nTaskProgID;

	};

	CTask	m_theLoginTask;
	CTask	m_theLogoutTask;
	CTask	m_theExchangeReginTask;

	void SetCurrentTask( UINT nID ) { m_nCurrentTaskID = nID; /*::InterlockedExchange( &m_nCurrentTaskID, nID );*/ }
	UINT GetCurrentTask() { return m_nCurrentTaskID; /*::InterlockedExchange( &m_nCurrentTaskID, m_nCurrentTaskID );*/ }

	UINT	m_nCurrentTaskID;
	
	void InitTaskProcessor();

	/*
	 * Information
	 */
	OnlineGameLib::Win32::_tstring	m_sAccountName;
	OnlineGameLib::Win32::_tstring	m_sPassword;
	DWORD							m_dwAccountID;

	static const int s_nRoleListCount;

	IClient *m_pAccSvrClient;
	IServer *m_pPlayerServer;
	IClient	*m_pDBRoleClient;

	/*
	 * class CDataQueue
	 */
	class CDataQueue
	{
	public:

		/*
		 * Struct
		 */
		typedef struct tagData
		{
			WPARAM wParam;
			LPARAM lParam;

		}DQ, NEAR *PDQ, FAR *LPDQ;

		explicit CDataQueue( size_t bufferSize = sizeof( DQ ), size_t maxFreeBuffers = 3 );
		
		~CDataQueue();

		/*
		 * Function
		 */
		bool	Set( LONG lnID, WPARAM wParam, LPARAM lParam );

		bool	Peek( LONG lnID, LPDQ lpDQ );
		bool	Get( LONG lnID, LPDQ lpDQ );

		void	Clear( LONG lnID );

		void	Empty();

	private:
		/*
		 * Data
		 */
		typedef map< LONG, OnlineGameLib::Win32::CBuffer * > stdDataMap;

		stdDataMap									m_theData;

		OnlineGameLib::Win32::CBuffer::Allocator	m_theDQAllocator;

		OnlineGameLib::Win32::CCriticalSection		m_csQueue;
	};

	CDataQueue	m_theDataQueue[enumOwnerTotal];

	OnlineGameLib::Win32::CPackager	m_thePackReceiver;
	
protected:

	/*
	 * Helper function
	 */
	bool TaskCompleted();

	bool SmallPackProcess( UINT nOwner, const void *pData, size_t dataLength );
	bool LargePackProcess( UINT nOwner, const void *pData, size_t dataLength );
	
	bool DispatchTaskForAccount( const void *pData, size_t dataLength );
	bool DispatchTaskForDBRole( const void *pData, size_t dataLength );
	bool DispatchTaskForPlayer( const void *pData, size_t dataLength );

	/*
	 * Task function
	 */
	UINT QueryAccPwd();
	UINT VerifyAccount();
	UINT QueryRoleList();
	UINT ProcessRoleList();
	UINT SelectRole();
	UINT ProcessRoleInfo();

	/*
	 * Help function for task
	 */
	bool _VerifyAccount_ToPlayer( UINT nQueryResult );
	bool _QueryRoleInfo_ToDBRole( const char *pRoleName );
	bool _SyncRoleInfo_ToGameServer( const void *pData, size_t dataLength );
	
};

#endif // __INCLUDE_GAMEPLAYER_H__