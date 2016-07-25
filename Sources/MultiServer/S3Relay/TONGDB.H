// TongDB.h: interface for the CTongDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TONGDB_H__8C487F07_C490_4562_AD01_77953CCCBCE2__INCLUDED_)
#define AFX_TONGDB_H__8C487F07_C490_4562_AD01_77953CCCBCE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DBTable.h"
#include "KTongControl.h"

#define MAX_TONG_COUNT 10000

/*
enum enumMemberClass		//帮众类型
{
	mcMaster = 0,			//帮主
	mcDirector,			//长老
	mcManager,			//长老
	mcMember			//帮众
};
*/

typedef struct
{
	int			nCamp;								// 帮会阵营
	DWORD		dwMoney;								// 帮会资金
	int			nCredit;								// 帮会声望
	int			nLevel;								// 帮会等级

	char		szName[defTONG_STR_LENGTH];			// 帮会名
	char		szPassword[defTONG_STR_LENGTH];		// 帮会密码

	char		szMasterTitle[defTONG_STR_LENGTH];	// 帮主称号
	char		szDirectorTitle[defTONG_MAX_DIRECTOR][defTONG_STR_LENGTH];// 各长老称号
	char		szManagerTitle[defTONG_MAX_MANAGER][defTONG_STR_LENGTH];	// 各队长称号
	char		szNormalTitle[defTONG_STR_LENGTH];	// 帮会帮众称号
}TTongStruct;	//用作存入数据库的结构

typedef struct
{
	char		szName[defTONG_STR_LENGTH];			// 帮众名
	char		szTong[defTONG_STR_LENGTH];		// 帮会名
	TONG_MEMBER_FIGURE		MemberClass;			//帮众类型
	int			nTitleIndex;					//帮众称号索引
}TMemberStruct;	//用作存入数据库的结构

class CTongDB  
{
private:
	ZDBTable* m_TongTable;						//帮会数据表
	ZDBTable* m_MemberTable;					//帮众数据表
	
	TTongList m_TongList[MAX_TONG_COUNT];	//帮会列表
	int m_TongListCount;						//帮会列表数
public:
	CTongDB();
	virtual ~CTongDB(){};

	bool Open();
	void Close();
	
	bool ChangeTong(CTongControl& aTongControl);	//创建和改变帮派
	bool DelTong(char* aTongName);	//删除帮派
	bool SearchTong(char* aTongName, CTongControl& aTongControl);	//从数据库中取得帮派
	int GetTongCount();							//取得门派数
	int GetTongList(TTongList* aTongList, int aMaxNum);	//取得帮会列表

	bool ChangeMember(TMemberStruct aMember);//增加帮众
	bool DelMember(char* aMemberName);		//删除帮众
	
};

#endif // !defined(AFX_TONGDB_H__8C487F07_C490_4562_AD01_77953CCCBCE2__INCLUDED_)
