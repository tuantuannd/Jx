// FriendMgr.cpp: implementation of the CFriendMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "global.h"
#include "FriendMgr.h"
#include "assert.h"
#include "malloc.h"
#include "S3Relay.h"

#include <list>
#include <map>


//////////////////////////////////////////////////////////////////////

#define STR_NULL	_T("")

//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////


static char file_friendcfg[]			= "relay_friendcfg.ini";

const char sec_database[]				= "database";
const char key_path[]					= "path";
const char key_file[]					= "file";
const char key_updateinterval[]			= "updateinterval";
const char key_storeplayerperudtdb[]	= "storeplayerperudtdb";
const char key_loadplayerperudtdb[]		= "loadplayerperudtdb";


//////////////////////////////////////////////////////////////////////

static std::_tstring s_dbpath = gGetPrivateProfileStringEx(sec_database, key_path, file_friendcfg);
static std::_tstring s_dbfile = gGetPrivateProfileStringEx(sec_database, key_file, file_friendcfg);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFriendMgr::CFriendMgr()
	: m_elapseUpdateDB(0), m_tickLast(0),
	m_StorePlayerPerUdtDB(0), m_LoadPlayerPerUdtDB(0), 
	m_dbFriend(s_dbpath.c_str(), s_dbfile.c_str()),
	m_dbFriendRO(s_dbpath.c_str(), s_dbfile.c_str()),
	m_cursorBkgrndUdtDB(0)
{

}

CFriendMgr::~CFriendMgr()
{

}


BOOL CFriendMgr::Initialize()
{
	//open DB
	{{
	AUTOLOCKWRITE(m_lockDB);

	if (!m_dbFriend.open() || !m_dbFriendRO.open())
		return FALSE;
	}}

	{{
	AUTOLOCKWRITE(m_lockFriend);

	m_mapPlayers.clear();
	}}


	m_cursorBkgrndUdtDB = 0;

	m_StorePlayerPerUdtDB = (size_t)gGetPrivateProfileIntEx(sec_database, key_storeplayerperudtdb, file_friendcfg, 0);
	m_LoadPlayerPerUdtDB = (size_t)gGetPrivateProfileIntEx(sec_database, key_loadplayerperudtdb, file_friendcfg, 0);

	m_elapseUpdateDB = (UINT)gGetPrivateProfileIntEx(sec_database, key_updateinterval, file_friendcfg, 0);
	if (m_elapseUpdateDB > 0)
	{
		if (!gStartFriendTimingUpdateDB(m_elapseUpdateDB))
			return FALSE;
	}


	return TRUE;
}

BOOL CFriendMgr::Uninitialize()
{
	if (m_elapseUpdateDB > 0)
		gStopFriendTimingUpdateDB();

	//update DB
	{{
	AUTOLOCKWRITE(m_lockFriend);

	for (PLAYERSMAP::iterator itPlayer = m_mapPlayers.begin(); itPlayer != m_mapPlayers.end(); itPlayer++)
	{
		std::_tstring someone((*itPlayer).first);
		PLAYERINFO& rPlayerInfo = (*itPlayer).second;

		if (!rPlayerInfo.loaded)
		{
			PLAYERSMAP::iterator itLoadPlayer = _LoadSomeone(someone);
			assert(itLoadPlayer == itPlayer);
		}

		_StoreSomeone(itPlayer);
	}

	m_mapPlayers.clear();
	}}

	m_cursorBkgrndUdtDB = 0;


	//close DB
	{{
	AUTOLOCKWRITE(m_lockDB);

	m_dbFriendRO.close();
	m_dbFriend.close();
	}}

	return TRUE;
}


