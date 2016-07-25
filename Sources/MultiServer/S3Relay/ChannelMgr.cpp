// ChannelMgr.cpp: implementation of the CChannelMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "ChannelMgr.h"
#include "S3Relay.h"
#include "time.h"
#include "malloc.h"

#include <list>
#include <map>

/////////////////////////////////////////////////////

static const size_t MAX_SENTLEN = 256;

static const char NUMBER_CHARS[] = "0123456789";
static const char NULL_STR[] = "";

/////////////////////////////////////////////////////


const char file_channcfg[]	= "relay_channcfg.ini";

const char sec_system[]		= "system";
const char key_nameGM[]		= "nameGM";
const char key_charEsc[]	= "charEsc";
const char key_charSplt[]	= "charSplt";
const char key_defCost[]	= "defCost";

const char sec_team[]		= "team";
const char sec_faction[]	= "faction";
const char sec_tong[]		= "tong";
const char sec_screen[]		= "screen";
const char sec_broadcast[]	= "broadcast";
const char key_escSpec[]	= "escSpec";
const char key_minID[]		= "minID";
const char key_maxID[]		= "maxID";




const char file_relaychann[]	= "relay_channel.ini";
const char key_cost[]			= "cost";



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChannelMgr::CChannelMgr()
	: m_lastChannID(0), m_channidGM(-1),
	m_chChannEsc(0), m_chChannSplt(0),
	m_chChannTEAM(0), m_chChannFAC(0), m_chChannTONG(0), m_chChannSCRN(0),
	m_minTeamID(0), m_maxTeamID(-1), m_minFacID(0), m_maxFacID(-1), m_minTongID(0), m_maxTongID(-1),
	m_defCost(0),
	m_costTeam(0), m_costFac(0), m_costTong(0), m_costScrn(0), m_costBc(0)
{

}

CChannelMgr::~CChannelMgr()
{

}


BOOL CChannelMgr::Initialize()
{
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	m_chChannEsc  = gGetPrivateProfileCharEx(sec_system, key_charEsc, file_channcfg, defCHANN_esc);
	m_chChannSplt = gGetPrivateProfileCharEx(sec_system, key_charSplt, file_channcfg, defCHANN_splt);

	m_chChannTEAM = gGetPrivateProfileCharEx(sec_team, key_escSpec, file_channcfg, defCHANN_TEAM);
	m_chChannFAC  = gGetPrivateProfileCharEx(sec_faction, key_escSpec, file_channcfg, defCHANN_FAC);
	m_chChannTONG = gGetPrivateProfileCharEx(sec_tong, key_escSpec, file_channcfg, defCHANN_TONG);
	m_chChannSCRN = gGetPrivateProfileCharEx(sec_screen, key_escSpec, file_channcfg, defCHANN_SCRN);
	m_chChannBC   = gGetPrivateProfileCharEx(sec_broadcast, key_escSpec, file_channcfg, defCHANN_BC);

	m_minTeamID	= gGetPrivateProfileIntEx(sec_team, key_minID, file_channcfg, 0);
	m_maxTeamID	= gGetPrivateProfileIntEx(sec_team, key_maxID, file_channcfg, -1);
	m_minFacID	= gGetPrivateProfileIntEx(sec_faction, key_minID, file_channcfg, 0);
	m_maxFacID	= gGetPrivateProfileIntEx(sec_faction, key_maxID, file_channcfg, -1);
	m_minTongID	= gGetPrivateProfileIntEx(sec_tong, key_minID, file_channcfg, 0);
	m_maxTongID	= gGetPrivateProfileIntEx(sec_tong, key_maxID, file_channcfg, -1);

	m_defCost = (BYTE)gGetPrivateProfileIntEx(sec_system, key_defCost, file_channcfg, 0);

	m_costTeam = (BYTE)gGetPrivateProfileIntEx(sec_team, key_cost, file_channcfg, (INT)(UINT)m_defCost);
	m_costFac  = (BYTE)gGetPrivateProfileIntEx(sec_faction, key_cost, file_channcfg, (INT)(UINT)m_defCost);
	m_costTong = (BYTE)gGetPrivateProfileIntEx(sec_tong, key_cost, file_channcfg, (INT)(UINT)m_defCost);
	m_costScrn = (BYTE)gGetPrivateProfileIntEx(sec_screen, key_cost, file_channcfg, (INT)(UINT)m_defCost);
	m_costBc   = (BYTE)gGetPrivateProfileIntEx(sec_broadcast, key_cost, file_channcfg, (INT)(UINT)m_defCost);

	std::_tstring nameChannGM = gGetPrivateProfileStringEx(sec_system, key_nameGM, file_channcfg);
	if (!nameChannGM.empty())
	{
		//create the channel
		m_channidGM = GenChannID();
		if (m_channidGM == DWORD(-1))
			return FALSE;

		m_mapChann2ID[nameChannGM] = m_channidGM;

		CHANNINFO& rChannInfo = m_mapChannid2Info[m_channidGM];

		rChannInfo.god = TRUE;
		rChannInfo.stock = TRUE;
		rChannInfo.channname = nameChannGM;
		rChannInfo.gmuse = FALSE;
		rChannInfo.gmsub = FALSE;
		rChannInfo.cost = (BYTE)gGetPrivateProfileIntEx(nameChannGM.c_str(), key_cost, file_relaychann, (INT)(UINT)m_defCost);

		rTRACE("Create Channel: [Stock, GM] <%08X> %s", m_channidGM, nameChannGM.c_str());
	}


	const std::vector<std::_tstring>& rVecChann = gGetPrivateProfileSectionNamesEx(file_relaychann);

	for (std::vector<std::_tstring>::const_iterator it = rVecChann.begin(); it != rVecChann.end(); it++)
	{
		std::_tstring channname = *it;
		assert(!channname.empty());

		if (_tstring_equal()(channname, nameChannGM))	//gm, created already
			continue;

		DWORD channid = GenChannID();
		if (channid == DWORD(-1))
			return FALSE;

		m_mapChann2ID[channname] = channid;

		CHANNINFO& rChannInfo = m_mapChannid2Info[channid];
		rChannInfo.god = FALSE;
		rChannInfo.stock = TRUE;
		rChannInfo.channname = channname;
		rChannInfo.gmuse = FALSE;
		rChannInfo.gmsub = FALSE;
		rChannInfo.cost = (BYTE)gGetPrivateProfileIntEx(channname.c_str(), key_cost, file_relaychann, (INT)(UINT)m_defCost);

		rTRACE("Create Channel: [Stock] <%08X> %s", channid, channname.c_str());
	}

	return TRUE;
}

BOOL CChannelMgr::Uninitialize()
{
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	m_mapChann2ID.clear();
	m_mapChannid2Info.clear();


	m_defCost = 0;

	m_costTeam = 0;
	m_costFac  = 0;
	m_costTong = 0;
	m_costScrn = 0;
	m_costBc   = 0;


	m_channidGM = -1;

	m_chChannEsc = 0;
	m_chChannSplt = 0;
	m_chChannTEAM = 0;
	m_chChannFAC = 0;
	m_chChannSCRN = 0;
	m_chChannBC = 0;

	m_minTeamID	= 0;
	m_maxTeamID	= -1;
	m_minFacID	= 0;
	m_maxFacID	= -1;
	m_minTongID	= 0;
	m_maxTongID	= -1;

	return TRUE;
}


