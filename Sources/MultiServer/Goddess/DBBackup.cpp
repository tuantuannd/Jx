// DBBackup.cpp: implementation of the CDBBackup class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DBTABLE.H"
#include "DBBackup.h"
#include "DBDumpLoad.h"
#include <FSTREAM>
#include <direct.h>
#include "Macro.h"
#include "CRC32.h"

using namespace std;

#define IS_OUTPUT_LOG true

#define ROLE_FILTER_FILE "RoleFilter.ini"
#define ROLE_FILTER_COUNT 300

static char DBPath[32] = {0};						//数据库目录
static char DBName[32] = {0};						//数据库名
static ZDBTable* RunTable = NULL;

static bool IsBackupWorking = false;				//是否正在备份
static bool IsThreadWorking = false;				//线程是否在工作
static bool IsSuspended = false;					//线程是否挂起
static WORD BackupTime = 0;				//备份间隔时间
static bool IsTimeToBackup = true;

static CDBBackup::TStatData oldGameStatData;//旧的游戏统计数据
static CDBBackup::TStatData newGameStatData;//新的游戏统计数据

static TGAME_STAT_DATA SendStatData;				//取得发送给客户端的统计数据结构

int CDBBackup::GetIndexByName(char* aName, TRoleList* aList, int aListSize)
{
	if(aList[0].Name[0] == '\0') return -1;
	for(int i=0;i<aListSize;++i)
	{
		if(aList[i].Name[0] == '\0') return -1;
		if (strcmp(aName, aList[i].Name) == 0)
			return i;
	}
	return -1;
}

CDBBackup::TRoleList* CDBBackup::GetMin(
					TRoleList* const aRoleList,
					const int n,
					const StatType aType)
{//查找出列表中钱/级别（或其他）最少的一个元素
	TRoleList* tmpData;
	tmpData = &aRoleList[0];
	while(true)
	{
		for(int i=0;i<n;++i)
		{
			if(aRoleList[i].Name[0] == '\0')
				return &aRoleList[i];
			int aSourse, aDest;
			switch(aType)
			{
			case stMoney:
				aDest = tmpData->Money;
				aSourse = aRoleList[i].Money;
				break;
			case stLevel:
				aDest = tmpData->Level;
				aSourse = aRoleList[i].Level;
				break;
			case stKiller:
				aDest = tmpData->KillNum;
				aSourse = aRoleList[i].KillNum;
				break;
			}
			if(aDest > aSourse)
			{
				tmpData = &aRoleList[i];
				break;
			}
		}
int i;
		if(i==n)break;
	}
	return tmpData;
}


void CDBBackup::ListSort(
					TRoleList* const aRoleList,
					const int n,
					const StatType aType)
{//对列表做特定的排序
	int i,j;
	for(i=0;i<n;++i)
	{
		TRoleList* tmpData = &aRoleList[i];
		for(j=i+1;j<n;++j)
		{
			__int64 aSourse, aDest;
			switch(aType)
			{
			case stMoney:
				aDest = tmpData->Money;
				aSourse = aRoleList[j].Money;
				break;
			case stLevel:
				aDest = (__int64)tmpData->Level * 0xffffffff + (__int64)tmpData->FightExp;
				aSourse = (__int64)aRoleList[j].Level * 0xffffffff + (__int64)aRoleList[j].FightExp;
				break;
			case stKiller:
				aDest = tmpData->KillNum;
				aSourse = aRoleList[j].KillNum;
				break;
			}
			if( aSourse > aDest )
					swap(*tmpData, aRoleList[j]);
		}
	}
}

void CDBBackup::RoleDataCopy(TRoleList* Desc, TRoleData* Source)
{//把RoleData有用的结构复制到RoleList结构中
	//strcpy(Desc->Account, Source->BaseInfo.caccname);
	strcpy(Desc->Name, Source->BaseInfo.szName);
	Desc->Sect = Source->BaseInfo.nSect;
	Desc->Money = Source->BaseInfo.imoney + Source->BaseInfo.isavemoney;
	Desc->Level = Source->BaseInfo.ifightlevel;
	Desc->KillNum = Source->BaseInfo.nKillPeopleNumber;
	Desc->FightExp = Source->BaseInfo.fightexp;
}