BOOL CFriendMgr::SomeoneLogin(const std::string& role)
{
	//use for notify online friends
	typedef std::list<tagPlusSrcInfo>	PLAYERLIST;
	typedef std::map<DWORD, PLAYERLIST>	IP2PLAYERSMAP;
	IP2PLAYERSMAP mapIp2Players;


	{{
	AUTOLOCKWRITE(m_lockFriend);

	PLAYERSMAP::iterator itPlayer = _LoadSomeone(role);
	if (itPlayer == m_mapPlayers.end())
		return FALSE;

	PLAYERINFO& rPlayerInfo = (*itPlayer).second;
	assert(rPlayerInfo.loaded);



	for (FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin(); itFriend != rPlayerInfo.friends.end(); itFriend++)
	{
		std::_tstring dst = (*itFriend).first;
		FRIENDINFO& rInfo = (*itFriend).second;

		assert(rInfo.pCheated);
		if (!rInfo.pCheated)
			continue;

		assert(!rInfo.cheating || !*rInfo.pCheated);
		if (rInfo.cheating && *rInfo.pCheated)
			continue;


		if (!*rInfo.pCheated)
		{//this friend is sincere for someone
			CNetConnectDup conndup2;
			DWORD nameid2 = 0;
			unsigned long param2 = 0;
			if (g_HostServer.FindPlayerByRole(NULL, dst, &conndup2, NULL, &nameid2, &param2))
			{//this friend is online
				tagPlusSrcInfo srcinfo;
				srcinfo.nameid = nameid2;
				srcinfo.lnID = param2;

				PLAYERLIST& rPlayers = mapIp2Players[conndup2.GetIP()];
				rPlayers.push_back(srcinfo);
			}
		}
	}

	}}



	if (!mapIp2Players.empty())
	{
		char buffer2[max_packagesize];
		size_t datasize2 = sizeof(FRIEND_STATE) + role.size() + 2;
		size_t basesize2 = sizeof(EXTEND_PASSTOBEVY) + datasize2;

		EXTEND_PASSTOBEVY* pEpb2 = (EXTEND_PASSTOBEVY*)buffer2;
		pEpb2->ProtocolFamily = pf_extend;
		pEpb2->ProtocolID = extend_s2c_passtobevy;
		pEpb2->datasize = datasize2;
		pEpb2->playercount = 0;

		FRIEND_STATE* pFs2 = (FRIEND_STATE*)(pEpb2 + 1);
		pFs2->ProtocolFamily = pf_friend;
		pFs2->ProtocolID = friend_s2c_friendstate;
		pFs2->state = stateOnline;

		char* pSrcRole2 = (char*)(pFs2 + 1);
		strcpy(pSrcRole2, role.c_str());
		pSrcRole2[role.size() + 1] = 0;

		tagPlusSrcInfo* pPlayer2 = (tagPlusSrcInfo*)(buffer2 + basesize2);


		for (IP2PLAYERSMAP::iterator itIP = mapIp2Players.begin(); itIP != mapIp2Players.end(); itIP++)
		{
			DWORD ip2 = (*itIP).first;
			PLAYERLIST& rPlayers = (*itIP).second;

			CNetConnectDup tongconndup2;
			tongconndup2 = g_TongServer.FindTongConnectByIP(ip2);
			if (!tongconndup2.IsValid())
				continue;


			size_t cursor2 = basesize2;

			for (PLAYERLIST::iterator itPlayer = rPlayers.begin(); itPlayer != rPlayers.end(); itPlayer++)
			{
				tagPlusSrcInfo& rSrcInfo = *itPlayer;

				
				if (cursor2 + sizeof(tagPlusSrcInfo) > max_packagesize)
				{
					tongconndup2.SendPackage(buffer2, cursor2);

					pEpb2->playercount = 0;
					cursor2 = basesize2;
				}

				pPlayer2[pEpb2->playercount ++] = rSrcInfo;
				cursor2 += sizeof(tagPlusSrcInfo);
			}

			if (cursor2 > basesize2)
			{
				assert(pEpb2->playercount > 0);
				tongconndup2.SendPackage(buffer2, cursor2);
			}
		}
	}


	return TRUE;
}

BOOL CFriendMgr::SomeoneLogout(const std::string& role)
{
	AUTOLOCKWRITE(m_lockFriend);

	PLAYERSMAP::iterator itPlayer = m_mapPlayers.find(role);
	if (itPlayer == m_mapPlayers.end())
		return FALSE;


	PLAYERINFO& rPlayerInfo = (*itPlayer).second;
	assert(rPlayerInfo.loaded);


	size_t datasize2 = sizeof(FRIEND_STATE) + role.size() + 2;
	size_t pckgsize2 = sizeof(EXTEND_PASSTOSOMEONE) + datasize2;

	EXTEND_PASSTOSOMEONE* pEps2 = (EXTEND_PASSTOSOMEONE*)_alloca(pckgsize2);
	pEps2->ProtocolFamily = pf_extend;
	pEps2->ProtocolID = extend_s2c_passtosomeone;
	pEps2->datasize = datasize2;

	FRIEND_STATE* pFs2 = (FRIEND_STATE*)(pEps2 + 1);
	pFs2->ProtocolFamily = pf_friend;
	pFs2->ProtocolID = friend_s2c_friendstate;
	pFs2->state = stateOffline;

	char* pSrcRole2 = (char*)(pFs2 + 1);
	strcpy(pSrcRole2, role.c_str());
	pSrcRole2[role.size() + 1] = 0;


	for (FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin(); itFriend != rPlayerInfo.friends.end(); itFriend++)
	{
		std::_tstring dst = (*itFriend).first;
		FRIENDINFO& rInfo = (*itFriend).second;

		assert(rInfo.pCheated);
		if (!rInfo.pCheated)
			continue;

		assert(!rInfo.cheating || !*rInfo.pCheated);
		if (rInfo.cheating && *rInfo.pCheated)
			continue;

		if (!*rInfo.pCheated)
		{//notify
			CNetConnectDup conndup2;
			DWORD nameid2 = 0;
			unsigned long param2 = -1;
			if (g_HostServer.FindPlayerByRole(NULL, dst, &conndup2, NULL, &nameid2, &param2))
			{//this friend is online
				CNetConnectDup tongconndup2 = g_TongServer.FindTongConnectByIP(conndup2.GetIP());
				if (tongconndup2.IsValid())
				{
					pEps2->nameid = nameid2;
					pEps2->lnID = param2;

					tongconndup2.SendPackage(pEps2, pckgsize2);
				}
			}
		}
	}


	//不能并发写，因此这里不写DB
	//_StoreSomeone(itPlayer);


	return TRUE;
}


