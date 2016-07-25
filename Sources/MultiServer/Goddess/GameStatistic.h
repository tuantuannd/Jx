// GameStatistic.h: interface for the GameStatistic class.
//游戏统计类
//By Fellow
//////////////////////////////////////////////////////////////////////

#if !defined(GAMESTATISTIC_H)
#define GAMESTATISTIC_H

#include "DBTable.h"
#include "S3DBInterface.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TGameStatistic  
{
public://统计列表的数据结构
	struct TRoleList
	{
		char Name[32];
		//char Account[32];
		int Level;
		int Money;
		int Sect;			//门派
	};
	struct TStatData
	{
		TRoleList MoneyStat[100];			//金钱最多排名列表（一百个玩家）
		TRoleList LevelStat[100];			//级别最多排名列表（一百个玩家）
		TRoleList MoneyStatBySect[11][10];	//各门派金钱最多排名列表[门派][玩家数]
		TRoleList LevelStatBySect[11][10];	//各门派级别最多排名列表[门派][玩家数]
		int SectPlayerNum[11];				//各个门派的玩家数
		int SectMoneyMost[11];				//财富排名前一百玩家中各门派所占比例数
		int SectLevelMost[11];				//级别排名前一百玩家中各门派所占比例数
		
	};
private:
	ZDBTable* DBTable;			//数据库
	
	enum StatType{stMoney, stLevel};//需要判断的类别（stMoney, stLevel）
	
	TRoleList* GetMin(			//查找出列表中钱/级别（或其他）最少的一个元素
		TRoleList* const aRoleList,	//列表
		const int n,				//列表大小
		const StatType aType);		//需要判断的类别（stMoney, stLevel）

	void ListSort(			//对列表做特定的排序
		TRoleList* const aRoleList,	
		const int n,
		const StatType aType);
	void RoleDataCopy(TRoleList* Desc, TRoleData* Source);//把RoleData有用的结构复制到RoleList结构中
public:
	TGameStatistic(){DBTable = NULL;};
	TGameStatistic(ZDBTable * aTable){DBTable = aTable;};
	virtual ~TGameStatistic(){};
	void SetDBTable(ZDBTable * aTable){DBTable = aTable;};
//操作
	bool AllStat(TStatData& aStatData);//全部统计
};

#endif // !defined(GAMESTATISTIC_H)