DWORD CChannelMgr::GenChannID()
{
	//assert(m_lockChannel._IsWriting());

	if (m_lastChannID != DWORD(-1))
		return m_lastChannID++;

	//no base idle, then search idle
	DWORD id = 0;
	for (CHANNID2INFOMAP::iterator it = m_mapChannid2Info.begin(); it != m_mapChannid2Info.end(); it++)
	{
		if ((*it).first != id)
			return id;
		id ++;
	}
	return id;
}



std::_tstring CChannelMgr::GetChannelName(DWORD channelid, BOOL adv)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::const_iterator it = m_mapChannid2Info.find(channelid);
	if (it == m_mapChannid2Info.end())
		return NULL_STR;

	std::_tstring channname = (*it).second.channname;

	return adv ? channname : ReduceChannelName(channname);
}

DWORD CChannelMgr::GetChannelID(const std::_tstring& channname, DWORD advIP)
{
	std::_tstring advChannName = advIP == 0 ? channname : MakeChannelName(channname, advIP);
	if (advChannName.empty())
		return -1;

	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANN2IDMAP::iterator it = m_mapChann2ID.find(advChannName);
	if (it == m_mapChann2ID.end())
		return -1;

	return (*it).second;
}


BOOL CChannelMgr::IsSubscribed(DWORD ip, unsigned long param, DWORD channid)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::iterator itChannid = m_mapChannid2Info.find(channid);
	if (itChannid == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*itChannid).second;

	IP2PLAYERSMAP::iterator itIP = rChannInfo.mapIp2Players.find(ip);
	if (itIP == rChannInfo.mapIp2Players.end())
		return FALSE;

	PLAYERSMAP& rPlayers = (*itIP).second;
	PLAYERSMAP::iterator itRole = rPlayers.find(param);
	if (itRole == rPlayers.end())
		return FALSE;

	PLAYERINFO& rInfo = (*itRole).second;
	return rInfo.subed;
}


BOOL CChannelMgr::GmSubscribe(DWORD channid)
{
	if (channid == -1)
		return FALSE;

	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	if (channid == m_channidGM)
		return TRUE;

	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;
	if (!rChannInfo.gmuse)
		return FALSE;

	rChannInfo.gmsub = TRUE;

	return TRUE;
}

BOOL CChannelMgr::GmUnsubscribe(DWORD channid)
{
	if (channid == m_channidGM)
		return FALSE;

	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;
	if (!rChannInfo.gmuse)
		return FALSE;

	rChannInfo.gmsub = FALSE;

	return TRUE;
}

BOOL CChannelMgr::IsGmSubscribed(DWORD channid)
{
	if (channid == m_channidGM)
		return TRUE;

	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;

	return rChannInfo.gmsub;
}

BOOL CChannelMgr::GmQueryChannelID(const std::_tstring& channel, BOOL force)
{
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	DWORD theChannID = -1;
	BYTE cost = 0;

	CHANN2IDMAP::iterator it = m_mapChann2ID.find(channel);
	if (it != m_mapChann2ID.end())
	{//channel exist
		DWORD channid = (*it).second;

		CHANNINFO& rChannInfo = m_mapChannid2Info[channid];

		rChannInfo.gmuse = TRUE;

		theChannID = channid;
		cost = rChannInfo.cost;
	}
	else
	{
		if (force)
		{
			//erase the channel
			DWORD channid = GenChannID();
			if (channid != DWORD(-1))
			{
				m_mapChann2ID[channel] = channid;

				CHANNINFO& rChannInfo = m_mapChannid2Info[channid];
				rChannInfo.god = FALSE;
				rChannInfo.stock = FALSE;
				rChannInfo.channname = channel;
				rChannInfo.gmuse = TRUE;
				rChannInfo.gmsub = FALSE;
				rChannInfo.cost = GainPreCost(channel);

				rTRACE("GM Create Channel: <%08X> %s", channid, channel.c_str());

				theChannID = channid;
				cost = rChannInfo.cost;
			}
		}

	}

	{{
	size_t pckgsize = sizeof(RELAY_ASKWAY_DATA) + sizeof(PLAYERCOMM_NOTIFYCHANNELID);

	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);
	pAskWayData->ProtocolFamily = pf_relay;
	pAskWayData->ProtocolID = relay_c2c_askwaydata;
	pAskWayData->nFromIP = 0;
	pAskWayData->nFromRelayID = -1;
	pAskWayData->seekRelayCount = 0;
	pAskWayData->seekMethod = rm_gm;
	pAskWayData->wMethodDataLength = 0;
	pAskWayData->routeDateLength = sizeof(PLAYERCOMM_NOTIFYCHANNELID);

	PLAYERCOMM_NOTIFYCHANNELID* pPlayerCommNotiChannID = (PLAYERCOMM_NOTIFYCHANNELID*)(pAskWayData + 1);
	pPlayerCommNotiChannID->ProtocolFamily = pf_playercommunity;
	pPlayerCommNotiChannID->ProtocolID = playercomm_s2c_notifychannelid;
	strcpy(pPlayerCommNotiChannID->channel, channel.c_str());
	pPlayerCommNotiChannID->channelid = theChannID;
	pPlayerCommNotiChannID->cost = cost;

	g_RootClient.SendPackage(pAskWayData, pckgsize);
	}}

	return TRUE;
}

BOOL CChannelMgr::GmFreeChannID(DWORD channid)
{
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;

	//ensure
	assert(!rChannInfo.gmsub);
	rChannInfo.gmsub = FALSE;

	rChannInfo.gmuse = FALSE;

	if (!rChannInfo.stock
		&& rChannInfo.mapIp2Players.empty())
	{
		rTRACE("GM Destroy Channel: <%08X> %s", channid, rChannInfo.channname.c_str());

		m_mapChann2ID.erase(rChannInfo.channname);
		m_mapChannid2Info.erase(it);
	}

	return TRUE;
}

BOOL CChannelMgr::IsUsed(DWORD ip, unsigned long param, DWORD channid)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::iterator itChannid = m_mapChannid2Info.find(channid);
	if (itChannid == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*itChannid).second;

	IP2PLAYERSMAP::iterator itIP = rChannInfo.mapIp2Players.find(ip);
	if (itIP == rChannInfo.mapIp2Players.end())
		return FALSE;

	PLAYERSMAP& rPlayers = (*itIP).second;
	PLAYERSMAP::iterator itRole = rPlayers.find(param);

	return itRole != rPlayers.end();
}

BOOL CChannelMgr::IsGmUsed(DWORD channid)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;

	return rChannInfo.gmuse;
}


