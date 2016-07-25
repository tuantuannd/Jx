#ifndef KPROTOCOL_H
#define KPROTOCOL_H

#ifndef __linux
#ifdef _STANDALONE
#include "GameDataDef.h"
#else
#include "../sources/Core/src/GameDataDef.h"
#endif
#else
#include "GameDataDef.h"
#include <string.h>
#endif

#include "KProtocolDef.h"
#include "KRelayProtocol.h"

#pragma pack(push, enter_protocol)
#pragma	pack(1)

#define	PROTOCOL_MSG_TYPE	BYTE
#define PROTOCOL_MSG_SIZE	(sizeof(PROTOCOL_MSG_TYPE))
#define	MAX_PROTOCOL_NUM	200

typedef struct
{
	BYTE	ProtocolType;
	BYTE	WalkSpeed;
	BYTE	RunSpeed;
	BYTE	AttackSpeed;
	BYTE	CastSpeed;
	BYTE	HelmType;
	BYTE	ArmorType;
	BYTE	WeaponType;
	BYTE	HorseType;
	BYTE	RankID;
	DWORD	ID;
	BYTE	m_btSomeFlag;	// 0x01 PKFlag 0x02 FightModeFlag 0x04 SleepModeFlag
//	BYTE	m_btPKFlag;
//	BYTE	m_btFightMode;
//	BYTE	m_SleepMode;
//	char	Name[32];	
} PLAYER_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	BYTE	WalkSpeed;			// 是否只要传一份就够了（只改变一个或是同时改变）
	BYTE	RunSpeed;
	BYTE	AttackSpeed;
	BYTE	CastSpeed;			// 是否考虑不改变施法速度，或施法速度等于攻击速度
	BYTE	HelmType;
	BYTE	ArmorType;
	BYTE	WeaponType;
	BYTE	HorseType;
	BYTE	RankID;
	BYTE	m_btSomeFlag;	// 0x01 PKFlag 0x02 FightModeFlag 0x04 SleepModeFlag 0x08 TongOpenFlag
//	BYTE	FightMode;
//	BYTE	m_btPKFlag;
//	BYTE	m_btSleepMode;
} PLAYER_NORMAL_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	Camp;				// 阵营
	BYTE	CurrentCamp;		// 当前阵营
	BYTE	m_bySeries;			// 五行系
	BYTE	LifePerCent;		// 生命百分比
	BYTE	m_btMenuState;		// 组队、交易等状态
	BYTE	m_Doing;			// 行为
	BYTE	m_btKind;			// npc类型
	DWORD	MapX;				// 位置信息
	DWORD	MapY;				// 位置信息
	DWORD	ID;					// Npc的唯一ID
	int		NpcSettingIdx;		// 客户端用于加载玩家资源及基础数值设定
	WORD	NpcEnchant;			// 加强的NPC（后面的bit表示加强类型，名字是否金色之类的由加强的数目在客户端确定）
	char	m_szName[32];		// 名字
} NPC_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	DWORD	MapX;
	DWORD	MapY;
	BYTE	Camp;
	BYTE	LifePerCent;
	BYTE	Doing;
	BYTE	State;
} NPC_NORMAL_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	//int		m_nRegionID;
	DWORD	m_dwNpcID;
	DWORD	m_dwMapX;
	DWORD	m_dwMapY;
	WORD	m_wOffX;
	WORD	m_wOffY;
	BYTE	m_byDoing;
	BYTE	m_btCamp;
	BYTE	m_btLifePerCent;	
} NPC_PLAYER_TYPE_NORMAL_SYNC;
                 
typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
} NPC_REMOVE_SYNC, NPC_SIT_SYNC, NPC_DEATH_SYNC, NPC_REQUEST_COMMAND, NPC_REQUEST_FAIL;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	int		nMpsX;
	int		nMpsY;
} NPC_WALK_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	BYTE	Type;
} NPC_REVIVE_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	int		nMpsX;
	int		nMpsY;
} NPC_JUMP_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	int		nMpsX;
	int		nMpsY;
} NPC_RUN_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	int		nFrames;
	int		nX;
	int		nY;
} NPC_HURT_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	BYTE	Camp;
} NPC_CHGCURCAMP_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	BYTE	Camp;
} NPC_CHGCAMP_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	ID;
	int		nSkillID;
	int		nSkillLevel;
	int		nMpsX;
	int		nMpsY;
} NPC_SKILL_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	int		nSkillID;
	int		nMpsX;
	int		nMpsY;
} NPC_SKILL_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	int		nMpsX;
	int		nMpsY;
} NPC_WALK_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
//	BYTE	ReviveType;
} NPC_REVIVE_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	int		nMpsX;
	int		nMpsY;
} NPC_RUN_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	int		nShopIndex;
} SALE_BOX_SYNC;

typedef struct 
{
	BYTE	ProtocolType;
	int		nNpcId;
} PLAYER_DIALOG_NPC_COMMAND; //主角与nNpcId对话的请求

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	int		m_nExp;				// 当前经验
} PLAYER_EXP_SYNC;				// 玩家同步经验

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
} PLAYER_APPLY_CREATE_TEAM;		// 客户端玩家创建队伍，向服务器发请求

struct PLAYER_SEND_CREATE_TEAM_SUCCESS
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	nTeamServerID;		// 队伍在服务器上的唯一标识
	PLAYER_SEND_CREATE_TEAM_SUCCESS() {nTeamServerID = -1;}
};	// 服务器通知玩家队伍创建成功

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btErrorID;		// 队伍创建不成功原因：0 同名 1 玩家本身已经属于某一支队伍 3 当前处于不能组队状态
} PLAYER_SEND_CREATE_TEAM_FALSE;// 服务器通知客户端队伍创建不成功

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwTarNpcID;		// 查询目标 npc id
} PLAYER_APPLY_TEAM_INFO;		// 客户端向服务器申请查询某个npc的组队情况

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
} PLAYER_APPLY_TEAM_INFO_FALSE;	// 服务器告知客户端申请查询某个npc的组队情况失败

typedef struct PLAYER_SEND_TEAM_INFO_DATA
{
	BYTE	ProtocolType;		// 协议名称
	int		m_nCaptain;			// 队长 npc id
	int		m_nMember[MAX_TEAM_MEMBER];	// 所有队员 npc id
	DWORD	nTeamServerID;		// 队伍在服务器上的唯一标识
	PLAYER_SEND_TEAM_INFO_DATA() {nTeamServerID = -1;};
} PLAYER_SEND_TEAM_INFO;		// 服务器向客户端发送某个队伍的信息数据

