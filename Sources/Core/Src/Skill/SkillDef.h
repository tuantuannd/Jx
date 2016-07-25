#ifndef __SKILLDEF_H__
#define __SKILLDEF_H__

#define MAX_SKILL  500
#define MAX_SKILLLEVEL 64
#define MaxMissleDir	64
#define MAXSKILLLEVELSETTINGNUM	10  //供填写技能升级时最大的相关数据种类
#define MAX_MISSLESTYLE  200
#define	MAX_MISSLE_STATUS 4

//各种等级下技能基本属性的变化的数值类型
enum eEffectType					
{
	//改变该技能的属性
		ET_SKLATR_BEGIN					= 1	,
		ET_SKLATR_MANACOST					,		// 消耗MANA
		ET_SKLATR_COSTTYPE					,		//	
		ET_SKLATR_MINTIMEPERCAST  			,		// 每次发魔法的间隔时间
		ET_SKLATR_ATTACKRATE				,		// 改变Npc的成功率
		ET_SKLATR_MISSLENUM					,		// 
		ET_SKLATR_MISSLESFORM				,		
		ET_SKLATR_PARAM1					,		
		ET_SKLATR_PARAM2					,
		ET_SKLATR_END					=999,		//
		
		//改变发出子弹的属性
		ET_CHGMSL_BEGIN				= 1000  ,
		ET_CHGMSL_MOVEKIND					,		// 子弹格式
		ET_CHGMSL_SPEED						,		// 子弹飞行速度
		ET_CHGMSL_LIFETIME					,		// 子弹的生命时间
		ET_CHGMSL_HEIGHT					,		// 子弹的高度
		ET_CHGMSL_DAMAGERANGE				,		// 子弹破坏范围
		ET_CHGMSL_RADIUS					,		// 子弹飞行半径
		ET_CHGMSL_END				= 1999  ,
		
		//改变目标Npc的属性				(被动技能)
		ET_CHGNPC_BEGIN				= 2000  ,
		ET_CHGNPC_LIFE						,       // 改变Npc当前的Life值
		ET_CHGNPC_LIFELIMITED				,		// 改变Npc当前的Life上限值
		ET_CHGNPC_LIFERESTORESPEED			,		// 改变Npc当前的Life回复速度
		ET_CHGNPC_MANA						,		// 改变Npc当前的Mana
		ET_CHGNPC_MANALIMITED				,		// 改变Npc当前的Mana上限值
		ET_CHGNPC_MANASTORESPEED			,		// 改变Npc当前的Mana回复速度
		ET_CHGNPC_STAMINA					,		// 改变Npc当前的体力
		ET_CHGNPC_STAMINALIMITED			,		// 改变Npc当前的体力上限值
		ET_CHGNPC_STAMINARESTORESPEED		,		// 改变Npc当前的体力回复速度
		ET_CHGNPC_SPEED						,		// Npc或角色行走速度
		ET_CHGNPC_ATTACK					,		// 功击力
		ET_CHGNPC_DEFEND					,		// 防御力
		ET_CHGNPC_VISION					,		// 视野范围
		ET_CHGNPC_ATTACKRATE				,		// 改变Npc的命中率
		//	ET_CHGNPC_LIFERESTORESPEED			,		// 生命回复速度
		ET_CHGNPC_ATTACKSPEED				,		// 攻击速度
		ET_CHGNPC_HITRECOVER				,		// 快速打击系数
		ET_CHGNPC_END			= 2999		,		
		
		//改变Npc的当前状态属性			(主动辅助)
		ET_CHGNPCST_BEGIN					,						
		ET_CHGNPCST_RESISTFIRE				,		// 抗火性
		ET_CHGNPCST_RESISTFREEZE			,		// 抗冰性
		ET_CHGNPCST_RESISTPOISON			,		// 抗毒性
		ET_CHGNPCST_RESISTELECTRIC			,		// 抗电性
		ET_CHGNPCST_RESISTPHYSICS			,		// 抗物理性
		ET_CHGNPCST_RESISTALLEFECT			,		// 抗所有元素伤害	
		
