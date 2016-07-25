/*****************************************************************************************
//	外界访问Core用到数据结构等的定义
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-12
------------------------------------------------------------------------------------------
	一些定义可能处于在游戏世界各模块的头文件中，请在此处包含那个头文件，并请那样的头文件
不要包含一些与游戏世界对外无关的内容。
    开发过程中游戏世界的外部客户在未获得游戏世界接口完整定义的情况下，会现先直接在此文件
定义它需要的数据定义，游戏世界各模块可根据自身需要与设计把定义作修改或移动到模块内的对外
头文件，并在此建立充要的包含。
*****************************************************************************************/
#ifndef GAMEDATADEF_H
#define GAMEDATADEF_H

#include "CoreObjGenreDef.h"
#include "CoreUseNameDef.h"

#define		ITEM_VERSION						1

#define		_CHAT_SCRIPT_OPEN

#define		MAX_TEAM_MEMBER						7		// 最大队员数量(不包括队长)
#define		MAX_SENTENCE_LENGTH					256		// 聊天每个语句最大长度

#define		FILE_NAME_LENGTH					80
#define		PLAYER_PICKUP_CLIENT_DISTANCE		63
#define		defMAX_EXEC_OBJ_SCRIPT_DISTANCE		200
#define		defMAX_PLAYER_SEND_MOVE_FRAME		5
#define		PLAYER_PICKUP_SERVER_DISTANCE		40000
#define		MAX_INT								0x7fffffff

#define		ROLE_NO								2
#define		PLAYER_MALE_NPCTEMPLATEID			-1
#define		PLAYER_FEMALE_NPCTEMPLATEID			-2

#define		PLAYER_SHARE_EXP_DISTANCE			768

#define		MAX_DEATH_PUNISH_PK_VALUE			10		// PK处罚，PK值从 0 到 10

enum ITEM_PART
{
	itempart_head = 0,	// 头
	itempart_body,		// 身体
	itempart_belt,		// 腰带
	itempart_weapon,	// 武器
	itempart_foot,
	itempart_cuff,
	itempart_amulet,
	itempart_ring1,
	itempart_ring2,
	itempart_pendant,
	itempart_horse,
	itempart_num,
};

typedef struct
{
	int		nIdx;
	int		nPlace;
	int		nX;
	int		nY;
} PlayerItem;

enum INVENTORY_ROOM
{
	room_equipment = 0,	// 装备栏
	room_repository,	// 贮物箱
	room_trade,			// 交易栏
	room_tradeback,		// 交易过程中装备栏的备份
	room_trade1,		// 交易过程中对方的交易栏
	room_immediacy,		// 快捷物品
	room_num,			// 空间数量
};

enum ITEM_POSITION
{
	pos_hand = 1,		// 手上
	pos_equip,			// 装备着的
	pos_equiproom,		// 道具栏
	pos_repositoryroom,	// 贮物箱
	pos_traderoom,		// 交易栏
	pos_trade1,			// 交易过程中对方的交易栏
	pos_immediacy,		// 快捷物品
};

#define		MAX_HAND_ITEM				1
#define		EQUIPMENT_ROOM_WIDTH		6
#define		EQUIPMENT_ROOM_HEIGHT		10
#define		MAX_EQUIPMENT_ITEM			(EQUIPMENT_ROOM_WIDTH * EQUIPMENT_ROOM_HEIGHT)
#define		REPOSITORY_ROOM_WIDTH		6
#define		REPOSITORY_ROOM_HEIGHT		10
#define		MAX_REPOSITORY_ITEM			(REPOSITORY_ROOM_WIDTH * REPOSITORY_ROOM_HEIGHT)
#define		TRADE_ROOM_WIDTH			10
#define		TRADE_ROOM_HEIGHT			4
#define		MAX_TRADE_ITEM				(TRADE_ROOM_WIDTH * TRADE_ROOM_HEIGHT)
#define		MAX_TRADE1_ITEM				MAX_TRADE_ITEM
#define		IMMEDIACY_ROOM_WIDTH		3
#define		IMMEDIACY_ROOM_HEIGHT		1
#define		MAX_IMMEDIACY_ITEM			(IMMEDIACY_ROOM_WIDTH * IMMEDIACY_ROOM_HEIGHT)
#define		MAX_PLAYER_ITEM_RESERVED	32
#define		MAX_PLAYER_ITEM				(MAX_EQUIPMENT_ITEM + MAX_REPOSITORY_ITEM + MAX_TRADE_ITEM + MAX_TRADE1_ITEM + MAX_IMMEDIACY_ITEM + itempart_num + MAX_HAND_ITEM + MAX_PLAYER_ITEM_RESERVED)