typedef struct PLAYER_SEND_SELF_TEAM_INFO_DATA
{
	BYTE	ProtocolType;							// 协议名称
	BYTE	m_btState;								// 队伍状态
	DWORD	m_dwNpcID[MAX_TEAM_MEMBER + 1];			// 每名成员的npc id （队长放在第一位）
	char	m_szNpcName[MAX_TEAM_MEMBER + 1][32];	// 每名成员的名字（队长放在第一位）
	DWORD	nTeamServerID;							// 队伍在服务器上的唯一标识
	DWORD	m_dwLeadExp;							// 玩家的统率力经验
	BYTE	m_btLevel[MAX_TEAM_MEMBER + 1];			// 每名成员的等级（队长放在第一位）
	PLAYER_SEND_SELF_TEAM_INFO_DATA() {memset(m_szNpcName, 0, 32 * (MAX_TEAM_MEMBER + 1)); nTeamServerID = -1;};
} PLAYER_SEND_SELF_TEAM_INFO;						// 服务器向客户端发送客户端自身队伍的信息数据

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btOpenClose;		// 打开或关闭
} PLAYER_TEAM_OPEN_CLOSE;		// 队伍队长向服务器申请开放、关闭队伍是否允许接收成员状态

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwTarNpcID;		// 目标队伍队长npc id 或者 申请人 npc id
} PLAYER_APPLY_ADD_TEAM;		// 玩家向服务器申请加入某个队伍或者服务器向某个队长转发某个玩家的加入申请

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwNpcID;			// 被接受入队伍的npc id
} PLAYER_ACCEPT_TEAM_MEMBER;	// 玩家通知服务器接受某个玩家入队伍

typedef struct PLAYER_TEAM_ADD_MEMBER_DATA
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btLevel;			// 加入者等级
	DWORD	m_dwNpcID;			// 加入者npc id
	char	m_szName[32];		// 加入者姓名
	PLAYER_TEAM_ADD_MEMBER_DATA() {memset(m_szName, 0, 32);};
} PLAYER_TEAM_ADD_MEMBER;		// 服务器通知队伍中的各个玩家有新成员加入

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
} PLAYER_APPLY_LEAVE_TEAM;		// 客户端玩家申请离队

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwNpcID;			// 离队npc id
} PLAYER_LEAVE_TEAM;			// 服务器通知各队员某人离队

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwNpcID;			// 离队npc id
} PLAYER_TEAM_KICK_MEMBER;		// 队长踢除某个队员

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwNpcID;			// 目标npc id
} PLAYER_APPLY_TEAM_CHANGE_CAPTAIN;// 队长向服务器申请把自己的队长身份交给别的队员

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwCaptainID;		// 新队长npc id
	DWORD	m_dwMemberID;		// 新队员npc id
} PLAYER_TEAM_CHANGE_CAPTAIN;	// 服务器通知各队员更换队长

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
} PLAYER_APPLY_TEAM_DISMISS;	// 向服务器申请解散队伍

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btPKFlag;			// pk 开关
} PLAYER_SET_PK;				// 向服务器申请打开、关闭PK

typedef struct
{
	BYTE	ProtocolType;			// 协议名称
	BYTE	m_btCamp;				// 新阵营
	BYTE	m_btCurFaction;			// 当前门派
	BYTE	m_btFirstFaction;		// 首次加入门派
	int		m_nAddTimes;			// 加入门派次数
} PLAYER_FACTION_DATA;				// 服务器发给客户端门派信息

typedef struct
{
	BYTE	ProtocolType;			// 协议名称
} PLAYER_LEAVE_FACTION;				// 服务器通知玩家离开门派

typedef struct
{
	BYTE	ProtocolType;			// 协议名称
	BYTE	m_btCurFactionID;		// 当前门派id
	BYTE	m_btLevel;				// 技能开放等级
} PLAYER_FACTION_SKILL_LEVEL;		// 服务器通知玩家开放当前门派技能到某个等级

typedef struct
{
	BYTE	ProtocolType;			// 协议名称
} PLAYER_APPLY_FACTION_DATA;		// 客户端申请更新门派数据

typedef struct PLAYER_SEND_CHAT_DATA_COMMAND
{
	BYTE	ProtocolType;		// 协议名称
	WORD	m_wLength;
	BYTE	m_btCurChannel;		// 当前聊天频道
	BYTE	m_btType;			// MSG_G_CHAT 或 MSG_G_CMD 或……
	BYTE	m_btChatPrefixLen;	// 格式控制字符长度
	WORD	m_wSentenceLen;		// 聊天语句长度
	DWORD	m_dwTargetID;		// 聊天对象 id
	int		m_nTargetIdx;		// 聊天对象在服务器端的 idx
	char	m_szSentence[MAX_SENTENCE_LENGTH + CHAT_MSG_PREFIX_MAX_LEN];	// 聊天语句内容
	PLAYER_SEND_CHAT_DATA_COMMAND() {memset(m_szSentence, 0, sizeof(m_szSentence));};
} PLAYER_SEND_CHAT_COMMAND;		// 客户端聊天内容发送给服务器

typedef struct PLAYER_SEND_CHAT_DATA_SYNC
{
	BYTE	ProtocolType;		// 协议名称
	WORD	m_wLength;
	BYTE	m_btCurChannel;		// 当前聊天状态
	BYTE	m_btNameLen;		// 名字长度
	BYTE	m_btChatPrefixLen;	// 控制字符长度
	WORD	m_wSentenceLen;		// 聊天语句长度
	DWORD	m_dwSourceID;		// 
	char	m_szSentence[32 + CHAT_MSG_PREFIX_MAX_LEN + MAX_SENTENCE_LENGTH];	// 聊天语句内容
	PLAYER_SEND_CHAT_DATA_SYNC() { memset(m_szSentence, 0, sizeof(m_szSentence)); };
} PLAYER_SEND_CHAT_SYNC;		// 客户端聊天内容发送给服务器

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btState;
	int		m_nID;
	int		m_nDataID;
	int		m_nXpos;
	int		m_nYpos;
	int		m_nMoneyNum;
	int		m_nItemID;
	BYTE	m_btDir;
	BYTE	m_btItemWidth;
	WORD	m_wCurFrame;
	BYTE	m_btItemHeight;
	BYTE	m_btColorID;
	BYTE	m_btFlag;
	char	m_szName[32];
} OBJ_ADD_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	BYTE	m_btState;
	int		m_nID;
} OBJ_SYNC_STATE;

typedef struct
{
	BYTE	ProtocolType;
	BYTE	m_btDir;
	int		m_nID;
} OBJ_SYNC_DIR;

typedef struct
{
	BYTE	ProtocolType;
	int		m_nID;
	BYTE	m_btSoundFlag;
} OBJ_SYNC_REMOVE;

typedef struct
{
	BYTE	ProtocolType;
	int		m_nID;
	int		m_nTarX;
	int		m_nTarY;
} OBJ_SYNC_TRAP_ACT;

