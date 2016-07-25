#ifndef	KNpcSetH
#define	KNpcSetH

//#include "KFindBinTree.h"
#include "KLinkArray.h"
#include "KNpc.h"
#include "GameDataDef.h"
#include "KNpcGold.h"

#ifndef _SERVER
#define		MAX_NPC_REQUEST	20
#endif

/*
enum NPC_ENCHANT
{
	enchant_treasure = 0,				// 掉的装备更多
	enchant_aura,						// 光环技能
	enchant_lifeenhance,				// 生命更长
	enchant_lifereplenish,				// 自动回血
	enchant_attackratingenhance,		// 打得更准
	enchant_defenseenhance,				// 防御加强
	enchant_damageenhance,				// 伤害更高
	enchant_speedenhance,				// 速度更快
	enchant_selfresist,					// 自己的五行属性的抗性为100%
	enchant_conquerresist,				// 相生的五行属性的抗性为100%
	enchant_num,
};
*/

#ifndef _SERVER
#define		MAX_INSTANT_STATE	20
#define		MAX_INSTANT_SOUND	30
class	KInstantSpecial
{
private:
	int		m_nLoadFlag;
	char	m_szSprName[MAX_INSTANT_STATE][FILE_NAME_LENGTH];
	char	m_szSoundName[MAX_INSTANT_SOUND][FILE_NAME_LENGTH];

	KCacheNode	*m_pSoundNode;	// 声效指针
	KWavSound	*m_pWave;		// 声效wav指针

private:
	void	LoadSprName();
	void	LoadSoundName();

public:
	KInstantSpecial();
	void	GetSprName(int nNo, char *lpszName, int nLength);
	void	PlaySound(int nNo);
};
#endif

typedef struct
{
	DWORD	dwRequestId;
	DWORD	dwRequestTime;
} RequestNpc;

typedef struct
{
#ifndef _SERVER
	int		nStandFrame[2];
	int		nWalkFrame[2];
	int		nRunFrame[2];
#endif
	int		nWalkSpeed;
	int		nRunSpeed;
	int		nAttackFrame;
	int		nHurtFrame;
} PlayerBaseValue;

class KNpcSet
{
public:
	KNpcGoldTemplate	m_cGoldTemplate;					// 黄金怪物模板
	PlayerBaseValue		m_cPlayerBaseValue;					// 玩家标准数据

#ifdef _SERVER
	int					m_nPKDamageRate;					// PK时伤害乘一个系数
	int					m_nFactionPKFactionAddPKValue;		// 三大阵营之间PK，等级差太大时PK者PK值增加
	int					m_nKillerPKFactionAddPKValue;		// 杀手与三大阵营PK，等级差太大时PK者PK值增加
	int					m_nEnmityAddPKValue;				// 仇杀时PK者PK值增加
	int					m_nBeKilledAddPKValue;				// 被PK致死着PK值增加，应该是个负数
	int					m_nLevelDistance;					// 等级差多少算是PK新手
#endif
#ifndef _SERVER
	KInstantSpecial		m_cInstantSpecial;
#endif
private:
	DWORD				m_dwIDCreator;						//	游戏世界中的ID计数器
	KLinkArray			m_FreeIdx;							//	可用表
	KLinkArray			m_UseIdx;							//	已用表
#ifndef _SERVER
	enum
	{
		PATE_CHAT = 0x01,
		PATE_NAME = 0x02,
		PATE_LIFE = 0x04,
		PATE_MANA = 0x08,
	};
	int					m_nShowPateFlag;					// 是否全部显示玩家的名字在头顶上 zroc add
	RequestNpc			m_RequestNpc[MAX_NPC_REQUEST];		//	向服务器申请的ID表
	KLinkArray			m_RequestFreeIdx;					//	向服务器申请可用表
	KLinkArray			m_RequestUseIdx;					//	向服务器申请空闲表
#endif
public:
	KNpcSet();
	void			Init();
	int				GetNpcCount(int nKind = -1, int nCamp = -1);
	int				SearchName(LPSTR szName);
	int				SearchID(DWORD dwID);
	int				SearchNameID(DWORD dwID);
	BOOL			IsNpcExist(int nIdx, DWORD dwId);
	int				Add(int nNpcSetingIdxInfo, int nSubWorld, int nRegion, int nMapX, int nMapY, int nOffX = 0, int nOffY = 0);
	int				Add(int nNpcSetingIdxInfo, int nSubWorld, int nMpsX, int nMpsY);
	int				Add(int nSubWorld, void* pNpcInfo);
	void			Remove(int nIdx);
	void			RemoveAll();
	NPC_RELATION	GetRelation(int nIdx1, int nIdx2);
	int				GetNearestNpc(int nMapX, int nMapY, int nId, int nRelation);
	static int		GetDistance(int nIdx1, int nIdx2);
	static int		GetDistanceSquare(int nIdx1, int nIdx2);
	int				GetNextIdx(int nIdx);
	// 把所有npc的 bActivateFlag 设为 FALSE (每次游戏循环处理所有npc的activate之前做这个处理)
	void			ClearActivateFlagOfAllNpc();
	void			LoadPlayerBaseValue(LPSTR szFile);
	int				GetPlayerWalkSpeed() { return m_cPlayerBaseValue.nWalkSpeed; };
	int				GetPlayerRunSpeed() { return m_cPlayerBaseValue.nRunSpeed; };
	int				GetPlayerAttackFrame() { return m_cPlayerBaseValue.nAttackFrame; };
	int				GetPlayerHurtFrame() { return m_cPlayerBaseValue.nHurtFrame; };
#ifndef _SERVER
	int				GetPlayerStandFrame(BOOL bMale) 
	{ 
		if (bMale)
			return m_cPlayerBaseValue.nStandFrame[0];
		else
			return m_cPlayerBaseValue.nStandFrame[1];
	};
	int				GetPlayerWalkFrame(BOOL bMale)
	{
		if (bMale)
			return m_cPlayerBaseValue.nWalkFrame[0];
		else
			return m_cPlayerBaseValue.nWalkFrame[1];
	};
	int				GetPlayerRunFrame(BOOL bMale)
	{
		if (bMale)
			return m_cPlayerBaseValue.nRunFrame[0];
		else
			return m_cPlayerBaseValue.nRunFrame[1];
	};
	BOOL			IsNpcRequestExist(DWORD	dwID);
	void			InsertNpcRequest(DWORD dwID);
	void			RemoveNpcRequest(DWORD dwID);
	int				GetRequestIndex(DWORD dwID);
	// 添加一个客户端npc（需要设定ClientNpcID）
	int				AddClientNpc(int nTemplateID, int nRegionX, int nRegionY, int nMpsX, int nMpsY, int nNo);
	// 从npc数组中寻找属于某个region的 client npc ，添加进去
	void			InsertNpcToRegion(int nRegionIdx);
	// 查找某个ClientID的npc是否存在
	int				SearchClientID(KClientNpcID sClientID);
	// 某座标上精确查找Npc，客户端专用
	int				SearchNpcAt(int nX, int nY, int nRelation, int nRange);
	void			CheckBalance();
	int				GetAroundPlayerForTeamInvite(KUiPlayerItem *pList, int nCount);	// 获得周围玩家列表(用于队伍邀请列表)
	void			GetAroundOpenCaptain(int nCamp);		// 获得周围同阵营的已开放队伍队长列表
	int				GetAroundPlayer(KUiPlayerItem *pList, int nCount);	// 获得周围玩家列表(用于列表)

