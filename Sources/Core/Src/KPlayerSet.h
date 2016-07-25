#ifndef KPlayerSetH
#define	KPlayerSetH

#include "KLinkArray.h"

#define		MAX_LEVEL					150		// 玩家最大等级
#define		MAX_SERIES					5
#define		MAX_LEAD_LEVEL				100		// 最大统率力等级

typedef struct
{
	int		m_nExp;
	int		m_nMoney;
	int		m_nItem;
	int		m_nEquip;
} KPK_DEATH_PUNISH_PARAM;

typedef struct
{
	int		m_nLevel;
	int		m_nLeadLevel;
	int		m_nMoney;
} STONG_CREATE_PARAM;


class KLevelAdd
{
private:
	int		m_nLevelExp[MAX_LEVEL];				// 升级经验
	int		m_nLifePerLevel[MAX_SERIES];			// 每个系升级加生命点
	int		m_nStaminaPerLevel[MAX_SERIES];			// 每个系升级加体力点
	int		m_nManaPerLevel[MAX_SERIES];			// 每个系升级加内力点
	int		m_nLifePerVitality[MAX_SERIES];			// 每个系活力点增加一点后生命点增长
	int		m_nStaminaPerVitality[MAX_SERIES];		// 每个系活力点增加一点后体力点增长
	int		m_nManaPerEnergy[MAX_SERIES];			// 每个系精力点增加一点后内力点增长
	int		m_nLeadExpShare[MAX_SERIES];			// 每个系获得战斗经验时分配统率力经验的参数
	int		m_nFireResistPerLevel[MAX_SERIES];		// 每个系升级加火抗性
	int		m_nColdResistPerLevel[MAX_SERIES];		// 每个系升级加冰抗性
	int		m_nPoisonResistPerLevel[MAX_SERIES];	// 每个系升级加毒抗性
	int		m_nLightResistPerLevel[MAX_SERIES];		// 每个系升级加电抗性
	int		m_nPhysicsResistPerLevel[MAX_SERIES];	// 每个系升级加物理抗性
public:
	KLevelAdd();
	BOOL		Init();
	int			GetLevelExp(int nLevel);
	int			GetLifePerLevel(int nSeries);
	int			GetStaminaPerLevel(int nSeries);
	int			GetManaPerLevel(int nSeries);
	int			GetLifePerVitality(int nSeries);
	int			GetStaminaPerVitality(int nSeries);
	int			GetManaPerEnergy(int nSeries);
	int			GetLeadExpShare(int nSeries);
	int			GetFireResist(int nSeries, int nLevel);
	int			GetColdResist(int nSeries, int nLevel);
	int			GetPoisonResist(int nSeries, int nLevel);
	int			GetLightResist(int nSeries, int nLevel);
	int			GetPhysicsResist(int nSeries, int nLevel);
};


class KTeamLeadExp
{
public:
	struct LEADLEVEL
	{
		DWORD	m_dwExp;								// 统率力升级经验
		DWORD	m_dwMemNum;								// 统率等级决定能统率的队员人数
	}	m_sPerLevel[MAX_LEAD_LEVEL];
public:
	KTeamLeadExp();
	BOOL		Init();									// 初始化
	int			GetLevel(DWORD dwExp, int nCurLeadLevel = 0);// 传入经验获得等级数 (如果传入当前等级，可以加快搜索速度)
	int			GetMemNumFromExp(DWORD dwExp);			// 传入经验获得可带队员数
	int			GetMemNumFromLevel(int nLevel);			// 传入等级获得可带队员数
	int			GetLevelExp(int nLevel);				// 传入等级获得升级所需经验值
};

class KNewPlayerAttribute
{
public:
	int			m_nStrength[series_num];
	int			m_nDexterity[series_num];
	int			m_nVitality[series_num];
	int			m_nEngergy[series_num];
	int			m_nLucky[series_num];
public:
	KNewPlayerAttribute();
	BOOL		Init();
};

class KPlayerSet
{
private:	// 用于优化查找速度
	KLinkArray		m_FreeIdx;				//	可用表
	KLinkArray		m_UseIdx;				//	已用表
	int				m_nListCurIdx;			// 用于 GetFirstPlayer 和 GetNextPlayer
#ifdef _SERVER
	unsigned long	m_ulNextSaveTime;
	unsigned long	m_ulDelayTimePerSave;	//1mins
	unsigned long	m_ulMaxSaveTimePerPlayer; //30mins
#endif
public:
	KLevelAdd				m_cLevelAdd;
	KTeamLeadExp			m_cLeadExp;
	KNewPlayerAttribute		m_cNewPlayerAttribute;
	STONG_CREATE_PARAM		m_sTongParam;

#ifdef _SERVER
	KPK_DEATH_PUNISH_PARAM	m_sPKPunishParam[MAX_DEATH_PUNISH_PK_VALUE + 1];	// PK惩罚参数
#endif

public:
	KPlayerSet();
	BOOL	Init();
	int		FindSame(DWORD dwID);
	int		GetFirstPlayer();				// 遍历所有玩家第一步
	int		GetNextPlayer();				// 遍历所有玩家下一步(这支函数必须在上一支调用之后才能调用)
	int		GetOnlinePlayerCount() { return m_UseIdx.GetCount(); }
//	int		FindClient(int nClient);

#ifdef	_SERVER
	void	Activate();
	int		Add(LPSTR szPlayerID, void* pGuid);
	void	PrepareRemove(int nClientIdx);
	void	PrepareExchange(int nIndex);
	void	PrepareLoginFailed(int nIndex);
	void	RemoveQuiting(int nIndex);
	void	RemoveLoginTimeOut(int nIndex);
	void	RemoveExchanging(int nIndex);
	void	AutoSave();
	void	ProcessClientMessage(int nClient, const char* pChar, int nSize);
	int		GetPlayerNumber() { return m_nNumPlayer; }
	BOOL	GetPlayerName(int nClient, char* szName);
	BOOL	GetPlayerAccount(int nClient, char* szName);
	int		Broadcasting(char* pMessage, int nLen);
	void	SetSaveBufferPtr(void* pData);
	int		AttachPlayer(const unsigned long lnID, GUID* pGuid);
	int		GetPlayerIndexByGuid(GUID* pGuid);
#endif

private:
	int		FindFree();

#ifdef	_SERVER
	//welcome message deliver
public:
	void	ReloadWelcomeMsg();

private:
	void	SendWelcomeMsg(int nPlayerIndex);
	void*	m_pWelcomeMsg;

private:
	int		m_nNumPlayer;
#endif
};

extern KPlayerSet PlayerSet;
#endif //KPlayerSetH