typedef struct
{
	BYTE	ProtocolType;
	int		m_nID;
} OBJ_CLIENT_SYNC_ADD;

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	DWORD	m_dwLeadExp;		// 统率力经验值
} PLAYER_LEAD_EXP_SYNC;			// 同步统率力经验值

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btLevel;			// 当前等级
	int		m_nExp;				// 当前经验
	int		m_nAttributePoint;	// 剩余属性点
	int		m_nSkillPoint;		// 剩余技能点
	int		m_nBaseLifeMax;		// 当前最大生命值
	int		m_nBaseStaminaMax;	// 当前最大体力值
	int		m_nBaseManaMax;		// 当前最大内力值
} PLAYER_LEVEL_UP_SYNC;			// 玩家升级

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btLevel;			// 当前等级
	DWORD	m_dwTeammateID;		// 队友 npc id
} PLAYER_TEAMMATE_LEVEL_SYNC;	// 玩家升级的时候通知队友

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btAttribute;		// 属性(0=Strength 1=Dexterity 2=Vitality 3=Engergy)
	int		m_nAddNo;			// 加的点数
} PLAYER_ADD_BASE_ATTRIBUTE_COMMAND;	// 玩家添加基本属性点

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	int		m_nSkillID;			// 技能id
	int		m_nAddPoint;		// 要加的点数
} PLAYER_ADD_SKILL_POINT_COMMAND;// 玩家申请增加某个技能的点数

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btAttribute;		// 属性(0=Strength 1=Dexterity 2=Vitality 3=Engergy)
	int		m_nBasePoint;		// 基本点数
	int		m_nCurPoint;		// 当前点数
	int		m_nLeavePoint;		// 剩余未分配属性点
} PLAYER_ATTRIBUTE_SYNC;		// 玩家同步属性点

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	int		m_nSkillID;			// 技能id
	int		m_nSkillLevel;		// 技能等级
	int		m_nLeavePoint;		// 剩余未分配技能点
} PLAYER_SKILL_LEVEL_SYNC;		// 玩家同步技能点

//typedef struct
//{
//	BYTE	ProtocolType;		// 协议名称
//	int		m_nItemID;			// 物品id
//	int		m_nSourcePos;		// 来源位置
//	int		m_nTargetPos;		// 目的位置
//} PLAYER_EQUIP_ITEM_COMMAND;	// 玩家鼠标右键点击使用物品(装备)

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	BYTE	m_btPlace;			// 药品位置
	BYTE	m_btX;				// 药品位置
	BYTE	m_btY;				// 药品位置
	int		m_nItemID;			// 物品id
} PLAYER_EAT_ITEM_COMMAND;		// 玩家鼠标右键点击使用物品(吃药)

typedef struct
{
	BYTE	ProtocolType;		// 协议名称
	int		m_nObjID;			// 鼠标点击的obj的id
	BYTE	m_btPosType;		// 位置类型
	BYTE	m_btPosX;			// 坐标 x
	BYTE	m_btPosY;			// 坐标 y
} PLAYER_PICKUP_ITEM_COMMAND;		// 玩家获得物品（鼠标点击地图上的obj）

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_ID;				// 物品的ID
	BYTE			m_Genre;			// 物品的类型
	BYTE			m_Detail;			// 物品的类别
	BYTE			m_Particur;			// 物品的详细类别
	BYTE			m_Series;			// 物品的五行
	BYTE			m_Level;			// 物品的等级
	BYTE			m_btPlace;			// 坐标
	BYTE			m_btX;				// 坐标
	BYTE			m_btY;				// 坐标
	BYTE			m_Luck;				// MF
	BYTE			m_MagicLevel[6];	// 生成参数
	WORD			m_Version;			// 装备版本
	WORD			m_Durability;		// 耐久度
	UINT			m_RandomSeed;		// 随机种子
} ITEM_SYNC;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_ID;				// 物品的ID
} ITEM_REMOVE_SYNC;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_ID;				// 物品的ID
} PLAYER_SELL_ITEM_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_BuyIdx;			// 买第几个东西
	BYTE			m_Place;			// 放在身上哪个地方
	BYTE			m_X;				// 坐标X
	BYTE			m_Y;				// 坐标Y
} PLAYER_BUY_ITEM_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nMoney1;			// 装备栏
	int				m_nMoney2;			// 贮物箱
	int				m_nMoney3;			// 交易栏
} PLAYER_MONEY_SYNC;					// 服务器通知客户端钱的数量

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btDownPos;
	BYTE			m_btDownX;
	BYTE			m_btDownY;
	BYTE			m_btUpPos;
	BYTE			m_btUpX;
	BYTE			m_btUpY;
} PLAYER_MOVE_ITEM_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btDownPos;
	BYTE			m_btDownX;
	BYTE			m_btDownY;
	BYTE			m_btUpPos;
	BYTE			m_btUpX;
	BYTE			m_btUpY;
} PLAYER_MOVE_ITEM_SYNC;

// s2c_ItemAutoMove
typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btSrcPos;
	BYTE			m_btSrcX;
	BYTE			m_btSrcY;
	BYTE			m_btDestPos;
	BYTE			m_btDestX;
	BYTE			m_btDestY;
} ITEM_AUTO_MOVE_SYNC;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
} PLAYER_THROW_AWAY_ITEM_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	short			m_shLife;
	short			m_shStamina;
	short			m_shMana;
	short			m_shAngry;
	BYTE			m_btTeamData;
} CURPLAYER_NORMAL_SYNC;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	// npc部分
	DWORD			m_dwID;				// Npc的ID
	BYTE			m_btLevel;			// Npc的等级
	BYTE			m_btSex;			// 性别
	BYTE			m_btKind;			// Npc的类型
	BYTE			m_btSeries;			// Npc的五行系
	WORD			m_wLifeMax;			// Npc的最大生命
	WORD			m_wStaminaMax;		// Npc的最大体力
	WORD			m_wManaMax;			// Npc的最大内力
	int				m_HeadImage;
	// player 部分
	WORD			m_wAttributePoint;	// 未分配属性点
	WORD			m_wSkillPoint;		// 未分配技能点
	WORD			m_wStrength;		// 玩家的基本力量（决定基本伤害）
	WORD			m_wDexterity;		// 玩家的基本敏捷（决定命中、体力）
	WORD			m_wVitality;		// 玩家的基本活力（决定生命、体力）
	WORD			m_wEngergy;			// 玩家的基本精力（决定内力）
	WORD			m_wLucky;			// 玩家的基本幸运值
	int				m_nExp;				// 当前经验值(当前等级在npc身上)
	DWORD			m_dwLeadExp;		// 统率力经验值

	// 门派
	BYTE			m_btCurFaction;		// 当前门派
	BYTE			m_btFirstFaction;	// 第一次加入的是哪个门派
	int				m_nFactionAddTimes;	// 加入各种门派的总次数
	
	// 排名
	WORD			m_wWorldStat;		// 世界排名
	WORD			m_wSectStat;		// 门派排名

	// 钱
	int				m_nMoney1;
	int				m_nMoney2;
} CURPLAYER_SYNC;

