//------------------------------------------------------------------------------------------
//	界面--login窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-13
//	Modifyed by Yangxiaodong later
//------------------------------------------------------------------------------------------
#pragma once

#define LOGIN_A_TAG						0
#define LOGIN_R_TAG						0

#define LOGIN_ACCOUNT_MIN_LEN			4
#define LOGIN_ACCOUNT_MAX_LEN			30
#define LOGIN_PASSWORD_MIN_LEN			6
#define LOGIN_PASSWORD_MAX_LEN			30
#define LOGIN_REALNAME_MIN_LEN			4
#define LOGIN_REALNAME_MAX_LEN			30
#define GAMESERVERNAME_MAX_LEN			30

#define def_DBUSERNAME_MAX_LEN			16
#define def_DBUSERNAME_MIN_LEN			6
#define def_DBPASSWORD_MAX_LEN			16
#define def_DBPASSWORD_MIN_LEN			6
//login action return value
//the folloin' value is used in login connect operaion
#define LOGIN_ACTION_FILTER				0xffff0000	//登陆过程中的操作类型过滤数值
// Parameters from client to server
#define LOGIN_A_CONNECT					0x010000	//连接
#define LOGIN_A_NEWACCOUNT				0x020000	//新建账号
#define LOGIN_A_SERVERLIST				0x030000	//获取服务器列表
#define LOGIN_A_REPORT					0x040000	//通知服务器在线(定期向服务器发送)
#define LOGIN_A_LOGIN					0x050000	//登陆服务器(未开始游戏)
#define LOGIN_A_LOGOUT					0x060000	//注销
#define LOGIN_A_DEPOSIT					0x070000	//点卡冲值
#define LOGIN_A_DROPACCOUNT				0x080000	//删除账号
#define LOGIN_A_BEGINGAME				0x090000	//选定游戏服务器开始玩游戏
#define LOGIN_A_DECACCOUNTDEPOSIT		0x0a0000	//扣减账号点卡金额(单位为秒),并通知账号服务器其在线

// 数据库传输消息类型定义
//------>BEGIN
#define DBLOGIN_A_LOGIN					0x100000	//登录数据库
#define DBLOGIN_A_LOGOUT				0x200000	//从数据库注销
#define DBLOGIN_A_ADDDBUSER				0x300000	//添加数据库管理员
#define DBLOGIN_A_DELDBUSER				0x400000	//删除数据库管理员
#define DBLOGIN_A_QUERYUSERLIST			0x500000	//获取数据库管理员列表
#define DBLOGIN_A_LOCKDATABASE			0x600000	//锁住账号数据库
#define DBLOGIN_A_ACTIVATEDATABASE		0x700000	//解锁账号数据库
#define DBLOGIN_A_CREATEACCOUNT			0x800000	//生成新账号
#define DBLOGIN_A_DELETEACCOUNT			0x900000	//删除账号
//<------END

// Parameters from server to client
#define LOGIN_R_CONNECT					LOGIN_A_CONNECT
#define LOGIN_R_NEWACCOUNT				LOGIN_A_NEWACCOUNT
#define LOGIN_R_SERVERLIST				LOGIN_A_SERVERLIST
#define LOGIN_R_LOGIN					LOGIN_A_LOGIN
#define LOGIN_R_REPORT					LOGIN_A_REPORT
#define LOGIN_R_LOGOUT					LOGIN_A_LOGOUT
#define LOGIN_R_DROPACCOUNT				LOGIN_A_DROPACCOUNT
#define LOGIN_R_BEGINGAME				LOGIN_A_BEGINGAME
#define LOGIN_R_DECACCOUNTDEPOSIT		LOGIN_A_DECACCOUNTDEPOSIT
// 数据库传输消息类型定义
//------>BEGIN
#define DBLOGIN_R_LOGIN					DBLOGIN_A_LOGIN
#define DBLOGIN_R_LOGOUT				DBLOGIN_A_LOGOUT
#define DBLOGIN_R_ADDDBUSER				DBLOGIN_A_ADDDBUSER
#define DBLOGIN_R_DELDBUSER				DBLOGIN_A_DELDBUSER
#define DBLOGIN_R_QUERYUSERLIST			DBLOGIN_A_QUERYUSERLIST
#define DBLOGIN_R_LOCKDATABASE			DBLOGIN_A_LOCKDATABASE
#define DBLOGIN_R_ACTIVATEDATABASE		DBLOGIN_A_ACTIVATEDATABASE
#define DBLOGIN_R_CREATEACCOUNT			DBLOGIN_A_CREATEACCOUNT
#define DBLOGIN_R_DELETEACCOUNT			DBLOGIN_A_DELETEACCOUNT
#define DBLOGIN_R_SUCCESS				0xa
#define DBLOGIN_R_FAILED				0xb
#define DBLOGIN_R_HASLOGGEDIN			0xc
#define DBLOGIN_R_HASNOENOUGHPRIORITY	0xd
#define DBLOGIN_R_USERHASEXISTED		0xe
//<------END
#define LOGIN_R_REQUEST					0x0	//when the login request is send from client to server
#define LOGIN_R_SUCCESS					0x1
#define LOGIN_R_FAILED					0x2
#define LOGIN_R_ACCOUNT_OR_PASSWORD_ERROR	0x3
#define LOGIN_R_ACCOUNT_EXIST			0x4
#define	LOGIN_R_TIMEOUT					0x5
#define LOGIN_R_IN_PROGRESS				0x6
#define LOGIN_R_NO_IN_PROGRESS			0x7
#define LOGIN_R_VALID					0x8	//合法用户
#define LOGIN_R_INVALID					0x9	//非法用户
#define LOGIN_R_NOTREGISTERED			0xa //用户未注册
#define LOGIN_R_BEDISCONNECTED			0xb //用户已经离线
#define LOGIN_R_ACCDBISLOCKED			0xc //账号数据库被锁定
#define LOGIN_R_NODEPOSIT				0xd //点卡余额为零、或无点卡

struct KLoginStructHead
{
	DWORD Size;		// size of the struct, if the struct is been inherit,the size is ref the the derive struct
	DWORD Param;	// be one of the LOGIN_R_* define value combin with a LOGIN_A_* value.
};
#define KLOGINSTRUCTHEADSIZE			8	// = 2 * sizeof( DWORD )

struct KLoginAccountInfo : KLoginStructHead
{
	char Account[LOGIN_ACCOUNT_MAX_LEN+2];		//account
	char Password[LOGIN_PASSWORD_MAX_LEN+2];	//password
};

struct KLoginGameServer
{
	char Title[GAMESERVERNAME_MAX_LEN+2];		//服务器文字说明
	DWORD Address;								//服务器域名地址
	short Port;									//服务器端口号
	DWORD ID;									//服务器ID
	DWORD dwConns;								//连接的玩家数
};

struct KLoginServerList : KLoginStructHead
{
	DWORD Count;				//此次传送的服务器项的数目
	KLoginGameServer* Server;	//服务器信息(变长数组)
};


typedef struct tag_DBLOGINSTRUCT : KLoginStructHead
{
	char szUserName[def_DBUSERNAME_MAX_LEN];
	char szPassword[def_DBPASSWORD_MAX_LEN];
}_DBLOGINSTRUCT, *_PDBLOGINSTRUCT;
