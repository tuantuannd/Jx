#ifndef KNpcH
#define KNpcH
//---------------------------------------------------------------------------
class ISkill;
#include "KCore.h"
#include "KSkillList.h"
#include "KMagicAttrib.h"
#include "GameDataDef.h"
#include "KNpcFindPath.h"
#include "KNpcDeathCalcExp.h"
#include "KIndexNode.h"
#include "KNpcGold.h"

class KSkill;
#ifndef _SERVER
#include "KNpcRes.h"
#endif
//---------------------------------------------------------------------------
#ifdef	_SERVER
#define	MAX_NPC			16384
#else
#define	MAX_NPC			256
#endif
#define MAX_NPCSTYLE	2000

#define		MAX_AI_PARAM					11
#define		MAX_NPC_USE_SKILL				4

//	跑动速度与行走速度的倍数
//#define	WALK_RUN_TIMES	3

#define		STATE_FREEZE	0x01
#define		STATE_POISON	0x02
#define		STATE_CONFUSE	0x04
#define		STATE_STUN		0x08

enum NPCATTRIB
{
	attrib_mana,
	attrib_stamina,
	attrib_life,
	attrib_maxmana,
	attrib_maxstamina,
	attrib_maxlife,
};

enum NPCCMD
{
	do_none,		// 什么也不干
	do_stand,		// 站立
	do_walk,		// 行走
	do_run,			// 跑动
	do_jump,		// 跳跃
	do_skill,		// 发技能的命令
	do_magic,		// 施法
	do_attack,		// 攻击
	do_sit,			// 打坐
	do_hurt,		// 受伤
	do_death,		// 死亡
	do_defense,		// 格挡
	do_idle,		// 喘气
	do_specialskill,// 技能控制动作
	do_special1,	// 特殊1
	do_special2,	// 特殊2
	do_special3,	// 特殊3
	do_special4,	// 特殊4
	do_runattack,
	do_manyattack,
	do_jumpattack,
	do_revive,
};

enum CLIENTACTION
{
	cdo_fightstand,
	cdo_stand,
	cdo_stand1,
	cdo_fightwalk,
	cdo_walk,
	cdo_fightrun,
	cdo_run,
	cdo_hurt,
	cdo_death,
	cdo_attack,
	cdo_attack1,
	cdo_magic,
	cdo_sit,
	cdo_jump,
	cdo_none,   
	cdo_count,
};

enum DAMAGE_TYPE
{
	damage_physics = 0,		// 物理伤害
	damage_fire,			// 火焰伤害
	damage_cold,			// 冰冻伤害
	damage_light,			// 闪电伤害
	damage_poison,			// 毒素伤害
	damage_magic,			// 无属性伤害
	damage_num,				// 伤害类型数目
};

// DoDeath 时的参数，对应不同的死亡惩罚
enum	enumDEATH_MODE
{
	enumDEATH_MODE_NPC_KILL = 0,		// 被npc杀死
	enumDEATH_MODE_PLAYER_NO_PUNISH,	// 切磋模式被玩家杀死
	enumDEATH_MODE_PLAYER_PUNISH,		// 被玩家PK致死，根据PK值进行惩罚
	enumDEATH_MODE_PKBATTLE_PUNISH,		// 类式于国战时的惩罚处理
	enumDEATH_MODE_NUM,
};

typedef struct
{
	NPCCMD		CmdKind;		// 命令C
	int			Param_X;		// 参数X
	int			Param_Y;		// 参数Y
	int			Param_Z;		// 参数Y
} NPC_COMMAND;

typedef struct
{
	int		nTotalFrame;
	int		nCurrentFrame;
} DOING_FRAME;

struct KState
{
	int	nMagicAttrib;
	int	nValue[2];
	int	nTime;
};

struct	KSyncPos
{
	DWORD	m_dwRegionID;
	int		m_nMapX;
	int		m_nMapY;
	int		m_nOffX;
	int		m_nOffY;
	int		m_nDoing;
};