#define		REMOTE_REVIVE_TYPE			0
#define		LOCAL_REVIVE_TYPE			1

#define		MAX_MELEE_WEAPON			6
#define		MAX_RANGE_WEAPON			3
#define		MAX_ARMOR					14
#define		MAX_HELM					14
#define		MAX_RING					1
#define		MAX_BELT					2
#define		MAX_PENDANT					2
#define		MAX_AMULET					2
#define		MAX_CUFF					2
#define		MAX_BOOT					4
#define		MAX_HORSE					6

#define		MAX_NPC_TYPE	300
#define		MAX_NPC_LEVEL	100


#define		MAX_NPC_DIR		64
#define		MAX_WEAPON		MAX_MELEE_WEAPON + MAX_RANGE_WEAPON
#define		MAX_SKILL_STATE 10
#define		MAX_NPC_HEIGHT	128
#define		MAX_RESIST		95
#define		MAX_HIT_PERCENT	95
#define		MIN_HIT_PERCENT	5
#define		MAX_NPC_RECORDER_STATE 8 //最大Npc可以记录下来的状态数量;

#define		PLAYER_MOVE_DO_NOT_MANAGE_DISTANCE	5

#define	NORMAL_NPC_PART_NO		5		// 普通npc图像默认为只有一个部件，这是第几个

#ifndef _SERVER
#define		C_REGION_X(x)	(LOWORD(SubWorld[0].m_Region[ (x) ].m_RegionID))
#define		C_REGION_Y(y)	(HIWORD(SubWorld[0].m_Region[ (y) ].m_RegionID))
#endif

enum
{
	CHAT_S_STOP = 0,						// 非聊天状态
	CHAT_S_SCREEN,							// 与同屏幕玩家聊天
	CHAT_S_SINGLE,							// 与同服务器某玩家私聊
	CHAT_S_TEAM,							// 与队伍全体成员交谈
	CHAT_S_NUM,								// 聊天状态中类数
};

enum PLAYER_INSTANT_STATE
{
	enumINSTANT_STATE_LEVELUP = 0,
	enumINSTANT_STATE_REVIVE,
	enumINSTANT_STATE_CREATE_TEAM,
	enumINSTANT_STATE_LOGIN,
	enumINSTANT_STATE_NUM,
};

enum CHAT_STATUS
{
	CHAT_S_ONLINE = 0,		//在线
	CHAT_S_BUSY,			//忙碌
	CHAT_S_HIDE,			//隐身
	CHAT_S_LEAVE,			//离开
	CHAT_S_DISCONNECT,		//掉线
};

// 注意：此枚举不允许更改(by zroc)
enum OBJ_ATTRIBYTE_TYPE
{
	series_metal = 0,		//	金系
	series_wood,			//	木系
	series_water,			//	水系
	series_fire,			//	火系
	series_earth,			//	土系
	series_num,
};

enum OBJ_GENDER
{
	OBJ_G_MALE	= 0,	//雄性，男的
	OBJ_G_FEMALE,		//雌的，女的
};

enum NPCCAMP
{
	camp_begin,				// 新手阵营（加入门派前的玩家）
	camp_justice,			// 正派阵营
	camp_evil,				// 邪派阵营
	camp_balance,			// 中立阵营
	camp_free,				// 杀手阵营（出师后的玩家）
	camp_animal,			// 野兽阵营
	camp_event,				// 路人阵营
	camp_num,				// 阵营数
};

enum ITEM_IN_ENVIRO_PROP
{
	IIEP_NORMAL = 0,	//一般/正常/可用
	IIEP_NOT_USEABLE,	//不可用/不可装配
	IIEP_SPECIAL,		//特定的不同情况
};

#define	GOD_MAX_OBJ_TITLE_LEN	1024	//128临时改为1024为了兼容旧代码 to be modified
#define	GOD_MAX_OBJ_PROP_LEN	516
#define	GOD_MAX_OBJ_DESC_LEN	516

