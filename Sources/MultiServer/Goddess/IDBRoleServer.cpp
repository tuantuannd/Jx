#include "stdafx.h"
#include "IDBRoleServer.h"
#include "DBTable.h"
#include "DBBackup.h"
#include "../../../../Headers/KGmProtocol.h"
#include "CRC32.h"

#include <iostream>
#include <strstream>

using namespace std;

#include "Macro.h"

static ZDBTable *db_table = NULL;
static size_t nMaxRoleCount_InAccount = 3;
static CDBBackup::TStatData GameStatData;//游戏统计数据（服务器初始化时填入）
static CDBBackup* DBBackup = NULL;

static HANDLE hDeadLockThread = NULL;		//检查数据库有没有死锁的线程
static HANDLE hRemoveLogThread = NULL;	//删除没用日志文件的线程

HWND hListOutput = NULL;	//输出list

int get_account(DB *db, const DBT *pkey, const DBT *pdata, DBT *ikey) 
{
	//给定一个完整的buffer，得到account作为索引
	memset( ikey, 0, sizeof( DBT ) );
	TRoleData *pRoleData = (TRoleData *)pdata->data;

	ikey->data = pRoleData->BaseInfo.caccname;
	ikey->size = strlen( pRoleData->BaseInfo.caccname ) + 1;

	return 0;
}

//==========检查数据库有没有死锁的线程 Add By Fellow 2003.9.10==============
DWORD WINAPI DeadlockProc(LPVOID lpParameter) {
	while(!db_table->bStop) {
		Sleep(5 * 1000);	//5秒钟一次
		db_table->deadlock();
	}
	return 0;
}
//==========删除没用日志文件的线程 Add By Fellow 2003.9.10==============
DWORD WINAPI RemoveLogProc(LPVOID lpParameter) {
	while(!db_table->bStop) {
		db_table->removeLog();
		Sleep(60 * 60 * 1000);			//1小时一次
	}
	return 0;
}

BOOL InitDBInterface( size_t nMaxRoleCount )
{
	nMaxRoleCount_InAccount = nMaxRoleCount;

	db_table = new ZDBTable( "database", "roledb" );

	db_table->addIndex( get_account );
	if ( db_table && db_table->open() )
	{
		DWORD dwThreadId, dwThrdParam = 1;
/*

		hDeadLockThread = CreateThread(
			NULL,				// no security attributes 
			0,					// use default stack size  
			DeadlockProc,		// thread function 
			&dwThrdParam,		// argument to thread function 
			0,					// use default creation flags 
			&dwThreadId);		// returns the thread identifier 
		if(!hDeadLockThread)
		{
			//创建线程失败,暂时没有处理
		}
*/

		hRemoveLogThread = CreateThread(
			NULL,				// no security attributes 
			0,					// use default stack size  
			RemoveLogProc,		// thread function 
			&dwThrdParam,		// argument to thread function 
			0,					// use default creation flags 
			&dwThreadId);		// returns the thread identifier 
		if(!hRemoveLogThread)
		{
			//创建线程失败,暂时没有处理
		}

		return TRUE;
	}
		
	return FALSE;
}

void ReleaseDBInterface()		//释放数据库引擎
{
	if ( db_table )
	{
		db_table->commit();
/*

		if(!hDeadLockThread)
			TerminateThread(hDeadLockThread, 0);
*/

		if(!hRemoveLogThread)
			TerminateThread(hRemoveLogThread, 0);

		db_table->removeLog();
		
		StopBackupTimer();//停止备份线程
		
		db_table->close();
		delete db_table;
	}
}

//[wxb 2003-7-23]
void SetRoleInfoForGM(int nInfoID, char* pRoleBuffer, char* strUser, int nBufLen)
{
	char* aBuffer = new char[64 * 1024];
	TRoleData* pRoleData = NULL;
	int size;

	GetRoleInfo(aBuffer, strUser, size);
	if(size)
	{
		pRoleData = (TRoleData*)aBuffer;
		switch (nInfoID)
		{
		case gm_role_entergame_position:
			ASSERT(nBufLen == sizeof(GM_ROLE_DATA_SUB_ENTER_POS));
			memcpy(&(pRoleData->BaseInfo.ientergameid), pRoleBuffer, sizeof(GM_ROLE_DATA_SUB_ENTER_POS));
			break;
		default:
			ASSERT(0);
			return;
			break;
		}
		SaveRoleInfo(aBuffer, strUser, false);
	}
	
    delete aBuffer;
}

//[wxb 2003-7-22]
void *GetRoleInfoForGM(int nInfoID, char * pRoleBuffer, char * strUser, int &nBufLen)
{
	int size = 0;
	ZCursor *cursor = db_table->search( strUser, strlen( strUser ) + 1 );
//	char *buffer = db_table->search( strUser, strlen( strUser ) + 1, size );
	if ( cursor )
	{
		TRoleData* pRole = (TRoleData*)cursor->data;
		switch(nInfoID) {
		case gm_role_entergame_position:
			nBufLen = sizeof(pRole->BaseInfo.ientergameid) + sizeof(pRole->BaseInfo.ientergamex) + sizeof(pRole->BaseInfo.ientergamey);
			memcpy( pRoleBuffer, &(pRole->BaseInfo.ientergameid), nBufLen );
			break;
		default:
			ASSERT(0);
			nBufLen = 0;
			break;
		}

		db_table->closeCursor(cursor);
	}
	else
	{
		nBufLen = 0;
	}

	return pRoleBuffer;
}