#define MAX_SCIRPTACTION_BUFFERNUM 300

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wProtocolLong;
	BYTE	m_nOperateType;				//操作类型
	BYTE	m_bUIId, m_bOptionNum, m_bParam1, m_bParam2;// m_bParam1,主信息是数字标识还是字符串标识, m_bParam2,是否是与服务器交互的选择界面
	int		m_nParam;
	int		m_nBufferLen;
	char	m_pContent[MAX_SCIRPTACTION_BUFFERNUM];				//带控制符
} PLAYER_SCRIPTACTION_SYNC;

typedef struct
{
	WORD	SkillId;
	BYTE	SkillLevel;
} SKILL_SEND_ALL_SYNC_DATA;

typedef struct
{
	BYTE						ProtocolType;
	WORD						m_wProtocolLong;
	SKILL_SEND_ALL_SYNC_DATA	m_sAllSkill[80];
} SKILL_SEND_ALL_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	BYTE	WeatherID;
} SYNC_WEATHER;

typedef struct defWORLD_SYNC
{
	BYTE	ProtocolType;
	int		SubWorld;
	int		Region;
	BYTE	Weather;
	DWORD	Frame;
} WORLD_SYNC;

typedef struct 
{
	BYTE	ProtocolType;
	int		nSelectIndex;
}PLAYER_SELECTUI_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwTakeChannel;	// 订阅频道
} CHAT_SET_CHANNEL_COMMAND;				// 设定订阅频道

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;
	DWORD			m_dwTargetNpcID;	// 目标 npc id
	char			m_szInfo[MAX_SENTENCE_LENGTH];// 给对方的话
} CHAT_APPLY_ADD_FRIEND_COMMAND;		// 聊天添加好友

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nSrcPlayerIdx;	// 来源 player idx
	char			m_szSourceName[32];	// 来源玩家名字
	char			m_szInfo[MAX_SENTENCE_LENGTH];// 对方给的话
} CHAT_APPLY_ADD_FRIEND_SYNC;			// 聊天添加好友

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nTargetPlayerIdx;	// 被接受player idx
} CHAT_ADD_FRIEND_COMMAND;				// 添加某玩家为聊天好友

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nTargetPlayerIdx;	// 被拒绝player idx
} CHAT_REFUSE_FRIEND_COMMAND;			// 拒绝添加某玩家为聊天好友

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 新添加好友的 id
	int				m_nIdx;				// 新添加好友在 player 数组中的位置
	char			m_szName[32];		// 新添加好友的名字
} CHAT_ADD_FRIEND_SYNC;					// 通知客户端成功添加一个聊天好友

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;
	char			m_szName[32];		// 拒绝者名字
} CHAT_REFUSE_FRIEND_SYNC;				// 通知客户端添加聊天好友的申请被拒绝

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nTargetPlayerIdx;	// 出错 player idx (一般可能是此player下线或者换服务器了)
} CHAT_ADD_FRIEND_FAIL_SYNC;			// 通知客户端添加聊天好友失败

typedef struct
{
	BYTE			ProtocolType;	// c2s_viewequip
	DWORD			m_dwNpcID;
} VIEW_EQUIP_COMMAND;

//此结构已经被tagDBSelPlayer结构替换
/*typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nSelect;			// 
} DB_PLAYERSELECT_COMMAND;*/

/*
 * { Add by liupeng 2003.05.10
 *
 * #pragma pack( push, 1 )
*/

/*
 * Nonstandard extension used : zero-sized array in struct/union
 */
#pragma warning(disable: 4200)

#define KSG_PASSWORD_MAX_SIZE   64

typedef struct tagKSG_PASSWORD
{
    char szPassword[KSG_PASSWORD_MAX_SIZE];    // 现在采用MD5的字符串，由于是32个字符，加上末尾'\0'，需要至少33个空间，因此使用64
} KSG_PASSWORD;

#define _NAME_LEN	32

struct tagProtoHeader
{
	BYTE	cProtocol;
};

struct tagResult : public tagProtoHeader
{
	BYTE	cResult;
};

struct tagDBSelPlayer : public tagProtoHeader
{
	char	szRoleName[_NAME_LEN];
};

struct tagDBDelPlayer : public tagProtoHeader
{
	char	        szAccountName[_NAME_LEN];
    KSG_PASSWORD    Password;
	char	        szRoleName[_NAME_LEN];
	char szPassword[KSG_PASSWORD_MAX_SIZE];
};

//删除与新建角色的返回消息带的数据
struct tagNewDelRoleResponse : public tagDBSelPlayer
{
	bool	bSucceeded;		//是否成功
};

// 2003.05.11
struct tagDBSyncPlayerInfo : public tagProtoHeader
{
	size_t	dataLength;
	char	szData[0];
};

/*
 * 2003.06.27
 * s2c_gateway_broadcast
*/

#define	AP_WARNING_ALL_PLAYER_QUIT			1
#define	AP_NOTIFY_GAMESERVER_SAFECLOSE		2
#define	AP_NOTIFY_ALL_PLAYER				3
#define	MAX_GATEWAYBROADCAST_LEN	260
struct tagGatewayBroadCast : public tagProtoHeader
{
	UINT	uCmdType;
	char	szData[MAX_GATEWAYBROADCAST_LEN];
};

/*
 * 2003.05.22
 * s2c_syncgamesvr_roleinfo_cipher
*/
struct tagGuidableInfo : public tagProtoHeader
{
	GUID guid;
	WORD nExtPoint;			//可用的附送点
	WORD nChangePoint;		//变化的附送点
	size_t	datalength;
	char	szData[0];

};

/*
 * c2s_permitplayerlogin
 */
struct tagPermitPlayerLogin : public tagProtoHeader
{
	GUID guid;

	BYTE szRoleName[_NAME_LEN];

	/*
	 * Succeeded : true
	 * Failed	 : false
	 */
	bool bPermit;
};

struct tagPermitPlayerExchange
{
	BYTE cProtocol;
	GUID guid;
	DWORD dwIp;
	WORD wPort;
	bool bPermit;
};
/*
 * c2s_notifyplayerlogin
 */
struct tagNotifyPlayerLogin : public tagPermitPlayerLogin
{
	UINT			nIPAddr;
	unsigned short	nPort;
};

/*
 * s2c_querymapinfo
 */
struct tagQueryMapInfo : public tagProtoHeader
{	
};

