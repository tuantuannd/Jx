#pragma once
//using namespace std;
//#include <list>
#include "KRoleInfomation.h"
#include "KRoleDBServer.h"
#include "windows.h"
#include "..\..\Headers\KProtocolDef.h"
#include "S3DBInterface.h"
#include "CriticalSection.h"

using OnlineGameLib::Win32::CCriticalSection;

extern OnlineGameLib::Win32::CCriticalSection g_RoleInfoMutex;
extern OnlineGameLib::Win32::CCriticalSection g_RoleInfoSetMutex;
//extern OnlineGameLib::Win32::CCriticalSection g_RoleInfoMapMutex;
extern OnlineGameLib::Win32::CCriticalSection g_MainMsgListMutex;
extern OnlineGameLib::Win32::CCriticalSection g_NetMsgListMutex;
extern OnlineGameLib::Win32::CCriticalSection g_GetRoleInfoMutex;
//extern OnlineGameLib::Win32::CCriticalSection g_NetEventMutex;


extern DWORD g_dwPerSaveTime;//每次存档的时间
extern DWORD g_dwMaxUnModifyTime;
extern list<TCmd*> g_MainThreadCmdList;
extern list<TGetRoleInfoFromDB*> g_DBLoadThreadCmdList;
extern list<TCmd*> g_NetServiceThreadCmdList;
extern RoleDBServer::KNetServer g_NetServer;
extern DWORD WINAPI RoleServerMainThreadFun(void * pParam);
extern DWORD WINAPI DatabaseLoadThreadFun(void * pParam);
extern DWORD WINAPI DatabaseSaveThreadFun(void *);
extern int InitServer();
extern DWORD ReleaseServer();
extern DWORD WINAPI RoleNetWorkServiceThreadFun(void * pParam);
extern char * GetRoleNameFromRoleBuffer(char * pRoleBuffer);

#define MAX_MARK_COUNT 100
#define SAVEMARK_NO 10
#define REMOVEMARK_NO 30
#define LOADMARK_NO 20




extern unsigned char g_LoadDBThreadMark;
extern unsigned char g_SaveDBThreadMark;
extern unsigned char g_NetServerMark;
extern unsigned char g_MainLoopMark;
extern size_t g_dwRecvLen;
extern size_t g_dwSendLen;
extern int  g_nMistakeCount;

extern char g_SaveMarkString[200];
extern char g_RemoveMarkString[200];
extern char g_LoadMarkString[200];

#define MARK '.'

#define MARK1 0
//'+'
#define MARK2 0
//'-'


struct TThreadData
{
	DWORD dwThreadId;
	HANDLE hThread;
};

enum
{
	LoadDBThread,
	SaveDBThread,
	NetService,
	MainMsgLoop,
};


extern TThreadData g_ThreadData[4];