char* CDBBackup::LoadFilterName()
{//取得要过滤的用户名列表 二维数组 [最大名字长度20][数目]
	char* aRoleList = new char[ROLE_FILTER_COUNT * 20];
	char aBuffer[20];
	char aAppPath[MAX_PATH] = {0};
	getcwd(aAppPath,MAX_PATH);
	strcat(aAppPath,"\\");
	strcat(aAppPath,ROLE_FILTER_FILE);

	fstream aFile(aAppPath,ios::in);
	if(!aFile.is_open())
	{
		SAFE_DELETE_ARRAY(aRoleList);
		return NULL;
	}
	for(int i=0;i<ROLE_FILTER_COUNT;++i)
	{
		aFile.getline(aBuffer, 20);
		if(!aBuffer[0])break;
		int aLen = strlen(aBuffer);
		strcpy(&aRoleList[i * 20],aBuffer);
		aRoleList[i *20 + 19] = '\0';
	}
	aFile.close();

	return aRoleList;
}

bool CDBBackup::IsRoleFilter(char* aName, char* aList)
{//过滤某些角色
	if(!aList) return false;
	for(int i=0;i<ROLE_FILTER_COUNT;++i)
	{
		if(aList[i * 20] == '\0') return false;
		if (strcmp(aName, &aList[i * 20]) == 0)
			return true;
	}
	return false;
}

CDBBackup::CDBBackup(char* aPath, char* aName, ZDBTable* aRunTable)
{
	memset(DBPath,0,32);
	memset(DBName,0,32);
	strcpy(DBPath, aPath);
	strcpy(DBName, aName);
	BackupTime = 0;
	m_hThread = NULL;
	m_hManualThread = NULL;
	RunTable = aRunTable;
	
	memset(&oldGameStatData, 0, sizeof(TStatData));
	memset(&newGameStatData, 0, sizeof(TStatData));
	memset(&SendStatData, 0, sizeof(TGAME_STAT_DATA));
}