class KStateNode : public KNode
{
public:

	int				m_SkillID;					// 技能ID
	int				m_Level;					// 技能等级
	int				m_LeftTime;					// 剩余时间
	KMagicAttrib	m_State[MAX_SKILL_STATE];	// 修改属性列表
	int				m_StateGraphics;			// 状态动画索引
};

#ifndef _SERVER
/*--- 用于标明客户端npc是哪个region的第几个npc ，
如果这是一个服务器控制的npc ，ID 值为 0 ，No 值为 -1  ---*/
struct	KClientNpcID
{
	DWORD	m_dwRegionID;
	int		m_nNo;
};
#endif

#ifdef _SERVER
struct KItemDropRate
{
	struct	KItemParam
	{
		int		nGenre;
		int		nDetailType;
		int		nParticulType;
		int		nRate;
	};
	int			nCount;
	int			nMagicRate;
	int			nMaxRandRate;
	int			nMoneyRate;
	int			nMoneyScale;
	int			nMinItemLevelScale;
	int			nMaxItemLevelScale;
	int			nMinItemLevel;
	int			nMaxItemLevel;
	KItemParam*	pItemParam;
};
#endif

class KNpc
{
	friend class KNpcSet;
public:
	DWORD				m_dwID;					// Npc的ID
	int					m_Index;				// Npc的索引
	KIndexNode			m_Node;					// Npc's Node
	int					m_Level;				// Npc的等级
	DWORD				m_Kind;					// Npc的类型
	int					m_Series;				// Npc的系
	int					m_Height;				// Npc的高度(跳跃的时候非零)
	BYTE				m_btRankId;
	int					m_nStature;				//Tall 

	BYTE				m_btStateInfo[MAX_NPC_RECORDER_STATE];	// Npc当前最新的几个状态 
	int					m_nNextStatePos;		// 下一次状态在m_btState的位置	

//	DWORD				m_NpcEnchant;			// Npc的加强类型（黄金怪物）
	KNpcGold			m_cGold;

	//用于Npc模板库中，当FALSE表示该Npc数据当前是无效的 ，数值未经过脚本计算，需要生成.
	//TRUE表示有效数据
	BOOL				m_bHaveLoadedFromTemplate;// 
	
	KState				m_PowerState;			// 怒火状态
	KState				m_PoisonState;			// 中毒状态
	KState				m_FreezeState;			// 冰冻状态
	KState				m_BurnState;			// 燃烧状态
	KState				m_ConfuseState;			// 混乱状态
	KState				m_StunState;			// 眩晕状态
	KState				m_LifeState;			// 补血状态
	KState				m_ManaState;			// 补MANA状态
	KState				m_MenuState;			// 菜单
	KState				m_DrunkState;			// 醉酒状态

	KState				m_PhysicsArmor;
	KState				m_ColdArmor;
	KState				m_LightArmor;
	KState				m_PoisonArmor;
	KState				m_FireArmor;
	KState				m_ManaShield;

	KList				m_StateSkillList;		// 附加状态技能列表（同技能不叠加）
	int					m_Camp;					// Npc的阵营
	int					m_CurrentCamp;			// Npc的当前阵营
	NPCCMD				m_Doing;				// Npc的行为
	CLIENTACTION		m_ClientDoing;			// Npc的客户端行为
	DOING_FRAME			m_Frames;				// Npc的行为帧数
	KSkillList			m_SkillList;			// Npc的技能列表
	int					m_SubWorldIndex;		// Npc所在的SubWorld ID
	int					m_RegionIndex;			// Npc所在的Region ID
	int					m_ActiveSkillID;		// Npc激活的技能ID

	int					m_ActiveAuraID;			// Npc激活的光环技能ID

