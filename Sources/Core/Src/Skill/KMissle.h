// KMissle.h: interface for the KMissle class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	KMissleH
#define KMissleH

#ifdef  _SERVER
#define MAX_MISSLE  5000
#else
#define MAX_MISSLE 500
#endif
#include "kcore.h"
#include "skilldef.h"
#include "KObj.h"
#include "KMissleRes.h"
#include "KNode.h"
#include "KITabFile.h"
#include "KNpcSet.h"
#include "GameDataDef.h"
//#include "Skill.h"

#ifdef _SERVER
#include "KMissleMagicAttribsData.h"
#endif
enum MISSLE_RELATION
{
		MissleRelation_None = 0,
		MissleRelation_EnemyOnly  ,
		MissleRelation_AllyOnly,
		MissleRelation_All,
};

#ifndef TOOLVERSION
class KMissle
#else
class CORE_API KMissle  
#endif
{
	friend class	KSkill;
private:
public:
	//	子弹设定文件获得的数据
	char				m_szMissleName[30];		//	子弹的名称
	int					m_nAction;				//	当前行为
	BOOL				m_bIsSlow;				//	是否被减速
	BOOL				m_bClientSend;			//	是否需要
	BOOL				m_bRemoving;			//	获知需要下一个循环删除该子弹所有资源
	BOOL				m_bIsMelee;				//	子弹是否是近身攻击
	TSkillParam			m_SkillParam;
	unsigned long		m_dwID;
#ifndef _SERVER
	KMissleRes			m_MissleRes;			//	子弹的资源
	BOOL				CreateSpecialEffect(eMissleStatus eStatus,  int nPX, int nPY, int nPZ, int nNpcIndex = 0);
#endif
	eMissleMoveKind		m_eMoveKind;			//	子弹运动类型(爆炸、直线飞行等……)
	eMissleFollowKind	m_eFollowKind;			//	子碟发出时的参照类型
	int					m_nHeight;				//	子弹高度
	int					m_nHeightSpeed;			//	子弹纵行的飞行速度
	int					m_nLifeTime;			//	生命周期
	int					m_nSpeed;				//	飞行速度
	int					m_nSkillId;				//	对应哪个技能
	BOOL				m_bRangeDamage;			//	是否为区域伤害，即是否多人受到伤害
	MISSLE_RELATION		m_eRelation;			//	目标与发射者的关系
	bool				m_bAutoExplode;			//  Is Missle Would AutoExpode ItSelf For Collision When It's LiftTime Is Over;
	
	
	//BOOL				m_bByMissle;

	BOOL				m_bBaseSkill;			//	是否为最基本技能
	BOOL				m_bByMissle;			//	当由父技能产生时，是否是根据玩家为基点还是以当前的子弹为基点
	
	
	int					m_nCollideRange;		//	碰撞范围（简化多边形碰撞用）
	int					m_nDamageRange;			//	伤害范围
	BOOL				m_bCollideVanish;		//	碰撞后是否消亡
	BOOL				m_bCollideFriend;		//	是否会碰撞到同伴
	BOOL				m_bCanSlow;				//	是否会被减速（比如说Slow Missle类的技能）
	int					m_nKnockBack;			//	震退距离
	int					m_nStunTime;			//	晕眩时间
	
	BOOL				m_bFlyEvent;			//	整个飞行过程中的
	int					m_nFlyEventTime;
	BOOL				m_bSubEvent;			//	是否需要在飞行过程消息发生是，调用相关回调函数
	BOOL				m_bStartEvent;			//	是否需要在技能第一次Active时，调用相关回调函数
	BOOL				m_bCollideEvent;		//	是否需要在子技能魔法碰撞时，调用相关回调函数
	BOOL				m_bVanishedEvent;		//	是否需要在子技能消亡时，调用相关的回调函数
	
	
#ifndef _SERVER
	BOOL				m_bMultiShow;			//	子弹有两个显示
	bool				m_bFollowNpcWhenCollid; //	爆炸效果跟随被击中的人物
#endif
	
	BOOL				m_bMustBeHit;			//  必中，即子弹无特殊情况一定能打中对方，无论是否实际碰撞到对方.
	
	//	技能获得的数据
	int					m_nCurrentLife;			//	当前生命时间
	int					m_nStartLifeTime;		//	当技能发生后，第几帧开始
	int					m_nCollideOrVanishTime; 
	int					m_nCurrentMapX;			//	当前的X坐标
	int					m_nCurrentMapY;			//	当前的Y坐标
	int					m_nCurrentMapZ;			//	当前的Z坐标
	int					m_nXOffset;				//	当前的X方向偏移
	int					m_nYOffset;				//	当前的Y方向偏移
	int					m_nSrcMapX;				//	源坐标
	int					m_nSrcMapY;				
	int					m_nSrcXOffset;		
	int					m_nSrcYOffset;
#ifndef _SERVER	
	unsigned int		m_SceneID;
#endif
	//单一飞行子单时，精确命中！
	int					m_nXFactor;
	int					m_nYFactor;
	int					m_nLevel;				//	技能等级
	//int					m_nFollowNpcIdx;		//	跟随谁
	//int					m_nLauncher;			//	发射者在NpcSet中的Index
	//DWORD				m_dwLauncherId;			//	发射者的唯一ID
	//eSkillLauncherType	m_eSkillLauncherType;
	//	自生成的动态数据	
	int					m_nCurrentSpeed;		//	当前速度（可能被减速）
	int					m_nZAcceleration;		//	Z轴的加速度
	eMissleStatus		m_eMissleStatus;		//	子弹当前的状态
	int					m_nMissleId;			//	子弹在Missle中的ID
	int					m_nSubWorldId;			//	子世界ID
	int					m_nRegionId;			//	区域ID
	//各项伤害数据
	int					m_nMaxDamage;			//	最大伤害
	int					m_nElementType;			//	元素伤害类型
	int					m_nMaxElementDamage;	//	最大元素伤害(首次)
	int					m_nElementTime;			//	元素持续时间
	int					m_nElementInterval;		//	元素间隔时间
	int					m_nElementPerDamage;	//	元素伤害时，每次发作所受的伤害值
	int					m_nParam1;				//	参数一
	int					m_nParam2;				//	参数二
	int					m_nParam3;				//	参数三
	int					m_nDirIndex;			//	当前运动方向的索引
	int					m_nDir;					//	当前的运行方向
	int					m_nAngle;				//	
	DWORD				m_dwBornTime;			//	该子弹产生时的时间
public:
	KMissle();
	virtual ~KMissle();
#ifndef		_SERVER
	void				Paint();
#endif
private:
	BOOL				Init( int nLauncher, int nMissleId, int nXFactor, int nYFactor, int nLevel);
	void				OnVanish();//即将消失
	void				OnCollision();//碰撞
	void				OnFly();//飞行过程中
	void				OnWait();
	void				DoWait();
	void				DoFly();
	void				PrePareFly();
	void				DoVanish();
	void				DoCollision();
	friend				class KMissleSet;
	void				Release();
	BOOL				CheckCollision(int nBarrierHeight);//检测是否碰撞
	BOOL				CheckBeyondRegion();//检测是否越界
	int					GetDir(int dx,int dy);
public:
	KMissle&			operator=(KMissle& Missle);
	DWORD				GetCurrentSubWorldTime();
	BOOL				ProcessDamage(int nNpcId);
	int					ProcessCollision();//处理碰撞
	BOOL				IsRelationSuitable(NPC_RELATION npc_relation, MISSLE_RELATION msl_relation)
	{
		if (msl_relation == MissleRelation_All)
			return TRUE;
		else if (msl_relation == MissleRelation_AllyOnly && npc_relation == relation_ally)
			return TRUE;
		else if (msl_relation == MissleRelation_EnemyOnly && npc_relation == relation_enemy) 
			return TRUE;
		else
			return FALSE;
		
		return FALSE;
	};
	
public:	
	BOOL				IsMatch(DWORD dwID){return m_dwID == dwID;	};
	BOOL				GetInfoFromTabFile(int nMissleId);
	BOOL				GetInfoFromTabFile(KITabFile * pTabFile, int nMissleId);
	//TEffectData			*m_pEffectData;
#ifdef _SERVER
	KMissleMagicAttribsData * m_pMagicAttribsData;
	int					SetMagicAttribsData(KMissleMagicAttribsData * pData)
	{
		
		m_pMagicAttribsData = pData;
		if (pData)
			return pData->AddRef();
		else 
			return 0;
	};
#endif //_SERVER
	BOOL				Save();
	int					Activate();
	int					ProcessCollision(int nLauncherIdx, int nRegionId, int nMapX, int nMapY, int nRange , MISSLE_RELATION eRelation);
	void				GetMpsPos(int *pPosX, int *pPosY)	;

};
extern CORE_API KMissle			Missle[MAX_MISSLE];
extern CORE_API KMissle			g_MisslesLib[MAX_MISSLESTYLE];//Base 1

#endif
