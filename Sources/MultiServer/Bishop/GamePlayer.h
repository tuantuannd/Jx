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

#include "CriticalSection.h"
#include "tstring.h"
#include "Buffer.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "IGServer.h"
#include "GameServer.h"

#include "PlayerCreator.h"

#include <vector>
#include <map>
#include <list>

using namespace std;

/*
 * class CGamePlayer
 */
class CGamePlayer : public IPlayer
{
public:

	CGamePlayer( UINT nIdentityID = ( UINT )( -1 ) );

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
		enumSelAddDelRole,
		enumLoginCreateRole,
		enumLoginSelectRole,
		enumLoginDeleteRole,
		
		enumSafeClose,

		/*
		 * Task status
		 */
		enumToGotoTask,
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
	virtual bool Run();
	virtual bool AppendData( UINT nOwner, const void *pData, size_t dataLength );
	
	static bool SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers );

	virtual bool Attach( const char *pRoleName );

	virtual bool IsActive() { return m_bActiveStatus; };
	virtual bool Active();
	virtual bool Inactive();

	static bool Add( const char *pRoleName, IPlayer *pPlayer );
	static bool Del( const char *pRoleName );
	static IPlayer *Get( const char *pRoleName );

	static void ATTACH_NETWORK( IClient *pAccSvrClient, 
				IServer *pPlayerServer, 
				IClient	*pDBRoleClient );

	static void DETACH_NETWORK();

	static LONG GetCapability() { return ::InterlockedExchange( &m_slnIdentityCounts, m_slnIdentityCounts ); }
	static LONG GetWorking() { return ::InterlockedExchange( &m_lnWorkingCounts, m_lnWorkingCounts ); }

private:

	static OnlineGameLib::Win32::CBuffer::Allocator	m_theGlobalAllocator;

	static LONG m_slnIdentityCounts;
	LONG		m_lnIdentityID;
	
	static LONG m_lnWorkingCounts;

	int m_nExtPoint;

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

		//OnlineGameLib::Win32::CCriticalSection	m_csTask;

	private:

		CGamePlayer *m_pReceiver;

		typedef vector< ICommand * > stdVector;

		stdVector	m_stdCommand;
		size_t		m_indexCmd;

		UINT		m_nTaskProgID;

	};

	CTask	m_theLoginTask;

	CTask	m_theSelAddDelTask;

	CTask	m_theLoginCreateRoleTask;
	CTask	m_theLoginSelectRoleTask;
	CTask	m_theLoginDeleteRoleTask;

	CTask	m_theSafeCloseTask;

	void SetCurrentTask( LONG nID ) { /* m_nCurrentTaskID = nID; */ ::InterlockedExchange( &m_nCurrentTaskID, nID ); }
	UINT GetCurrentTask() { return /* m_nCurrentTaskID; */ ::InterlockedExchange( &m_nCurrentTaskID, m_nCurrentTaskID ); }

	LONG m_nCurrentTaskID;
	
	void InitTaskProcessor();

	/*
	 * Information
	 */

	OnlineGameLib::Win32::_tstring	m_sAccountName;
	OnlineGameLib::Win32::_tstring	m_sPassword;
	OnlineGameLib::Win32::_tstring	m_sRoleName;

    bool    m_bUseSuperPassword;
    OnlineGameLib::Win32::_tstring	m_sSuperPassword;
    OnlineGameLib::Win32::_tstring	m_sDelRoleName;

	size_t	m_nAttachServerID;
	
	bool	m_bActiveStatus;

	DWORD	m_dwTaskBeginTimer;
	DWORD	m_dwTaskTotalTimer;
	
	bool	m_bAutoUnlockAccount;

	static OnlineGameLib::Win32::CCriticalSection CGamePlayer::m_csMapSP;

	typedef map< OnlineGameLib::Win32::_tstring, IPlayer * > stdMapSP;

	static stdMapSP	m_sthePlayerTable;

	static const int s_nRoleListCount;
	static const int s_nLoginTimeoutTimer;
	static const int s_nProcessTimeoutTimer;

//	static IClient *m_pAccSvrClient;
	static IServer *m_pPlayerServer;
	static IClient *m_pDBRoleClient;

	/*
	 * class CDataQueue
	 */
	class CDataQueue
	{
	public:

		explicit CDataQueue( size_t bufferSize = 1024 * 64, size_t maxFreeBuffers = 1 );
		
		~CDataQueue();

		/*
		 * Function
		 */
		bool		AddData( LONG lnID, const BYTE *pData, size_t datalength );

		OnlineGameLib::Win32::CBuffer *Attach( LONG lnID );
		void		Detach( LONG lnID );

		void		Clear( LONG lnID );

		void		Empty();

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

	OnlineGameLib::Win32::CPackager			m_thePackager;

	static CPlayerCreator					m_thePlayerCreator;
	
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
	// {{

		/*
		 * Login
		 */
		UINT WaitForAccPwd();
		UINT QueryAccPwd();
		UINT VerifyAccount();
		UINT QueryRoleList();
		UINT ProcessRoleList();
		UINT SelAddDelRole();
		UINT DelRole_WaitForVerify();
		
		UINT WaitForCreateResult();

		UINT WaitForDeleteResult();

		UINT ProcessRoleInfo();
		UINT WaitForGameSvrPermit();

		/*
		 * Logout
		 */

	// }}

	UINT SafeClose();

	/*
	 * Help function for task
	 */
	void _ClearTaskQueue();

	bool _VerifyAccount_ToPlayer( UINT nQueryResult , unsigned long nLeftTime);
	bool _QueryRoleInfo_ToDBRole( const char *pRoleName );
	bool _SyncRoleInfo_ToGameServer( const void *pData, size_t dataLength );

	bool _CreateNewPlayer_ToDBRole( const char *pRoleName,
		int nRoleSex /* male or female */, 
		int nRoleClass, 
		unsigned short nMapID );

	UINT _DeleteRole_ToDBRole( const void *pData, size_t dataLength );

	bool _UnlockAccount();
	
};

#endif // __INCLUDE_GAMEPLAYER_H__