	// Npc的实际数据（已经经过装备及技能的运算了）
	int					m_CurrentExperience;	// Npc被杀后送出的经验
	int					m_CurrentLife;			// Npc的当前生命
	int					m_CurrentLifeMax;		// Npc的当前生命最大值
	int					m_CurrentLifeReplenish;	// Npc的当前生命回复速度
	int					m_CurrentMana;			// Npc的当前内力
	int					m_CurrentManaMax;		// Npc的当前最大内力
	int					m_CurrentManaReplenish;	// Npc的当前内力回复速度
	int					m_CurrentStamina;		// Npc的当前体力
	int					m_CurrentStaminaMax;	// Npc的当前最大体力
	int					m_CurrentStaminaGain;	// Npc的当前体力回复速度
	int					m_CurrentStaminaLoss;	// Npc的当前体力下降速度
	KMagicAttrib		m_PhysicsDamage;		// Npc的当前伤害(由力量敏捷与武器伤害决定，不考虑直接加伤害的魔法属性)
	KMagicAttrib		m_CurrentFireDamage;	// Npc的当前火伤害
	KMagicAttrib		m_CurrentColdDamage;	// Npc的当前冰伤害
	KMagicAttrib		m_CurrentLightDamage;	// Npc的当前电伤害
	KMagicAttrib		m_CurrentPoisonDamage;	// Npc的当前毒伤害
	int					m_CurrentAttackRating;	// Npc的当前命中率
	int					m_CurrentDefend;		// Npc的当前防御

	int					m_CurrentFireResist;	// Npc的当前火抗性
	int					m_CurrentColdResist;	// Npc的当前冰抗性
	int					m_CurrentPoisonResist;	// Npc的当前毒抗性
	int					m_CurrentLightResist;	// Npc的当前电抗性
	int					m_CurrentPhysicsResist;	// Npc的当前物理抗性
	int					m_CurrentFireResistMax;		// Npc的当前最大火抗性
	int					m_CurrentColdResistMax;		// Npc的当前最大冰抗性
	int					m_CurrentPoisonResistMax;	// Npc的当前最大毒抗性
	int					m_CurrentLightResistMax;	// Npc的当前最大电抗性
	int					m_CurrentPhysicsResistMax;	// Npc的当前最大物理抗性

	int					m_CurrentWalkSpeed;		// Npc的当前走动速度
	int					m_CurrentRunSpeed;		// Npc的当前跑动速度
	int					m_CurrentJumpSpeed;		// Npc的当前跳跃速度
	int					m_CurrentJumpFrame;		// Npc的当前跳跃帧数
	int					m_CurrentAttackSpeed;	// Npc的当前攻击速度
	int					m_CurrentCastSpeed;		// Npc的当前施法速度
	int					m_CurrentVisionRadius;	// Npc的当前视野范围
	int					m_CurrentAttackRadius;	// Npc的当前攻击范围
	int					m_CurrentActiveRadius;	// Npc的当前活动范围
	int					m_CurrentHitRecover;	// Npc的当前受击回复速度
	int					m_CurrentTreasure;		// Npc丢落装备的数量

// 以下数据只需要当前值，基础值均为0
	int					m_CurrentMeleeDmgRetPercent;	// Npc近程伤害返回的百分比
	int					m_CurrentMeleeDmgRet;			// Npc近程被击时返回的伤害点数
	int					m_CurrentRangeDmgRetPercent;	// Npc远程伤害返回的百分比
	int					m_CurrentRangeDmgRet;			// Npc远程被击时返回的伤害点数
	BOOL				m_CurrentSlowMissle;			// Npc是否在慢速子弹状态下

	int					m_CurrentDamageReduce;			// 物理伤害减少
	int					m_CurrentElementDamageReduce;	// 元素伤害减少

