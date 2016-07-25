/*****************************************************************************************
//	界面--login窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-13
------------------------------------------------------------------------------------------
	登陆过程中，所有客户端发给服务器端的消息都是c2s_login捆绑着一个KLoginStructHead结构的
数据；所有客户端发给服务器端的消息都是s2c_login捆绑着一个KLoginStructHead结构的数据。对于
一些情况来说，捆绑的是一个以KLoginStructHead为第一个成员的更大的结构。

    一个完整的与账号服务器建立连接过程中消息往返如下：
	发送请求：c2s_login & KLoginAccountInfo; KLoginInfo::Param = LOGIN_A_LOGIN | LOGIN_R_REQUEST;
	服务器返回登陆结果：s2c_login & KLoginAccountInfo;
		KLoginInfo::Param = LOGIN_A_LOGIN | (LOGIN_R_SUCCESS or LOGIN_R_FAILED or LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR);
	登陆过程结束。

	申请账号过程过程中消息往返如下：(注意：只有在调试版本才可以提供此操作。)
	发送请求：c2s_login & KLoginAccountInfo; KLoginInfo::Param = LOGIN_A_NEWACCOUNT | LOGIN_R_REQUEST;
	服务器返回的申请结果：s2c_login & KLoginAccountInfo;
		KLoginInfo::Param = LOGIN_A_NEWACCOUNT | (LOGIN_R_SUCCESS or LOGIN_R_FAILED or LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR or LOGIN_R_ACCOUNT_EXIST);
	申请过程结束。

	■■登陆过程在实际的执行中相互间可能会有时段重叠，所以客户端要依据KLoginInfo中的Account
	与Password数据项来判断服务器返回的消息是否为针对最后一次的登陆请求。如果不是则忽略之。■■

  一个完整的从账号服务器获取游戏服务器列表过程中消息往返如下：
	发送请求：c2s_login & KLoginStructHead; 
		KLoginStructHead::Param = LOGIN_A_SERVERLIST | LOGIN_R_REAUEST;
	服务器传送账号列表：s2c_login & KLoginServerList; 
		KLoginServerList::Param 的值为 LOGIN_A_SERVERLIST | (LOGIN_R_SUCCESS or LOGIN_R_VALID or LOGIN_R_INVALID or  LOGIN_R_FAILED);
		当列表包含的服务器项数目很多时，可以分多次分批发送，KLoginServerList::Param 的值为
		(LOGIN_A_SERVERLIST | LOGIN_R_VALID)时表示列表尚未传送完毕，下个数据包将继续传送未完列表，
		KLoginServerList::Param的值为LOGIN_A_SERVERLIST | LOGIN_R_SUCCESS时候，表示列表传送结束。
		如果客户端等待了指定的一段时间后，没有收到服务器列表数据包，则作列表结束处理。
	请求游戏服务器列表过程结束。

	■■上述规则描述对于请求游戏服务器列表的操作没有作登陆或连接前提限制，匿名暨可操作。■■

	■■可以考虑把传送的数据的结构里的空间定长字符串都改为变长（根据实例确定长度），以缩小
	需要网络传送的数据的长度。定长的存储账号与密码的结构是否改为变长，将依据网络加密方式特
	性单独确定。可以考虑把连接请求与申请账号过程中的多次账号密码传送改为只在发送请求时传送
	一次，同时辅以特殊的标识数值，一来减少账号密码的网络传送次数，增加一点安全性；二来也可
	减小要网络传送数据的量。■■
*****************************************************************************************/
#pragma once
#pragma pack(push, 1)

#include "KProtocol.h"

#define	LOGIN_ACCOUNT_MIN_LEN	6
#define LOGIN_ACCOUNT_MAX_LEN	16
#define LOGIN_PASSWORD_MIN_LEN	8
#define LOGIN_PASSWORD_MAX_LEN	16
#define LOGIN_ROLE_NAME_MIN_LEN	6
#define	LOGIN_ROLE_NAME_MAX_LEN	16

//login action return value
//the folloin' value is used in login connect operaion
#define	LOGIN_ACTION_FILTER					0xff0000	//登陆过程中的操作类型过滤数值
#define LOGIN_A_CONNECT						0x010000	//连接
#define	LOGIN_A_NEWACCOUNT					0x020000	//新建账号（账号服务器）
#define	LOGIN_A_SERVERLIST					0x030000	//获取服务器列表（账号服务器）
#define	LOGIN_A_REPORT						0x040000	//通知服务器在线(定期向服务器发送)
#define	LOGIN_A_LOGIN						0x050000	//登陆服务器（账号、游戏服务器）
#define	LOGIN_A_LOGOUT						0x060000	//注销
#define	LOGIN_A_CHARACTERLIST				0x070000	//获取角色列表（游戏服务器）


#define	LOGIN_R_REQUEST						0	//when the login request is send form client to server
#define	LOGIN_R_SUCCESS						1
#define	LOGIN_R_FAILED						2
#define	LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR	3
#define	LOGIN_R_ACCOUNT_EXIST				4
#define	LOGIN_R_TIMEOUT						5
#define	LOGIN_R_IN_PROGRESS					6
#define	LOGIN_R_NO_IN_PROGRESS				7
#define	LOGIN_R_VALID						8    //合法
#define	LOGIN_R_INVALID						9    //非法
// Add by Freeway Chen in 2003.7.1
#define LOGIN_R_INVALID_PROTOCOLVERSION     10   //协议版本不兼容，详见 KProtocolDef.h's KPROTOCOL_VERSION
#define LOGIN_R_FREEZE						11	//被冻结


struct KLoginStructHead
{
	unsigned short	Size;		//size of the struct, if the struct is been inherit, the size is ref the the derive struct
	int				Param;		//be one of the LOGIN_R_* define value combin with a LOGIN_A_* value.
};

struct KLoginAccountInfo : KLoginStructHead
{
	char	        Account[32];	//account
	KSG_PASSWORD    Password;
	unsigned long   nLeftTime;     //剩余时间
    
    #ifdef USE_KPROTOCOL_VERSION
    // Add by Freeway Chen in 2003.7.1
    unsigned    ProtocolVersion;     // 协议版本，详见 KProtocolDef.h's KPROTOCOL_VERSION
    #endif
};


#pragma pack(pop)