		ET_CHGNPCST_FIRETIME				,		// 燃烧时间即燃烧次数
		ET_CHGNPCST_FREEZETIME				,		// 冰冻时间
		ET_CHGNPCST_POISONTIME				,		// 中毒时间
		ET_CHGNPCST_ELECTRICTIME			,		// 眩晕时间 玩家不能动
		ET_CHGNPCST_FIREPERDAMAGE			,		// 火伤害每次变化
		ET_CHGNPCST_FREEZEPERDAMAGE			,		// 冰伤害每次变化
		ET_CHGNPCST_POISONPERDAMAGE			,		// 毒伤害每次变化
		ET_CHGNPCST_ELECTRICPERDAMAGE		,		// 电伤害每次变化
		
		ET_CHGNPCST_FIRETHORN				,		// 火反弹系数
		ET_CHGNPCST_FREEZETHORN				,		// 冰冻反弹系数
		ET_CHGNPCST_POISONTHORN				,		// 毒反弹系数
		ET_CHGNPCST_ELECTRICTHORN			,		// 电反弹系数
		ET_CHGNPCST_PHYSICSTHORN			,		// 物理反弹系数
		ET_CHGNPCST_PHYSICSLIMITED			,		// 物理伤害值上下限
		ET_CHGNPCST_MISSLESPEED				,		// 子弹的速度变化
		ET_CHGNPCST_NPCCAMP					,		// 改变Npc的阵营
		
		ET_CHGNPCST_END ,
		
		ET_RECV_BEGIN				= 3000  ,
		ET_RECV_FIREDAMAGE					,		// 火伤害
		ET_RECV_POISONDAMAGE				,		// 毒伤害
		ET_RECV_FREEZEDAMAGE				,		// 冰伤害
		ET_RECV_DAMAGE						,		// 基本伤害
		ET_RECV_END					= 3999	,	
		
		ET_EFECTTYPE						,		// 元素伤害类型
		ET_EFECTDAMAGEPERTIME				,		// 元素伤害每时间
		ET_EFECTINTERVAL					,		// 元素伤害的间隔时间
		ET_EFECTDAMAGENUM					,		// 元素伤害次数		
		
};


//---------------------------------------------------------------------------
// MoveKind 运动类型
//---------------------------------------------------------------------------
enum eMissleMoveKind
{
	MISSLE_MMK_Stand,							//	原地
		MISSLE_MMK_Line,							//	直线飞行
		MISSLE_MMK_Random,							//	随机飞行（暗黑二女巫的Charged Bolt）
		MISSLE_MMK_Circle,							//	环行飞行（围绕在身边，暗黑二刺客的集气）
		MISSLE_MMK_Helix,							//	阿基米德螺旋线（暗黑二游侠的Bless Hammer）
		MISSLE_MMK_Follow,							//	跟踪目标飞行
		MISSLE_MMK_Motion,							//	玩家动作类
		MISSLE_MMK_Parabola,						//	抛物线
		MISSLE_MMK_SingleLine,						//	必中的单一直线飞行魔法
};

//---------------------------------------------------------------------------
// FollowKind 跟随类型	(主要是针对原地、环行与螺旋线飞行有意义)
//---------------------------------------------------------------------------
enum eMissleFollowKind
{
	MISSLE_MFK_None,							//	不跟随任何物件
	MISSLE_MFK_NPC,								//	跟随NPC或玩家
	MISSLE_MFK_Missle,							//	跟随子弹
};

enum eMissleStatus
{
	MS_DoWait,
	MS_DoFly,
	MS_DoVanish,
	MS_DoCollision,
};

struct TEffectData 
{
	eEffectType type;
	int			value;
};


enum eSkillLRInfo
{
	BothSkill,          //左右键皆可
	leftOnlySkill,		//左键
	RightOnlySkill,		//右键
	NoneSkill,			//都不可
};




//--------------------------------------------------------Skill.h


//技能发送者的类型
enum eGameActorType
{
	Actor_Npc,
	Actor_Obj,
	Actor_Missle,
	Actor_Sound,
	Actor_None,
};
enum eSkillLauncherType
{
	SKILL_SLT_Npc = 0,
	SKILL_SLT_Obj ,
	SKILL_SLT_Missle,
};

class KEffectDataNode :public KNode
{
public:
	TEffectData EffectData;
};

#ifndef _SERVER