BOOL CFriendMgr::SomeoneSyncFriends(const std::string& role, unsigned long param, DWORD nameid, const CNetConnectDup& conndup, BOOL full, BYTE pckgid)
{
	typedef std::pair<BYTE, std::_tstring>	THEFRIEND;	//first: state, second: name
	typedef std::list<THEFRIEND>	FRIENDLIST;
	typedef std::map<std::_tstring, FRIENDLIST, _tstring_less>	GRP2FRIENDSMAP;

	GRP2FRIENDSMAP mapGrp2Friends;

	{{
	AUTOLOCKREAD(m_lockFriend);

	PLAYERSMAP::iterator itPlayer = m_mapPlayers.find(role);
	if (itPlayer == m_mapPlayers.end())
		return FALSE;

	PLAYERINFO& rPlayerInfo = (*itPlayer).second;
	assert(rPlayerInfo.loaded);
	if (!rPlayerInfo.loaded)
		return FALSE;


	for (FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin(); itFriend != rPlayerInfo.friends.end(); itFriend++)
	{
		std::_tstring dst = (*itFriend).first;
		FRIENDINFO& rInfo = (*itFriend).second;

		assert(rInfo.pCheated);
		if (!rInfo.pCheated)
			continue;

		assert(!rInfo.cheating || !*rInfo.pCheated);
		if (rInfo.cheating && *rInfo.pCheated)
			continue;

		if (!rInfo.cheating)
		{
			BOOL online = g_HostServer.FindPlayerByRole(NULL, dst, NULL, NULL, NULL, NULL);

			if (!online && !full)
				continue;

			FRIENDLIST& rFriends = mapGrp2Friends[rInfo.group];
			rFriends.push_back(THEFRIEND(online ? stateOnline : stateOffline, dst));
		}
	}
	}}


	if (!mapGrp2Friends.empty())
	{
		char buffer[max_packagesize];
		size_t basesize = sizeof(EXTEND_PASSTOSOMEONE) + sizeof(REP_SYNCFRIENDLIST);

		EXTEND_PASSTOSOMEONE* pEps = (EXTEND_PASSTOSOMEONE*)buffer;
		pEps->ProtocolFamily = pf_extend;
		pEps->ProtocolID = extend_s2c_passtosomeone;
		pEps->nameid = nameid;
		pEps->lnID = param;

		REP_SYNCFRIENDLIST* pRsfl = (REP_SYNCFRIENDLIST*)(pEps + 1);
		pRsfl->ProtocolFamily = pf_friend;
		pRsfl->ProtocolID = friend_s2c_repsyncfriendlist;
		pRsfl->pckgid = pckgid;


		for (GRP2FRIENDSMAP::iterator itGrp = mapGrp2Friends.begin(); itGrp != mapGrp2Friends.end(); itGrp++)
		{
			size_t basecursor = basesize;

			std::_tstring grp = (*itGrp).first;
			FRIENDLIST& rFriends = (*itGrp).second;

			buffer[basecursor++] = specGroup;
			strcpy(buffer + basecursor, grp.c_str());
			basecursor += grp.size() + 1;


			size_t cursor = basecursor;

			for (FRIENDLIST::iterator itFriend = rFriends.begin(); itFriend != rFriends.end(); itFriend++)
			{
				THEFRIEND theFriend = *itFriend;

				//					spec	state	name						0
				size_t appendsize = 1	+	1	+	theFriend.second.size()	+	1;

				//				item			specOver
				if (cursor	+	appendsize	+	1	>	 max_packagesize)
				{
					buffer[cursor++] = specOver;
					pEps->datasize = cursor - sizeof(EXTEND_PASSTOSOMEONE);

					conndup.SendPackage(buffer, cursor);

					cursor = basecursor;
				}

				buffer[cursor + 0] = specRole;
				buffer[cursor + 1] = theFriend.first;
				strcpy(buffer + cursor + 2, theFriend.second.c_str());
				cursor += appendsize;
			}

			if (cursor > basecursor)
			{
				buffer[cursor++] = specOver;
				pEps->datasize = cursor - sizeof(EXTEND_PASSTOSOMEONE);

				conndup.SendPackage(buffer, cursor);
			}
		}
	}

	rTRACE("sync friend list: %s (%s)", role.c_str(), full ? "full" : "part");


	return TRUE;
}


BOOL CFriendMgr::PlayerAddFriend(const std::_tstring& someone, const std::_tstring& dst)
{
	if (_tstring_equal()(someone, dst))
		return FALSE;

	AUTOLOCKWRITE(m_lockFriend);

	PLAYERINFO& rSrcPlayerInfo = m_mapPlayers[someone];
	assert(rSrcPlayerInfo.loaded);
	FRIENDINFO& rSrcInfo = rSrcPlayerInfo.friends[dst];
	rSrcInfo.cheating = false;

	PLAYERINFO& rDstPlayerInfo = m_mapPlayers[dst];
	FRIENDINFO& rDstInfo = rDstPlayerInfo.friends[someone];
	rDstInfo.cheating = false;

	//binding
	rSrcInfo.pCheated = &rDstInfo.cheating;
	rDstInfo.pCheated = &rSrcInfo.cheating;


	rTRACE("Player Add Friend: %s [%s]", someone.c_str(), dst.c_str());

	return TRUE;
}