/*
 * s2c_querygameserverinfo
 */
struct tagQueryGameSvrInfo : public tagProtoHeader
{	
};

/*
 * s2c_notifysvrip
 */
struct tagNotifySvrIp : public tagProtoHeader
{
	WORD	pckgID;

	BYTE	cIPType;
	DWORD	dwMapID;

	DWORD	dwSvrIP;
};

/*
 * s2c_notifyplayerexchange
 */
struct tagNotifyPlayerExchange : public tagProtoHeader
{
	GUID			guid;
	UINT			nIPAddr;
	unsigned short	nPort;
};

/*
 * c2s_requestsvrip
 */

/*
 * BYTE	cIPType
 */
#define INTRANER_IP	0
#define INTERNET_IP 1

struct tagRequestSvrIp : public tagProtoHeader
{
	WORD	pckgID;

	BYTE	cIPType;
	DWORD	dwMapID;
};

/*
 * c2c_notifyexchange
 */
struct tagSearchWay : public tagProtoHeader
{
	int		lnID;
	int		nIndex;
	DWORD	dwPlayerID;
};

/*
 * c2s_updatemapinfo
 */
struct tagUpdateMapID : public tagProtoHeader
{
	/*
	 * For example : Are your clear older information when it 
	 *		update local informatin
	 */
	BYTE cReserve;

	BYTE cMapCount;

	BYTE szMapID[0];	// C4200 warning
};

/*
 * c2s_updategameserverinfo
 */
struct tagGameSvrInfo : public tagProtoHeader
{
	UINT			nIPAddr_Intraner;
	UINT			nIPAddr_Internet;

	unsigned short	nPort;
	WORD			wCapability;
};

/*
 * s2c_identitymapping
 */
struct tagIdentityMapping : public tagGameSvrInfo
{
	GUID guid;
};

/*
 * c2s_logiclogin
 * s2c_gmgateway2relaysvr
 * s2c_gmnotify
 */
struct tagLogicLogin : public tagProtoHeader
{
	GUID guid;
};

/*
 * s2c_logiclogout
 */
struct tagLogicLogout : public tagProtoHeader
{
	BYTE szRoleName[_NAME_LEN];
};

/*
 * c2s_registeraccount
 */
struct tagRegisterAccount : public tagProtoHeader
{
	BYTE szAccountName[_NAME_LEN];
};

/*
 * c2s_entergame
 */
struct tagEnterGame : public tagProtoHeader
{
	/*
	 * Succeeded : content is account name
	 * Failed	 : content is null
	 */

	BYTE szAccountName[_NAME_LEN];
};

struct tagEnterGame2 : public EXTEND_HEADER
{
	char szAccountName[_NAME_LEN];
	char szCharacterName[_NAME_LEN];
	DWORD	dwNameID;
	unsigned long	lnID;
};

/*
 * c2s_leavegame
 */

/*
 * BYTE cCmdType
 */
#define NORMAL_LEAVEGAME	0x0		// lock account
#define HOLDACC_LEAVEGAME	0x1A	// clear resource but don't to unlock account 

struct tagLeaveGame : public tagProtoHeader
{
	BYTE cCmdType;
	WORD nExtPoint;        //将要扣除的附送点
	/*
	 * Succeeded : content is account name
	 * Failed	 : content is null
	 */

	char szAccountName[_NAME_LEN];
};

struct tagLeaveGame2 : public EXTEND_HEADER
{
	BYTE cCmdType;
	char szAccountName[_NAME_LEN];
};
/*
*  c2s_registerfamily
*/
struct tagRegisterFamily : public tagProtoHeader
{
	BYTE bRegister;		//1 is Register, 0 is unRegister
	BYTE nFamily;
	BYTE RelayMethod;
};

/*
 * c2s_gmsvr2gateway_saverole
 */
struct tagGS2GWSaveRole : public tagProtoHeader
{
	size_t	datalength;
	BYTE	szData[0];
};

/*
 * #pragma pack( pop )
 *
 * } End of the struct define
 */

typedef struct
{
	char	szName[32];
	BYTE	Sex;
	BYTE	Series;
	BYTE	HelmType;//
	BYTE	ArmorType;//
	BYTE	WeaponType;//
	BYTE	Level;
} RoleBaseInfo/* client */, S3DBI_RoleBaseInfo /* server */;

typedef struct
{
	BYTE				ProtocolType;
	RoleBaseInfo		m_RoleList[MAX_PLAYER_IN_ACCOUNT];
} ROLE_LIST_SYNC;

//移自RoleDBManager/kroledbheader.h
//用来替换上面的ROLE_LIST_SYNC,ROLE_LIST_SYNC结构不再需要了
struct TProcessData
{
	unsigned char	nProtoId;
	size_t			nDataLen;//TRoleNetMsg时表示该Block的实际数据长度,TProcessData时表示Stream的实际数据长度
	unsigned long	ulIdentity;
	bool			bLeave;
	char			pDataBuffer[1];//实际的数据
};

struct tagRoleEnterGame
{
	BYTE			ProtocolType;
	bool			bLock;
	char			Name[_NAME_LEN];
};

//新建角色的信息结构
//注释：新建决消息c2s_newplayer，传送的参数为TProcessData结构描述的数据，其中TProcessData::pDataBuffer要扩展为NEW_PLAYER_COMMAND
struct NEW_PLAYER_COMMAND
{
	BYTE			m_btRoleNo;			// 角色编号
	BYTE			m_btSeries;			// 五行系
	unsigned short	m_NativePlaceId;	//出生地ID
	char			m_szName[32];		// 姓名
};


typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 好友 id
	int				m_nPlayerIdx;		// 好友 player index
} CHAT_LOGIN_FRIEND_NONAME_SYNC;		// 玩家登录时发送玩家聊天好友数据（不带名字）

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;
	DWORD			m_dwID;				// 好友 id
	int				m_nPlayerIdx;		// 好友 player index
	char			m_szName[32];		// 好友名字
} CHAT_LOGIN_FRIEND_NAME_SYNC;			// 玩家登录时发送玩家聊天好友数据（带名字）

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
} CHAT_APPLY_RESEND_ALL_FRIEND_NAME_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 好友 id
} CHAT_APPLY_SEND_ONE_FRIEND_NAME_COMMAND;	// 申请得到玩家某个聊天好友的完整数据

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 好友 id
	int				m_nPlayerIdx;		// 好友 player index
	char			m_szName[32];		// 好友名字
} CHAT_ONE_FRIEND_DATA_SYNC;			// 发送玩家某一个聊天好友数据（带名字）

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 好友 id
	int				m_nPlayerIdx;		// 好友 player index
} CHAT_FRIEND_ONLINE_SYNC;				// 通知客户端有好友上线

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 被删除id
} CHAT_DELETE_FRIEND_COMMAND;			// 删除某个聊天好友

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 删除id
} CHAT_DELETE_FRIEND_SYNC;				// 被某个聊天好友删除

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 被删除id
} CHAT_REDELETE_FRIEND_COMMAND;			// 删除某个聊天好友

