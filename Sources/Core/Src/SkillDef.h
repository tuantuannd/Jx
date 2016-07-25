#ifndef __SKILLDEF_H__
#define __SKILLDEF_H__

#define MAX_SKILL  500
#define MAX_SKILLLEVEL 64
#define MaxMissleDir	64
#define MAXSKILLLEVELSETTINGNUM	10  //供填写技能升级时最大的相关数据种类
#define MAX_MISSLESTYLE  200
#define MISSLE_MIN_COLLISION_ZHEIGHT 0	  //子弹落地碰撞的高度。
#define MISSLE_MAX_COLLISION_ZHEIGHT 20   //子弹高于该高度时,不计算碰撞	

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
		MISSLE_MMK_RollBack = 100,					//  子单来回飞行
		MISSLE_MMK_Toss		,						//	左右震荡
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

#define	MAX_MISSLE_STATUS 4
enum eMissleStatus
{
	MS_DoWait,
	MS_DoFly,
	MS_DoVanish,
	MS_DoCollision,
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
		SKILL_SS_Thief,					//	偷窃类
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

enum eMeleeForm
{
	Melee_AttackWithBlur = SKILL_MF_COUNT,
	Melee_Jump,
	Melee_JumpAndAttack,
	Melee_RunAndAttack,
	Melee_ManyAttack,
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

typedef struct 
{
	int dx;
	int dy;
}TCollisionOffset;

typedef struct 
{
	int nRegion;
	int nMapX;
	int nMapY;
}
TMisslePos;

typedef struct 
{
	TCollisionOffset m_Offset [4];
}
TCollisionMatrix;
extern TCollisionMatrix g_CollisionMatrix[64];


typedef struct 
{
	int nLauncher;	
	DWORD dwLauncherID;			
	eSkillLauncherType eLauncherType; //发送者，一般为Npc


	int nParent;
	eSkillLauncherType eParentType;	  //母	 
	DWORD dwParentID;

	int nParam1;
	int nParam2;
	int nWaitTime;
	int nTargetId;
	DWORD dwTargetNpcID;
}
TOrdinSkillParam, * LPOrdinSkillParam;

#endif //__SKILLDEF_H__