bool CDBBackup::Open(int aTime)
{
	aTime = aTime % 24;
	if((DBPath[0] == '\0')||(DBName[0] == '\0'))
		return false;//初始化有问题就退出
	
	BackupTime = aTime;
	DWORD dwThreadId, dwThrdParam = 1;
	m_hThread = CreateThread(
		NULL,                        // no security attributes 
        0,                           // use default stack size  
        TimerThreadFunc,             // thread function 
        &dwThrdParam,                // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadId);                // returns the thread identifier 
	if(!m_hThread)
	{
		BackupTime = 0;
		return false;//创建线程失败
	}
	
	char aAppPath[MAX_PATH] = {0};
	getcwd(aAppPath,MAX_PATH);
	strcat(aAppPath,"\\StatData.dat");
	//读取旧排名
	memset(&newGameStatData,0,sizeof(CDBBackup::TStatData));
	FILE* aStatFile = fopen(aAppPath,"rb");
	if(aStatFile)
	{		
		int a = fread(&newGameStatData, 1, sizeof(CDBBackup::TStatData), aStatFile);
		fclose(aStatFile);
	}
	MakeSendStatData();
	
	return true;
}

bool CDBBackup::Close()
{
	BackupTime = 0;
	if(m_hManualThread) CloseHandle( m_hManualThread );
	return (CloseHandle( m_hThread ) == TRUE);

}

bool CDBBackup::Suspend()
{
	if(!m_hThread) return false;	//如果线程没有初始化好就不能挂起
	if(!IsThreadWorking) return false;	//如果线程没有开始就不能挂起
	if(IsBackupWorking) return false;	//如果正在备份就中止挂起
	if(IsSuspended) return false;	//如果正在备份就中止挂起
	if(SuspendThread(m_hThread) == -1)return false;
	IsSuspended = true;
	return true;
}

bool CDBBackup::Resume()
{//继续执行线程
	if(!m_hThread) return false;	//如果线程没有初始化好就不能挂起
	if(!IsSuspended) return false;	//如果正在备份就中止挂起
	if(!IsThreadWorking) return false;	//如果线程没有开始就不能继续执行线程

	if(ResumeThread(m_hThread) == -1)return false;
	IsSuspended = false;
	return true;
}

TGAME_STAT_DATA CDBBackup::GetSendStatData()
{//取得发送给客户端的统计数据结构
	return SendStatData;
}

bool CDBBackup::IsWorking()
{//线程是否在工作
	return IsThreadWorking;
}

bool CDBBackup::IsBackuping()
{//备份是否在工作
	return IsBackupWorking;
}

bool CDBBackup::ManualBackup()
{//手工备份
	if(!m_hThread) return false;	//如果线程没有初始化好就不能挂起
	if(!IsThreadWorking) return false;	//如果线程没有开始就不能继续执行线程

	DWORD dwThreadId, dwThrdParam = 1;
	m_hManualThread = CreateThread(
		NULL,                        // no security attributes 
        0,                           // use default stack size  
        ManualThreadFunc,             // thread function 
        &dwThrdParam,                // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadId);                // returns the thread identifier 
	if(!m_hManualThread)
	{
		BackupTime = 0;
		return false;//创建线程失败
	}

	return true;
}

DWORD WINAPI CDBBackup::TimerThreadFunc( LPVOID lpParam )
{//备份计时线程
	IsThreadWorking = true;
	while(true)
	{
		SYSTEMTIME aSysTime;
		GetLocalTime(&aSysTime);
		if(aSysTime.wHour == BackupTime)
		{	
			if(IsTimeToBackup) Backup();
			IsTimeToBackup = false;
		}
		else
		{
			IsTimeToBackup = true;
		}
		
		Sleep(1000 * 60 * 30);
		//Sleep(BackupTime);
	}
	IsThreadWorking = false;
	return 0;
}

DWORD WINAPI CDBBackup::ManualThreadFunc( LPVOID lpParam )
{//手工备份线程
	Backup();
	return 0;
}

void CDBBackup::Backup()
{
	IsBackupWorking = true;
	
	//打开备份状态log文件
	char aLogPath[MAX_PATH] = {0};
	getcwd(aLogPath,MAX_PATH);
	strcat(aLogPath,"\\Backup.log");

	fstream aLogFile(aLogPath,ios::out);
	
	SYSTEMTIME aSysTime;
	GetSystemTime(&aSysTime);	
	string aBackupDir;
	char aIntStr[10];
	itoa(aSysTime.wYear,aIntStr,10);
	aBackupDir = aIntStr;
	itoa(aSysTime.wMonth,aIntStr,10);
	aBackupDir += aIntStr;
	itoa(aSysTime.wDay,aIntStr,10);
	aBackupDir += aIntStr;
	aBackupDir += "_";
	itoa(aSysTime.wHour,aIntStr,10);
	aBackupDir += aIntStr;
	itoa(aSysTime.wMinute,aIntStr,10);
	aBackupDir += aIntStr;
	itoa(aSysTime.wSecond,aIntStr,10);
	aBackupDir += aIntStr;
	itoa(aSysTime.wMilliseconds,aIntStr,10);
	aBackupDir += aIntStr;
	aLogFile<<"Backup Log File Open Time:"<<aSysTime.wYear<<"."<<
				aSysTime.wMonth<<"."<<aSysTime.wDay<<" "<<
				aSysTime.wHour<<":"<<aSysTime.wMinute<<":"<<aSysTime.wSecond<<endl;
	int i,j;
	char aSavePath[MAX_PATH] = {0};
	getcwd(aSavePath, MAX_PATH);
	strcat(aSavePath,"\\");
	strcat(aSavePath,DBPath);
	strcat(aSavePath,"\\");
	#ifdef WIN32
		mkdir(aSavePath);
	#else
        mkdir(aSavePath, 0);
	#endif
	strcat(aSavePath,aBackupDir.c_str());
	strcat(aSavePath,"\\");
	#ifdef WIN32
		mkdir(aSavePath);
	#else
        mkdir(aSavePath, 0);
	#endif
	strcat(aSavePath,aBackupDir.c_str());
	
	//数据库记录统计（维护查看用）==========Add by Fellow,2003.08.26
	char aDBSFullPath[MAX_PATH] = {0};
	strcpy(aDBSFullPath, aSavePath);
	strcat(aDBSFullPath,".txt");
	fstream aDBSOutput(aDBSFullPath,ios::out);
	aDBSOutput<<"账号名\t角色名\t门派\t等级\t金钱"<<endl;
	int aDBSPlayerCount=0;
	int aDBSSectPlayerCount[12] = {0};
	double aDBSMoneyCount=0;
	double aDBSSectMoneyCount[12] = {0};
	int aDBSLevelPlayerCount[200] = {0};
	//==================

	strcat(aSavePath,".bak");
	CDBDump DBDump;
	DBDump.Open(aSavePath);
	
	if(!RunTable)
	{
		aLogFile<<"RunTable NULL Error."<<endl;
		return;
	}
	
	char* RoleFilterList = LoadFilterName();	//取得要过滤的用户名列表
	TStatData aStatData;//实时备份后产生的游戏统计数据
	memset(&aStatData, 0, sizeof(TStatData));

	ZCursor *cursor = RunTable->first();
	aLogFile<<"RunTable cursor Opened. Backup begin."<<endl;
	while(cursor)
	{		
		if(!DBDump.AddData(cursor->key,cursor->key_size,cursor->data,cursor->size))
		{
			aLogFile<<"Role["<<cursor->key<<"] Dump Error."<<endl;
		}

		TRoleList* tmpData;
		TRoleData* pRoleData = (TRoleData*)cursor->data;

		//数据库记录统计（维护查看用）==========Add by Fellow,2003.08.26
		char aDBSSect[32] = {0};
		int aDBSSectIndex = (int)pRoleData->BaseInfo.nSect;
		switch(aDBSSectIndex)
		{
			case 0:strcpy(aDBSSect,"少林派");break;
			case 1:strcpy(aDBSSect,"天王帮");break;
			case 2:strcpy(aDBSSect,"唐门");break;
			case 3:strcpy(aDBSSect,"五毒教");break;
			case 4:strcpy(aDBSSect,"峨嵋派");break;
			case 5:strcpy(aDBSSect,"翠烟门");break;
			case 6:strcpy(aDBSSect,"丐帮");break;
			case 7:strcpy(aDBSSect,"天忍教");break;
			case 8:strcpy(aDBSSect,"武当派");break;
			case 9:strcpy(aDBSSect,"昆仑派");break;
			default:
				if(pRoleData->BaseInfo.ijoincount == 0)
					{strcpy(aDBSSect,"新手");break;}
				else
					{strcpy(aDBSSect,"出师");break;}
		}
		if(aDBSSectIndex == 255)
		{
			if(pRoleData->BaseInfo.ijoincount == 0)
			{
				++aDBSSectPlayerCount[10];
			aDBSSectMoneyCount[10] += pRoleData->BaseInfo.isavemoney + pRoleData->BaseInfo.imoney;
			}
			else
			{
				++aDBSSectPlayerCount[11];
				aDBSSectMoneyCount[11] += pRoleData->BaseInfo.isavemoney + pRoleData->BaseInfo.imoney;
			}
		}
		else
		{
			++aDBSSectPlayerCount[aDBSSectIndex];
			aDBSSectMoneyCount[aDBSSectIndex] += pRoleData->BaseInfo.isavemoney + pRoleData->BaseInfo.imoney;
		}
		aDBSMoneyCount += pRoleData->BaseInfo.isavemoney + pRoleData->BaseInfo.imoney;
		++aDBSPlayerCount;
		if(((pRoleData->BaseInfo.ifightlevel >0))&&(pRoleData->BaseInfo.ifightlevel < 200))
			aDBSLevelPlayerCount[pRoleData->BaseInfo.ifightlevel]++;
		else
		{
			aDBSOutput<<"***级别问题*** ";
			aDBSLevelPlayerCount[0]++;
		}
		aDBSOutput<<pRoleData->BaseInfo.caccname<<"\t"<<
					pRoleData->BaseInfo.szName<<"\t"<<
					aDBSSect<<"\t"<<
					pRoleData->BaseInfo.ifightlevel<<"\t"<<
					pRoleData->BaseInfo.isavemoney + pRoleData->BaseInfo.imoney<<endl;
		
		//=======================================
		
		if(IsRoleFilter(pRoleData->BaseInfo.szName, RoleFilterList))
		{//过滤某些角色
			if(!RunTable->next(cursor))break;
			continue;
		}
		//////////////////////////////玩家统计////////////////////////////////////
		//对金钱排序
		tmpData = GetMin(aStatData.MoneyStat, MONEYSTATNUM, stMoney);
		if( tmpData->Money < pRoleData->BaseInfo.imoney + pRoleData->BaseInfo.isavemoney)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}

		//对级别排序
		tmpData = GetMin(aStatData.LevelStat, LEVELSTATNUM, stLevel);
		if( tmpData->Level < pRoleData->BaseInfo.ifightlevel)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}
		
		//对杀人数排序
		tmpData = GetMin(aStatData.KillerStat, KILLERSTATNUM, stKiller);
		if( tmpData->KillNum < pRoleData->BaseInfo.nKillPeopleNumber)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}
		
		//各门派对金钱排序
		if( (pRoleData->BaseInfo.nSect <=10) && (pRoleData->BaseInfo.nSect >= 1) )
		{
			tmpData = GetMin(aStatData.MoneyStatBySect[pRoleData->BaseInfo.nSect + 1], SECTMAXSTATNUM, stMoney);
		}
		else
		{
			tmpData = GetMin(aStatData.MoneyStatBySect[0], SECTMAXSTATNUM, stMoney);
		}
		if( tmpData->Money < pRoleData->BaseInfo.imoney  + pRoleData->BaseInfo.isavemoney)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}

		//各门派对级别排序
		if( (pRoleData->BaseInfo.nSect <=10) && (pRoleData->BaseInfo.nSect >= 1) )
		{
			tmpData = GetMin(aStatData.LevelStatBySect[pRoleData->BaseInfo.nSect + 1], SECTMAXSTATNUM, stMoney);
		}
		else
		{
			tmpData = GetMin(aStatData.LevelStatBySect[0], SECTMAXSTATNUM, stMoney);
		}
		if( tmpData->Level < pRoleData->BaseInfo.ifightlevel)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}
		
		//////////////////////////////门派统计////////////////////////////////////
		//各个门派的玩家数统计
		if( (pRoleData->BaseInfo.nSect <=10) && (pRoleData->BaseInfo.nSect >= 1) )
		{
			++aStatData.SectPlayerNum[pRoleData->BaseInfo.nSect + 1];
		}
		else
		{
			++aStatData.SectPlayerNum[0];
		}
		
		if(!RunTable->next(cursor))break;
	}	
	DBDump.Close();		//关闭备份数据库
	aLogFile<<"DB Dump Finished."<<endl;
	aLogFile<<"RunTable cursor closed."<<endl;

	//数据库记录统计（维护查看用）==========Add by Fellow,2003.08.26
	aDBSOutput<<"==记录结束=="<<endl<<endl;
	aDBSOutput<<"==统计=="<<endl;
	aDBSOutput<<"总人数："<<aDBSPlayerCount<<endl;
	for(i=0;i<12;++i)
	{
		char aDBSSect[32] = {0};
		switch(i)
		{
			case 0:strcpy(aDBSSect,"少林派");break;
			case 1:strcpy(aDBSSect,"天王帮");break;
			case 2:strcpy(aDBSSect,"唐门");break;
			case 3:strcpy(aDBSSect,"五毒教");break;
			case 4:strcpy(aDBSSect,"峨嵋派");break;
			case 5:strcpy(aDBSSect,"翠烟门");break;
			case 6:strcpy(aDBSSect,"丐帮");break;
			case 7:strcpy(aDBSSect,"天忍教");break;
			case 8:strcpy(aDBSSect,"武当派");break;
			case 9:strcpy(aDBSSect,"昆仑派");break;
			case 10:strcpy(aDBSSect,"新手");break;
			case 11:strcpy(aDBSSect,"出师");break;
		}		
		aDBSOutput<<aDBSSect<<"人数："<<aDBSSectPlayerCount[i]<<endl;
	}
	aDBSOutput<<"------------------------------------------------"<<endl;
	aDBSOutput<<"总金钱数："<<aDBSMoneyCount<<endl;
	for(i=0;i<12;++i)
	{
		char aDBSSect[32] = {0};
		switch(i)
		{
			case 0:strcpy(aDBSSect,"少林派");break;
			case 1:strcpy(aDBSSect,"天王帮");break;
			case 2:strcpy(aDBSSect,"唐门");break;
			case 3:strcpy(aDBSSect,"五毒教");break;
			case 4:strcpy(aDBSSect,"峨嵋派");break;
			case 5:strcpy(aDBSSect,"翠烟门");break;
			case 6:strcpy(aDBSSect,"丐帮");break;
			case 7:strcpy(aDBSSect,"天忍教");break;
			case 8:strcpy(aDBSSect,"武当派");break;
			case 9:strcpy(aDBSSect,"昆仑派");break;
			case 10:strcpy(aDBSSect,"新手");break;
			case 11:strcpy(aDBSSect,"出师");break;
		}		
		aDBSOutput<<aDBSSect<<"金钱数："<<aDBSSectMoneyCount[i]<<endl;
	}
	aDBSOutput<<"------------------------------------------------"<<endl;
	aDBSOutput<<"角色级别分布[1-200级]："<<endl;
	for(i=1;i<200;++i)
		if(aDBSLevelPlayerCount[i] != 0)
			aDBSOutput<<i<<"级人数："<<aDBSLevelPlayerCount[i]<<endl;
	aDBSOutput<<"级别异常人数："<<aDBSLevelPlayerCount[0]<<endl;
	//======================================

	for(i=0;i<MONEYSTATNUM;++i)
	{	//财富排名前一百玩家中各门派所占比例数
		if( (aStatData.MoneyStat[i].Sect <=10) && (aStatData.MoneyStat[i].Sect >= 1) )
		{
			++aStatData.SectMoneyMost[aStatData.MoneyStat[i].Sect + 1];
		}
		else
		{
			++aStatData.SectMoneyMost[0];
		}
	}
	
	for(i=0;i<LEVELSTATNUM;++i)
	{//级别排名前一百玩家中各门派所占比例数
		if( (aStatData.LevelStat[i].Sect <=10) && (aStatData.LevelStat[i].Sect >= 1) )
		{
			++aStatData.SectLevelMost[aStatData.LevelStat[i].Sect + 1];
		}
		else
		{
			++aStatData.SectLevelMost[0];
		}
	}

	//////////////////////////////排序////////////////////////////////////
	ListSort(aStatData.MoneyStat, MONEYSTATNUM, stMoney);
	ListSort(aStatData.LevelStat, LEVELSTATNUM, stLevel);
	ListSort(aStatData.KillerStat, KILLERSTATNUM, stKiller);
	for(i=0;i<11;++i)
	{//各门派金钱排序
		ListSort(aStatData.MoneyStatBySect[i], SECTMAXSTATNUM, stMoney);
	}
	for(i=0;i<11;++i)
	{//各门派级别排序
		ListSort(aStatData.LevelStatBySect[i], SECTMAXSTATNUM, stLevel);
	}
	
	SAFE_DELETE_ARRAY(RoleFilterList);
	///////////////以下为对统计数据的处理//////////////////////////////////////////////////////
	char aAppPath[MAX_PATH] = {0};
	getcwd(aAppPath,MAX_PATH);
	strcat(aAppPath,"\\StatData.dat");
	//读取旧排名
	memset(&oldGameStatData,0,sizeof(CDBBackup::TStatData));
	FILE* aStatFile = fopen(aAppPath,"rb");
	if(aStatFile)
	{		
		int a = fread(&oldGameStatData, 1, sizeof(CDBBackup::TStatData), aStatFile);
		fclose(aStatFile);
	}
	
	newGameStatData = aStatData;
	
	//找出前十名的oldGameStatData中的排名（或名次上升还是下降）
	for(i=0;i<10;++i)
	{
		int aIndex;
		//等级
		aIndex = CDBBackup::GetIndexByName(
				newGameStatData.LevelStat[i].Name,oldGameStatData.LevelStat, LEVELSTATNUM);
		if(aIndex != -1)
		{
			if(i < aIndex)
				newGameStatData.LevelStat[i].Sort = 1;
			else if(i > aIndex)
				newGameStatData.LevelStat[i].Sort = 255;
			else
				newGameStatData.LevelStat[i].Sort = 0;
		}
		else
			newGameStatData.LevelStat[i].Sort = 1;

		//金钱
		aIndex = CDBBackup::GetIndexByName(
				newGameStatData.MoneyStat[i].Name,oldGameStatData.MoneyStat, MONEYSTATNUM);
		if(aIndex != -1)
		{
			if(i < aIndex)
				newGameStatData.MoneyStat[i].Sort = 1;
			else if(i > aIndex)
				newGameStatData.MoneyStat[i].Sort = 255;
			else
				newGameStatData.MoneyStat[i].Sort = 0;
		}
		else
			newGameStatData.MoneyStat[i].Sort = 1;

		//杀人数
		aIndex = CDBBackup::GetIndexByName(
				newGameStatData.KillerStat[i].Name,oldGameStatData.KillerStat, KILLERSTATNUM);
		if(aIndex != -1)
		{
			if(i < aIndex)
				newGameStatData.KillerStat[i].Sort = 1;
			else if(i > aIndex)
				newGameStatData.KillerStat[i].Sort = 255;
			else
				newGameStatData.KillerStat[i].Sort = 0;
		}
		else
			newGameStatData.KillerStat[i].Sort = 1;

		for(j=0;j<11;++j)
		{//各个门派
			//等级
			aIndex = CDBBackup::GetIndexByName(
					newGameStatData.LevelStatBySect[j][i].Name,oldGameStatData.LevelStatBySect[j], SECTMAXSTATNUM);
			if(aIndex != -1)
			{
				if(i < aIndex)
					newGameStatData.LevelStatBySect[j][i].Sort = 1;
				else if(i > aIndex)
					newGameStatData.LevelStatBySect[j][i].Sort = 255;
				else
					newGameStatData.LevelStatBySect[j][i].Sort = 0;
			}
			else
				newGameStatData.LevelStatBySect[j][i].Sort = 1;

			//金钱
			aIndex = CDBBackup::GetIndexByName(
					newGameStatData.MoneyStatBySect[j][i].Name,oldGameStatData.MoneyStatBySect[j], SECTMAXSTATNUM);
			if(aIndex != -1)
			{
				if(i < aIndex)
					newGameStatData.MoneyStatBySect[j][i].Sort = 1;
				else if(i > aIndex)
					newGameStatData.MoneyStatBySect[j][i].Sort = 255;
				else
					newGameStatData.MoneyStatBySect[j][i].Sort = 0;
			}
			else
				newGameStatData.MoneyStatBySect[j][i].Sort = 1;
		}
	}
	
	//储存新排名
	aStatFile = fopen(aAppPath,"wb");
	if(aStatFile)
	{
		int a = fwrite(&newGameStatData, 1, sizeof(CDBBackup::TStatData), aStatFile);

		fclose(aStatFile);
	}

	MakeSendStatData();//把统计数据写到发送给客户端的统计数据结构中

	//test===========
	if(IS_OUTPUT_LOG){//是否输出统计数据
		aLogFile<<"DB Statistic Log:"<<endl;
		aLogFile<<"--------------Level--------------"<<endl;
		for(i=0;i<10;++i)
		{			
			aLogFile<<SendStatData.LevelStat[i].Name<<"\t"<<
				SendStatData.LevelStat[i].nValue<<"\t"<<
				(int)SendStatData.LevelStat[i].bySort<<endl;
		}
		aLogFile<<"--------------Money--------------"<<endl;
		for(i=0;i<10;++i)
		{
			aLogFile<<SendStatData.MoneyStat[i].Name<<"\t"<<
				SendStatData.MoneyStat[i].nValue<<"\t"<<
				(int)SendStatData.MoneyStat[i].bySort<<endl;
		}
		aLogFile<<"--------------Killer--------------"<<endl;
		for(i=0;i<10;++i)
		{
			aLogFile<<SendStatData.KillerStat[i].Name<<"\t"<<
				SendStatData.KillerStat[i].nValue<<"\t"<<
				(int)SendStatData.KillerStat[i].bySort<<endl;
		}
		for(i=0;i<11;++i)
		{
			aLogFile<<"--------------Sect "<<i<<" Level--------------"<<endl;
			for(j=0;j<10;++j)
			{
				aLogFile<<SendStatData.LevelStatBySect[i][j].Name<<"\t"<<
				SendStatData.LevelStatBySect[i][j].nValue<<"\t"<<
				(int)SendStatData.LevelStatBySect[i][j].bySort<<endl;
			}
			aLogFile<<"-------------Sect "<<i<<" Money---------------"<<endl;
			for(j=0;j<10;++j)
			{
				aLogFile<<SendStatData.MoneyStatBySect[i][j].Name<<"\t"<<
				SendStatData.MoneyStatBySect[i][j].nValue<<"\t"<<
				(int)SendStatData.MoneyStatBySect[i][j].bySort<<endl;
			}
		}
		aLogFile<<"各个门派的玩家数"<<endl;
		for(i=0;i<11;++i)
		{
			aLogFile<<"Sect "<<i<<" ："<<SendStatData.SectPlayerNum[i]<<endl;
		}
		aLogFile<<"财富排名前 "<<MONEYSTATNUM<<" 玩家中各门派所占比例数"<<endl;
		for(i=0;i<11;++i)
		{
			aLogFile<<"Sect "<<i<<" ："<<SendStatData.SectMoneyMost[i]<<endl;
		}
		aLogFile<<"级别排名前 "<<MONEYSTATNUM<<" 玩家中各门派所占比例数"<<endl;
		for(i=0;i<11;++i)
		{
			aLogFile<<"Sect "<<i<<" ："<<SendStatData.SectLevelMost[i]<<endl;
		}
	}
	//================

	IsBackupWorking = false;
}