std::_tstring CChannelMgr::MakeChannelName(const std::_tstring& chann, DWORD ip)
{
	if (chann.empty())
		return NULL_STR;

	if (chann[0] != m_chChannEsc)
		return chann;

	if (chann.size() < 2)
		return NULL_STR;

	if (chann[1] == m_chChannTEAM
		|| chann[1] == m_chChannFAC
		|| chann[1] == m_chChannTONG)
	{
		if (chann.size() <= 2)
			return NULL_STR;
		if (chann.find_last_not_of(NUMBER_CHARS) > 1)
			return NULL_STR;

		DWORD id = _ttol(chann.c_str() + 2);

		if (chann[1] == m_chChannFAC)
			return IsValidFacID(id) ? chann : NULL_STR;
		if (chann[1] == m_chChannTONG)
			return IsValidTongID(id) ? chann : NULL_STR;

		if (ip == 0 || !IsValidTeamID(id))
			return NULL_STR;

		char buffer[16];
		std::_tstring channname(chann);
		channname += m_chChannSplt;
		channname += _ultot(ip, buffer, 10);

		return channname;
	}
	else if (chann[1] == m_chChannSCRN
		|| chann[1] == m_chChannBC)
	{
		if (chann.size() > 2)
			return NULL_STR;

		char buffer[16];
		std::_tstring channname(chann);
		channname += _ultot(ip, buffer, 10);

		return channname;
	}

	return NULL_STR;
}

std::_tstring CChannelMgr::ReduceChannelName(const std::_tstring& chann)
{
	if (chann.empty())
		return NULL_STR;

	if (chann[0] != m_chChannEsc)
		return chann;

	if (chann.size() < 2)
		return NULL_STR;

	if (chann[1] == m_chChannTEAM)
	{
		int spltpos = chann.find_last_of(m_chChannSplt);
		if (spltpos == -1)
			return NULL_STR;
		return std::_tstring(chann.c_str(), spltpos);
	}
	if (chann[1] == m_chChannFAC
		|| chann[1] == m_chChannTONG)
	{
		return chann;
	}
	if (chann[1] == m_chChannSCRN
		|| chann[1] == m_chChannBC)
	{
		return std::_tstring(chann.c_str(), 2);
	}

	assert(FALSE);
	return NULL_STR;
}

DWORD CChannelMgr::ParseChannelName(const std::_tstring& chann, DWORD* pIP)
{
	if (pIP != NULL)
		*pIP = 0;

	if (chann.empty())
		return -1;

	if (chann[0] != m_chChannEsc)
		return -1;

	if (chann.size() < 2)
		return -1;

	std::_tstring basechann;
	if (chann[1] == m_chChannTEAM)
	{
		int spltpos = chann.find_last_of(m_chChannSplt);
		if (spltpos == -1)
			return -1;

		if (pIP != NULL)
			*pIP = _ttol(chann.c_str() + spltpos + 1);
		return _ttol(chann.c_str() + 2);
	}
	if (chann[1] == m_chChannFAC
		|| chann[1] == m_chChannTONG)
	{
		return _ttol(chann.c_str() + 2);
	}
	if (chann[1] == m_chChannSCRN
		|| chann[1] == m_chChannBC)
	{
		if (pIP != NULL)
			*pIP = _ttol(chann.c_str() + 2);
		return 0;
	}

	return -1;
}


BYTE CChannelMgr::GainPreCost(const std::_tstring& channel)
{
	if (channel.size() >= 2 && channel[0] == m_chChannEsc)
	{
		if (channel[1] == m_chChannTEAM)
			return m_costTeam;
		else if (channel[1] == m_chChannFAC)
			return m_costFac;
		else if (channel[1] == m_chChannTONG)
			return m_costTong;
		else if (channel[1] == m_chChannSCRN)
			return m_costScrn;
		else if (channel[1] == m_chChannBC)
			return m_costBc;
	}

	return m_defCost;
}


DWORD CChannelMgr::GetGMChannID()
{
	return m_channidGM;
}

BOOL CChannelMgr::IsGodChannel(DWORD channid)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::const_iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	const CHANNINFO& rChannInfo = (*it).second;

	return rChannInfo.god;
}

BOOL CChannelMgr::IsStockChannel(DWORD channid)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);

	CHANNID2INFOMAP::const_iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	const CHANNINFO& rChannInfo = (*it).second;

	return rChannInfo.stock;
}

BOOL CChannelMgr::PassToSpecMan(DWORD ToIP, unsigned long ToRelayID, DWORD ToNameID, const void* pData, size_t size)
{
	CNetConnectDup conndup = g_ChatServer.FindChatConnectByIP(ToIP);
	if (!conndup.IsValid())
		return FALSE;

	size_t pckgsize = sizeof(CHAT_SPECMAN) + size;

	CHAT_SPECMAN* pCsm = (CHAT_SPECMAN*)_alloca(pckgsize);
	pCsm->ProtocolType = chat_specman;
	pCsm->wSize = pckgsize - 1;
	pCsm->nameid = ToNameID;
	pCsm->lnID = ToRelayID;
	pCsm->wChatLength = size;

	void* pExPckg = pCsm + 1;
	memcpy(pExPckg, pData, size);

	conndup.SendPackage(pCsm, pckgsize);

	return TRUE;
}

BOOL CChannelMgr::PassToGroupMan(DWORD ToIP, BOOL verify, const MANLIST& mans, const void* pData, size_t size)
{
	if (mans.empty())
		return TRUE;

	const size_t playerlen = verify ? sizeof(tagPlusSrcInfo) : sizeof(WORD);
	size_t basesize = sizeof(CHAT_GROUPMAN) + size;
	if (basesize + playerlen > max_packagesize)
		return FALSE;

	CNetConnectDup conndup = g_ChatServer.FindChatConnectByIP(ToIP);
	if (!conndup.IsValid())
		return FALSE;


	BYTE buffer[max_packagesize];

	CHAT_GROUPMAN* pCgm = (CHAT_GROUPMAN*)buffer;
	pCgm->ProtocolType = chat_groupman;
	pCgm->wChatLength = size;
	pCgm->byHasIdentify = (BYTE)verify;

	void* pExPckg = pCgm + 1;
	memcpy(pExPckg, pData, size);


	size_t cursor = basesize;

	for (MANLIST::const_iterator it = mans.begin(); it != mans.end(); it++)
	{
		if (verify)
			*(tagPlusSrcInfo*)(buffer + cursor) = *it;
		else
			*(WORD*)(buffer + cursor)= (WORD)(*it).lnID;
		cursor += playerlen;

		if (cursor + playerlen > max_packagesize)
		{
			assert((cursor - basesize) % playerlen == 0);

			pCgm->wSize = cursor - 1;
			pCgm->wPlayerCount = (cursor - basesize) / playerlen;

			conndup.SendPackage(buffer, cursor);

			cursor = basesize;
		}
	}

	if (cursor > basesize)
	{
		assert((cursor - basesize) % playerlen == 0);

		pCgm->wSize = cursor - 1;
		pCgm->wPlayerCount = (cursor - basesize) / playerlen;

		conndup.SendPackage(buffer, cursor);
	}

	return TRUE;
}


