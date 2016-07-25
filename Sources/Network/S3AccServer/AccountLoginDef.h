// -------------------------------------------------------------------------
//	文件名		：	AccountLoginDef.h
//	创建者		：	万里
//	创建时间	：	2003-5-2 22:04:24
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __ACCOUNTLOGINDEF_H__
#define __ACCOUNTLOGINDEF_H__

#pragma once

#include <WTypes.h>

#pragma	pack(push, 1)

#define LOGIN_USER_ACCOUNT_MIN_LEN			4
#define LOGIN_USER_ACCOUNT_MAX_LEN			32
#define LOGIN_USER_PASSWORD_MIN_LEN			6
#define LOGIN_USER_PASSWORD_MAX_LEN			64

//login action return value
#define ACTION_SUCCESS					0x1
#define ACTION_FAILED					0x2
#define E_ACCOUNT_OR_PASSWORD			0x3
#define E_ACCOUNT_EXIST					0x4
#define E_ACCOUNT_NODEPOSIT				0x5 //点卡余额为零、或无点卡
#define E_ACCOUNT_ACCESSDENIED			0x6
#define E_ADDRESS_OR_PORT				0x7
#define E_ACCOUNT_FREEZE				0x8

#define ACCOUNT_CURRENT_VERSION 1

typedef enum
{
	AccountUser = 1,
	AccountUserLoginInfo,
	ServerAccountUserLoginInfo,
	AccountUserReturn,
	AccountUserTimeInfo,
	ServerOptionInfo,
	ServerInfo,
	AccountUserVerify,
	AccountUserReturnEx,
	AccountUserLogout,
} ACCOUNTHEADTYPE;

struct KAccountHead
{
	WORD Size;		// size of the struct
	WORD Version;	// ACCOUNT_CURRENT_VERSION
	WORD Type;
	DWORD Operate;	//gateway used
};

struct KAccountUser : public KAccountHead
{
	char Account[LOGIN_USER_ACCOUNT_MAX_LEN];	//account
};

struct KAccountUserLoginInfo : public KAccountUser
{
	char Password[LOGIN_USER_PASSWORD_MAX_LEN];	//password
};

struct KServerAccountUserLoginInfo : public KAccountUserLoginInfo
{
	DWORD Address;
	BYTE MacAddress[6];
};

struct KAccountUserReturn : public KAccountUser
{
	int nReturn;
};

struct KAccountUserReturnExt : public KAccountUserReturn
{
	WORD nExtPoint;        //可用的附送点
	DWORD nLeftTime;          //剩余时间,现在以秒为单位,需要欧文确认是否改为分钟
};

struct KAccountUserLogout : public KAccountUser
{
	WORD nExtPoint; //将要扣除的附送点
};

struct KAccountUserTimeInfo : public KAccountUserReturn
{
	DWORD nTime;
};

typedef enum
{
	server_OnlinePlayerCount = 0x0001,
	server_RegisterCount = 0x0002,
	server_Login = 0x0004,
	server_Logout = 0x0008,
	server_PlayerWhere = 0x0010,
	server_LoginAlready = 0x0020,
	server_PlayerWhereID = 0x0040,
	server_GWRelayID = 0x0080,
} SERVERTYPE;

struct KServerInfo : public KAccountUser
{
	short nServerType;
	DWORD nValue;
};

#pragma	pack(pop)

#endif // __ACCOUNTLOGINDEF_H__