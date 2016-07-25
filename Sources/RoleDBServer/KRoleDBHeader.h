#pragma once
#define MAX_BLOCK_SIZE	350
enum	enumGame2DBServerProtol
{
	PROTOL_ONLYSAVEROLE,//游戏服务器向数据库服务器要求保存数据
		PROTOL_SAVEORCREATEROLE,
		PROTOL_LOADROLE,//获得角色数据
		PROTOL_GETROLELIST,//获得某个帐号的角色列表
		PROTOL_DELETEROLE,
};

enum   enumDBServer2DBProtol
{
	PROTOL_ROLEINFO,
		PROTOL_ROLELIST,
};

struct  TRoleHeader
{
	unsigned char	nProtoId;
	size_t			nDataLen;
	//unsigned long	ulIdentity;
};

struct 	TRoleNetMsg	:TRoleHeader	//	每个数据包的结构
{
	unsigned char	bDataState;// 1 表示是新的第一段，0表示是上一次的包的继续
	size_t			nOffset;//When First ,it means TotalLen;Later, it means offset;
	char 			pDataBuffer[1];
};


struct TProcessData: TRoleHeader			//调用DB或游戏服务器的结构
{
	unsigned long   ulNetId;		//网络ID
	char			pDataBuffer[1];//实际的数据
};

typedef TProcessData TStreamData ;
typedef TRoleNetMsg KBlock;
