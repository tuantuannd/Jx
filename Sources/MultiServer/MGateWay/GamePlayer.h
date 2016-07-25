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

#pragma warning(disable : 4786)  // identifier was truncated to '255' characters 
                                 // in the debug information

#include "CriticalSection.h"
#include "tstring.h"
#include "Buffer.h"

#include "IClient.h"
#include "RainbowInterface.h"
#include "IServer.h"
#include "HeavenInterface.h"

#include "S3DBInterface.h"
#include "KRoleStreamArtery.h"



#include <vector>
#include <map>

using namespace std;

typedef vector< void* >	stdVector;
typedef map< LONG, const void * > stdMap;

/*
 * class CGamePlayer
 */
class CGamePlayer : public IPlayer
{
public:
	
	explicit CGamePlayer( IServer *pPlayerSender, IClient *pRoleDBSender, LONG nIdentityID = -1 );
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
		enumComplete,
		enumRepeat,
		enumNone
	};
	
	virtual bool DispatchTask( LONG lnTaskID );
	
	virtual bool IsWorking() { return false; }

	virtual int Run();
	
	virtual bool AppendData( const void *pData );

	virtual bool Create( const char * const pName, const char * const pPassword );

	virtual bool Destroy( const char * const pPassword );

	static bool SetupGlobalAllocator( size_t bufferSize, size_t maxFreeBuffers );
	
private:

	/*
	 * Event option
	 */
	enum
	{
		enumQuitEvent = 0,	// the first value must be set zero
		enumWorkEvent,
		enumEventTotal		// must put it on last, be used to total counts of variable
	};

	static LONG m_slnIdentityCounts;
	LONG		m_lnIdentityID;

	bool ( CGamePlayer::* EventProcessor[enumEventTotal] )();

	void InitEventProcessor();
	
	/*
	 * Task system
	 */
	class CTask
	{
	public:
		explicit CTask( CGamePlayer *pReceiver, UINT nTaskID, UINT nNextTaskID = enumComplete );
		~CTask();

		typedef bool ( CGamePlayer::* Action )();

		size_t	AddCmd( Action pFun, UINT nFaildValue = enumNone );
		
		UINT	Execute();
		void	Reset() { m_indexCmd = 0; }

	protected:
		CTask();

		OnlineGameLib::Win32::CCriticalSection	m_cs;

	private:		
		CGamePlayer *m_pReceiver;

		stdVector	m_stdCommand;
		size_t		m_indexCmd;

		UINT		m_nTaskProgID;
		UINT		m_nNextTaskID;

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

	OnlineGameLib::Win32::_tstring	m_sBlockBuffer;	// temporal variable

	static OnlineGameLib::Win32::CBuffer::Allocator	m_theGlobalAllocator;

	enum
	{
		enumRoleListCount = 3
	};

	S3DBI_RoleBaseInfo	m_theRoleDBList[enumRoleListCount];

	struct tagRoleInfomation
	{
		int			nIndexInList;
		TRoleData	theRoleData;
	};

	tagRoleInfomation	m_theRoleInfomation;
	
	/*
	 * Network
	 */
	/*
	 * The role dabase
	 */
	IClient	*m_pRoleDBSender;
	IServer	*m_pPlayerServer;

	KRoleStreamVein	m_theRoleStreamVein;

	class CDataQueue
	{
	private:
		stdMap	m_theData;

		OnlineGameLib::Win32::CCriticalSection	m_csQueue;

	public:
		~CDataQueue();

		void		Empty();

		size_t		Push( LONG lnID, const void * pData );
		
		const void *Top( LONG lnID );
		size_t		Pop( LONG lnID );
	};

	CDataQueue	m_theDataQueue;

protected:

	/*
	 * Event function
	 */
	bool Quit();
	bool Work();

	/*
	 * Helper function
	 */
	bool TaskCompleted();
	
	/*
	 * Task function
	 */
	bool QueryAccPwd();
	bool VerifyAccount();
	bool QueryRoleList();
	bool GetRoleListInfo();
	bool QueryRoleInfo();
	bool ProcessRoleInfo();

};

#endif // __INCLUDE_GAMEPLAYER_H__