void CDBBackup::MakeSendStatData()
{//生成发送给客户端的统计数据结构
	int i,j;
	memset(&SendStatData,0,sizeof(TGAME_STAT_DATA));
	for(i=0;i<10;++i)
	{
		strcpy(SendStatData.LevelStat[i].Name, newGameStatData.LevelStat[i].Name);
		SendStatData.LevelStat[i].nValue = newGameStatData.LevelStat[i].Level;
		SendStatData.LevelStat[i].bySort = newGameStatData.LevelStat[i].Sort;

		strcpy(SendStatData.MoneyStat[i].Name, newGameStatData.MoneyStat[i].Name);
		SendStatData.MoneyStat[i].nValue = newGameStatData.MoneyStat[i].Money;
		SendStatData.MoneyStat[i].bySort = newGameStatData.MoneyStat[i].Sort;

		strcpy(SendStatData.KillerStat[i].Name, newGameStatData.KillerStat[i].Name);
		SendStatData.KillerStat[i].nValue = newGameStatData.KillerStat[i].KillNum;
		SendStatData.KillerStat[i].bySort = newGameStatData.KillerStat[i].Sort;
		
		for(j=0;j<11;++j)
		{//各个门派
			strcpy(SendStatData.LevelStatBySect[j][i].Name, newGameStatData.LevelStatBySect[j][i].Name);
			SendStatData.LevelStatBySect[j][i].nValue = newGameStatData.LevelStatBySect[j][i].Level;
			SendStatData.LevelStatBySect[j][i].bySort = newGameStatData.LevelStatBySect[j][i].Sort;

			strcpy(SendStatData.MoneyStatBySect[j][i].Name, newGameStatData.MoneyStatBySect[j][i].Name);
			SendStatData.MoneyStatBySect[j][i].nValue = newGameStatData.MoneyStatBySect[j][i].Money;
			SendStatData.MoneyStatBySect[j][i].bySort = newGameStatData.MoneyStatBySect[j][i].Sort;

			SendStatData.SectLevelMost[j] = newGameStatData.SectLevelMost[j];
			SendStatData.SectMoneyMost[j] = newGameStatData.SectMoneyMost[j];
			SendStatData.SectPlayerNum[j] = newGameStatData.SectPlayerNum[j];
		}
	}
}

