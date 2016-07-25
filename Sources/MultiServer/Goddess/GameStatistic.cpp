// GameStatistic.cpp: implementation of the GameStatistic class.
//游戏统计类
//By Fellow
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameStatistic.h"

using namespace std;

TGameStatistic::TRoleList* TGameStatistic::GetMin(
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
			}
			if(aDest > aSourse)
			{
				tmpData = &aRoleList[i];
				break;
			}
		}
		if(i==n)break;
	}
	return tmpData;
}


void TGameStatistic::ListSort(
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
			int aSourse, aDest;
			switch(aType)
			{
			case stMoney:
				aDest = tmpData->Money;
				aSourse = aRoleList[j].Money;
				break;
			case stLevel:
				aDest = tmpData->Level;
				aSourse = aRoleList[j].Level;
				break;
			}
			if( aSourse > aDest )
					swap(*tmpData, aRoleList[j]);
		}
	}
}

void TGameStatistic::RoleDataCopy(TRoleList* Desc, TRoleData* Source)
{//把RoleData有用的结构复制到RoleList结构中
	//strcpy(Desc->Account, Source->BaseInfo.caccname);
	strcpy(Desc->Name, Source->BaseInfo.szName);
	Desc->Sect = Source->BaseInfo.nSect;
	Desc->Money = Source->BaseInfo.imoney + Source->BaseInfo.isavemoney;
	Desc->Level = Source->BaseInfo.ifightlevel;
}

bool TGameStatistic::AllStat(TStatData& aStatData)
{//全部统计
	if( !DBTable ) return false;
	int i;
	
	memset(&aStatData, 0, sizeof(TStatData));
	char* aBuffer;
	int size;
	TRoleData* pRoleData;

	///////数据库遍历/////////////
	aBuffer = DBTable->GetRecord(size, ZDBTable::cpFirst, 0);
	while(aBuffer)
	{
		TRoleList* tmpData;
		pRoleData = (TRoleData*)aBuffer;
		
		//////////////////////////////玩家统计////////////////////////////////////
		//对金钱排序
		tmpData = GetMin(aStatData.MoneyStat, 100, stMoney);
		if( tmpData->Money < pRoleData->BaseInfo.imoney + pRoleData->BaseInfo.isavemoney)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}

		//对级别排序
		tmpData = GetMin(aStatData.LevelStat, 100, stLevel);
		if( tmpData->Level < pRoleData->BaseInfo.ifightlevel)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}
		
		//各门派对金钱排序
		if( (pRoleData->BaseInfo.nSect <=10) && (pRoleData->BaseInfo.nSect >= 1) )
		{
			tmpData = GetMin(aStatData.MoneyStatBySect[pRoleData->BaseInfo.nSect], 10, stMoney);
		}
		else
		{
			tmpData = GetMin(aStatData.MoneyStatBySect[0], 10, stMoney);
		}
		if( tmpData->Money < pRoleData->BaseInfo.imoney  + pRoleData->BaseInfo.isavemoney)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}

		//各门派对级别排序
		if( (pRoleData->BaseInfo.nSect <=10) && (pRoleData->BaseInfo.nSect >= 1) )
		{
			tmpData = GetMin(aStatData.LevelStatBySect[pRoleData->BaseInfo.nSect], 10, stMoney);
		}
		else
		{
			tmpData = GetMin(aStatData.LevelStatBySect[0], 10, stMoney);
		}
		if( tmpData->Level < pRoleData->BaseInfo.ifightlevel)
		{//如果当前数据较大就把当前数据代替列表中最小的
			RoleDataCopy(tmpData, pRoleData);
		}
		

		//////////////////////////////门派统计////////////////////////////////////
		//各个门派的玩家数统计
		if( (pRoleData->BaseInfo.nSect <=10) && (pRoleData->BaseInfo.nSect >= 1) )
		{
			++aStatData.SectPlayerNum[pRoleData->BaseInfo.nSect];
		}
		else
		{
			++aStatData.SectPlayerNum[0];
		}
				
		//清除当前缓存，游标指向下一个记录
		delete [] aBuffer;
		aBuffer = DBTable->GetRecord(size,ZDBTable::cpNext);
	}

	//////////////////////////////门派统计////////////////////////////////////
	for(i=0;i<100;++i)
	{	//财富排名前一百玩家中各门派所占比例数
		if( (aStatData.MoneyStat[i].Sect <=10) && (aStatData.MoneyStat[i].Sect >= 1) )
		{
			++aStatData.SectMoneyMost[aStatData.MoneyStat[i].Sect];
		}
		else
		{
			++aStatData.SectMoneyMost[0];
		}
		
		//级别排名前一百玩家中各门派所占比例数
		if( (aStatData.LevelStat[i].Sect <=10) && (aStatData.LevelStat[i].Sect >= 1) )
		{
			++aStatData.SectLevelMost[aStatData.LevelStat[i].Sect];
		}
		else
		{
			++aStatData.SectLevelMost[0];
		}
	}

	//////////////////////////////排序////////////////////////////////////
	ListSort(aStatData.MoneyStat, 100, stMoney);
	ListSort(aStatData.LevelStat, 100, stLevel);
	for(i=0;i<11;++i)
	{//各门派金钱排序
		ListSort(aStatData.MoneyStatBySect[i], 10, stMoney);
	}
	for(i=0;i<11;++i)
	{//各门派级别排序
		ListSort(aStatData.LevelStatBySect[i], 10, stLevel);
	}
	
	return true;
}