BOOL CChannelMgr::PassToGM(const void* pData, size_t size)
{
	size_t pckgsize = sizeof(RELAY_ASKWAY_DATA) + size;

	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);
	pAskWayData->ProtocolFamily = pf_relay;
	pAskWayData->ProtocolID = relay_c2c_askwaydata;
	pAskWayData->nFromIP = 0;
	pAskWayData->nFromRelayID = 0;
	pAskWayData->seekRelayCount = 0;
	pAskWayData->seekMethod = rm_gm;
	pAskWayData->wMethodDataLength = 0;
	pAskWayData->routeDateLength = size;

	void* pExPckg = pAskWayData + 1;
	memcpy(pExPckg, pData, size);

	g_RootClient.SendPackage(pAskWayData, pckgsize);

	return TRUE;
}

BOOL CChannelMgr::IsValidTeamID(DWORD idTeam)
{
	if (m_minTeamID > m_maxTeamID)
		return TRUE;

	return idTeam >= m_minTeamID && idTeam <= m_maxTeamID;
}

BOOL CChannelMgr::IsValidFacID(DWORD idFac)
{
	if (m_minFacID > m_maxFacID)
		return TRUE;

	return idFac >= m_minFacID && idFac <= m_maxFacID;
}

BOOL CChannelMgr::IsValidTongID(DWORD idTong)
{
	if (m_minTongID > m_maxTongID)
		return TRUE;

	return idTong >= m_minTongID && idTong <= m_maxTongID;
}


BOOL CChannelMgr::SomeoneChat(DWORD srcIP, const tagPlusSrcInfo& SrcInfo, const CHAT_SOMEONECHAT_CMD* pSomeoneChatCmd)
{
	if (pSomeoneChatCmd->sentlen <= 0 || pSomeoneChatCmd->sentlen > MAX_SENTLEN)
		return FALSE;
	if (!gIsLegalString(pSomeoneChatCmd->someone, 1, _NAME_LEN))
		return FALSE;

	std::_tstring srcrole;
	if (!g_HostServer.FindPlayerByIpParam(NULL, srcIP, SrcInfo.lnID, NULL, NULL, &srcrole, NULL))
		return FALSE;


	BOOL sent = FALSE;

	CNetConnectDup hostconndup;
	DWORD nameid = 0;
	unsigned long param = 0;
	if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(pSomeoneChatCmd->someone), &hostconndup, NULL, &nameid, &param))
	{
		size_t syncsize = sizeof(CHAT_SOMEONECHAT_SYNC) + pSomeoneChatCmd->sentlen;
		CHAT_SOMEONECHAT_SYNC* pCscSync = (CHAT_SOMEONECHAT_SYNC*)_alloca(syncsize);
		pCscSync->ProtocolType = chat_someonechat;
		pCscSync->wSize = syncsize - 1;
		pCscSync->packageID = pSomeoneChatCmd->packageID;
		strcpy(pCscSync->someone, srcrole.c_str());
		pCscSync->sentlen = pSomeoneChatCmd->sentlen;
		memcpy(pCscSync + 1, pSomeoneChatCmd + 1, pSomeoneChatCmd->sentlen);

		if (PassToSpecMan(hostconndup.GetIP(), param, nameid, pCscSync, syncsize))
			sent = TRUE;
	}


	{{
	size_t _exsize = sizeof(DWORD) + _NAME_LEN + sizeof(BYTE) + pSomeoneChatCmd->sentlen;	////X
	size_t feedbacksize = sizeof(CHAT_FEEDBACK) + _exsize;
	CHAT_FEEDBACK* pCfb = (CHAT_FEEDBACK*)_alloca(feedbacksize);
	pCfb->ProtocolType = chat_feedback;
	pCfb->packageID = pSomeoneChatCmd->packageID;
	pCfb->code = sent ? codeSucc : codeFail;

	////X
	DWORD* pChannelid = (DWORD*)(pCfb + 1);
	*pChannelid = -1;
	char* pDstName = (char*)(pChannelid + 1);
	strcpy(pDstName, pSomeoneChatCmd->someone);
	BYTE* pSentlen = (BYTE*)(pDstName + _NAME_LEN);
	*pSentlen = pSomeoneChatCmd->sentlen;
	void* pSent = pSentlen + 1;
	memcpy(pSent, pSomeoneChatCmd + 1, pSomeoneChatCmd->sentlen);
	////X

	PassToSpecMan(srcIP, SrcInfo.lnID, SrcInfo.nameid, pCfb, feedbacksize);
	}}


//	dTRACE("Chat Someone: sentence %u Bs from [%s] to [%s]", pSomeoneChatCmd->sentlen, srcrole.c_str(), pSomeoneChatCmd->someone);


	return TRUE;
}