void *GetRoleInfo( char * pRoleBuffer, char * strUser, int &nBufLen )
{
	//输出数据======
	char aStr[1024];
	sprintf(aStr,"GetRoleInfo:%s",strUser);
	AddOutputString(hListOutput,aStr);
	//===============
	int size = 0;
	ZCursor *cursor = db_table->search( strUser, strlen( strUser ) + 1 );
//	char *buffer = db_table->search( strUser, strlen( strUser ) + 1, size );
	if ( cursor )
	{
		nBufLen = size;
		memcpy( pRoleBuffer, cursor->data, cursor->size );
		nBufLen = cursor->size;
		db_table->closeCursor(cursor);
	}
	else
	{
		nBufLen = 0;
	}

	return pRoleBuffer;
}

//保存角色的信息，如果数据库不存在该玩家，则增加该玩家
//bAutoInsertWhenNoExistUser 设为TRUE时表示，如果需要保存的该玩家在数据库中并不存在则自动加入到数据库中，FALSE则不增加直接返回错误
//注意INI文件只须存放将需要改动的数据，不需改动的数据将自动保存原状。
int	SaveRoleInfo( char * pRoleBuffer, const char *strUser, BOOL bAutoInsertWhenNoExistUser )
{
	ASSERT( pRoleBuffer );

	//需要存放帐号首先找到数据
	TRoleData *pRoleData = ( TRoleData * )pRoleBuffer;
	
	//输出数据======
	char aStr[1024];
	sprintf(aStr,"SaveRoleInfo:%s dwDataLen=%d",pRoleData->BaseInfo.szName,pRoleData->dwDataLen);
	AddOutputString(hListOutput,aStr);
	//===============

	if(pRoleData->dwDataLen >= 64 * 1024) return 0;//如果数据大于64K就不添加到数据库

	if(bAutoInsertWhenNoExistUser)
	{//如果是新增角色就把账号名转成小写
		char *ptr = pRoleData->BaseInfo.caccname;	//把账号名转成小写
		while(*ptr) {
		if(*ptr >= 'A' && *ptr <= 'Z') *ptr += 'a' - 'A';
		ptr++;
		}
	}

	if (!bAutoInsertWhenNoExistUser)
	{
		DWORD	dwCRC = 0;
		dwCRC = CRC32(dwCRC, pRoleData, pRoleData->dwDataLen - 4);
		DWORD	dwOrigCRC = *(DWORD *)(pRoleBuffer + pRoleData->dwDataLen - 4);
		if (dwCRC != dwOrigCRC)
		{
			// TODO:
			FILE *fLog = fopen("crc_error", "a+b");
			if(fLog) {
				char buffer[255];
				sprintf(buffer, "----\r\n%s\r\b%s\r\n", pRoleData->BaseInfo.szName, pRoleData->BaseInfo.caccname);
				fwrite(buffer, 1, strlen(buffer), fLog);
				fwrite(pRoleBuffer, 1, pRoleData->dwDataLen, fLog);
				fclose(fLog);
			}
			return 0;
		}
	}

	char szAccountName[32];

	int nLength = strlen( pRoleData->BaseInfo.caccname );

	ASSERT( nLength > 0 );

	nLength = nLength > 31 ? 31 : nLength;

	memcpy( szAccountName, pRoleData->BaseInfo.caccname, nLength );
	szAccountName[nLength] = '\0';

	char szName[32];
	const char *pName = szName;

	if ( NULL == strUser || strUser[0] == 0 )
	{
		int len = strlen( pRoleData->BaseInfo.szName );

		ASSERT( len > 0 );

		len = len > 31 ? 31 : len;

		memcpy( szName, pRoleData->BaseInfo.szName, len );
		szName[len] = '\0';
	}
	else
	{
		int len = strlen( strUser );

		ASSERT( len > 0 );

		memcpy( szName, strUser, len );
		szName[len] = '\0';
	}
	
	if ( bAutoInsertWhenNoExistUser )
	{
		int nCount = 0;
		int size = 0;

		/*
		 * Role of same name only
		 */
		ZCursor *user_cursor = db_table->search(pName, strlen( pName ) + 1 );
//		char *user_data = db_table->search( pName, strlen( pName ) + 1, size );
		if ( user_cursor )
		{
			db_table->closeCursor(user_cursor);

			return 0;
		}

		/*
		 * Get count of role by the key of account
		 */
		ZCursor *cursor = db_table->search( szAccountName, strlen( szAccountName ) + 1, 0 );
//		char *buffer = db_table->search( szAccountName, strlen( szAccountName ) + 1, size, 0 );

		while ( cursor )
		{
			nCount ++;
			if(!db_table->next( cursor ))break;
		}

		if ( nCount >= nMaxRoleCount_InAccount )
		{
			return 0;
		}
	}

	if ( db_table->add( pName, strlen( pName ) + 1, pRoleBuffer, pRoleData->dwDataLen ) )
	{
		return 1;
	}

	return 0;
}