typedef struct 
{
	BYTE			ProtocolType;
	BYTE			m_LogoutType;
} LOGOUT_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	BYTE			szAccName[32];
} LOGIN_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;				// 好友 id
} CHAT_FRIEND_OFFLINE_SYNC;				// 通知客户端有好友下线

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;
	char			m_szSentence[MAX_SENTENCE_LENGTH];
} TRADE_APPLY_OPEN_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
} TRADE_APPLY_CLOSE_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btState;			// if == 0 close if == 1 open if == 2 trading
	DWORD			m_dwNpcID;			// 如果是开始交易，对方的 npc id
} TRADE_CHANGE_STATE_SYNC;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;
	DWORD			m_dwID;
	BYTE			m_btState;
	char			m_szSentence[MAX_SENTENCE_LENGTH];
} NPC_SET_MENU_STATE_SYNC;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwID;
} TRADE_APPLY_START_COMMAND;

// 服务器转发交易申请
typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nDestIdx;			// 申请者在服务器端的player idx
	DWORD			m_dwNpcId;			// 申请者的 npc id
} TRADE_APPLY_START_SYNC;

// 接受或拒绝别人的交易申请
typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_bDecision;		// 同意 1 不同意 0
	int				m_nDestIdx;			// 交易对方在服务器端的player idx
} TRADE_REPLY_START_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nMoney;
} TRADE_MOVE_MONEY_COMMAND;		// c2s_trademovemoney

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	int				m_nMoney;
} TRADE_MONEY_SYNC;				// s2c_trademoneysync

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btDecision;		// 确定交易 1  退出交易 0  取消确定 4  锁定交易 2  取消锁定 3
} TRADE_DECISION_COMMAND;				// 交易执行或取消 c2s_tradedecision

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btDecision;		// 交易ok 1  交易取消 0  锁定 2  取消锁定 3
} TRADE_DECISION_SYNC;					// s2c_tradedecision

typedef struct
{
	BYTE			ProtocolType;		
	BYTE			m_byDir;			// 取钱的方向（0存，1取）
	DWORD			m_dwMoney;			// 钱数
} STORE_MONEY_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;			// 长度
	BYTE			m_btError;			// 错误类型	0 对方关闭了此频道，1 找不到对方
	char			m_szName[32];		// 对方名字
} CHAT_SCREENSINGLE_ERROR_SYNC;

typedef struct 
{
	BYTE			ProtocolType;		// 协议类型
	BYTE			m_btStateInfo[MAX_NPC_RECORDER_STATE];
	DWORD			m_ID;				// Npc的GID
}	NPC_SYNC_STATEINFO;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	DWORD			m_dwNpcID;
} TEAM_INVITE_ADD_COMMAND;

typedef struct
{
	BYTE			ProtocolType;		// 协议类型
	WORD			m_wLength;			// 长度
	int				m_nIdx;
	char			m_szName[32];
} TEAM_INVITE_ADD_SYNC;

typedef struct
{
	BYTE			ProtocolType;		//
	int				m_nAuraSkill;
} SKILL_CHANGEAURASKILL_COMMAND;		//更换光环技能

typedef struct
{
	BYTE			ProtocolType;
	BYTE			m_btResult;
	int				m_nIndex;
} TEAM_REPLY_INVITE_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	BYTE			m_btSelfLock;
	BYTE			m_btDestLock;
	BYTE			m_btSelfOk;
	BYTE			m_btDestOk;
} TRADE_STATE_SYNC;

typedef struct
{
	BYTE			ProtocolType;
	WORD			m_wLength;
	DWORD			m_dwSkillID;		// 技能
	int				m_nLevel;
	int				m_nTime;			// 时间
	KMagicAttrib	m_MagicAttrib[MAX_SKILL_STATE];
} STATE_EFFECT_SYNC;

typedef struct
{
	BYTE			ProtocolType;
	DWORD			m_dwTime;
} PING_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	DWORD			m_dwReplyServerTime;
	DWORD			m_dwClientTime;
} PING_CLIENTREPLY_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	BYTE			m_btSitFlag;
} NPC_SIT_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	int				nMpsX;
	int				nMpsY;
} NPC_JUMP_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	int				m_dwRegionID;
	int				m_nObjID;
} OBJ_MOUSE_CLICK_SYNC;

typedef struct tagSHOW_MSG_SYNC
{
	BYTE			ProtocolType;
	WORD			m_wLength;
	WORD			m_wMsgID;
	LPVOID			m_lpBuf;
	tagSHOW_MSG_SYNC() {m_lpBuf = NULL;};
	~tagSHOW_MSG_SYNC() {Release();}
	void	Release() {if (m_lpBuf) delete []m_lpBuf; m_lpBuf = NULL;}
} SHOW_MSG_SYNC;

typedef struct
{
	BYTE			ProtocolType;
	BYTE			m_btFlag;
} PK_APPLY_NORMAL_FLAG_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	BYTE			m_btFlag;
} PK_NORMAL_FLAG_SYNC;

typedef struct
{
	BYTE			ProtocolType;
	DWORD			m_dwNpcID;
} PK_APPLY_ENMITY_COMMAND;

typedef struct
{
	BYTE			ProtocolType;
	WORD			m_wLength;
	BYTE			m_btState;
	DWORD			m_dwNpcID;
	char			m_szName[32];
} PK_ENMITY_STATE_SYNC;

typedef struct
{
	BYTE			ProtocolType;
	WORD			m_wLength;
	BYTE			m_btState;
	DWORD			m_dwNpcID;
	char			m_szName[32];
} PK_EXERCISE_STATE_SYNC;

typedef struct
{
	BYTE			ProtocolType;
	int				m_nPKValue;
} PK_VALUE_SYNC;

typedef struct
{
	int		m_nID;				// 物品的ID
	BYTE	m_btGenre;			// 物品的类型
	BYTE	m_btDetail;			// 物品的类别
	BYTE	m_btParticur;		// 物品的详细类别
	BYTE	m_btSeries;			// 物品的五行
	BYTE	m_btLevel;			// 物品的等级
	BYTE	m_btLuck;			// MF
	BYTE	m_btMagicLevel[6];	// 生成参数
	WORD	m_wVersion;			// 装备版本
	DWORD	m_dwRandomSeed;		// 随机种子
} SViewItemInfo;

typedef struct
{
	BYTE			ProtocolType;
	DWORD			m_dwNpcID;
	SViewItemInfo	m_sInfo[itempart_num];
} VIEW_EQUIP_SYNC;				// s2c_viewequip