BOOL CChannelMgr::ChannelChat(DWORD srcIP, const tagPlusSrcInfo& SrcInfo, const CHAT_CHANNELCHAT_CMD* pChannelChatCmd)
{
	if (pChannelChatCmd->channelid == -1)
		return FALSE;
	if (pChannelChatCmd->sentlen <= 0 || pChannelChatCmd->sentlen > MAX_SENTLEN)
		return FALSE;


	std::_tstring srcacc;
	std::_tstring srcrole;
	if (!g_HostServer.FindPlayerByIpParam(NULL, srcIP, SrcInfo.lnID, NULL, &srcacc, &srcrole, NULL))
		return FALSE;


	BOOL sent = FALSE;


	DUMMY_AUTOLOCKREAD(m_lockChannel);


	CHANNID2INFOMAP::iterator itChannid = m_mapChannid2Info.find(pChannelChatCmd->channelid);
	if (itChannid == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*itChannid).second;

	if (pChannelChatCmd->cost != rChannInfo.cost)
		return FALSE;


	if (rChannInfo.god)
	{
		if (g_RootClient.IsReady())
		{
			size_t pckgsize = sizeof(CHAT_MSG_EX) + pChannelChatCmd->sentlen;
			CHAT_MSG_EX* pChatMsgEx = (CHAT_MSG_EX*)_alloca(pckgsize);
			pChatMsgEx->ProtocolFamily = pf_playercommunity;
			pChatMsgEx->ProtocolID = playercomm_channelchat;
			strcpy(pChatMsgEx->m_szSourceName, srcrole.c_str());
			strcpy(pChatMsgEx->m_szAccountName, srcacc.c_str());
			pChatMsgEx->SentenceLength = pChannelChatCmd->sentlen;
			memcpy(pChatMsgEx + 1, pChannelChatCmd + 1, pChannelChatCmd->sentlen);

			PassToGM(pChatMsgEx, pckgsize);

			sent = TRUE;
		}
	}
	else
	{
		if (IsNeedRelegate(rChannInfo.channname, pChannelChatCmd->filter))
		{//relegate to gamesvr
			assert(rChannInfo.channname[0] == m_chChannEsc);

			DWORD channIP = 0;
			DWORD theID = ParseChannelName(rChannInfo.channname, &channIP);
			if (theID == -1 && channIP == 0)
				return FALSE;


			assert(rChannInfo.channname.size() >= 2);


			size_t syncsize = sizeof(CHAT_CHANNELCHAT_SYNC) + pChannelChatCmd->sentlen;
			size_t pckgsize = sizeof(CHAT_RELEGATE) + syncsize;

			CHAT_RELEGATE* pCr = (CHAT_RELEGATE*)_alloca(pckgsize);
			pCr->ProtocolType = chat_relegate;
			pCr->wSize = pckgsize - 1;
			pCr->nFromIP = srcIP;
			pCr->nFromRelayID = SrcInfo.lnID;
			pCr->channelid = pChannelChatCmd->channelid;
			pCr->routeDateLength = syncsize;

			CHAT_CHANNELCHAT_SYNC* pCccSync = (CHAT_CHANNELCHAT_SYNC*)(pCr + 1);
			pCccSync->ProtocolType = chat_channelchat;
			pCccSync->wSize = syncsize - 1;
			pCccSync->packageID = pChannelChatCmd->packageID;
			strcpy(pCccSync->someone, srcrole.c_str());
			pCccSync->channelid = pChannelChatCmd->channelid;
			pCccSync->sentlen = pChannelChatCmd->sentlen;
			memcpy(pCccSync + 1, pChannelChatCmd + 1, pChannelChatCmd->sentlen);


			if (rChannInfo.channname[1] == m_chChannTEAM)
			{
				if (theID == -1 || channIP == 0)
					return FALSE;

				CHANNINFO& rChannInfo = (*itChannid).second;

				CNetConnectDup connDup = g_ChatServer.FindChatConnectByIP(channIP);
				if (connDup.IsValid())
				{
					pCr->TargetCls = tgtcls_team;
					pCr->TargetID = theID;

					connDup.SendPackage(pCr, pckgsize);

					sent = TRUE;
				}
			}
			else if (rChannInfo.channname[1] == m_chChannFAC)
			{
				if (theID == -1)
					return FALSE;

				pCr->TargetCls = tgtcls_fac;
				pCr->TargetID = theID;

				g_ChatServer.BroadPackage(pCr, pckgsize);

				sent = TRUE;
			}
			else if (rChannInfo.channname[1] == m_chChannTONG)
			{
				if (theID == -1)
					return FALSE;

				pCr->TargetCls = tgtcls_tong;
				pCr->TargetID = theID;

				g_ChatServer.BroadPackage(pCr, pckgsize);

				sent = TRUE;
			}
			else if (rChannInfo.channname[1] == m_chChannSCRN
				|| rChannInfo.channname[1] == m_chChannBC)
			{
				if (channIP == 0)
					return FALSE;

				assert(channIP != 0 && theID == 0);

				CNetConnectDup connDup = g_ChatServer.FindChatConnectByIP(channIP);
				if (connDup.IsValid())
				{
					BOOL isScrn = rChannInfo.channname[1] == m_chChannSCRN;

					pCr->TargetCls = isScrn ? tgtcls_scrn : tgtcls_bc;
					pCr->TargetID = isScrn ? SrcInfo.lnID : 0;

					connDup.SendPackage(pCr, pckgsize);

					sent = TRUE;
				}
			}
			else
			{
				assert(FALSE);
				return FALSE;
			}
		}
		else
		{//broad on channel
			//== PassToPlayer
			//do like this for optimize

			size_t syncsize = sizeof(CHAT_CHANNELCHAT_SYNC) + pChannelChatCmd->sentlen;
			CHAT_CHANNELCHAT_SYNC* pCccSync = (CHAT_CHANNELCHAT_SYNC*)_alloca(syncsize);
			pCccSync->ProtocolType = chat_channelchat;
			pCccSync->wSize = syncsize - 1;
			pCccSync->packageID = pChannelChatCmd->packageID;
			strcpy(pCccSync->someone, srcrole.c_str());
			pCccSync->channelid = pChannelChatCmd->channelid;
			pCccSync->sentlen = pChannelChatCmd->sentlen;
			memcpy(pCccSync + 1, pChannelChatCmd + 1, pChannelChatCmd->sentlen);


			for (IP2PLAYERSMAP::iterator itIP = rChannInfo.mapIp2Players.begin(); itIP != rChannInfo.mapIp2Players.end(); itIP++)
			{
				DWORD IP = (*itIP).first;
				PLAYERSMAP& rPlayers = (*itIP).second;

				MANLIST mans;

				for (PLAYERSMAP::iterator itPlayer = rPlayers.begin(); itPlayer != rPlayers.end(); itPlayer++)
				{
					DWORD param = (*itPlayer).first;
					PLAYERINFO& rInfo = (*itPlayer).second;

					if (rInfo.subed)
					{
						tagPlusSrcInfo friendInfo;
						friendInfo.nameid = rInfo.nameid;
						friendInfo.lnID = param;

						mans.push_back(friendInfo);
					}
				}

				if (!mans.empty())
				{
					PassToGroupMan(IP, FALSE, mans, pCccSync, syncsize);
				}

			}

			sent = TRUE;
		}
	}


//	{{
//	size_t _exsize = sizeof(DWORD) + sizeof(BYTE) + pChannelChatCmd->sentlen;	////X
//	size_t feedbacksize = sizeof(CHAT_FEEDBACK) + _exsize;
//	CHAT_FEEDBACK* pCfb = (CHAT_FEEDBACK*)_alloca(feedbacksize);
//	pCfb->ProtocolType = chat_feedback;
//	pCfb->packageID = pChannelChatCmd->packageID;
//	pCfb->code = sent ? codeSucc : codeFail;
//
//	////X
//	DWORD* pChannelid = (DWORD*)(pCfb + 1);
//	*pChannelid = pChannelChatCmd->channelid;
//	BYTE* pSentlen = (BYTE*)(pChannelid + 1);
//	*pSentlen = pChannelChatCmd->sentlen;
//	void* pSent = pSentlen + 1;
//	memcpy(pSent, pChannelChatCmd + 1, pChannelChatCmd->sentlen);
//	////X
//
//	PassToSpecMan(srcIP, SrcInfo.lnID, SrcInfo.nameid, pCfb, feedbacksize);
//	}}

	
//	dTRACE("Chat Channel: sentence %u Bs from [%s] on <%08X>", pChannelChatCmd->sentlen, srcrole.c_str(), pChannelChatCmd->channelid);


	return TRUE;
}

BOOL CChannelMgr::IsNeedRelegate(const std::_tstring& channname, BOOL filter)
{
	if (channname[0] != m_chChannEsc)
		return FALSE;
	if (channname[1] == m_chChannSCRN)
		return TRUE;
	return !filter;
}



BOOL CChannelMgr::QueryChannelID(const std::_tstring& channel, DWORD ip, unsigned long param, DWORD nameid)
{
	assert(FALSE);
	return FALSE;

	BLOCKCHANNELOP_QID* pOp = m_allcQID.allocate(1, NULL);
	pOp->opr = _BLOCKCHANNELOP::op_querychannelid;
	pOp->param = param;
	pOp->channelname = channel;
	pOp->nameid = nameid;

	DUMMY_AUTOLOCKWRITE(m_lockBlockOp);
	BLOCKOPLIST& rOps = m_mapIp2Ops[ip];
	rOps.push_back(pOp);

	return TRUE;
}

BOOL CChannelMgr::Subscribe(DWORD ip, unsigned long param, DWORD channelid)
{
	assert(FALSE);
	return FALSE;

	BLOCKCHANNELOP_CMN* pOp = m_allcCMN.allocate(1, NULL);
	pOp->opr = _BLOCKCHANNELOP::op_subscribe;
	pOp->param = param;
	pOp->channelid = channelid;

	DUMMY_AUTOLOCKWRITE(m_lockBlockOp);
	BLOCKOPLIST& rOps = m_mapIp2Ops[ip];
	rOps.push_back(pOp);

	return TRUE;
}

