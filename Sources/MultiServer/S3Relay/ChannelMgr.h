// ChannelMgr.h: interface for the CChannelMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELMGR_H__513C195F_C7DE_4591_A165_EF0FF2087213__INCLUDED_)
#define AFX_CHANNELMGR_H__513C195F_C7DE_4591_A165_EF0FF2087213__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <list>

class CChannelMgr  
{
public:
	CChannelMgr();
	virtual ~CChannelMgr();

public:
	BOOL Initialize();
	BOOL Uninitialize();

public:
	BOOL B_QueryChannelID(const std::_tstring& channel, DWORD ip, unsigned long param, DWORD nameid);
	BOOL B_Subscribe(DWORD ip, unsigned long param, DWORD channelid);
	BOOL B_Unsubscribe(DWORD ip, unsigned long param, DWORD channelid);
	BOOL B_FreeChannID(DWORD channid, DWORD ip, unsigned long param);
	BOOL B_ClearPlayer(DWORD ip, unsigned long param);

	BOOL QueryChannelID(const std::_tstring& channel, DWORD ip, unsigned long param, DWORD nameid);
	BOOL Subscribe(DWORD ip, unsigned long param, DWORD channelid);
	BOOL Unsubscribe(DWORD ip, unsigned long param, DWORD channelid);
	BOOL FreeChannID(DWORD channid, DWORD ip, unsigned long param);
	BOOL ClearPlayer(DWORD ip, unsigned long param);

	DWORD DoBlockOp(DWORD lmt);

	BOOL IsUsed(DWORD ip, unsigned long param, DWORD channid);
	BOOL IsSubscribed(DWORD ip, unsigned long param, DWORD channid);

	BOOL GmQueryChannelID(const std::_tstring& channel, BOOL force);
	BOOL GmFreeChannID(DWORD channid);
	BOOL IsGmUsed(DWORD channid);

	BOOL GmSubscribe(DWORD channid);
	BOOL GmUnsubscribe(DWORD channid);
	BOOL IsGmSubscribed(DWORD channid);

	std::_tstring GetChannelName(DWORD channelid, BOOL adv = FALSE);
	DWORD GetChannelID(const std::_tstring& channname, DWORD advIP = 0);

public:
	std::_tstring MakeChannelName(const std::_tstring& chann, DWORD ip);
	std::_tstring ReduceChannelName(const std::_tstring& chann);
	DWORD ParseChannelName(const std::_tstring& chann, DWORD* pIP);

	BYTE GainPreCost(const std::_tstring& channel);

	DWORD GetGMChannID();
	BOOL IsGodChannel(DWORD channid);
	BOOL IsStockChannel(DWORD channid);

private:
	enum {
		defCHANN_esc	= '\\',
		defCHANN_splt	= ':',

		defCHANN_TEAM	= 'T',
		defCHANN_FAC	= 'F',
		defCHANN_TONG	= 'O',
		defCHANN_SCRN	= 'S',
		defCHANN_BC		= 'B',
	};

	TCHAR m_chChannEsc;
	TCHAR m_chChannSplt;

	TCHAR m_chChannTEAM;
	TCHAR m_chChannFAC;
	TCHAR m_chChannTONG;
	TCHAR m_chChannSCRN;
	TCHAR m_chChannBC;

private:
	DWORD GenChannID();
	BOOL IsNeedRelegate(const std::_tstring& channname, BOOL filter);

	BOOL PassToGM(const void* pData, size_t size);
	BOOL PassToSpecMan(DWORD ToIP, unsigned long ToRelayID, DWORD ToNameID, const void* pData, size_t size);

	typedef std::list<tagPlusSrcInfo>	MANLIST;
	BOOL PassToGroupMan(DWORD ToIP, BOOL verify, const MANLIST& mans, const void* pData, size_t size);

private:
	BOOL IsValidTeamID(DWORD idTeam);
	BOOL IsValidFacID(DWORD idFac);
	BOOL IsValidTongID(DWORD idTong);

private:
	DWORD m_lastChannID;

