#pragma once
#define MAX_BLOCK_SIZE	350
#define TRACEE
#include "KProtocol.h"
#ifdef TRACEE
#include "windows.h"
#endif

extern void ShowTrace(char* pMsg);

#pragma pack(push)
#pragma	pack(1)
struct  TRoleHeader
{
	unsigned char	nProtoId;
	size_t			nDataLen;//TRoleNetMsg时表示该Block的实际数据长度,TProcessData时表示Stream的实际数据长度
	unsigned long	ulIdentity;
};

struct 	TRoleNetMsg	:TRoleHeader	//	每个数据包的结构
{
	unsigned char	bDataState;// 1 表示是新的第一段，0表示是上一次的包的继续
	size_t			nOffset;//When First ,it means TotalLen;Later, it means offset;
	char 			pDataBuffer[1];
};

/*
struct TProcessData: TRoleHeader			//调用DB或游戏服务器的结构
{
	unsigned long   ulNetId;		//网络ID
	char			pDataBuffer[1];//实际的数据
};
*/

struct TCmd 
{
	unsigned long  ulNetId;
	TProcessData ProcessData;
};

typedef TProcessData TStreamData ;
typedef TRoleNetMsg KBlock;

#pragma pack(pop)