	int					m_CurrentDamage2Mana;			// 伤害转内力百分比
	int					m_CurrentManaPerEnemy;			// 杀一个敌人加多少点内力
	int					m_CurrentLifeStolen;			// 偷生命百分比
	int					m_CurrentManaStolen;			// 偷内力百分比
	int					m_CurrentStaminaStolen;			// 偷体力百分比
	int					m_CurrentKnockBack;				// 震退百分比
	int					m_CurrentDeadlyStrike;			// 致命一击百分比
	int					m_CurrentBlindEnemy;			// 致盲敌人百分比
	int					m_CurrentPiercePercent;			// 穿透攻击百分比
	int					m_CurrentFreezeTimeReducePercent;	// 冰冻时间减少百分比
	int					m_CurrentPoisonTimeReducePercent;	// 中毒时间减少百分比
	int					m_CurrentStunTimeReducePercent;		// 眩晕时间减少百分比

	int					m_CurrentFireEnhance;			// 火加强
	int					m_CurrentColdEnhance;			// 冰加强
	int					m_CurrentPoisonEnhance;			// 毒加强
	int					m_CurrentLightEnhance;			// 电加强
	int					m_CurrentAddPhysicsDamage;		// 直接的物理伤害加强点数
	int					m_CurrentMeleeEnhance[MAX_MELEE_WEAPON];	// 近程物理加强
	int					m_CurrentRangeEnhance;			// 远程物理加强
	int					m_CurrentHandEnhance;			// 空手物理加强
// 只需要当前值的数据结束
	int					m_Dir;							// Npc的方向
	int					m_RedLum;						// Npc的亮度
	int					m_GreenLum;
	int					m_BlueLum;
	int					m_MapX, m_MapY, m_MapZ;			// Npc的地图坐标
	int					m_OffX, m_OffY;					// Npc在格子中的偏移坐标（放大了1024倍）
	int					m_DesX, m_DesY;					// Npc的目标坐标
	int					m_SkillParam1, m_SkillParam2;
	int					m_OriginX, m_OriginY;			// Npc的原始坐标
	int					m_NextAITime;
	BYTE				m_AIMAXTime;//NpcAI
// Npc的装备（决定客户端的换装备）	
	int					m_HelmType;					// Npc的头盔类型
	int					m_ArmorType;				// Npc的盔甲类型
	int					m_WeaponType;				// Npc的武器类型
	int					m_HorseType;				// Npc的骑马类型
	BOOL				m_bRideHorse;				// Npc是否骑马

	char				Name[32];				// Npc的名称
	int					m_nSex;					// Npc的性别0为男，1为女
	int					m_NpcSettingIdx;		// Npc的设定文件索引
	int					m_CorpseSettingIdx;		// Npc的尸体定义索引
	char				ActionScript[80];		// Npc的行为脚本
	DWORD				m_ActionScriptID;		// Npc的行为脚本ID（使用时用这个来检索）
	
	DWORD				m_TrapScriptID;			// Npc的当前Trap脚本ID;

	int					m_nPeopleIdx;			// 对象人物
	int					m_nLastDamageIdx;		// 最后一次伤害的人物索引
	int					m_nLastPoisonDamageIdx;	// 最后一次毒伤害的人物索引
	int					m_nObjectIdx;			// 对象物品

