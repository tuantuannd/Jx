//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KRelayProtocol.h
// Date:	2003.06.02
// Code:	xiemaopei
//---------------------------------------------------------------------------

#ifndef KPLAYERPROTOCOL_H
#define KPLAYERPROTOCOL_H

#pragma	pack(push, 1)

enum PROTOCOL_FAMILY
{
	pf_normal = 0,
	pf_relay,
	pf_gameworld,
	pf_gamemaster,
	pf_playercommunity,
	pf_chat,	//reserved
	pf_tong,
	pf_friend,

	pf_extend,
};


struct EXTEND_HEADER
{
	BYTE	ProtocolFamily;							//协议所属的家族
	BYTE	ProtocolID;								//协议名称
};

enum ROUTESERVER_METHOD			//用于relay之间转换之用
{
	rm_account_id = 1,			//根据帐号映射为相应标识(tagEnterGame2, tagLeaveGame2同步路由数据)
	rm_map_id,					//根据地图映射为相应标识(通过网关来取得路由数据)
	rm_role_id,					//根据角色映射为相应标识(tagEnterGame2, tagLeaveGame2同步路由数据)
	rm_gm,						//直接发给GM的东东
};

// 协议id：relay_c2c_data  （变长协议）
// 协议id：relay_s2c_loseway  （变长协议）			nToIP 不能广播
struct RELAY_DATA : EXTEND_HEADER
{
	DWORD	nToIP;									//255.255.255.255是广播, 0.0.0.0是目的地已经到达
	DWORD	nToRelayID;								//0是网内广播, -1是网外广播
	DWORD	nFromIP;								//0.0.0.0时表明刚进入Relay体系，Relay会填写nFromIP和nFromRelayID
	DWORD	nFromRelayID;
	WORD	routeDateLength;						//要转发数据长度，数据从结构结尾处开始
	//对应要转发的协议包数据
};


// 协议id：relay_c2c_askwaydata  （变长协议）

struct RELAY_ASKWAY_DATA : EXTEND_HEADER
{
	DWORD	nFromIP;								//0.0.0.0时表明刚进入Relay体系，Relay会填写nFromIP和nFromRelayID
	DWORD	nFromRelayID;
	DWORD	seekRelayCount;							//查找经历的Relay的个数，到-1时为不可达
	WORD	seekMethod;								//指定查找方法	
	WORD	wMethodDataLength;						//方法所需数据长度，数据从结构结尾处开始
	WORD	routeDateLength;						//要转发数据长度，数据从方法所需数据结尾处开始
	//对应方法所需数据
	//转发数据
};




////////////////////////////////////////////////////////
//pf_playercommunity

//extend protocol package's header
struct tagExtendProtoHeader
{
	BYTE ProtocolType;	//c2s_extend / s2c_extend
	WORD wLength;
	//extend pckg
};


#define __X_NAME_LEN_	32


//c2s_extend 包到达GameSvr时，GameSvr向包后添加此结构，以表明包来源
struct tagPlusSrcInfo
{
	DWORD nameid;
	unsigned long lnID;
};

//playercomm_c2s_querychannelid
//普通Channel：不以'\'开头的名称为普通Channel名称
//特殊Channel：（区分大小写）
//队伍：\Tnnnn，其中nnnn为队伍ID
//门派：\Fnnnn，其中nnnn为门派ID
//同屏：\S
struct PLAYERCOMM_QUERYCHANNELID : EXTEND_HEADER
{
	char channel[__X_NAME_LEN_];
};

//playercomm_s2c_notifychannelid
struct PLAYERCOMM_NOTIFYCHANNELID : EXTEND_HEADER
{
	char	channel[__X_NAME_LEN_];
	DWORD	channelid;
	BYTE	cost;
};

//playercomm_c2s_freechannelid
struct PLAYERCOMM_FREECHANNELID : EXTEND_HEADER
{
	DWORD channelid;
};

//playercomm_c2s_subscribe
struct PLAYERCOMM_SUBSCRIBE : EXTEND_HEADER
{
	DWORD channelid;
	BOOL subscribe;
};

//playercomm_s2c_gmquerychannelid
struct PLAYERCOMM_GMQUERYCHANNELID : EXTEND_HEADER
{
	char channel[__X_NAME_LEN_];
	BYTE force;
};

//playercomm_s2c_gmfreechannelid
struct PLAYERCOMM_GMFREECHANNELID : EXTEND_HEADER
{
	DWORD channelid;
};

//playercomm_s2c_gmlisten
struct PLAYERCOMM_GMSUBSCRIBE : EXTEND_HEADER
{
	DWORD channelid;
	BOOL subscribe;
};


struct CHAT_MSG_EX : EXTEND_HEADER
{
	char	m_szSourceName[__X_NAME_LEN_];	// 来源玩家角色名字
	char	m_szAccountName[__X_NAME_LEN_];	// 来源玩家账号名字
	WORD SentenceLength;
	//所说内容
};


#pragma pack(pop)

#endif