BOOL CFriendMgr::PlayerDelFriend(const std::_tstring& someone, const std::_tstring& dst)
{
	AUTOLOCKWRITE(m_lockFriend);

	PLAYERSMAP::iterator itSrcPlayer = m_mapPlayers.find(someone);
	if (itSrcPlayer == m_mapPlayers.end())
		return FALSE;

	PLAYERINFO& rSrcPlayerInfo = (*itSrcPlayer).second;

	assert(rSrcPlayerInfo.loaded);
	if (!rSrcPlayerInfo.loaded)
		return FALSE;

	FRIENDSMAP::iterator itSrcFriend = rSrcPlayerInfo.friends.find(dst);
	if (itSrcFriend == rSrcPlayerInfo.friends.end())
		return FALSE;

	FRIENDINFO& rSrcInfo = (*itSrcFriend).second;

	assert(rSrcInfo.pCheated);
	if (!rSrcInfo.pCheated)
		return FALSE;

	//assert(!rSrcInfo.cheating || !*rSrcInfo.pCheated);
	//if (rSrcInfo.cheating && *rSrcInfo.pCheated)
	//	return FALSE;
	
	rSrcInfo.cheating = true;
	if (!*rSrcInfo.pCheated)
		return TRUE;

	//erase...
	//dst
	PLAYERSMAP::iterator itDstPlayer = m_mapPlayers.find(dst);
	if (itDstPlayer != m_mapPlayers.end())
	{
		PLAYERINFO& rDstPlayerInfo = (*itDstPlayer).second;
		if (rDstPlayerInfo.loaded)
			rDstPlayerInfo.friends.erase(someone);
		else
		{
			FRIENDSMAP::iterator itDstFriend = rDstPlayerInfo.friends.find(someone);
			if (itDstFriend != rDstPlayerInfo.friends.end())
			{
				FRIENDINFO& rDstInfo = (*itDstFriend).second;
				rDstInfo.pCheated = NULL;
			}
		}
	}

	//src
	rSrcPlayerInfo.friends.erase(itSrcFriend);


	rTRACE("Player Del Friend: %s [%s]", someone.c_str(), dst.c_str());

	return TRUE;
}

BOOL CFriendMgr::PlayerIsFriend(const std::_tstring& someone, const std::_tstring& dst, BOOL rev)
{
	AUTOLOCKREAD(m_lockFriend);

	PLAYERSMAP::iterator itPlayer = m_mapPlayers.find(someone);
	if (itPlayer == m_mapPlayers.end())
		return FALSE;

	PLAYERINFO& rPlayerInfo = (*itPlayer).second;
	FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.find(dst);
	if (itFriend == rPlayerInfo.friends.end())
		return FALSE;

	FRIENDINFO& rInfo = (*itFriend).second;
	if (!rInfo.pCheated)
		return FALSE;

	return !(rev ? *rInfo.pCheated : rInfo.cheating);
}


BOOL CFriendMgr::SetFriendGroup(const std::_tstring& someone, const std::_tstring& dst, const std::_tstring& group)
{
	AUTOLOCKWRITE(m_lockFriend);

	PLAYERSMAP::iterator itPlayer = m_mapPlayers.find(someone);
	if (itPlayer == m_mapPlayers.end())
		return FALSE;

	PLAYERINFO& rPlayerInfo = (*itPlayer).second;
	FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.find(dst);
	if (itFriend == rPlayerInfo.friends.end())
		return FALSE;

	FRIENDINFO& rInfo = (*itFriend).second;
	assert(rInfo.pCheated);
	if (rInfo.pCheated == NULL)
		return FALSE;

	assert(!rInfo.cheating || !*rInfo.pCheated);
	if (rInfo.cheating && *rInfo.pCheated)
		return FALSE;

	rInfo.group = group;

	rTRACE("Player Group Friend: %s [%s] (%s)", someone.c_str(), dst.c_str(), group.c_str());

	return TRUE;
}

std::_tstring CFriendMgr::GetFriendGroup(const std::_tstring& someone, const std::_tstring& dst)
{
	AUTOLOCKREAD(m_lockFriend);

	PLAYERSMAP::iterator itPlayer = m_mapPlayers.find(someone);
	if (itPlayer == m_mapPlayers.end())
		return STR_NULL;

	PLAYERINFO& rPlayerInfo = (*itPlayer).second;
	FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.find(dst);
	if (itFriend == rPlayerInfo.friends.end())
		return STR_NULL;

	FRIENDINFO& rInfo = (*itFriend).second;
	assert(rInfo.pCheated);
	if (!rInfo.pCheated)
		return STR_NULL;
	assert(!rInfo.cheating || !*rInfo.pCheated);
	if (rInfo.cheating && *rInfo.pCheated)
		return STR_NULL;

	return rInfo.group;
}


BOOL CFriendMgr::TrackAddFriend(const std::_tstring& src, const std::_tstring& dst, BYTE id)
{
	AUTOLOCKWRITE(m_lockAddFriend);

	ADDFRIENDVALUE& rValue = m_mapAddFriends[ADDFRIENDKEY(id, src)];
	rValue.dst = dst;
	rValue.tick = ::GetTickCount();

	return TRUE;
}