BOOL CChannelMgr::Unsubscribe(DWORD ip, unsigned long param, DWORD channelid)
{
	assert(FALSE);
	return FALSE;

	BLOCKCHANNELOP_CMN* pOp = m_allcCMN.allocate(1, NULL);
	pOp->opr = _BLOCKCHANNELOP::op_unsubscribe;
	pOp->param = param;
	pOp->channelid = channelid;

	DUMMY_AUTOLOCKWRITE(m_lockBlockOp);
	BLOCKOPLIST& rOps = m_mapIp2Ops[ip];
	rOps.push_back(pOp);

	return TRUE;
}

BOOL CChannelMgr::FreeChannID(DWORD channid, DWORD ip, unsigned long param)
{
	assert(FALSE);
	return FALSE;

	BLOCKCHANNELOP_CMN* pOp = m_allcCMN.allocate(1, NULL);
	pOp->opr = _BLOCKCHANNELOP::op_freechannelid;
	pOp->param = param;
	pOp->channelid = channid;

	DUMMY_AUTOLOCKWRITE(m_lockBlockOp);
	BLOCKOPLIST& rOps = m_mapIp2Ops[ip];
	rOps.push_back(pOp);

	return TRUE;
}

BOOL CChannelMgr::ClearPlayer(DWORD ip, unsigned long param)
{
	assert(FALSE);
	return FALSE;

	BLOCKCHANNELOP_CLR* pOp = m_allcCLR.allocate(1, NULL);
	pOp->opr = _BLOCKCHANNELOP::op_clearplayer;
	pOp->param = param;

	DUMMY_AUTOLOCKWRITE(m_lockBlockOp);
	BLOCKOPLIST& rOps = m_mapIp2Ops[ip];
	rOps.push_back(pOp);

	return TRUE;
}

DWORD CChannelMgr::DoBlockOp(DWORD lmt)
{
	assert(FALSE);
	return FALSE;

	DUMMY_AUTOLOCKWRITE(m_lockBlockOp);

	for (IP2OPSMAP::iterator itIP = m_mapIp2Ops.begin(); itIP != m_mapIp2Ops.end(); itIP++)
	{
		DWORD ip = (*itIP).first;
		BLOCKOPLIST& rOps = (*itIP).second;

		for (BLOCKOPLIST::iterator itOp = rOps.begin(); itOp != rOps.end(); itOp++)
		{
			_BLOCKCHANNELOP* pOp = *itOp;
			assert(pOp);

			switch (pOp->opr)
			{
			case _BLOCKCHANNELOP::op_querychannelid:
				{{
				BLOCKCHANNELOP_QID* pTheOp = (BLOCKCHANNELOP_QID*)pOp;

				Block_QueryChannelID(pTheOp->channelname, ip, pTheOp->param, pTheOp->nameid);

				m_allcQID.deallocate(pTheOp, 1);
				}}
				break;
			case _BLOCKCHANNELOP::op_subscribe:
				{{
				BLOCKCHANNELOP_CMN* pTheOp = (BLOCKCHANNELOP_CMN*)pOp;

				Block_Subscribe(ip, pTheOp->param, pTheOp->channelid);

				m_allcCMN.deallocate(pTheOp, 1);
				}}
				break;
			case _BLOCKCHANNELOP::op_unsubscribe:
				{{
				BLOCKCHANNELOP_CMN* pTheOp = (BLOCKCHANNELOP_CMN*)pOp;

				Block_Unsubscribe(ip, pTheOp->param, pTheOp->channelid);

				m_allcCMN.deallocate(pTheOp, 1);
				}}
				break;
			case _BLOCKCHANNELOP::op_freechannelid:
				{{
				BLOCKCHANNELOP_CMN* pTheOp = (BLOCKCHANNELOP_CMN*)pOp;

				Block_FreeChannID(ip, pTheOp->param, pTheOp->channelid);

				m_allcCMN.deallocate(pTheOp, 1);
				}}
				break;
			case _BLOCKCHANNELOP::op_clearplayer:
				{{
				BLOCKCHANNELOP_CLR* pTheOp = (BLOCKCHANNELOP_CLR*)pOp;

				Block_ClearPlayer(ip, pTheOp->param);

				m_allcCLR.deallocate(pTheOp, 1);
				}}
				break;
			default:
				assert(FALSE);
			}
		}
	}

	m_mapIp2Ops.clear();

	return 0;
}


BOOL CChannelMgr::Block_QueryChannelID(const std::_tstring& channel, DWORD ip, unsigned long param, DWORD nameid)
{
	std::_tstring channname = MakeChannelName(channel, ip);
	if (channname.empty())
		return FALSE;

	CNetConnectDup conndup;
	std::_tstring rolename;
	if (!g_HostServer.FindPlayerByIpParam(NULL, ip, param, &conndup, NULL, &rolename, NULL))
		return FALSE;



	DUMMY_AUTOLOCKWRITE(m_lockChannel);


	DWORD channid = -1;
	BYTE cost = 0;
	IP2PLAYERSMAP* pMapI2P = NULL;


	dTRACE("Query ChannelID: %s from (%08X, %08X)", channel.c_str(), ip, param);


	CHANN2IDMAP::iterator it = m_mapChann2ID.find(channname);
	if (it != m_mapChann2ID.end())
	{//channel exist
		channid = (*it).second;

		CHANNINFO& rChannInfo = m_mapChannid2Info[channid];

		cost = rChannInfo.cost;
		if (channid != m_channidGM)
			pMapI2P = &rChannInfo.mapIp2Players;
	}
	else
	{
		if (channname[0] != m_chChannEsc)
			return FALSE;

		//create the channel
		channid = GenChannID();
		if (channid != DWORD(-1))
		{
			m_mapChann2ID[channname] = channid;

			CHANNINFO& rChannInfo = m_mapChannid2Info[channid];
			rChannInfo.god = FALSE;
			rChannInfo.stock = FALSE;
			rChannInfo.channname = channname;
			rChannInfo.gmuse = FALSE;
			rChannInfo.gmsub = FALSE;
			rChannInfo.cost = GainPreCost(channel);

			rTRACE("Create Channel: <%08X> %s", channid, channname.c_str());

			cost = rChannInfo.cost;
			pMapI2P = &rChannInfo.mapIp2Players;
		}
	}

	if (channid != DWORD(-1))
	{
		if (pMapI2P)
		{
			PLAYERSMAP& rPlayers = (*pMapI2P)[ip];
			PLAYERINFO& rInfo = rPlayers[param];
			rInfo.nameid = nameid;
			rInfo.subed = FALSE;
		}
	}


	{{
	//notify channelid
	size_t methodsize = _NAME_LEN + sizeof(DWORD) * 2;
	size_t pckgsize = sizeof(RELAY_ASKWAY_DATA) + methodsize + sizeof(PLAYERCOMM_NOTIFYCHANNELID);

	RELAY_ASKWAY_DATA* pAskWayData = (RELAY_ASKWAY_DATA*)_alloca(pckgsize);
	pAskWayData->ProtocolFamily = pf_relay;
	pAskWayData->ProtocolID = relay_c2c_askwaydata;
	pAskWayData->nFromIP = 0;
	pAskWayData->nFromRelayID = 0;
	pAskWayData->seekRelayCount = 0;
	pAskWayData->seekMethod = rm_role_id;
	pAskWayData->wMethodDataLength = methodsize;
	pAskWayData->routeDateLength = sizeof(PLAYERCOMM_NOTIFYCHANNELID);

	char* pRoleName = (char*)(pAskWayData + 1);
	strcpy(pRoleName, rolename.c_str());

	DWORD* pRoleInfo = (DWORD*)(pRoleName + _NAME_LEN);
	pRoleInfo[0] = nameid;
	pRoleInfo[1] = param;

	PLAYERCOMM_NOTIFYCHANNELID* pPlayerCommNID = (PLAYERCOMM_NOTIFYCHANNELID*)(pRoleInfo + 2);
	pPlayerCommNID->ProtocolFamily = pf_playercommunity;
	pPlayerCommNID->ProtocolID = playercomm_s2c_notifychannelid;
	strcpy(pPlayerCommNID->channel, channel.c_str());
	pPlayerCommNID->channelid = channid;
	pPlayerCommNID->cost = cost;

	conndup.SendPackage(pAskWayData, pckgsize);
	}}


	return TRUE;
}