	// 设定是否全部显示玩家的名字  bFlag ==	TRUE 显示，bFlag == FALSE 不显示 zroc add
	void			SetShowNameFlag(BOOL bFlag);
	// 判断是否全部显示玩家的名字  返回值 TRUE 显示，FALSE 不显示
	BOOL			CheckShowName();
	// 设定是否全部显示玩家的聊天  bFlag ==	TRUE 显示，bFlag == FALSE 不显示 zroc add
	void			SetShowChatFlag(BOOL bFlag);
	// 判断是否全部显示玩家的聊天  返回值 TRUE 显示，FALSE 不显示
	BOOL			CheckShowChat();
	// 设定是否全部显示玩家的血  bFlag ==	TRUE 显示，bFlag == FALSE 不显示 zroc add
	void			SetShowLifeFlag(BOOL bFlag);
	// 判断是否全部显示玩家的血  返回值 TRUE 显示，FALSE 不显示
	BOOL			CheckShowLife();
	// 设定是否全部显示玩家的内力  bFlag ==	TRUE 显示，bFlag == FALSE 不显示 zroc add
	void			SetShowManaFlag(BOOL bFlag);
	// 判断是否全部显示玩家的内力  返回值 TRUE 显示，FALSE 不显示
	BOOL			CheckShowMana();
	
#endif
#ifdef _SERVER
	BOOL			SyncNpc(DWORD dwID, int nClientIdx);
#endif
private:
	void			SetID(int m_nIndex);
	int				FindFree();

    // Add By Freeway Chen in 2003.7.14
private:
    // [第一个NPC.m_Kind][第二个NPC.m_Kind][第一个NPC.m_CurrentCamp][第二个个NPC.m_CurrentCamp]
    unsigned char m_RelationTable[kind_num][kind_num][camp_num][camp_num];

    // 生成关系表
    int GenRelationTable();
    NPC_RELATION GenOneRelation(NPCKIND Kind1, NPCKIND Kind2, NPCCAMP Camp1, NPCCAMP Camp2);

};

// modify by Freeway Chen in 2003.7.14
// 确定两个NPC之间的战斗关系

extern KNpcSet NpcSet;
#endif