//==================================
//	游戏对象的描述
//==================================
struct KGameObjDesc
{
	char	szTitle[GOD_MAX_OBJ_TITLE_LEN];	//标题，名称
	char	szProp[GOD_MAX_OBJ_PROP_LEN];	//属性，每行可以tab划分为靠左与靠右对齐两部分
	char	szDesc[GOD_MAX_OBJ_DESC_LEN];	//描述
};

//==================================
//	问题与可选答案
//==================================
struct KUiAnswer
{
	char	AnswerText[256];	//可选答案文字（可以包含控制符）
	int		AnswerLen;			//可选答案存储长度（包括控制符，不包含结束符）
};

struct KUiQuestionAndAnswer
{
	char		Question[512];	//问题文字（可以包含控制符）
	int			QuestionLen;	//问题文字存储长度（包括控制符，不包含结束符）
	int			AnswerCount;	//可选答案的数目
	KUiAnswer	Answer[1];		//候选答案
};

//==================================
//	简略表示游戏对象的结构
//==================================
struct KUiGameObject
{
	unsigned int uGenre;	//对象类属
	unsigned int uId;		//对象id
//	int			 nData;		//与对象实例相关的某数据
};

//==================================
//	以坐标表示的一个区域范围
//==================================
struct KUiRegion
{
	int		h;		//左上角起点横坐标
	int		v;		//左上角起点纵坐标
	int		Width;	//区域横宽
	int		Height;	//区域纵宽
};

//==================================
//	可以游戏对象容纳的地方
//==================================
enum UIOBJECT_CONTAINER
{
	UOC_IN_HAND	= 1,		//手中拿着
	UOC_GAMESPACE,			//游戏窗口
	UOC_IMMEDIA_ITEM,		//快捷物品
	UOC_IMMEDIA_SKILL,		//快捷武功0->右键武功，1,2...-> F1,F2...快捷武功
	UOC_ITEM_TAKE_WITH,		//随身携带
	UOC_TO_BE_TRADE,		//要被买卖，买卖面板上
	UOC_OTHER_TO_BE_TRADE,	//买卖面板上，别人要卖给自己的，
	UOC_EQUIPTMENT,			//身上装备
	UOC_NPC_SHOP,			//npc买卖场所
	UOC_STORE_BOX,			//储物箱
	UOC_SKILL_LIST,			//列出全部拥有技能的窗口，技能窗口
	UOC_SKILL_TREE,			//左、右可用技能树
};

//==================================
// iCoreShell::GetGameData函数调用,uDataId取值为GDI_TRADE_DATA时，
// uParam的许可取值列表
// 注释中的Return:行表示相关的GetGameData调用的返回值的含义
//==================================
enum UI_TRADE_OPER_DATA
{
	UTOD_IS_WILLING,		//是否交易意向(叫卖中)
	//Return: 返回自己是否处于叫卖中的布尔值
	UTOD_IS_LOCKED,			//自己是否处于已锁定状态
	//Return: 返回自己是否处于已锁定状态的布尔值
	UTOD_IS_TRADING,		//是否可以正在等待交易操作（交易是否已确定）
	//Return: 返回是否正在等待交易操作（交易是否已确定）
	UTOD_IS_OTHER_LOCKED,	//对方是否已经处于锁定状态
	//Return: 返回对方是否已经处于锁定状态的布尔值
};

//==================================
//	买卖物品
//==================================
struct KUiItemBuySelInfo
{
	char			szItemName[64];	//物品名称
	int				nPrice;			//买卖价钱，正值为卖价格，负值表示买入的价格为(-nPrice)
};

//==================================
//	表示某个游戏对象在坐标区域范围的信息结构
//==================================
struct KUiObjAtRegion
{
	KUiGameObject	Obj;
	KUiRegion		Region;
};

struct KUiObjAtContRegion : public KUiObjAtRegion
{
	union
	{
		UIOBJECT_CONTAINER	eContainer; 
		int					nContainer;
	};
};

struct KUiMsgParam
{
	unsigned char	eGenre;	//取值范围为枚举类型MSG_GENRE_LIST,见MsgGenreDef.h文件
	unsigned char	cChatPrefixLen;
	unsigned short	nMsgLength;
	char			szName[32];
#define	CHAT_MSG_PREFIX_MAX_LEN	16
	unsigned char	cChatPrefix[CHAT_MSG_PREFIX_MAX_LEN];
};