BOOL CChannelMgr::Block_Subscribe(DWORD ip, unsigned long param, DWORD channelid)
{
	if (channelid == -1)
		return FALSE;
	
	if (channelid == m_channidGM)
	{
		dTRACE("Subscribe Channel: GM <%08X> by (%08X,%08X)", channelid, ip, param);
		return TRUE;
	}

	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channelid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;
	PLAYERSMAP& rPlayers = rChannInfo.mapIp2Players[ip];
	PLAYERSMAP::iterator itRole = rPlayers.find(param);
	if (itRole == rPlayers.end())
		return FALSE;

	PLAYERINFO& rPlayerInfo = (*itRole).second;
	rPlayerInfo.subed = TRUE;

	dTRACE("Subscribe Channel: %08X by (%08X,%08X)", channelid, ip, param);

	return TRUE;
}

BOOL CChannelMgr::Block_Unsubscribe(DWORD ip, unsigned long param, DWORD channelid)
{
	if (channelid == m_channidGM)
		return FALSE;

	
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	CHANNID2INFOMAP::iterator itChannid = m_mapChannid2Info.find(channelid);
	if (itChannid == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*itChannid).second;

	IP2PLAYERSMAP::iterator itPlayers = rChannInfo.mapIp2Players.find(ip);
	if (itPlayers == rChannInfo.mapIp2Players.end())
		return FALSE;

	PLAYERSMAP& rPlayers = (*itPlayers).second;
	PLAYERSMAP::iterator itRole = rPlayers.find(param);
	if (itRole == rPlayers.end())
		return FALSE;

	PLAYERINFO& rInfo = (*itRole).second;
	rInfo.subed = FALSE;

	dTRACE("Unsubscribe Channel: %08X by (%08X,%08X)", channelid, ip, param);

	return TRUE;
}

BOOL CChannelMgr::Block_FreeChannID(DWORD channid, DWORD ip, unsigned long param)
{
	if (channid == -1)
		return FALSE;

	dTRACE("Free ChannelID: <%08X> from (%08X, %08X)", channid, ip, param);

	
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	if (channid == m_channidGM)
		return TRUE;


	CHANNID2INFOMAP::iterator it = m_mapChannid2Info.find(channid);
	if (it == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*it).second;

	if (rChannInfo.god)
		return TRUE;

	IP2PLAYERSMAP::iterator itPlayers = rChannInfo.mapIp2Players.find(ip);
	if (itPlayers == rChannInfo.mapIp2Players.end())
		return FALSE;

	PLAYERSMAP& rPlayers = (*itPlayers).second;
	rPlayers.erase(param);
	if (rPlayers.empty())
	{
		rChannInfo.mapIp2Players.erase(itPlayers);

		if (!rChannInfo.stock && !rChannInfo.gmuse
			&& rChannInfo.mapIp2Players.empty())
		{
			rTRACE("Destroy Channel: <%08X> %s", channid, rChannInfo.channname.c_str());

			m_mapChann2ID.erase(rChannInfo.channname);
			m_mapChannid2Info.erase(it);
		}
	}


	return TRUE;
}

BOOL CChannelMgr::Block_ClearPlayer(DWORD ip, unsigned long param)
{
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	CHANNID2INFOMAP::iterator itChann = m_mapChannid2Info.begin();
	while (itChann != m_mapChannid2Info.end())
	{
		CHANNID2INFOMAP::iterator itChannMe = itChann++;
		DWORD channid = (*itChannMe).first;
		CHANNINFO& rChannInfo = (*itChannMe).second;

		if (channid == m_channidGM)
			continue;

		IP2PLAYERSMAP::iterator itIP = rChannInfo.mapIp2Players.find(ip);
		if (itIP == rChannInfo.mapIp2Players.end())	//没找到此GS
			continue;

		IP2PLAYERSMAP::iterator itIPMe = itIP++;
		PLAYERSMAP& rPlayers = (*itIPMe).second;

		//删除此人
		rPlayers.erase(param);

		if (!rPlayers.empty())	//此GS上还有人
			continue;

		//GS上无人，删除此GS
		rChannInfo.mapIp2Players.erase(itIPMe);

		if (rChannInfo.stock || rChannInfo.gmuse)	//频道不可删
			continue;

		if (!rChannInfo.mapIp2Players.empty())	//如果此频道还有GS
			continue;

		rTRACE("Destroy Channel: <%08X> %s", channid, rChannInfo.channname.c_str());

		//删除频道
		m_mapChann2ID.erase(rChannInfo.channname);
		m_mapChannid2Info.erase(itChannMe);
	}

	return TRUE;
}

BOOL CChannelMgr::B_QueryChannelID(const std::_tstring& channel, DWORD ip, unsigned long param, DWORD nameid)
{
	return Block_QueryChannelID(channel, ip, param, nameid);
}

BOOL CChannelMgr::B_Subscribe(DWORD ip, unsigned long param, DWORD channelid)
{
	return Block_Subscribe(ip, param, channelid);
}

BOOL CChannelMgr::B_Unsubscribe(DWORD ip, unsigned long param, DWORD channelid)
{
	return Block_Unsubscribe(ip, param, channelid);
}

BOOL CChannelMgr::B_FreeChannID(DWORD channid, DWORD ip, unsigned long param)
{
	return Block_FreeChannID(channid, ip, param);
}

BOOL CChannelMgr::B_ClearPlayer(DWORD ip, unsigned long param)
{
	return Block_ClearPlayer(ip, param);
}


size_t CChannelMgr::GetChannelCount()
{
	DUMMY_AUTOLOCKWRITE(m_lockChannel);

	return m_mapChannid2Info.size();
}