BOOL CFriendMgr::ExtractAddFriend(const std::_tstring& src, const std::_tstring& dst, BYTE id)
{
	AUTOLOCKWRITE(m_lockAddFriend);

	ADDFRIENDMAP::iterator it = m_mapAddFriends.find(ADDFRIENDKEY(id, src));
	if (it == m_mapAddFriends.end())
		return FALSE;

	ADDFRIENDVALUE& rValue = (*it).second;
	if (!_tstring_equal()(rValue.dst, dst))
		return FALSE;

	m_mapAddFriends.erase(it);

	return TRUE;
}

BOOL CFriendMgr::TidyAddFriend()
{
	return TRUE;

/*
	DWORD tick = ::GetTickCount();
	if (tick < m_tickLast + TIMEOUT_ADDFRIEND)
		return TRUE;


	AUTOLOCKWRITE(m_lockAddFriend);

	ADDFRIENDMAP::iterator it = m_mapAddFriends.begin();
	while (it != m_mapAddFriends.end())
	{
		ADDFRIENDMAP::iterator itMe = it++;

		ADDFRIENDVALUE& rValue = (*itMe).second;
		if (tick < rValue.tick + TIMEOUT_ADDFRIEND)
			continue;

		m_mapAddFriends.erase(itMe);
	}

	return TRUE;
*/
}



CFriendMgr::PLAYERSMAP::iterator CFriendMgr::_LoadSomeone(const std::_tstring& someone)
{
	//assert(m_lockFriend._IsWriting());

	std::pair<PLAYERSMAP::iterator, bool> insret = m_mapPlayers.insert(PLAYERSMAP::value_type(someone, PLAYERINFO()));

	PLAYERINFO& rPlayerInfo = (*insret.first).second;
	if (!insret.second)	//not insert someone
	{
		if (rPlayerInfo.loaded)
			return insret.first;	//already loaded
	}

	rTRACE("load someone: %s", someone.c_str());

	//load from DB
	MEM_FRIENDRECORDLIST memFriends;
	if (!DB_LoadSomeone(someone, &memFriends))
	{
		rTRACE("except: load someone fail: %s", someone.c_str());
		return m_mapPlayers.end();
	}

	//update
	for (MEM_FRIENDRECORDLIST::iterator itMemFriend = memFriends.begin(); itMemFriend != memFriends.end(); itMemFriend++)
	{
		MEM_FRIENDRECORD& rmemFriendRec = *itMemFriend;

		std::pair<FRIENDSMAP::iterator, bool> insretfriend = rPlayerInfo.friends.insert(FRIENDSMAP::value_type(rmemFriendRec.name, FRIENDINFO()));
		FRIENDINFO& rInfo = (*insretfriend.first).second;

		if (insretfriend.second)	//insert this friend
		{
			rInfo.group = rmemFriendRec.group;
			rInfo.cheating = rmemFriendRec.cheating;
			rInfo.orig_cheating = rmemFriendRec.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
			rInfo.orig_cheated = rmemFriendRec.cheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;


			PLAYERINFO& rPlayerInfo2 = m_mapPlayers[rmemFriendRec.name];
			FRIENDINFO& rInfo2 = rPlayerInfo2.friends[someone];
			rInfo2.cheating = rmemFriendRec.cheated;

			rInfo.pCheated = &rInfo2.cheating;
			rInfo2.pCheated = &rInfo.cheating;
		}
		else
		{
			if (rInfo.pCheated == NULL)
			{
				rPlayerInfo.friends.erase(insretfriend.first);

				rPlayerInfo.dirty = TRUE;

				continue;
			}

			rInfo.group = rmemFriendRec.group;
			rInfo.orig_cheating = rmemFriendRec.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
			rInfo.orig_cheated = rmemFriendRec.cheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
		}

	}

	//clearup invalid items
	FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin();
	while (itFriend != rPlayerInfo.friends.end())
	{
		FRIENDSMAP::iterator itHere = itFriend++;

		FRIENDINFO& rInfo = (*itHere).second;
		if (rInfo.pCheated == NULL)
		{
			rPlayerInfo.friends.erase(itHere);

			rPlayerInfo.dirty = TRUE;
		}
	}


	rPlayerInfo.loaded = TRUE;


	return insret.first;
}