struct KUiInformationParam
{
	char	sInformation[256];	//消息文字内容
	char	sConfirmText[64];	//确认消息(按钮)的标题文字
	short	nInforLen;			//消息文字内容的存储长度
	bool	bNeedConfirmNotify;	//是否要发回确认消息(给core)
	bool	bReserved;			//保留，值固定为0
};

enum PLAYER_ACTION_LIST
{
	PA_NONE = 0,	//无动作
	PA_RUN  = 0x01,	//跑
	PA_SIT  = 0x02,	//打坐
	PA_RIDE = 0x04,	//骑（马）
};

//==================================
//	系统消息分类
//==================================
enum SYS_MESSAGE_TYPE
{
	SMT_NORMAL = 0,	//不参加分类的消息
	SMT_SYSTEM,		//系统，连接相关
	SMT_PLAYER,		//玩家相关
	SMT_TEAM,		//组队相关
	SMT_FRIEND,		//聊天好友相关
	SMT_MISSION,	//任务相关
	SMT_CLIQUE,		//帮派相关
};

//==================================
//	系统消息响应方式
//==================================
enum SYS_MESSAGE_CONFIRM_TYPE
{
	SMCT_NONE,				//在对话消息窗口直接掠过，不需要响应。
	SMCT_CLICK,				//点击图标后立即删除。
	SMCT_MSG_BOX,			//点击图标后弹出消息框。
	SMCT_UI_RENASCENCE,		//选择重生
	SMCT_UI_ATTRIBUTE,		//打开属性页面
	SMCT_UI_SKILLS,			//打开技能页面
	SMCT_UI_ATTRIBUTE_SKILLS,//打开属性页面技能页面
	SMCT_UI_TEAM_INVITE,	//答应或拒绝加入队伍的邀请,
	//						pParamBuf 指向一个KUiPlayerItem结构的数据，表示邀情人(队长)
	SMCT_UI_TEAM_APPLY,		//答应或拒绝加入队伍的申请,
	//						pParamBuf 指向一个KUiPlayerItem结构的数据，表示申请人
	SMCT_UI_TEAM,			//打开队伍管理面板
	SMCT_UI_INTERVIEW,		//打开聊天对话界面,
	//						pParamBuf 指向一个KUiPlayerItem结构的数据，表示发来消息的好友
	SMCT_UI_FRIEND_INVITE,	//批准或拒绝别人加自己为好友
	//						pParamBuf 指向一个KUiPlayerItem结构的数据，表示发出好友邀请的人
	SMCT_UI_TRADE,			//答应或拒绝交易的请求,
	//						pParamBuf 指向一个KUiPlayerItem结构的数据，表示发出交易邀请的人
	SMCT_DISCONNECT,		//断线
	SMCT_UI_TONG_JOIN_APPLY,//答应或拒绝加入帮会的申请
};

//==================================
//	系统消息
//==================================
struct KSystemMessage
{
	char			szMessage[64];	//消息文本
	unsigned int	uReservedForUi;	//界面使用的数据域,core里填0即可
	unsigned char	eType;			//消息分类取值来自枚举类型 SYS_MESSAGE_TYPE
	unsigned char	byConfirmType;	//响应类型
	unsigned char	byPriority;		//优先级,数值越大，表示优先级越高
	unsigned char	byParamSize;	//伴随GDCNI_SYSTEM_MESSAGE消息的pParamBuf所指参数缓冲区空间的大小。
};

//==================================
//	聊天频道的描述
//==================================
struct KUiChatChannel
{
	int			 nChannelNo;
	unsigned int uChannelId;
	union
	{
		int		 nChannelIndex;
		int		 nIsSubscibed;	//是否被订阅
	};
	char		 cTitle[32];
};

//==================================
//	聊天好友的一个分组的信息
//==================================
struct KUiChatGroupInfo
{
	char	szTitle[32];	//分组的名称
	int		nNumFriend;		//组内好友的数目
};

//==================================
//	好友发来的聊天话语
//==================================
struct KUiChatMessage
{
	unsigned int uColor;
	short	nContentLen;
	char	szContent[256];
};

