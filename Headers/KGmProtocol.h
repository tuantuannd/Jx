//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KGmProtocol.h
// Date:	2003.06.02
// Code:	边城浪子
//---------------------------------------------------------------------------

#ifndef KGMPROTOCOL_H
#define KGMPROTOCOL_H

#pragma	pack(push, 1)

#define	defACCOUNT_STRING_LENGTH		32
#define defGAMEWORLD_STRING_LENGTH		32


typedef struct tagGMCmdHeader
{
	BYTE	ProtocolFamily;							// 协议所属的家族
	BYTE	ProtocolType;							// 协议名称
	char	AccountName[defACCOUNT_STRING_LENGTH];	// 操作账号名称
}GM_HEADER;

// GM对游戏中玩家的各种操作
// 协议id：gm_c2s_execute  （变长协议）
typedef struct tagGMExecuteCommand : tagGMCmdHeader
{
	WORD	wExecuteID;								// 操作id（== 1 执行脚本；>= 2 其他操作）
	WORD	wLength;								// 所携数据长度，数据从结构结尾处开始
	// 后面应该紧跟着对应操作id的操作参数数据，根据不同的操作id确定长度及相应内容
} GM_EXECUTE_COMMAND;

// 对游戏中玩家设定各种特殊被管理状态(实际操作时用拦截具体协议实现)
// 协议id：gm_c2s_disable  （变长协议）
typedef struct tagGMDisableActionCmd : tagGMCmdHeader
{
	BYTE	btProtocol;								// 被拦截协议编号
	WORD	wLength;								// 所携数据长度，数据从结构结尾处开始
	// 后面应该紧跟着对应被拦截协议编号的具体参数数据，长度由被拦截协议编号决定
} GM_DISABLE_ACTION_COMMAND;

// 对游戏中玩家取消各种特殊被管理状态
// 协议id：gm_c2s_enable  （变长协议）
typedef struct tagGMEnableActionCmd : tagGMCmdHeader
{
	BYTE	btProtocol;								// 取消被拦截协议编号
	WORD	wLength;								// 所携数据长度，数据从结构结尾处开始
	// 后面应该紧跟着对应取消被拦截协议编号的具体参数数据，长度由取消被拦截协议编号决定
} GM_ENABLE_ACTION_COMMAND;

// 对特定玩家数据进行跟踪
// 协议id：gm_c2s_tracking  （定长协议）
typedef struct tagTrackCmd : tagGMCmdHeader
{
	BYTE	ProtocolFamily;							//协议所属的家族
	BYTE	ProtocolType;							// 协议名称
	BYTE	btTrackFlag;							// 是否跟踪，1跟踪，0取消跟踪
} GM_TRACK_COMMAND;

// 发送特定玩家的跟踪数据给GM
// 协议id：gm_s2c_tracking  （变长协议）
typedef struct tagGMTrackSync : tagGMCmdHeader
{
	WORD	wLength;								// 所携数据包长度，数据从结构结尾处开始
	// 后面应该紧跟着一个数据包，这个数据包由游戏服务器发给玩家客户端的各种协议组成
} GM_TRACK_SYNC;

enum enumGMGetRoleCmds
{
	gm_role_entergame_position = 0,	//获取玩家进入游戏的位置，返回 GM_GET_ROLE_DATA_SUB_ENTER_POS
};

typedef struct tagGMGetRoleEnterGamePos
{
	int nSubWorldID;
	int	nEnterGameX;
	int nEnterGameY;
}GM_ROLE_DATA_SUB_ENTER_POS;

// 设定和取得角色的各种数值：（只能处理部分数据，如果需要比较大块的数据，直接查询角色服务器）
// 设定：协议id gm_c2s_setrole  （变长协议）
typedef struct tagGMSetRoleDataCmd : tagGMCmdHeader
{
	WORD	wSetID;									// 操作ID，设定哪一类数据
	WORD	wLength;								// 所携数据长度，数据从结构结尾处开始
	// 后面应该紧跟着对应操作ID的具体操作参数数据，根据不同的操作ID确定长度及相应内容
} GM_SET_ROLE_DATA_COMMAND;

// 取得：协议id gm_c2s_getrole  （变长协议）
typedef struct tagGMGetRoleDataCmd : tagGMCmdHeader
{
	WORD	wGetID;									// 操作ID，取得哪一类数据
	WORD	wLength;								// 所携数据长度，数据从结构结尾处开始
	// 后面应该紧跟着对应操作ID的具体操作参数数据，根据不同的操作ID确定长度及相应内容
} GM_GET_ROLE_DATA_COMMAND;

// 回复，游戏服务器发送玩家数据给GM系统
// 协议id：gm_s2c_getrole  （变长协议）
typedef struct tagGMGetRoleDataSync : tagGMCmdHeader
{
	WORD	wGetID;									// 操作ID，取得哪一类数据
	WORD	wLength;								// 所携数据长度，数据从结构结尾处开始
	// 后面应该紧跟着对应操作ID的具体操作参数数据，根据不同的操作ID确定长度及相应内容
} GM_GET_ROLE_DATA_SYNC;

// 获取：玩家所在游戏服务器（定长协议）
typedef struct tagGMGetPlayerAtGWCmd : tagGMCmdHeader
{
} GM_GET_PLAYER_AT_GW_CMD;


// 回复，游戏服务器返回GW服务器的名称
typedef struct tagGMGetPlayerAtGWSync : tagGMCmdHeader
{
	char szCurrentRoleName[32];
	int nX;
	int nY;
	int nWorldID;
} GM_GET_PLAYER_AT_GW_SYNC;

// 获取：从*数据库*获取玩家账号包含的所有角色名(定长)
typedef struct tagGMGetRoleListCmd : tagGMCmdHeader
{
} GM_GET_ROLE_LIST_CMD;

// 回复，游戏服务器返回GW服务器的名称(变长)
typedef struct tagGMGetRoleListSync : tagGMCmdHeader
{
	WORD wRoleCount;	//后面跟的三个 RoleBaseInfo 中有效结构的个数
} GM_GET_ROLE_LIST_SYNC;

// 账号解锁（定长协议）
typedef struct tagGMUnlockAccount : tagGMCmdHeader
{
}GM_UNLOCK_ACCOUNT;

// 广播协议（变长）
typedef struct tagGMBroadCastChatCmd : tagGMCmdHeader
{
	WORD wSentenceLen;	//后边跟广播的字符串
}GM_BROADCAST_CHAT_CMD;

typedef struct tagGMExecuteSync : tagGMCmdHeader
{
	WORD wStrLength;	//后面跟返回的字符串(char)长度，包含最后的0
}GM_EXECUTE_SYNC;

#pragma	pack(pop)
#endif