BOOL CFriendMgr::_StoreSomeone(PLAYERSMAP::iterator itSomeone)
{
	//assert(m_lockFriend._IsRead());

	std::_tstring someone = (*itSomeone).first;
	PLAYERINFO& rPlayerInfo = (*itSomeone).second;

	assert(rPlayerInfo.loaded);
	if (!rPlayerInfo.loaded)
	{
		rTRACE("except: store someone outof mem: %s", someone.c_str());
		return FALSE;
	}


	rTRACE("store someone: %s", someone.c_str());


	BOOL needstore = rPlayerInfo.dirty;

	rPlayerInfo.dirty = FALSE;


	MEM_FRIENDRECORDLIST memFriends;

	for (FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin(); itFriend != rPlayerInfo.friends.end(); itFriend++)
	{
		std::_tstring name = (*itFriend).first;
		FRIENDINFO& rInfo = (*itFriend).second;

		assert(rInfo.pCheated);
		if (!rInfo.pCheated)
			continue;
		assert(!rInfo.cheating || !*rInfo.pCheated);
		if (rInfo.cheating && *rInfo.pCheated)
			continue;

		MEM_FRIENDRECORD memFriendRec;
		memFriendRec.name = name;
		memFriendRec.group = rInfo.group;
		memFriendRec.cheating = rInfo.cheating;
		memFriendRec.cheated = *rInfo.pCheated;

		memFriends.push_back(memFriendRec);

		if (!needstore
			&& ((rInfo.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE) != rInfo.orig_cheating
			|| (*rInfo.pCheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE) != rInfo.orig_cheated))
		{
			needstore = TRUE;
		}

		rInfo.orig_cheating = rInfo.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
		rInfo.orig_cheated = *rInfo.pCheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
	}


	if (!needstore)	//mem & db is sync
		return TRUE;

	//store this friend's info into DB
	if (!DB_StoreSomeone(someone, memFriends))
	{
		rPlayerInfo.dirty = TRUE;

		rTRACE("error: store someone: %s", someone.c_str());

		return FALSE;
	}

	return TRUE;
}


std::_tstring CFriendMgr::NameToDBKey(const std::_tstring& name)
{
	return name;
}


BOOL CFriendMgr::DB_AddSomeone(const std::_tstring& someone, const MEM_FRIENDRECORDLIST& memFriends)
{
	return DB_StoreSomeone(someone, memFriends);
}

BOOL CFriendMgr::DB_DeleteSomeone(const std::_tstring& someone)
{
	std::_tstring dbkey = NameToDBKey(someone);

	AUTOLOCKWRITE(m_lockDB);
	m_dbFriend.remove(dbkey.data(), dbkey.size());

	return TRUE;
}


BOOL CFriendMgr::DB_LoadSomeone(const std::_tstring& someone, MEM_FRIENDRECORDLIST* pmemFriends)
{
	assert(pmemFriends && pmemFriends->empty());

	std::_tstring dbkey = NameToDBKey(someone);

	int valsize = 0;
	char* pValue = m_dbFriendRO.search(dbkey.data(), dbkey.size(), valsize);
	if (!pValue)
		return TRUE;

	DB_FRIENDRECORDLIST* pdbFriendList = (DB_FRIENDRECORDLIST*)pValue;
	DB_FRIENDRECORD* pCursor = (DB_FRIENDRECORD*)(pdbFriendList + 1);

	for (BYTE i = 0; i < pdbFriendList->count; i++)
	{
		MEM_FRIENDRECORD memFriend;
		memFriend.cheating = pCursor->cheating;
		memFriend.cheated = pCursor->cheated;

		_BASIC_STR* pName = (_BASIC_STR*)(pCursor + 1);
		assert(pName->strlen > 0 && pName->strlen < _NAME_LEN);
		char* strName = (char*)(pName + 1);
		memFriend.name.assign(strName, pName->strlen);

		_BASIC_STR* pGroup = (_BASIC_STR*)(strName + pName->strlen);
		assert(pGroup->strlen >= 0 && pGroup->strlen < _NAME_LEN);
		char* strGroup = (char*)(pGroup + 1);
		memFriend.group.assign(strGroup, pGroup->strlen);

		pmemFriends->push_back(memFriend);

		pCursor = (DB_FRIENDRECORD*)(strGroup + pGroup->strlen);
	}

	delete []pValue;
	return TRUE;
}

BOOL CFriendMgr::DB_StoreSomeone(const std::_tstring& someone, const MEM_FRIENDRECORDLIST& memFriends)
{
	assert(!someone.empty());

	size_t cntFriends = memFriends.size();

	if (cntFriends <= 0)
	{//erase this someone record, because it's empty
		DB_DeleteSomeone(someone);
		return TRUE;
	}

	size_t sizeMax = sizeof(DB_FRIENDRECORD) + (sizeof(_BASIC_STR) + _NAME_LEN) * cntFriends;

	DB_FRIENDRECORDLIST* pdbFriendList = (DB_FRIENDRECORDLIST*)_alloca(sizeMax);
	pdbFriendList->count = cntFriends;

	DB_FRIENDRECORD* pCursor = (DB_FRIENDRECORD*)(pdbFriendList + 1);
	for (MEM_FRIENDRECORDLIST::const_iterator it = memFriends.begin(); it != memFriends.end(); it++)
	{
		const MEM_FRIENDRECORD& rmemFriendRec = *it;
		assert(rmemFriendRec.name.size() > 0 && rmemFriendRec.name.size() < _NAME_LEN);
		assert(rmemFriendRec.group.size() >= 0 && rmemFriendRec.group.size() < _NAME_LEN);

		pCursor->cheating = rmemFriendRec.cheating;
		pCursor->cheated = rmemFriendRec.cheated;

		_BASIC_STR* pName = (_BASIC_STR*)(pCursor + 1);
		pName->strlen = rmemFriendRec.name.size();
		char* strName = (char*)(pName + 1);
		memcpy(strName, rmemFriendRec.name.data(), rmemFriendRec.name.size());

		_BASIC_STR* pGroup = (_BASIC_STR*)(strName + rmemFriendRec.name.size());
		pGroup->strlen = rmemFriendRec.group.size();
		char* strGroup = (char*)(pGroup + 1);
		memcpy(strGroup, rmemFriendRec.group.data(), rmemFriendRec.group.size());

		pCursor = (DB_FRIENDRECORD*)(strGroup + rmemFriendRec.group.size());


		assert((BYTE*)pCursor <= (BYTE*)pdbFriendList + sizeMax);
	}

	size_t sizeUsed = (BYTE*)pCursor - (BYTE*)pdbFriendList;
	assert(sizeUsed > 0);

	std::_tstring dbkey = NameToDBKey(someone);

	AUTOLOCKWRITE(m_lockDB);
	return m_dbFriend.add(dbkey.data(), dbkey.size(), (const char*)pdbFriendList, sizeUsed);
}