	DWORD m_channidGM;

	struct PLAYERINFO
	{
		DWORD nameid;
		BOOL subed;
	};
	typedef std::map<unsigned long, PLAYERINFO>	PLAYERSMAP;
	typedef std::map<DWORD, PLAYERSMAP>	IP2PLAYERSMAP;
	struct CHANNINFO
	{
		BOOL god;
		BOOL stock;
		std::_tstring channname;
		IP2PLAYERSMAP mapIp2Players;
		BOOL gmuse;
		BOOL gmsub;
		BYTE cost;	//
	};
	typedef std::map<DWORD, CHANNINFO>	CHANNID2INFOMAP;
	CHANNID2INFOMAP m_mapChannid2Info;

	typedef std::map<std::_tstring, DWORD, _tstring_less>	CHANN2IDMAP;
	CHANN2IDMAP m_mapChann2ID;

	DWORD m_minTeamID;
	DWORD m_maxTeamID;
	DWORD m_minFacID;
	DWORD m_maxFacID;
	DWORD m_minTongID;
	DWORD m_maxTongID;

	BYTE m_defCost;

	BYTE m_costTeam;
	BYTE m_costFac;
	BYTE m_costTong;
	BYTE m_costScrn;
	BYTE m_costBc;

	CLockMRSW m_lockChannel;

public:
	BOOL SomeoneChat(DWORD srcIP, const tagPlusSrcInfo& SrcInfo, const CHAT_SOMEONECHAT_CMD* pSomeoneChatCmd);
	BOOL ChannelChat(DWORD srcIP, const tagPlusSrcInfo& SrcInfo, const CHAT_CHANNELCHAT_CMD* pChannelChatCmd);

public:
	BOOL SayOnSomeone(DWORD ip, unsigned long param, DWORD nameid, const std::_tstring& name, const std::_tstring& sent);
	BOOL SayOnChannel(DWORD channid, BOOL filter, const std::_tstring& ids, const std::_tstring& name, const std::_tstring& sent);

private:
	struct _BLOCKCHANNELOP
	{
		enum {
			op_querychannelid,
			op_subscribe,
			op_unsubscribe,
			op_freechannelid,
			op_clearplayer,
		} opr;
		unsigned long param;
	};
	struct BLOCKCHANNELOP_QID : _BLOCKCHANNELOP	//querychannelid
	{
		std::_tstring channelname;
		DWORD nameid;
	};
	std::allocator<BLOCKCHANNELOP_QID> m_allcQID;

	struct BLOCKCHANNELOP_CMN : _BLOCKCHANNELOP	//(un)subscribe, freechannelid
	{
		DWORD channelid;
	};
	std::allocator<BLOCKCHANNELOP_CMN> m_allcCMN;
	struct BLOCKCHANNELOP_CLR : _BLOCKCHANNELOP	//clearplayer
	{
	};
	std::allocator<BLOCKCHANNELOP_CLR> m_allcCLR;

	typedef std::list<_BLOCKCHANNELOP*>	BLOCKOPLIST;
	typedef std::map<DWORD, BLOCKOPLIST>	IP2OPSMAP;
	IP2OPSMAP m_mapIp2Ops;

	CLockMRSW m_lockBlockOp;

private:
	BOOL Block_QueryChannelID(const std::_tstring& channel, DWORD ip, unsigned long param, DWORD nameid);
	BOOL Block_FreeChannID(DWORD channid, DWORD ip, unsigned long param);
	BOOL Block_Subscribe(DWORD ip, unsigned long param, DWORD channelid);
	BOOL Block_Unsubscribe(DWORD ip, unsigned long param, DWORD channelid);
	BOOL Block_ClearPlayer(DWORD ip, unsigned long param);

public:
	size_t GetChannelCount();
};

#endif // !defined(AFX_CHANNELMGR_H__513C195F_C7DE_4591_A165_EF0FF2087213__INCLUDED_)