int GetRoleListOfAccount( char * szAccountName, S3DBI_RoleBaseInfo * RoleBaseList, int nMaxCount )
{
	int size = 0;
	int count = 0;

	char *ptr = szAccountName;	//把账号名转成小写
	while(*ptr) {
		if(*ptr >= 'A' && *ptr <= 'Z') *ptr += 'a' - 'A';
		ptr++;
	}

	//输出数据======
	char aStr[1024];
	sprintf(aStr,"GetRoleListOfAccount:%s",szAccountName);
	AddOutputString(hListOutput,aStr);
	//===============

	S3DBI_RoleBaseInfo *base_ptr = RoleBaseList;

	ZCursor *cursor = db_table->search( szAccountName, strlen( szAccountName ) + 1, 0 );
//	char *buffer = db_table->search( szAccountName, strlen( szAccountName ) + 1, size, 0 );
	
	while ( count < nMaxCount && cursor )
	{
		TRoleData *pRoleData = (TRoleData *)cursor->data;
		
		strncpy( base_ptr->szName, pRoleData->BaseInfo.szName, 32 ); 
        base_ptr->szName[31] = '\0';
		
		base_ptr->Sex = pRoleData->BaseInfo.bSex;
		base_ptr->Series = pRoleData->BaseInfo.ifiveprop;
//		base_ptr->HelmType = pRoleData->BaseInfo.ihelmres;
//		base_ptr->ArmorType = pRoleData->BaseInfo.iarmorres;
//		base_ptr->WeaponType = pRoleData->BaseInfo.iweaponres;
		base_ptr->Level = pRoleData->BaseInfo.ifightlevel;
		
		base_ptr++;
		
		/*
		 * Get next info from database
		 */
		count++;
		if(!db_table->next( cursor ))break;
	}
	return count;
}

bool DeleteRole( const char * strUser )
{
	if ( db_table && strUser && strUser[0] )
	{
		return db_table->remove( strUser, strlen( strUser ) + 1 );
	}
	return false;
}

// 下面的函数可能存在内存泄漏问题
//char* GetAccountByUser(char * strUser)
//{//通过用户名查找帐户
//	char* aBuffer = new char[64 * 1024];
//	TRoleData* pRoleData;
//	int size;
//
//	GetRoleInfo(aBuffer, strUser, size);
//	if(size)
//	{
//		pRoleData = (TRoleData*)aBuffer;
//		return pRoleData->BaseInfo.caccname;
//	}
//	else
//	{
//		return NULL;
//	}
//}

//------------------------------------------------------------------------
//数据库备份与数据统计 Add By Fellow At 2003.08.14
bool StartBackupTimer(int aTime)
{//开始运行备份线程
	DBBackup = new CDBBackup( "database", "roledb", db_table );
	DBBackup->SaveStatInfo();
	bool aStartResult = DBBackup->Open(aTime);
	return aStartResult;
}

bool StopBackupTimer()
{//结束运行备份线程
	if(!DBBackup)return false;
	while(IsBackupWorking()){}//等待备份线程结束

	bool aResult = DBBackup->Close();
	delete DBBackup;
	DBBackup = NULL;
	return aResult;
}

bool SuspendBackupTimer()
{//挂起线程
	if(!DBBackup)return false;
	return DBBackup->Suspend();
}

bool ResumeBackupTimer()
{//继续运行线程
	if(!DBBackup)return false;
	return DBBackup->Resume();
}

bool IsBackupThreadWorking()
{//线程是否正在运行
	if(!DBBackup)return false;
	return DBBackup->IsWorking();
}

bool IsBackupWorking()
{//是否在备份
	if(!DBBackup)return false;
	return DBBackup->IsBackuping();
}

bool DoManualBackup()
{//手工备份
	if(!DBBackup)return false;
	return DBBackup->ManualBackup();
}

bool GetGameStat(TGAME_STAT_DATA* aStatData)
{
	if(!DBBackup)return false;
	TGAME_STAT_DATA tmpStatData = DBBackup->GetSendStatData();
	memcpy(aStatData, &tmpStatData, sizeof(TGAME_STAT_DATA));
	return true;
}

void AddOutputString(HWND hListCtrl, char* aStr)
{//添加操作输出文字
	if ( hListCtrl && ::IsWindow( hListCtrl ) )
	{
		int nCount = ::SendMessage( hListCtrl, LB_GETCOUNT, 0, 0 );
		if(nCount >= 100)
		{
			::SendMessage( hListCtrl, LB_DELETESTRING, 100, 0 );
		}
		int nIndex = ::SendMessage( hListCtrl, LB_INSERTSTRING, 0, ( LPARAM )aStr );//把新的信息添到第一个
	}
}