	// Npc的基本数据（未考虑装备、技能的影响）
	int					m_Experience;			// Npc被杀获得的经验
	int					m_LifeMax;				// Npc的最大生命
	int					m_LifeReplenish;		// Npc的生命回复速度
	int					m_ManaMax;				// Npc的最大内力
	int					m_ManaReplenish;		// Npc的内力回复速度
	int					m_StaminaMax;			// Npc的最大体力
	int					m_StaminaGain;			// Npc的体力回复速度
	int					m_StaminaLoss;			// Npc的体力下降速度
	int					m_AttackRating;			// Npc的命中率
	int					m_Defend;				// Npc的防御
	int					m_FireResist;			// Npc的火抗性
	int					m_ColdResist;			// Npc的冷抗性
	int					m_PoisonResist;			// Npc的毒抗性
	int					m_LightResist;			// Npc的电抗性
	int					m_PhysicsResist;		// Npc的物理抗性
	int					m_FireResistMax;		// Npc的最大火抗性
	int					m_ColdResistMax;		// Npc的最大冰抗性
	int					m_PoisonResistMax;		// Npc的最大毒抗性
	int					m_LightResistMax;		// Npc的最大电抗性
	int					m_PhysicsResistMax;		// Npc的最大物理抗性
	int					m_WalkSpeed;			// Npc的行走速度
	int					m_RunSpeed;				// Npc的跑动速度
	int					m_JumpSpeed;			// Npc的跳跃速度
	int					m_AttackSpeed;			// Npc的攻击速度
	int					m_CastSpeed;			// Npc的施法速度
	int					m_VisionRadius;			// Npc的视野范围
	int					m_DialogRadius;			// Npc的对话范围
	int					m_ActiveRadius;			// Npc的活动范围
	int					m_HitRecover;			// Npc的受击回复速度
	int					m_Treasure;				// Npc丢落装备的数量
	BOOL				m_bClientOnly;			// 

	int					m_nCurrentMeleeSkill;	// Npc当前正执行的格斗技能
	int					m_nCurrentMeleeTime;	
	
	// AI参数
	int					m_AiMode;				// AI模式
	int					m_AiParam[MAX_AI_PARAM];// 用于AI模块计算AI
	int					m_AiAddLifeTime;

	int					m_HeadImage;

	int					m_FightMode;			// 客户端处理动作用。
	int					m_OldFightMode;
	BOOL				m_bExchangeServer;
#ifdef _SERVER
	int					m_AiSkillRadiusLoadFlag;// 战斗npc技能范围是否已经初始化 只需要在构造的时候初始化一次
	KNpcDeathCalcExp	m_cDeathCalcExp;		// 战斗npc死亡后送出经验给不同的player
	KItemDropRate*		m_pDropRate;
	int					m_nCurPKPunishState;	// PK死亡时的惩罚性质，用于国战
#endif
#ifndef	_SERVER
	int					m_SyncSignal;			// 同步信号
	KClientNpcID		m_sClientNpcID;			// 用于标明客户端npc是哪个region的第几个npc
	DWORD				m_dwRegionID;			// 本npc所在region的id
	KSyncPos			m_sSyncPos;
	int					m_nPKFlag;	
	char				m_szChatBuffer[MAX_SENTENCE_LENGTH];
	int					m_nChatContentLen;
	int					m_nChatNumLine;
	int					m_nChatFontWidth;
	unsigned int		m_nCurChatTime;
	int					m_nSleepFlag;
	int					m_nHurtHeight;
	int					m_nHurtDesX;
	int					m_nHurtDesY;

	int					m_nTongFlag;			// 是否有招人图标
#endif
private:
	int					m_LoopFrames;			// 循环帧数
	int					m_nPlayerIdx;
	int					m_DeathFrame;			// 死亡帧数
	int					m_StandFrame;
	int					m_HurtFrame;
	int					m_AttackFrame;
	int					m_CastFrame;
	int					m_WalkFrame;
	int					m_RunFrame;
	int					m_StandFrame1;
	int					m_ReviveFrame;			// 重生帧数
	int					m_SitFrame;
	int					m_JumpFrame;
	int					m_JumpFirstSpeed;
	NPC_COMMAND			m_Command;				// 命令结构
	BOOL				m_ProcessAI;			// 处理AI标志
	BOOL				m_ProcessState;			// 处理状态标志
	int					m_XFactor;
	int					m_YFactor;
	int					m_JumpStep;
	int					m_JumpDir;
	int					m_SpecialSkillStep;		// 特殊技能步骤
	NPC_COMMAND			m_SpecialSkillCommand;	// 特殊技能行为命令
	KNpcFindPath		m_PathFinder;
	BOOL				m_bActivateFlag;

#ifndef	_SERVER
	int					m_ResDir;
	KNpcRes				m_DataRes;				// Npc的客户端资源（图象、声音）

