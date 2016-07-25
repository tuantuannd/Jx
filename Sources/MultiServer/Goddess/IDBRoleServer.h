/********************************************************************
	created:	2003/06/03
	file base:	IDBRoleServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_IDBROLESERVER_H__
#define __INCLUDE_IDBROLESERVER_H__

#include "S3DBInterface.h"

BOOL InitDBInterface( size_t nMaxRoleCount );

void ReleaseDBInterface();

void *GetRoleInfo( char * pRoleBuffer, char * strUser, int &nBufLen );
void *GetRoleInfoForGM(int nInfoID, char * pRoleBuffer, char * strUser, int &nBufLen);
void SetRoleInfoForGM(int nInfoID, char * pRoleBuffer, char * strUser, int nBufLen);

int	SaveRoleInfo( char * pRoleBuffer, const char * strUser, BOOL bAutoInsertWhenNoExistUser );

int GetRoleListOfAccount( char * szAccountName, S3DBI_RoleBaseInfo * RoleBaseList, int nMaxCount );

bool DeleteRole( const char * strUser );

char* GetAccountByUser(char * strUser);//通过用户名查找帐户

//------------------------------------------------------------------------
//数据库备份与数据统计 Add By Fellow At 2003.08.14
bool StartBackupTimer(int aTime);		//开始运行备份线程
bool StopBackupTimer();							//结束运行备份线程
bool SuspendBackupTimer();						//挂起线程
bool ResumeBackupTimer();						//继续运行线程
bool IsBackupThreadWorking();					//线程是否正在运行
bool IsBackupWorking();							//是否在备份
bool DoManualBackup();							//手工备份

bool GetGameStat(TGAME_STAT_DATA* aStatData);//取得用户统计数据
//------------------------------------------------------------------------
void AddOutputString(HWND hListCtrl, char* aStr);//添加操作输出文字
#endif // __INCLUDE_IDBROLESERVER_H__