//==================================
//	主角的一些不易变的数据
//==================================
struct KUiPlayerBaseInfo
{
	char	Agname[32];	//绰号
	char	Name[32];	//名字
	char	Title[32];	//称号
	int		nCurFaction;// 当前加入门派 id ，如果为 -1 ，当前没有在门派中
	int		nRankInWorld;//江湖排名值,值为0表示未上排名板
	unsigned int nCurTong;// 当前加入帮派name id ，如果为 0 ，当前没有在帮派中
};

//==================================
//	主角的一些易变的数据
//==================================
struct KUiPlayerRuntimeInfo
{
	int		nLifeFull;			//生命满值
	int		nLife;				//生命
	int		nManaFull;			//内力满值
	int		nMana;				//内力
	int		nStaminaFull;		//体力满值
	int		nStamina;			//体力
	int		nAngryFull;			//怒满值
	int		nAngry;				//怒
	int		nExperienceFull;	//经验满值
	int		nExperience;		//当前经验值
	int		nCurLevelExperience;//当前级别升级需要的经验值

	unsigned char	byActionDisable;//是否不可进行各种动作，为枚举PLAYER_ACTION_LIST取值的组合
	unsigned char	byAction;	//正在进行的行为动作，为枚举PLAYER_ACTION_LIST取值的组合
	unsigned short	wReserved;	//保留
};

//==================================
//	主角的一些属性数据索引
//==================================
enum UI_PLAYER_ATTRIBUTE
{
	UIPA_STRENGTH = 0,			//力量
	UIPA_DEXTERITY,				//敏捷
	UIPA_VITALITY,				//活力
	UIPA_ENERGY,				//精力
};

//==================================
//	主角的一些易变的属性数据
//==================================
struct KUiPlayerAttribute
{
	int		nMoney;				//银两
	int		nLevel;				//等级
	char	StatusDesc[16];		//状态描述

	int		nBARemainPoint;		//基本属性剩余点数
	int		nStrength;			//力量
	int		nDexterity;			//敏捷
	int		nVitality;			//活力
	int		nEnergy;			//精力

	int		nKillMAX;			//最大杀伤力
	int		nKillMIN;			//最小杀伤力
	int		nRightKillMax;		//右键最大杀伤力
	int		nRightKillMin;		//右键最小杀伤力

	int		nAttack;			//攻击力
	int		nDefence;			//防御力
	int		nMoveSpeed;			//移动速度
	int		nAttackSpeed;		//攻击速度

	int		nPhyDef;			//物理防御
	int		nCoolDef;			//冰冻防御
	int		nLightDef;			//闪电防御
	int		nFireDef;			//火焰防御
	int		nPoisonDef;			//毒素防御
};

//==================================
//	主角的立即使用物品与武功
//==================================
struct KUiPlayerImmedItemSkill
{
	KUiGameObject	ImmediaItem[3];
	KUiGameObject	IMmediaSkill[2];
};

//==================================
//	主角装备安换的位置
//==================================
enum UI_EQUIPMENT_POSITION
{
	UIEP_HEAD = 0,		//头戴
	UIEP_HAND = 1,		//手持
	UIEP_NECK = 2,		//脖子
	UIEP_FINESSE = 3,	//手腕
	UIEP_BODY = 4,		//身穿
	UIEP_WAIST = 5,		//腰部
	UIEP_FINGER1 = 6,	//手指甲
	UIEP_FINGER2 = 7,	//手指乙
	UIEP_WAIST_DECOR= 8,//腰坠
	UIEP_FOOT = 9,		//脚踩
	UIEP_HORSE = 10,	//马匹
};

//==================================
//	主角的生活技能数据
//==================================
struct KUiPlayerLiveSkillBase
{
	int		nRemainPoint;			//剩余技能点数
	int		nLiveExperience;		//当前技能经验值
	int		nLiveExperienceFull;	//升到下级需要的经验值
};

//==================================
//	单项技能数据
//==================================
struct KUiSkillData : public KUiGameObject
{
	union
	{
		int		nLevel;
		int		nData;
	};
};

//==================================
//	一个队伍中最多包含成员的数目
//==================================
#define	PLAYER_TEAM_MAX_MEMBER	8

//==================================
//	统帅能力相关的数据
//==================================
struct KUiPlayerLeaderShip
{
	int		nLeaderShipLevel;			//统帅力等级
	int		nLeaderShipExperience;		//统帅力经验值
	int		nLeaderShipExperienceFull;	//升到下级需要的经验值
};