BOOL CFriendMgr::BkgrndUpdateDB()
{
	if (m_StorePlayerPerUdtDB <= 0)
		return TRUE;

	ONEPLAYERLIST listOnePlayer;

	{{
	//dup the data into mem

	typedef std::list<std::_tstring>	MEMPLAYERS;
	MEMPLAYERS memPlayers;

	{{
	AUTOLOCKREAD(m_lockFriend);

	size_t playercount = m_mapPlayers.size();

	if (playercount <= 0)
		return TRUE;

	if (m_cursorBkgrndUdtDB >= playercount)
		m_cursorBkgrndUdtDB = 0;

	//seek to cursor
	PLAYERSMAP::iterator itBase;
	if (m_cursorBkgrndUdtDB <= playercount/2)
	{
		itBase = m_mapPlayers.begin();
		for (size_t i = 0; i < m_cursorBkgrndUdtDB; i++)
			++ itBase;
	}
	else
	{
		itBase = m_mapPlayers.end();
		for (size_t i = playercount; i > m_cursorBkgrndUdtDB; i--)
			-- itBase;
	}

	PLAYERSMAP::iterator itPlayer = itBase;
	do
	{
		//limit store player count
		if (m_StorePlayerPerUdtDB != size_t(-1)
			&& listOnePlayer.size() + memPlayers.size() > m_StorePlayerPerUdtDB)
			break;

		std::_tstring someone = (*itPlayer).first;
		PLAYERINFO& rPlayerInfo = (*itPlayer).second;

		if (rPlayerInfo.loaded)
		{
			listOnePlayer.push_back(ONEPLAYER());
			ONEPLAYER& rOnePlayer = listOnePlayer.back();


			BOOL needstore = rPlayerInfo.dirty;

			rPlayerInfo.dirty = FALSE;


			rOnePlayer.someone = someone;

			for (FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin(); itFriend != rPlayerInfo.friends.end(); itFriend++)
			{
				FRIENDINFO& rInfo = (*itFriend).second;
				assert(rInfo.pCheated);
				if (!rInfo.pCheated)
					continue;

				MEM_FRIENDRECORD memFriendRec;
				memFriendRec.name = (*itFriend).first;
				memFriendRec.group = rInfo.group;
				memFriendRec.cheating = rInfo.cheating;
				memFriendRec.cheated = *rInfo.pCheated;

				rOnePlayer.listFriends.push_back(memFriendRec);

				if (!needstore
					&& ((rInfo.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE) != rInfo.orig_cheating
					|| (*rInfo.pCheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE) != rInfo.orig_cheated))
				{
					needstore = TRUE;
				}

				rInfo.orig_cheating = rInfo.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
				rInfo.orig_cheated = *rInfo.pCheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
			}

			if (!needstore)
				listOnePlayer.pop_back();
		}
		else
		{
			//limit load player count
			if (m_LoadPlayerPerUdtDB != size_t(-1)
				&& memPlayers.size() < m_LoadPlayerPerUdtDB)
				memPlayers.push_back(someone);
		}

		++ itPlayer;
		if (itPlayer == m_mapPlayers.end())
			itPlayer = m_mapPlayers.begin();

	} while (itPlayer != itBase);
	}}

	{{
	if (!memPlayers.empty())
	{
		AUTOLOCKWRITE(m_lockFriend);

		for (MEMPLAYERS::iterator itMemPlayer = memPlayers.begin(); itMemPlayer != memPlayers.end(); itMemPlayer++)
		{
			std::_tstring player = *itMemPlayer;

			PLAYERSMAP::iterator itPlayer = _LoadSomeone(player);
			if (itPlayer != m_mapPlayers.end())
			{
				PLAYERINFO& rPlayerInfo = (*itPlayer).second;


				listOnePlayer.push_back(ONEPLAYER());
				ONEPLAYER& rOnePlayer = listOnePlayer.back();


				BOOL needstore = rPlayerInfo.dirty;

				rPlayerInfo.dirty = FALSE;


				rOnePlayer.someone = player;

				for (FRIENDSMAP::iterator itFriend = rPlayerInfo.friends.begin(); itFriend != rPlayerInfo.friends.end(); itFriend++)
				{
					FRIENDINFO& rInfo = (*itFriend).second;
					assert(rInfo.pCheated);

					MEM_FRIENDRECORD memFriendRec;
					memFriendRec.name = (*itFriend).first;
					memFriendRec.group = rInfo.group;
					memFriendRec.cheating = rInfo.cheating;
					memFriendRec.cheated = *rInfo.pCheated;

					rOnePlayer.listFriends.push_back(memFriendRec);
				
					if (!needstore
						&& ((rInfo.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE) != rInfo.orig_cheating
						|| (*rInfo.pCheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE) != rInfo.orig_cheated))
					{
						needstore = TRUE;
					}

					rInfo.orig_cheating = rInfo.cheating ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
					rInfo.orig_cheated = *rInfo.pCheated ? FRIENDINFO::cheatTRUE : FRIENDINFO::cheatFALSE;
				}

				if (!needstore)
					listOnePlayer.pop_back();
			}
		}
	}
	}}

	}}

	{{
	//store mem data into DB

	for (ONEPLAYERLIST::iterator itOnePlayer = listOnePlayer.begin(); itOnePlayer != listOnePlayer.end(); itOnePlayer++)
	{
		ONEPLAYER& rOnePlayer = *itOnePlayer;

		if (!DB_StoreSomeone(rOnePlayer.someone, rOnePlayer.listFriends))
		{
			PLAYERINFO& rPlayerInfo = m_mapPlayers[rOnePlayer.someone];

			rPlayerInfo.dirty = TRUE;

			rTRACE("error: bkgrnp store someone: %s", rOnePlayer.someone.c_str());
		}
	}

	}}

	m_cursorBkgrndUdtDB += listOnePlayer.size();


	return TRUE;
}


