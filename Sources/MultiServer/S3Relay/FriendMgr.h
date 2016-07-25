// FriendMgr.h: interface for the CFriendMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRIENDMGR_H__FC02470F_E921_4A06_8D80_0073F81DB6D9__INCLUDED_)
#define AFX_FRIENDMGR_H__FC02470F_E921_4A06_8D80_0073F81DB6D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include "DBTable.h"
#include <list>
#include <vector>

class CFriendMgr  
{
public:
	CFriendMgr();
	virtual ~CFriendMgr();

public:
	BOOL Initialize();
	BOOL Uninitialize();

public:
	BOOL SomeoneLogin(const std::string& role);
	BOOL SomeoneLogout(const std::string& role);
	BOOL SomeoneSyncFriends(const std::string& role, unsigned long param, DWORD nameid, const CNetConnectDup& conndup, BOOL full, BYTE pckgid);

	BOOL PlayerAddFriend(const std::_tstring& someone, const std::_tstring& dst);
	BOOL PlayerDelFriend(const std::_tstring& someone, const std::_tstring& dst);
	BOOL PlayerIsFriend(const std::_tstring& someone, const std::_tstring& dst, BOOL rev = FALSE);

	BOOL SetFriendGroup(const std::_tstring& someone, const std::_tstring& dst, const std::_tstring& group);
	std::_tstring GetFriendGroup(const std::_tstring& someone, const std::_tstring& dst);

private:
	CLockMRSW m_lockFriend;

	UINT m_elapseUpdateDB;
	size_t m_StorePlayerPerUdtDB;
	size_t m_LoadPlayerPerUdtDB;

	struct FRIENDINFO
	{
		std::_tstring group;
		bool cheating;
		bool* pCheated;
		enum {cheatNONE = -1, cheatFALSE = 0, cheatTRUE = 1} orig_cheating, orig_cheated;	//is valid when !-1 && loaded

		FRIENDINFO() : cheating(true), pCheated(NULL), orig_cheating(cheatNONE), orig_cheated(cheatNONE) {}
	};
	typedef std::map<std::_tstring, FRIENDINFO, _tstring_less>	FRIENDSMAP;

	struct PLAYERINFO
	{
		BOOL loaded;
		BOOL dirty;
		FRIENDSMAP friends;

		PLAYERINFO() : loaded(FALSE), dirty(FALSE) {}
	};
	typedef std::map<std::_tstring, PLAYERINFO, _tstring_less>	PLAYERSMAP;

	PLAYERSMAP m_mapPlayers;

private:
	PLAYERSMAP::iterator _LoadSomeone(const std::_tstring& someone);
	BOOL _StoreSomeone(PLAYERSMAP::iterator itSomeone);

private:
	struct ADDFRIENDKEY
	{
		BYTE pckgid;
		std::_tstring src;

		ADDFRIENDKEY() : pckgid(-1) {}
		ADDFRIENDKEY(BYTE pi, const std::_tstring& s) : pckgid(pi), src(s) {}

		bool operator<(const ADDFRIENDKEY& s) const
		{return pckgid < s.pckgid || (pckgid == s.pckgid && _tstring_less()(src, s.src));}
	};
	struct ADDFRIENDVALUE
	{
		std::_tstring dst;
		DWORD tick;
	};
	typedef std::map<ADDFRIENDKEY, ADDFRIENDVALUE>	ADDFRIENDMAP;

	ADDFRIENDMAP	m_mapAddFriends;
	DWORD			m_tickLast;
	CLockMRSW		m_lockAddFriend;

public:
	BOOL TrackAddFriend(const std::_tstring& src, const std::_tstring& dst, BYTE id);
	BOOL ExtractAddFriend(const std::_tstring& src, const std::_tstring& dst, BYTE id);
	BOOL TidyAddFriend();


private:
	CLockMRSW m_lockDB;
	ZDBTable m_dbFriend;
	CDBTableReadOnly m_dbFriendRO;

	struct MEM_FRIENDRECORD
	{
		std::_tstring name;
		std::_tstring group;
		bool cheating;
		bool cheated;
	};
	typedef std::list<MEM_FRIENDRECORD>	MEM_FRIENDRECORDLIST;

//Berkeley DB data
#pragma pack(push, 1)
	struct _BASIC_STR
	{
		BYTE strlen;
		//char str[strlen];
	};
	struct DB_FRIENDRECORD
	{
		bool cheating;
		bool cheated;
		//_BASIC_STR name;
		//_BASIC_STR group;
	};
	struct DB_FRIENDRECORDLIST
	{
		BYTE count;
		//DB_FRIENDRECORD friends[count];
	};
#pragma pack(pop)

private:
	std::_tstring NameToDBKey(const std::_tstring& name);

private:
	BOOL DB_AddSomeone(const std::_tstring& someone, const MEM_FRIENDRECORDLIST& memFriends);
	BOOL DB_DeleteSomeone(const std::_tstring& someone);
	BOOL DB_LoadSomeone(const std::_tstring& someone, MEM_FRIENDRECORDLIST* pmemFriends);
	BOOL DB_StoreSomeone(const std::_tstring& someone, const MEM_FRIENDRECORDLIST& memFriends);

private:
	struct ONEPLAYER
	{
		std::_tstring someone;
		MEM_FRIENDRECORDLIST listFriends;
	};
	typedef std::list<ONEPLAYER>	ONEPLAYERLIST;

private:
	size_t m_cursorBkgrndUdtDB;

public:
	BOOL BkgrndUpdateDB();

public:
	size_t GetPlayerCount();
	size_t GetAFQSize();

public:
	BOOL PlayerAssociate(const std::_tstring& someone, const std::_tstring& dst, const std::_tstring& group, BYTE bidir);

	typedef std::vector<std::_tstring>	_BEVY;
	BOOL PlayerAssociateBevy(const _BEVY& bevy, const std::_tstring& group);

};



#endif // !defined(AFX_FRIENDMGR_H__FC02470F_E921_4A06_8D80_0073F81DB6D9__INCLUDED_)