void CDBBackup::SaveStatInfo()
{//把游戏世界等级排名写到指定玩家角色中
	TStatData aStatData;
	char aAppPath[MAX_PATH] = {0};
	getcwd(aAppPath,MAX_PATH);
	strcat(aAppPath,"\\StatData.dat");
	//读取排名
	memset(&aStatData,0,sizeof(TStatData));
	FILE* aStatFile = fopen(aAppPath,"rb");
	if(aStatFile)
	{		
		int aRead = fread(&aStatData, sizeof(TStatData), 1, aStatFile);
		if(aRead != 1) return;
		fclose(aStatFile);
	}
	else
	{
		return;
	}
	
	int i,j;
	for(i=0;i<LEVELSTATNUM;++i)
	{
		if(aStatData.LevelStat[i].Name[0] == '\0')continue;
		ZCursor *cursor = RunTable->search(aStatData.LevelStat[i].Name, strlen(aStatData.LevelStat[i].Name) +1);
//		char* aBuffer = RunTable->search(aStatData.LevelStat[i].Name, strlen(aStatData.LevelStat[i].Name) +1,size);
		if(!cursor)
		{
			continue;
		}
		TRoleData* pRoleData = (TRoleData*)cursor->data;
		pRoleData->BaseInfo.nWorldStat = i+1;
		
		if (pRoleData->dwDataLen - 4 == pRoleData->dwFriendOffset)
		{
			DWORD dwCRC = 0;
			dwCRC = CRC32(dwCRC, pRoleData, pRoleData->dwDataLen - 4);
			memcpy(cursor->data + pRoleData->dwDataLen - 4, &dwCRC, 4);
		}

		RunTable->add(aStatData.LevelStat[i].Name, strlen(aStatData.LevelStat[i].Name) +1, cursor->data, cursor->size);
		RunTable->closeCursor(cursor);
	}


	//把门派等级排名写到指定玩家角色中
	for(i=0;i<11;++i)
	{
		for(j=0;j<SECTMAXSTATNUM;++j)
		{
			if(aStatData.LevelStatBySect[i][j].Name[0] == '\0')continue;
			ZCursor *cursor = RunTable->search( aStatData.LevelStatBySect[i][j].Name, strlen(aStatData.LevelStatBySect[i][j].Name) +1 );
//			char* aBuffer = RunTable->search(aStatData.LevelStatBySect[i][j].Name, strlen(aStatData.LevelStatBySect[i][j].Name) +1,size);
			if(!cursor)
			{
				continue;
			}
			TRoleData* pRoleData = (TRoleData*)cursor->data;
			pRoleData->BaseInfo.nSectStat = j+1;

			if (pRoleData->dwDataLen - 4 == pRoleData->dwFriendOffset)
			{
				DWORD dwCRC = 0;
				dwCRC = CRC32(dwCRC, pRoleData, pRoleData->dwDataLen - 4);
				memcpy(cursor->data + pRoleData->dwDataLen - 4, &dwCRC, 4);
			}
			RunTable->add(aStatData.LevelStatBySect[i][j].Name, strlen(aStatData.LevelStatBySect[i][j].Name) +1, cursor->data, cursor->size);
			RunTable->closeCursor(cursor);
		}
	}
}