typedef struct//该结构是所统计的玩家的基本数据
{
	char	Name[20];
	int		nValue;
	BYTE	bySort;
}TRoleList;

// 游戏统计结构
typedef struct
{
	TRoleList MoneyStat[10];			//金钱最多排名列表（十个玩家，最多可达到100个）
	TRoleList LevelStat[10];			//级别最多排名列表（十个玩家，最多可达到100个）
	TRoleList KillerStat[10];			//杀人最多排名列表
	
	//[门派号][玩家数]，其中[0]是没有加入门派的玩家
	TRoleList MoneyStatBySect[11][10];	//各门派金钱最多排名列表
	TRoleList LevelStatBySect[11][10];	//各门派级别最多排名列表

	//[门派号]，其中[0]是没有加入门派的玩家
	int SectPlayerNum[11];				//各个门派的玩家数
	int SectMoneyMost[11];				//财富排名前一百玩家中各门派所占比例数
	int SectLevelMost[11];				//级别排名前一百玩家中各门派所占比例数
}  TGAME_STAT_DATA;

typedef struct
{
	BYTE	ProtocolType;
	BYTE	bSleep;
	DWORD	NpcID;
} NPC_SLEEP_SYNC;

//////////////
//排名相关
typedef struct
{
	BYTE		ProtocolType;
	DWORD		dwLadderID;
	TRoleList	StatData[10];
} LADDER_DATA;

typedef struct
{
	BYTE		ProtocolType;
	WORD		wSize;
	int			nCount;
	DWORD		dwLadderID[0];
} LADDER_LIST;

typedef struct
{
	BYTE		ProtocolType;
	DWORD		dwLadderID;
} LADDER_QUERY;


///////////////////
//chat 相关

typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	DWORD	packageID;
	char	someone[_NAME_LEN];
	BYTE	sentlen;
} CHAT_SOMEONECHAT_CMD, CHAT_SOMEONECHAT_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	DWORD	packageID;
	BYTE	filter;
	DWORD	channelid;
	BYTE	cost;	//0: 无限制，1: 10元/句，2: <10Lv ? 不能说 : MaxMana/2/句, 3: MaxMana/10/句,4: <20Lv ? 不能说 : MaxMana*4/5/句
	BYTE	sentlen;
} CHAT_CHANNELCHAT_CMD;

typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	DWORD	packageID;
	char	someone[_NAME_LEN];
	DWORD	channelid;
	BYTE	sentlen;
} CHAT_CHANNELCHAT_SYNC;

enum {codeSucc, codeFail, codeStore};
typedef struct
{
	BYTE	ProtocolType;
	DWORD	packageID;
	BYTE	code;
} CHAT_FEEDBACK;

typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	WORD	wChatLength;
} CHAT_EVERYONE;


typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	WORD	wChatLength;
	BYTE	byHasIdentify;
	WORD	wPlayerCount;
} CHAT_GROUPMAN;


typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	DWORD	nameid;
	unsigned long lnID;
	WORD	wChatLength;
} CHAT_SPECMAN;


enum { tgtcls_team, tgtcls_fac, tgtcls_tong, tgtcls_scrn, tgtcls_bc};
typedef struct
{
	BYTE	ProtocolType;
	WORD	wSize;
	DWORD	nFromIP;
	DWORD	nFromRelayID;
	DWORD	channelid;
	BYTE	TargetCls;
	DWORD	TargetID;
	WORD	routeDateLength;
} CHAT_RELEGATE;


///////////////////////////////////////
// tong 相关

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
} S2C_TONG_HEAD;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
} STONG_PROTOCOL_HEAD;

typedef struct
{
	int		m_nPlayerIdx;
	int		m_nCamp;
	char	m_szTongName[defTONG_NAME_MAX_LENGTH + 1];
} STONG_SERVER_TO_CORE_APPLY_CREATE;

typedef struct
{
	int		m_nPlayerIdx;
	DWORD	m_dwNpcID;
} STONG_SERVER_TO_CORE_APPLY_ADD;

typedef struct
{
	int		m_nSelfIdx;
	int		m_nTargetIdx;
	DWORD	m_dwNameID;
} STONG_SERVER_TO_CORE_CHECK_ADD_CONDITION;

typedef struct
{
	int		m_nCamp;
	int		m_nPlayerIdx;
	DWORD	m_dwPlayerNameID;
	char	m_szTongName[64];
} STONG_SERVER_TO_CORE_CREATE_SUCCESS;

typedef struct
{
	int		m_nSelfIdx;
	int		m_nTargetIdx;
	DWORD	m_dwNameID;
} STONG_SERVER_TO_CORE_REFUSE_ADD;

typedef struct
{
	int		m_nSelfIdx;
	int		m_nInfoID;
	int		m_nParam1;
	int		m_nParam2;
	int		m_nParam3;
	char	m_szName[32];
} STONG_SERVER_TO_CORE_GET_INFO;

typedef struct
{
	int		m_nCamp;
	int		m_nPlayerIdx;
	DWORD	m_dwPlayerNameID;
	char	m_szTongName[32];
	char	m_szMasterName[32];
	char	m_szTitleName[32];
} STONG_SERVER_TO_CORE_ADD_SUCCESS;

typedef struct
{
	int		m_nPlayerIdx;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szTitle[32];
	char	m_szName[32];
} STONG_SERVER_TO_CORE_BE_INSTATED;

typedef struct
{
	int		m_nPlayerIdx;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szName[32];
} STONG_SERVER_TO_CORE_BE_KICKED;

typedef struct
{
	int		m_nPlayerIdx;
	BOOL	m_bSuccessFlag;
	char	m_szName[32];
} STONG_SERVER_TO_CORE_LEAVE;

typedef struct
{
	int		m_nPlayerIdx;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	DWORD	m_dwTongNameID;
	char	m_szName[32];
} STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER;

typedef struct
{
	int		m_nPlayerIdx;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	DWORD	m_dwTongNameID;
	char	m_szTitle[32];
	char	m_szName[32];
} STONG_SERVER_TO_CORE_CHANGE_AS;

typedef struct
{
	DWORD	m_dwTongNameID;
	char	m_szName[32];
} STONG_SERVER_TO_CORE_CHANGE_MASTER;

typedef struct
{
	DWORD	m_dwParam;
	int		m_nFlag;
	int		m_nCamp;
	int		m_nFigure;
	int		m_nPos;
	char	m_szTongName[32];
	char	m_szTitle[32];
	char	m_szMaster[32];
	char	m_szName[32];
} STONG_SERVER_TO_CORE_LOGIN;