//==================================
//	一个玩家角色项
//==================================
struct KUiPlayerItem
{
	char			Name[32];	//玩家角色姓名
	unsigned int	uId;		//玩家角色id
	int				nIndex;		//玩家角色索引
	int				nData;		//此玩家相关的一项数值，含义与具体的应用位置有关
};

//==================================
//	组队信息的描述
//==================================
struct KUiTeamItem
{
	KUiPlayerItem	Leader;
};

//==================================
//	队伍信息
//==================================
struct KUiPlayerTeam
{
	bool			bTeamLeader;			//玩家自己是否队长
	char			cNumMember;				//队员数目
	char			cNumTojoin;				//欲加入的人员的数目
	bool			bOpened;				//队伍是否允许其他人加入
	int				nTeamServerID;			//队伍在服务器上的id，用于标识该队伍，-1 为空
	int				nCaptainPower;
};

//==================================
//	魔法属性
//==================================
#ifndef MAGICATTRIB
#define MAGICATTRIB
struct KMagicAttrib
{
	int				nAttribType;					//属性类型
	int				nValue[3];						//属性参数
	KMagicAttrib(){nValue[0] = nValue[1] = nValue[2] = nAttribType = 0;};
};
#else
struct KMagicAttrib;
#endif

/* 这是旧的代码，新的已经放在KNpcGold里面处理了
//==================================
//	NPC加强
//==================================
struct KNpcEnchant
{
	int		nExp;					// 经验
	int		nLife;					// 生命
	int		nLifeReplenish;			// 回血
	int		nAttackRating;			// 命中
	int		nDefense;				// 防御
	int		nMinDamage;
	int		nMaxDamage;

	int		TreasureNumber;				// 装备
	int		AuraSkill;					// 光环
	int		DamageEnhance;				// 伤害
	int		SpeedEnhance;				// 速度
	int		SelfResist;					// 自身抗性
	int		ConquerResist;				// 相生抗性
#ifndef _SERVER
	char	NameModify[32];				// 改名
#endif
};

//==================================
//	NPC单项加强
//==================================
struct KNpcSpeicalEnchant
{
	int		ValueModify;
	char	NameModify[16];
};
*/

struct KMapPos
{
	int		nSubWorld;
	int		nRegion;
	int		nMapX;
	int		nMapY;
	int		nOffX;
	int		nOffY;
};

//==================================
//	选项设置项
//==================================
enum OPTIONS_LIST
{
	OPTION_PERSPECTIVE,		//透视模式  nParam = (int)(bool)bEnable 是否开启
	OPTION_DYNALIGHT,		//动态光影	nParam = (int)(bool)bEnable 是否开启
	OPTION_MUSIC_VALUE,		//音乐音量	nParam = 音量大小（取值为0到-10000）
	OPTION_SOUND_VALUE,		//音效音量	nParam = 音量大小（取值为0到-10000）
	OPTION_BRIGHTNESS,		//亮度调节	nParam = 亮度大小（取值为0到-100）
	OPTION_WEATHER,			//天气效果开关 nParam = (int)(bool)bEnable 是否开启
};

//==================================
//	所处的地域时间环境信息
//==================================
struct KUiSceneTimeInfo
{
	char	szSceneName[32];		//场景名
	int		nSceneId;				//场景id
	int		nScenePos0;				//场景当前坐标（东）
	int		nScenePos1;				//场景当前坐标（南）
	int		nGameSpaceTime;			//以分钟为单位
};

//==================================
//	光源信息
//==================================
//整数表示的三维点坐标
struct KPosition3
{
	int nX;
	int nY;
	int nZ;
};

struct KLightInfo
{
	KPosition3 oPosition;			// 光源位置
	DWORD dwColor;					// 光源颜色及亮度
	long  nRadius;					// 作用半径
};


//小地图的显示内容项
enum SCENE_PLACE_MAP_ELEM
{ 
	SCENE_PLACE_MAP_ELEM_NONE		= 0x00,		//无东西
	SCENE_PLACE_MAP_ELEM_PIC		= 0x01,		//显示缩略图
	SCENE_PLACE_MAP_ELEM_CHARACTER	= 0x02,		//显示人物
	SCENE_PLACE_MAP_ELEM_PARTNER	= 0x04,		//显示同队伍人
};