struct	TOrginSkill
{
	int		nNpcIndex;				//	Npc的index
	DWORD	nSkillId;				//	发送的skillid
};

#endif


enum eSkillParamType
{
	SKILL_SPT_TargetIndex	= -1,
	SKILL_SPT_Direction		= -2,
};

//技能的类型
enum eSKillStyle
{
	SKILL_SS_Missles = 0,			//	子弹类		本技能用于发送子弹类
		SKILL_SS_Melee,
		SKILL_SS_InitiativeNpcState,	//	主动类		本技能用于改变当前Npc的主动状态
		SKILL_SS_PassivityNpcState,		//	被动类		本技能用于改变Npc的被动状态
		SKILL_SS_CreateNpc,				//	产生Npc类	本技能用于生成一个新的Npc
		SKILL_SS_BuildPoison,			//	炼毒类		本技能用于炼毒
		SKILL_SS_AddPoison,				//	加毒类		本技能用于给武器加毒性
		SKILL_SS_GetObjDirectly,		//	取物类		本技能用于隔空取物
		SKILL_SS_StrideObstacle ,		//	跨越类		本技能用于跨越障碍
		SKILL_SS_BodyToObject,			//	变物类		本技能用于将尸体变成宝箱
		SKILL_SS_Mining,				//	采矿类		本技能用于采矿随机生成矿石
		SKILL_SS_RepairWeapon,			//	修复类		本技能用于修复装备
		SKILL_SS_Capture,				//	捕捉类		本技能用于捕捉动物Npc
};

//同时发出的多个子弹的方向起始格式
enum eMisslesForm
{
	SKILL_MF_Wall	= 0,			//墙形	多个子弹呈垂直方向排列，类式火墙状
		SKILL_MF_Line,					//线形	多个子弹呈平行于玩家方向排列
		SKILL_MF_Spread,				//散形	多个子弹呈一定的角度的发散状	
		SKILL_MF_Circle,				//圆形	多个子弹围成一个圈
		SKILL_MF_Random,				//随机	多个子弹随机排放
		SKILL_MF_Zone,					//区域	多个子弹放至在某个范围内
		SKILL_MF_AtTarget,				//定点	多个子弹根据
		SKILL_MF_AtFirer,				//本身	多个子弹停在玩家当前位置
		SKILL_MF_COUNT,
};

enum eSKillCostType
{
	SKILL_SCT_MANA		= 1,
		SKILL_SCT_LIFE		= 2,
		SKILL_SCT_STAMINA	= 8,
		SKILL_SCT_MONEY		= 16,
};

enum eMisslesGenerateStyle
{
	SKILL_MGS_NULL		= 0,
		SKILL_MGS_SAMETIME	,    //同时
		SKILL_MGS_ORDER		,	 //按顺序
		SKILL_MGS_RANDONORDER,
		SKILL_MGS_RANDONSAME,
		SKILL_MGS_CENTEREXTENDLINE,  //由中间向两周扩散
};


struct TGameActorPos
{
	int nX;
	int nY;
	int nZ;
	int nSubWorldIndex;
};

struct TGameActorInfo
{
	eGameActorType nActorType;
	int nActorIndex;
	DWORD dwActorID;
	void	GetMps(int &nPX, int &nPY);
};

enum CASTSKILLPARAM
{
	PARAM_LAUNCHER	= 0x01, 
	PARAM_TARGET	= 0x02,
	PARAM_REF		= 0x04,
	PARAM_ONLYDIR	= 0x08,
	PARAM_USEREF    = 0X16,
};

union TGameActorUnion
{
	TGameActorPos	tPoint;
	TGameActorInfo	tActorInfo;
};

struct TSkillParam
{
	
	
	int				nDir;

	TGameActorUnion tRef;
	TGameActorUnion tTarget;
	TGameActorUnion tLauncher;

	unsigned short	usSkillID;
	unsigned short	usSkillLevel;
	DWORD	dwSkillParams;
	int		nWaitTime;
	inline int GetSubWorldIndex();
	inline int GetCellHeight();
	inline int GetCellWidth();
	
};

enum eSkillEvent
{
	Event_Start,
	Event_Fly,
	Event_Collision,
	Event_Vanish,
};

#endif //__SKILLDEF_H__