// 玩家申请建立帮会 用扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	BYTE	m_btCamp;
	char	m_szName[defTONG_NAME_MAX_LENGTH + 1];
} TONG_APPLY_CREATE_COMMAND;

// 玩家申请加入帮会 用扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwNpcID;
} TONG_APPLY_ADD_COMMAND;

// 玩家申请加入帮会 用扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btCurFigure;
	BYTE	m_btCurPos;
	BYTE	m_btNewFigure;
	BYTE	m_btNewPos;
	char	m_szName[32];
} TONG_APPLY_INSTATE_COMMAND;

// 帮会建立失败 扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	BYTE	m_btFailId;
} TONG_CREATE_FAIL_SYNC;

// 转发加入帮会申请 扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	int		m_nPlayerIdx;
	char	m_szName[32];
} TONG_APPLY_ADD_SYNC;

// 通知玩家建立帮会成功 用普通协议
typedef struct
{
	BYTE	ProtocolType;
	BYTE	m_btCamp;
	char	m_szName[defTONG_NAME_MAX_LENGTH + 1];
} TONG_CREATE_SYNC;

// 通知玩家加入帮会 用扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	BYTE	m_btCamp;
	char	m_szTongName[32];
	char	m_szTitle[32];
	char	m_szMaster[32];
} TONG_Add_SYNC;

// 玩家申请解散帮会 用扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
} TONG_APPLY_DISMISS_COMMAND;

// 玩家帮会被解散 用普通协议
typedef struct
{
	BYTE	ProtocolType;
} TONG_DISMISS_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	int		m_nPlayerIdx;
	DWORD	m_dwNameID;
	BYTE	m_btFlag;			// 是否接受 TRUE 接受 FALSE 不接受
} TONG_ACCEPT_MEMBER_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	BYTE	m_btInfoID;
	int		m_nParam1;
	int		m_nParam2;
	int		m_nParam3;
	char	m_szBuf[64];
} TONG_APPLY_INFO_COMMAND;

typedef struct
{
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szTitle[32];
	char	m_szName[32];
} TONG_ONE_LEADER_INFO;

typedef struct
{
	char	m_szName[32];
} TONG_ONE_MEMBER_INFO;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwNpcID;
	DWORD	m_dwMoney;
	int		m_nCredit;
	BYTE	m_btCamp;
	BYTE	m_btLevel;
	BYTE	m_btDirectorNum;
	BYTE	m_btManagerNum;
	DWORD	m_dwMemberNum;
	char	m_szTongName[32];
	TONG_ONE_LEADER_INFO	m_sMember[1 + defTONG_MAX_DIRECTOR];
} TONG_HEAD_INFO_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwMoney;
	int		m_nCredit;
	BYTE	m_btCamp;
	BYTE	m_btLevel;
	BYTE	m_btDirectorNum;
	BYTE	m_btManagerNum;
	DWORD	m_dwMemberNum;
	BYTE	m_btStateNo;
	BYTE	m_btCurNum;
	char	m_szTongName[32];
	TONG_ONE_LEADER_INFO	m_sMember[defTONG_ONE_PAGE_MAX_NUM];
} TONG_MANAGER_INFO_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwMoney;
	int		m_nCredit;
	BYTE	m_btCamp;
	BYTE	m_btLevel;
	BYTE	m_btDirectorNum;
	BYTE	m_btManagerNum;
	DWORD	m_dwMemberNum;
	BYTE	m_btStateNo;
	BYTE	m_btCurNum;
	char	m_szTitle[32];
	char	m_szTongName[32];
	TONG_ONE_MEMBER_INFO	m_sMember[defTONG_ONE_PAGE_MAX_NUM];
} TONG_MEMBER_INFO_SYNC;

// 玩家自身在帮会中的信息 用扩展协议
typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	BYTE	m_btJoinFlag;
	BYTE	m_btFigure;
	BYTE	m_btCamp;
	char	m_szTongName[32];
	char	m_szTitle[32];
	char	m_szMaster[32];
} TONG_SELF_INFO_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btSuccessFlag;
	BYTE	m_btOldFigure;
	BYTE	m_btOldPos;
	BYTE	m_btNewFigure;
	BYTE	m_btNewPos;
	char	m_szTitle[32];
	char	m_szName[32];
} TONG_INSTATE_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szName[32];
} TONG_APPLY_KICK_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btSuccessFlag;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szName[32];
} TONG_KICK_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szName[32];
} TONG_APPLY_LEAVE_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btFigure;
	BYTE	m_btPos;
	char	m_szName[32];
} TONG_APPLY_CHANGE_MASTER_COMMAND;

typedef struct
{
	BYTE	ProtocolType;
	WORD	m_wLength;
	BYTE	m_btMsgId;
	DWORD	m_dwTongNameID;
	BYTE	m_btFailID;
	char	m_szName[32];
} TONG_CHANGE_MASTER_FAIL_SYNC;

// tong 相关 end
typedef struct
{
	BYTE	ProtocolType;
	DWORD	m_dwNpcID;
	WORD	m_wGoldFlag;
} NPC_GOLD_CHANGE_SYNC;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	dwItemID;
	int		nChange;
} ITEM_DURABILITY_CHANGE;

typedef struct
{
	BYTE	ProtocolType;
	DWORD	dwItemID;
} ITEM_REPAIR;

// 在调用这支函数之前必须判断是否处于交易状态，如果正在交易，不能调用这支函数
void SendClientCmdSell(int nID);
// 在调用这支函数之前必须判断是否处于交易状态，如果正在交易，不能调用这支函数
void SendClientCmdBuy(int nBuyIdx, int nPlace, int nX, int nY);
// 在调用这支函数之前必须判断是否处于交易状态，如果正在交易，不能调用这支函数
void SendClientCmdRun(int nX, int nY);
// 在调用这支函数之前必须判断是否处于交易状态，如果正在交易，不能调用这支函数
void SendClientCmdWalk(int nX, int nY);
// 在调用这支函数之前必须判断是否处于交易状态，如果正在交易，不能调用这支函数
void SendClientCmdSkill(int nSkillID, int nX, int nY);
//void SendClientCmdPing();
void SendClientCmdSit(int nSitFlag);
void SendClientCmdMoveItem(void* pDownPos, void* pUpPos);
void SendClientCmdQueryLadder(DWORD	dwLadderID);
void SendClientCmdRequestNpc(int nID);
void SendClientCmdJump(int nX, int nY);
void SendClientCmdStoreMoney(int nDir, int nMoney);
//void SendClientCmdRevive(int nReviveType);
void SendClientCmdRevive();
void SendObjMouseClick(int nObjID, DWORD dwRegionID);
void SendClientCmdRepair(DWORD dwID);

extern	int	g_nProtocolSize[MAX_PROTOCOL_NUM];
#pragma pack(pop, enter_protocol)
#endif