//场景的地图信息
struct KSceneMapInfo
{
	int	nScallH;		//真实场景相对于地图的横向放大比例
	int nScallV;		//真实场景相对于地图的纵向放大比例
	int	nFocusMinH;
	int nFocusMinV;
	int nFocusMaxH;
	int nFocusMaxV;
	int nOrigFocusH;
	int nOrigFocusV;
	int nFocusOffsetH;
	int nFocusOffsetV;
};

enum NPC_RELATION
{
	relation_none	= 1,
	relation_self	= 2,
	relation_ally	= 4,
	relation_enemy	= 8,
	relation_dialog	= 16,
	relation_all	= relation_none | relation_ally | relation_enemy | relation_self | relation_dialog,	
	relation_num,
};

enum NPCKIND
{
	kind_normal = 0	,		
	kind_player,
	kind_partner,
	kind_dialoger,	//对话者
	kind_bird,
	kind_mouse,
	/*kind_melee	= 0x0004,
	kind_range	= 0x0008,
	kind_escape	= 0x0010,
	kind_bird	= 0x0020,
	*/
    kind_num
};

enum	// 物件类型
{
	Obj_Kind_MapObj = 0,		// 地图物件，主要用于地图动画
	Obj_Kind_Body,				// npc 的尸体
	Obj_Kind_Box,				// 宝箱
	Obj_Kind_Item,				// 掉在地上的装备
	Obj_Kind_Money,				// 掉在地上的钱
	Obj_Kind_LoopSound,			// 循环音效
	Obj_Kind_RandSound,			// 随机音效
	Obj_Kind_Light,				// 光源（3D模式中发光的东西）
	Obj_Kind_Door,				// 门类
	Obj_Kind_Trap,				// 陷阱
	Obj_Kind_Prop,				// 小道具，可重生
	Obj_Kind_Num,				// 物件的种类数
};

//主角身份地位等一些关键属性项
enum PLAYER_BRIEF_PROP
{
	PBP_LEVEL = 1,	//登级变化	nParam表示当前等级
	PBP_FACTION,	//门派		nParam表示门派属性，如果nParam为-1表示没有门派
	PBP_CLIQUE,		//帮派		nParam为非0值表示入了帮派，0值表示脱离了帮派
};

//新闻消息的类型定义
enum NEWS_MESSAGE_TYPE
{
	NEWSMESSAGE_NORMAL,			//一般消息，显示（一次）就消息消亡了
								//无时间参数
	NEWSMESSAGE_COUNTING,		//倒计（秒）数消息，计数到0时，就消息就消亡了。
								//时间参数中的数据结构中仅秒数据有效，倒计数以秒为单位。
	NEWSMESSAGE_TIMEEND,		//定时消息，定时到时，消息就消完了，否则每半分钟显示一次。
								//时间参数表示消亡的指定时间。
};

#define MAX_MESSAGE_LENGTH 512

struct KNewsMessage
{
	int		nType;						//消息类型
	char	sMsg[MAX_MESSAGE_LENGTH];	//消息内容
	int		nMsgLen;					//消息内容存储长度
};

struct KRankIndex
{
	bool			bValueAppened;	//每一项是否有没有额外数据
	bool			bSortFlag;		//每一项是否有没有升降标记
	unsigned short	usIndexId;		//排名项ID数值
};

#define MAX_RANK_MESSAGE_STRING_LENGTH 128

struct KRankMessage
{
	char szMsg[MAX_RANK_MESSAGE_STRING_LENGTH];	// 文字内容
	unsigned short		usMsgLen;				// 文字内容的长度
	short				cSortFlag;				// 旗标值，QOO_RANK_DATA的时候表示出升降，负值表示降，正值表示升，0值表示位置未变
	int					nValueAppend;			// 此项附带的值

};

struct KMissionRecord
{
	char			sContent[256];	//存储任务提示信息的缓冲区，（字符串为控制符已经编码的字符串）
	int				nContentLen;	//sContent内有效内容的长度(单位：字节)，长度最大一定不超过256字节
	unsigned int	uValue;			//关联数值
};

//---------------------------- 帮会相关 ------------------------

#define		defTONG_MAX_DIRECTOR				7
#define		defTONG_MAX_MANAGER					56
#define		defTONG_ONE_PAGE_MAX_NUM			7

