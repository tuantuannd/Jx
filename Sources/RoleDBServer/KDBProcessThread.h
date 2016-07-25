#pragma once
//using namespace std;
#include <list>
#include "KRoleInfomation.h"
#include "windows.h"
extern CRITICAL_SECTION g_RoleInfoMutex;
extern CRITICAL_SECTION g_RoleInfoListMutex;
extern CRITICAL_SECTION g_RoleInfoMapMutex;
extern CRITICAL_SECTION g_MainMsgListMutex;
extern CRITICAL_SECTION g_NetMsgListMutex;
extern CRITICAL_SECTION g_GetRoleInfoMutex;
extern HANDLE			 g_NetEventMutex;
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
extern void TextOutRoleInfo(BYTE * pDataBuffer, char * FileName);

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