BOOL CChannelMgr::SayOnSomeone(DWORD ip, unsigned long param, DWORD nameid, const std::_tstring& name, const std::_tstring& sent)
{
	CNetConnectDup hostconndup;
	if (!g_HostServer.FindPlayerByIpParam(NULL, ip, param, &hostconndup, NULL, NULL, NULL))
		return FALSE;

	size_t syncsize = sizeof(CHAT_SOMEONECHAT_SYNC) + sent.size();
	CHAT_SOMEONECHAT_SYNC* pCscSync = (CHAT_SOMEONECHAT_SYNC*)_alloca(syncsize);
	pCscSync->ProtocolType = chat_someonechat;
	pCscSync->wSize = syncsize - 1;
	pCscSync->packageID = -1;
	strcpy(pCscSync->someone, name.c_str());
	pCscSync->sentlen = sent.size();
	memcpy(pCscSync + 1, sent.data(), sent.size());

	if (!PassToSpecMan(hostconndup.GetIP(), param, nameid, pCscSync, syncsize))
		return FALSE;

	return TRUE;
}

BOOL CChannelMgr::SayOnChannel(DWORD channid, BOOL filter, const std::_tstring& ids, const std::_tstring& name, const std::_tstring& sent)
{
	DUMMY_AUTOLOCKREAD(m_lockChannel);


	CHANNID2INFOMAP::iterator itChannid = m_mapChannid2Info.find(channid);
	if (itChannid == m_mapChannid2Info.end())
		return FALSE;

	CHANNINFO& rChannInfo = (*itChannid).second;

	if (rChannInfo.god)
	{
		if (!g_RootClient.IsReady())
			return FALSE;

		size_t pckgsize = sizeof(CHAT_MSG_EX) + sent.size();
		CHAT_MSG_EX* pChatMsgEx = (CHAT_MSG_EX*)_alloca(pckgsize);
		pChatMsgEx->ProtocolFamily = pf_playercommunity;
		pChatMsgEx->ProtocolID = playercomm_channelchat;
		strcpy(pChatMsgEx->m_szSourceName, name.c_str());
		strcpy(pChatMsgEx->m_szAccountName, ids.c_str());
		pChatMsgEx->SentenceLength = sent.size();
		memcpy(pChatMsgEx + 1, sent.data(), sent.size());

		PassToGM(pChatMsgEx, pckgsize);
	}
	else
	{
		if (IsNeedRelegate(rChannInfo.channname, filter))
		{//relegate to gamesvr
			assert(rChannInfo.channname[0] == m_chChannEsc);

			DWORD channIP = 0;
			DWORD theID = ParseChannelName(rChannInfo.channname, &channIP);
			if (theID == -1 && channIP == 0)
				return FALSE;


			assert(rChannInfo.channname.size() >= 2);


			size_t syncsize = sizeof(CHAT_CHANNELCHAT_SYNC) + sent.size();
			size_t pckgsize = sizeof(CHAT_RELEGATE) + syncsize;

			CHAT_RELEGATE* pCr = (CHAT_RELEGATE*)_alloca(pckgsize);
			pCr->ProtocolType = chat_relegate;
			pCr->wSize = pckgsize - 1;
			pCr->nFromIP = 0;
			pCr->nFromRelayID = -1;
			pCr->channelid = channid;
			pCr->routeDateLength = syncsize;

			CHAT_CHANNELCHAT_SYNC* pCccSync = (CHAT_CHANNELCHAT_SYNC*)(pCr + 1);
			pCccSync->ProtocolType = chat_channelchat;
			pCccSync->wSize = syncsize - 1;
			pCccSync->packageID = -1;
			strcpy(pCccSync->someone, name.c_str());
			pCccSync->channelid = channid;
			pCccSync->sentlen = sent.size();
			memcpy(pCccSync + 1, sent.data(), sent.size());


			if (rChannInfo.channname[1] == m_chChannTEAM)
			{
				if (theID == -1 || channIP == 0)
					return FALSE;

				CHANNINFO& rChannInfo = (*itChannid).second;

				CNetConnectDup connDup = g_ChatServer.FindChatConnectByIP(channIP);
				if (!connDup.IsValid())
					return FALSE;

				pCr->TargetCls = tgtcls_team;
				pCr->TargetID = theID;

				connDup.SendPackage(pCr, pckgsize);
			}
			else if (rChannInfo.channname[1] == m_chChannFAC)
			{
				if (theID == -1)
					return FALSE;

				pCr->TargetCls = tgtcls_fac;
				pCr->TargetID = theID;

				g_ChatServer.BroadPackage(pCr, pckgsize);
			}
			else if (rChannInfo.channname[1] == m_chChannTONG)
			{
				if (theID == -1)
					return FALSE;

				pCr->TargetCls = tgtcls_tong;
				pCr->TargetID = theID;

				g_ChatServer.BroadPackage(pCr, pckgsize);
			}
			else if (rChannInfo.channname[1] == m_chChannSCRN)
			{
				//screen
				assert(FALSE);
				return FALSE;
			}
			else if (rChannInfo.channname[1] == m_chChannBC)
			{
				if (channIP == 0)
					return FALSE;

				assert(channIP != 0 && theID == 0);

				CNetConnectDup connDup = g_ChatServer.FindChatConnectByIP(channIP);
				if (!connDup.IsValid())
					return FALSE;

				pCr->TargetCls = tgtcls_bc;
				pCr->TargetID = 0;

				connDup.SendPackage(pCr, pckgsize);
			}
			else
			{
				assert(FALSE);
				return FALSE;
			}
		}
		else
		{//broad on channel
			//== PassToPlayer
			//do like this for optimize

			size_t syncsize = sizeof(CHAT_CHANNELCHAT_SYNC) + sent.size();
			CHAT_CHANNELCHAT_SYNC* pCccSync = (CHAT_CHANNELCHAT_SYNC*)_alloca(syncsize);
			pCccSync->ProtocolType = chat_channelchat;
			pCccSync->wSize = syncsize - 1;
			pCccSync->packageID = -1;
			strcpy(pCccSync->someone, name.c_str());
			pCccSync->channelid = channid;
			pCccSync->sentlen = sent.size();
			memcpy(pCccSync + 1, sent.data(), sent.size());


			for (IP2PLAYERSMAP::iterator itIP = rChannInfo.mapIp2Players.begin(); itIP != rChannInfo.mapIp2Players.end(); itIP++)
			{
				DWORD IP = (*itIP).first;
				PLAYERSMAP& rPlayers = (*itIP).second;

				MANLIST mans;

				for (PLAYERSMAP::iterator itPlayer = rPlayers.begin(); itPlayer != rPlayers.end(); itPlayer++)
				{
					DWORD param = (*itPlayer).first;
					PLAYERINFO& rInfo = (*itPlayer).second;

					if (rInfo.subed)
					{
						tagPlusSrcInfo friendInfo;
						friendInfo.nameid = rInfo.nameid;
						friendInfo.lnID = param;

						mans.push_back(friendInfo);
					}
				}

				if (!mans.empty())
				{
					PassToGroupMan(IP, FALSE, mans, pCccSync, syncsize);
				}
			}
		}
	}


	return TRUE;
}