#define		defTONG_STR_LENGTH					32

#define		defTONG_NAME_MAX_LENGTH				8

enum TONG_MEMBER_FIGURE
{
	enumTONG_FIGURE_MEMBER,				// 帮众
	enumTONG_FIGURE_MANAGER,			// 队长
	enumTONG_FIGURE_DIRECTOR,			// 长老
	enumTONG_FIGURE_MASTER,				// 帮主
	enumTONG_FIGURE_NUM,
};

enum
{
	enumTONG_APPLY_INFO_ID_SELF,		// 申请查询自身信息
	enumTONG_APPLY_INFO_ID_MASTER,		// 申请查询帮主信息
	enumTONG_APPLY_INFO_ID_DIRECTOR,	// 申请查询长老信息
	enumTONG_APPLY_INFO_ID_MANAGER,		// 申请查询队长信息
	enumTONG_APPLY_INFO_ID_MEMBER,		// 申请查询帮众信息(一批帮众)
	enumTONG_APPLY_INFO_ID_ONE,			// 申请查询某帮会成员信息(一个帮众)
	enumTONG_APPLY_INFO_ID_TONG_HEAD,	// 申请查询某帮会信息，用于申请加入帮会
	enumTONG_APPLY_INFO_ID_NUM,
};

enum
{
	enumTONG_CREATE_ERROR_ID1,	// Player[m_nPlayerIndex].m_nIndex <= 0
	enumTONG_CREATE_ERROR_ID2,	// 交易过程中
	enumTONG_CREATE_ERROR_ID3,	// 帮会名问题
	enumTONG_CREATE_ERROR_ID4,	// 帮会阵营问题
	enumTONG_CREATE_ERROR_ID5,	// 已经是帮会成员
	enumTONG_CREATE_ERROR_ID6,	// 自己的阵营问题
	enumTONG_CREATE_ERROR_ID7,	// 等级问题	
	enumTONG_CREATE_ERROR_ID8,	// 钱问题
	enumTONG_CREATE_ERROR_ID9,	// 组队不能建帮会
	enumTONG_CREATE_ERROR_ID10,	// 帮会模块出错
	enumTONG_CREATE_ERROR_ID11,	// 名字字符串出错
	enumTONG_CREATE_ERROR_ID12,	// 名字字符串过长
	enumTONG_CREATE_ERROR_ID13,	// 帮会同名错误
	enumTONG_CREATE_ERROR_ID14,	// 帮会产生失败
};

//帮派信息
struct KTongInfo
{
	char  szName[8];			//帮会名字
	char  szMasterName[32];		//帮主名字
	int   nFaction;				//帮会阵营
	int   nMoney;				//帮会资金
	int   nReserve[2];
	int	  nMemberCount;         //帮众数量
	int   nManagerCount;        //队长数量
	int   nDirectorCount;       //长老数量
	int   nFigure;              //待删除
};

//对帮内成员做的动作，或改变自己与帮会的关系的动作
enum TONG_ACTION_TYPE
{
	TONG_ACTION_DISMISS,       //踢人
	TONG_ACTION_ASSIGN,        //任命
	TONG_ACTION_DEMISE,        //传位
	TONG_ACTION_LEAVE,         //离帮
	TONG_ACTION_RECRUIT,       //招人状态
	TONG_ACTION_APPLY,         //申请加入
};

//对帮内成员做的动作，或改变自己与帮会的关系的动作时的参数
struct KTongOperationParam
{
	TONG_ACTION_TYPE	eOper;				//操作项
	int                 nData[4];
	char				szPassword[32];		//操作项需要的密码，如果对应的操作项不需要密码，则直接填密码为空。
};

//帮会成员项
struct KTongMemberItem : public KUiPlayerItem
{
	char szAgname[64];	//在帮内的职务/称号
};

//某个玩家与XX的关系，(XX可以是帮会，队伍等等)
struct KUiPlayerRelationWithOther : KUiPlayerItem
{
	int		nRelation;
	int		nParam;
};

//通用的带名称描述游戏对象的结构
struct KUiGameObjectWithName
{
	char			szName[32];
	char			szString[32];
	int				nData;
	int				nParam;
	unsigned int 	uParam;
};

//-------------------------- 帮会相关 end ----------------------

#endif