	// 用于普通战斗npc冒血处理
	int					m_nBloodNo;
	int					m_nBloodAlpha;
	int					m_nBloodTime;
	char				m_szBloodNo[32];
#endif

private:
	BOOL				WaitForFrame();
	BOOL				IsReachFrame(int nPercent);
	void				DoStand();
	void				OnStand();
	void				DoRevive();
	void				OnRevive();
	void				DoWait();
	void				OnWait();
	void				DoWalk();
	void				OnWalk();
	void				DoRun();
	void				OnRun();
	void				DoSkill(int nX, int nY);
	int					DoOrdinSkill(KSkill * pSkill, int nX, int nY);
	void				OnSkill();
	void				DoAttack();
	void				DoMagic();
	void				DoJump();
	BOOL				OnJump();
	void				DoSit();
	void				OnSit();
	void				DoHurt(int nHurtFrames = 0, int nX = 0, int nY =0);
	void				OnHurt();

	// mode == 0 npc 导致 == 1 player 导致，不掉东西 == 2 player 导致，掉东西
	// 与 DeathPunish 的参数对应 具体参阅 enumDEATH_MODE
	void				DoDeath(int nMode = 0);

	void				OnDeath();
	void				DoDefense();
	void				OnDefense();
	void				DoIdle();
	void				OnIdle();
	
//	有关格斗技能的------------------------------------------


	BOOL				DoManyAttack();
	void				OnManyAttack();
	BOOL				DoBlurAttack();

	BOOL				DoJumpAttack();
	BOOL				OnJumpAttack();

	BOOL				DoRunAttack();
	void				OnRunAttack();
	BOOL				CastMeleeSkill(KSkill * pSkill);
//-----------------------------------------------------------

	
	void				OnPlayerTalk();