size_t CFriendMgr::GetPlayerCount()
{
	AUTOLOCKREAD(m_lockFriend);

	return m_mapPlayers.size();
}

size_t CFriendMgr::GetAFQSize()
{
	AUTOLOCKREAD(m_lockAddFriend);

	return m_mapAddFriends.size();
}



BOOL CFriendMgr::PlayerAssociate(const std::_tstring& someone, const std::_tstring& dst, const std::_tstring& group, BYTE bidir)
{
	AUTOLOCKWRITE(m_lockFriend);


	PLAYERINFO& rSrcPlayerInfo = m_mapPlayers[someone];
	assert(rSrcPlayerInfo.loaded);
	FRIENDINFO& rSrcInfo = rSrcPlayerInfo.friends[dst];
	rSrcInfo.cheating = false;
	rSrcInfo.group = group;

	PLAYERINFO& rDstPlayerInfo = m_mapPlayers[dst];
	FRIENDINFO& rDstInfo = rDstPlayerInfo.friends[someone];
	rDstInfo.cheating = bidir ? false : true;
	rDstInfo.group = group;

	//binding
	rSrcInfo.pCheated = &rDstInfo.cheating;
	rDstInfo.pCheated = &rSrcInfo.cheating;


	rTRACE("Player Associate: %s [%s] on <%s>, bidir: %u", someone.c_str(), dst.c_str(), group.c_str(), int(bidir));

	return TRUE;
}


BOOL CFriendMgr::PlayerAssociateBevy(const _BEVY& bevy, const std::_tstring& group)
{
	if (bevy.size() < 2)
		return TRUE;


	AUTOLOCKWRITE(m_lockFriend);


	std::vector<PLAYERSMAP::iterator> vecPlayer;

	for (_BEVY::const_iterator itS = bevy.begin(); itS != bevy.end(); itS++)
	{
		std::string someone = *itS;
		assert(!someone.empty());

		std::pair<PLAYERSMAP::iterator, bool> insit = m_mapPlayers.insert(PLAYERSMAP::value_type(someone, PLAYERINFO()));
		assert(!insit.second && insit.first != m_mapPlayers.end() && (*insit.first).second.loaded);

		vecPlayer.push_back(insit.first);
	}

	if (vecPlayer.size() < 2)
		return TRUE;



	const size_t count = vecPlayer.size();
	const size_t last = count - 1;

	for (size_t idxSrc = 0; idxSrc < last; idxSrc++)
	{
		PLAYERSMAP::iterator itSrc = vecPlayer[idxSrc];
		std::_tstring src = (*itSrc).first;
		PLAYERINFO& rSrcPlayerInfo = (*itSrc).second;


		for (size_t idxDst = idxSrc + 1; idxDst < count; idxDst++)
		{
			PLAYERSMAP::iterator itDst = vecPlayer[idxDst];
			std::_tstring dst = (*itDst).first;
			PLAYERINFO& rDstPlayerInfo = (*itDst).second;

			if (!_tstring_equal()(dst, src))
			{
				FRIENDINFO& rSrcInfo = rSrcPlayerInfo.friends[dst];
				rSrcInfo.cheating = false;
				rSrcInfo.group = group;

				FRIENDINFO& rDstInfo = rDstPlayerInfo.friends[src];
				rDstInfo.cheating = false;
				rDstInfo.group = group;

				
				//binding
				rSrcInfo.pCheated = &rDstInfo.cheating;
				rDstInfo.pCheated = &rSrcInfo.cheating;
			}
		};
	}


	rTRACE("Players AssociateBevy: %u on <%s>", bevy.size(), group.c_str());


	return TRUE;
}