	void				DoSpecial1();
	void				OnSpecial1();
	void				DoSpecial2();
	void				OnSpecial2();
	void				DoSpecial3();
	void				OnSpecial3();
	void				DoSpecial4();
	void				OnSpecial4();
	void				Goto(int nMpsX, int nMpsY);
	void				RunTo(int nMpsX, int nMpsY);
	void				JumpTo(int nMpsX, int nMpsY);
	void				ServeMove(int nSpeed);
	void				ServeJump(int nSpeed);
//	void				Enchant(void);
//	void				ModifyEnchant(void* pData, int nAttrib);
	BOOL				NewPath(int nMpsX, int nMpsY);
	BOOL				NewJump(int nMpsX, int nMpsY);
	BOOL				CheckHitTarget(int nAR, int nDf, int nIngore = 0);
#ifdef _SERVER
	void				LoseSingleItem(int nBelongPlayer);
	void				LoseMoney(int nBelongPlayer);
	void				PlayerDeadCreateMoneyObj(int nMoneyNum);	// 玩家死的时候掉出来的钱生成一个object
	void				UpdateNpcStateInfo();		//重新更新角色的状态信息
#endif

#ifndef _SERVER
	void				HurtAutoMove();
#endif

public:
	friend class KNpcAttribModify;
	friend class KThiefSkill;
	KNpc();
	void				SetActiveFlag(BOOL bFlag) { m_bActivateFlag = bFlag; };
	void				DoPlayerTalk(char *);
	void				CheckTrap();
	void				Init();
	void				Remove();
	void				Activate();
	BOOL				IsPlayer();
	void				SetFightMode(BOOL bFightMode);
	void				TurnTo(int nIdx);
	void				SendCommand(NPCCMD cmd, int x = 0, int y = 0, int z = 0);
	void				ProcCommand(int nAI);
	ISkill* 			GetActiveSkill();
	BOOL				ProcessState();
	void				ProcStatus();
	void				ModifyAttrib(int nAttacker, void* pData);
	int					GetSkillLevel(int nSkillId);
	void				SetId(DWORD	dwID)	{ m_dwID = dwID;};
	BOOL				IsMatch(DWORD dwID)	{ return dwID == m_dwID; };	// 是否ID与该Index匹配
	BOOL				Cost(NPCATTRIB nType, int nCost, BOOL bOnlyCheckCanCast = FALSE);				// 消耗内力体力等,如果OnlyCheckCanCost为TRUE,表示只是检查当前的内力等是否够消耗，并不实际的扣
	void				SelfDamage(int nDamage);						// 自身的伤害，如牺牲攻击
	void				Load(int nNpcSettingIdx, int nLevel);						// 从TabFile中加载
	void				GetMpsPos(int * pPosX, int *pPosY);
	BOOL				SetActiveSkill(int nSkillIdx);
	void				SetAuraSkill(int nSkillID);
	void				SetCamp(int nCamp);
	void				SetCurrentCamp(int nCamp);
	void				RestoreCurrentCamp();
	void				SetStateSkillEffect(int nLauncher, int nSkillID, int nLevel, void *pData, int nDataNum, int nTime = -1);	// 主动辅助技能与被动技能
	void				ClearStateSkillEffect();
	void				ReCalcStateEffect();
	void				ClearNormalState();
	void				SetImmediatelySkillEffect(int nLauncher, void *pData, int nDataNum);
	void				AppendSkillEffect(BOOL bIsPhysical, BOOL bIsMelee, void *pSrcData, void *pDesData);
	int					ModifyMissleLifeTime(int nLifeTime);
	int					ModifyMissleSpeed(int nSpeed);
	BOOL				ModifyMissleCollsion(BOOL bCollsion);
	void				RestoreNpcBaseInfo(); //Set Current_Data ;
	void				RestoreState();
	void				ClearNpcState();
	BOOL				SetPlayerIdx(int nIdx);
	void				DialogNpc(int nIndex);
	void				Revive();
	void				AddBaseLifeMax(int nLife);	// 增加基本最大生命点
	void				AddCurLifeMax(int nLife);
	void				AddBaseStaminaMax(int nStamina);// 增加基本最大体力点
	void				AddCurStaminaMax(int nStamina);
	void				AddBaseManaMax(int nMana);	// 增加基本最大内力点
	void				AddCurManaMax(int nMana);
//	void				ResetLifeReplenish();		// 重新计算生命回复速度
//	void				CalcCurLifeMax();			// 计算当前最大生命点
//	void				CalcCurStaminaMax();		// 计算当前最大体力点
//	void				CalcCurManaMax();			// 计算当前最大内力点
	void				CalcCurLifeReplenish();		// 计算当前生命回复速度
	void				SetSeries(int nSeries);// 设定此 npc 的五行属性（内容还没完成）
//	int					GetNpcLevelDataFromScript(KLuaScript * pScript, char * szDataName, int nLevel, char * szParam);
//	int					SkillString2Id(char * szSkillString);
	void				GetNpcCopyFromTemplate(int nNpcTemplateId, int nLevel);
//	void				InitNpcLevelData(KTabFile * pTabFile, int nNpcTemplateId, KLuaScript * pLevelScript, int nLevel);
//	void				InitNpcBaseData(int nNpcTemplateId);
	void				SetPhysicsDamage(int nMinDamage, int nMaxDamage);	// 设定物理攻击的最大最小值
	void				SetBaseAttackRating(int nAttackRating);					// 设定攻击命中率
	void				SetBaseDefence(int nDefence);							// 设定防御力
//	void				SetBaseWalkSpeed(int nSpeed);							// 设定行走速度
//	void				SetBaseRunSpeed(int nSpeed);							// 设定跑步速度
	int					GetCurActiveWeaponSkill();
	void				LoadDataFromTemplate(int nNpcTemplateId, int nLevel);

#ifdef	_SERVER
	void				ExecuteRevive(){DoRevive();};
	BOOL				SendSyncData(int nClient);						// 向一个客户端发完整同步数据
	void				NormalSync();									// 广播小同步
	void				BroadCastState();								// 广播角色的状态信息,光环，头带等等效果
	void				BroadCastRevive(int nType);
	int					GetPlayerIdx();
	void				CalcDamage(int nAttacker, int nMin, int nMax, DAMAGE_TYPE nType, BOOL bIsMelee, BOOL bDoHurt = TRUE, BOOL bReturn = FALSE);
	BOOL				ReceiveDamage(int nLauncher, BOOL bIsMelee, void *pData, BOOL bUseAR, BOOL bDoHurt);

	// mode == 0 npc 导致 == 1 player 导致，不掉东西 == 2 player 导致，掉东西
	// 与 DoDeath 的参数对应 具体参阅 enumDEATH_MODE
	void				DeathPunish(int nMode, int nBelongPlayer);

	void				RestoreLiveData();								// 重生后恢复Npc的基本数据
	int					SetPos(int nX, int nY);
	int					ChangeWorld(DWORD dwSubWorldID, int nX, int nY);	// 切换世界
	void				TobeExchangeServer(DWORD dwMapID, int nX, int nY);
	void				RestoreLife(){m_CurrentLife = m_CurrentLifeMax;	};
	void				RestoreMana(){m_CurrentMana = m_CurrentManaMax;};
	void				RestoreStamina(){m_CurrentStamina = m_CurrentStaminaMax; };
	void				SendDataToNearRegion(void* pBuffer, DWORD dwSize);// 向周围九屏广播
	int					DeathCalcPKValue(int nKiller);					// 死亡时候计算PK值

	BOOL				CheckPlayerAround(int nPlayerIdx);// 查找周围9个Region中是否有指定的 player
#endif

#ifndef _SERVER
	void				SetSleepMode(BOOL bSleep) { m_nSleepFlag = bSleep; m_DataRes.SetSleepState(bSleep);};
	void				SetNpcState(BYTE* pNpcState);
	void				RemoveRes();
	void				ProcNetCommand(NPCCMD cmd, int x = 0, int y = 0, int z = 0);
	void				Paint();
	int					PaintInfo(int nHeightOffset, bool bSelect, int nFontSize = 12, DWORD	dwBorderColor = 0);
	int					PaintChat(int nHeightOffset);
	int					SetChatInfo(const char* Name, const char* pMsgBuff, unsigned short nMsgLength);
	int					PaintLife(int nHeightOffset, bool bSelect);
	int					PaintMana(int nHeightOffset);
	void				DrawBorder();
	int					DrawMenuState(int n);
	void				DrawBlood();	//绘制血条和名字在固定位置
	BOOL				IsCanInput() { return m_ProcessAI; };
	void				SetMenuState(int nState, char *lpszSentence = NULL, int nLength = 0);	// 设定头顶状态
	int					GetMenuState();				// 获得头顶状态
	DWORD				SearchAroundID(DWORD dwID);	// 查找周围9个Region中是否有指定 ID 的 npc
	void				SetSpecialSpr(char *lpszSprName);// 设定特殊的只播放一遍的随身spr文件
	void				SetInstantSpr(int nNo);
	int					GetNormalNpcStandDir(int nFrame);
	KNpcRes*			GetNpcRes(){return &m_DataRes;};

	int GetNpcPate();
	int GetNpcPatePeopleInfo();

	// 冒血处理
	void				ClearBlood();
	void				SetBlood(int nNo);
	int				PaintBlood(int nHeightOffset);	//绘制冒血
#endif
	
};
#ifndef TOOLVERSION
extern KNpc Npc[MAX_NPC];
#else
extern CORE_API KNpc Npc[MAX_NPC];
#endif

#endif

