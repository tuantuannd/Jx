// KSkills.h: interface for the KSkills class.
//
//////////////////////////////////////////////////////////////////////
#ifndef KSkillsH
#define KSkillsH

#include "KMissle.h"
#include "KWorldMsg.h"
#include "KNpc.h"
#include "KNode.h"
#include "KMissleMagicAttribsData.h"
#include "Skill.h"
#define MAX_MISSLE_DAMAGEATTRIB 8
typedef  KSkill KFightSkill;
class KMissle;
class KSkill;
//Skill[0]无用，每个skill的[0]无用!!注意！
enum
{
	Missle_StartEvent = 1,
	Missle_FlyEvent,
	Missle_CollideEvent,
	Missle_VanishEvent,
};

#ifdef TOOLVERSION
class CORE_API KSkill  :public ISkill
#else
class KSkill :public ISkill
#endif
{
private:
	
public:
	unsigned long		m_ulLevel;

#ifdef _SERVER
	DWORD				m_dwSkillLevelUpScriptID;	//玩家点升级技能等级时触发的脚本ID
	DWORD				m_dwSkillLevelDataScriptId; //该技能升级的变动数据对应脚本ID
#endif

#ifndef _SERVER
	char				m_szSkillIcon[80];
	char				m_szPreCastEffectFile[100];
	char				m_szManPreCastSoundFile[100];
	char				m_szFMPreCastSoundFile[100];
	eSkillLRInfo		m_eLRSkillInfo;			//0 表示左右键皆可，1表示只可以作右键技能，2表示左右键都不可作
#endif
	unsigned short		m_usReqLevel;//		技能需要的最小等级
private:
	int					m_nCharClass;			//	属哪个系的
	eSKillStyle			m_eSkillStyle;			//  当前的技能类型
	int					m_nSkillTime;			//	该技能的作用时间
	BOOL				m_bClientSend;			//  该技能是否对服务器来说有效
	BOOL				m_bHaveLoad;			//	标识该技能是否已经加载，如果没有则加载先
	int					m_nInteruptTypeWhenMove;//子弹的激活是否受发送者的移动而中止
	BOOL				m_bHeelAtParent;	//	当子弹实际激活时，位置根据父当前位置而确定,而不是由产生那刻parent位置决定

	//技能限制项
	CLIENTACTION		m_nCharActionId;		//	发这个技能时人物做什么动作
	int					m_nWaitTime;			//	该技能正常情况下真正产生的时间
	BOOL				m_bIsAura;				//	是否为光环技能
	BOOL				m_bIsPassivity;			//	是否为被动技能
	BOOL				m_bIsMelee;				//	是否为近身攻击，近身攻击时
	BOOL				m_bUseAttackRate;		//	是否考虑命中率
	BOOL				m_bTargetOnly;			//	只有选中目标的时候才能发出
	BOOL				m_bTargetEnemy;			//	技能的目标是否为敌人
	BOOL				m_bTargetAlly;			//	技能的目标是否为同伴
	BOOL				m_bTargetObj;			//	技能的目标是否为物件
	BOOL				m_bTargetSelf;			//	技能的目标是否可以为自已
	int					m_eRelation;
	char				m_szName[32];			//	技能名称
	DWORD				m_nId;					//	技能Id
	BOOL				m_bBaseSkill;			//	是否为最基本技能
	BOOL				m_bByMissle;			//	当由父技能产生时，是否是根据玩家为基点还是以当前的子弹为基点
	BOOL				m_bIsPhysical;			//	是否为物理技能
	int					m_nCost;				//	技能使用时所需要花费的内力、体力、精力、金钱的类型
	NPCATTRIB			m_nSkillCostType;		//	发该技能所需的内力、体力等的消耗
	int					m_nMinTimePerCast;		//	发该技能的最小间阁时间
	int					m_nChildSkillNum;		//	同时发射子技能的数量	
	eMisslesForm		m_eMisslesForm;			//	多个子弹的起始格式
	int					m_nValue1;				//	附加整形数据1
	int					m_nValue2;				//	附加整形数据2
	int					m_nEventSkillLevel;
	KMagicAttrib		m_MissleAttribs[MAXSKILLLEVELSETTINGNUM];//生成子弹的相关数据。不同等级的技能，子弹的相关属性可以变化。
	int					m_nMissleAttribsNum;
	KMagicAttrib		m_DamageAttribs[MAX_MISSLE_DAMAGEATTRIB];	//应用到非状态相关的数据
	int					m_nDamageAttribsNum;
	KMagicAttrib		m_ImmediateAttribs[MAXSKILLLEVELSETTINGNUM];//非状态类数据,立即伤害，技能发的子弹打中对方就立即生效的伤害,不随技能等级变化而改变.
	int					m_nImmediateAttribsNum;
	KMagicAttrib		m_StateAttribs[MAXSKILLLEVELSETTINGNUM];     //应用到状态相关的数据
	int					m_nStateAttribsNum;		//个数，最大10
	eMissleFollowKind   m_eMissleFollowKind;	
	int					m_nFollowKindIndex;		
	DWORD				m_dwFollowKindID;
	int					m_nChildSkillId;		//	技能引发的子技能Id;	//当该技能为基本技能时，这项无用
	int					m_nChildSkillLevel;
	BOOL				m_bFlyingEvent;			//	是否需要在飞行过程消息发生是，调用相关回调函数
	BOOL				m_bStartEvent;			//	是否需要在技能第一次Active时，调用相关回调函数
	BOOL				m_bCollideEvent;		//	是否需要在子技能魔法碰撞时，调用相关回调函数
	BOOL				m_bVanishedEvent;		//	是否需要在子技能消亡时，调用相关的回调函数
	int					m_nFlySkillId;			//	整 个飞行的相关技能
	int					m_nFlyEventTime;		//	每多少帧回调FlyEvent;
	int					m_nStartSkillId;		//	技能刚刚才发出时所引发的事件时，所需要的相关技能id
	int					m_nVanishedSkillId;		//	技能发出的子弹结束时引发的技能Id;
	int					m_nCollideSkillId;		//	技能发出的子弹碰撞到物件引发的技能Id;
	BOOL				m_bMustBeHit;			//	是否为必中技能，即对其处理简单化以便降低服务器负荷
	eMisslesGenerateStyle m_eMisslesGenerateStyle;//同时生成的多个子弹，DoWait的时间顺序	
	int					m_nMisslesGenerateData;	//相关数据
	int					m_nMaxShadowNum;		//	最大保留的残影量
	BOOL				m_bNeedShadow;
	int					m_nAttackRadius;		// 射程
	int					m_nStateSpecialId;		//状态性魔法光环id号
	int					m_nMaxTimes;			//某些如陷阱魔法，最大同时出现多少个陷阱
	
	int					m_nEquiptLimited;		//发技能时对当前装备的限制
	
	int					m_nHorseLimited;		//骑马限制 0表示没任何限制
												//		   1表示不能骑马
												//		   2表示必须骑马
	BOOL				m_bDoHurt;

public:
	KSkill();
	virtual ~KSkill();
	//载入相对于g_OrdinSkillsSetting的列号
	//BOOL				
	BOOL				OnMissleEvent(unsigned short usEvent, KMissle * pMissle) const;
	BOOL				IsTargetOnly()const{return m_bTargetOnly;};
	BOOL				IsTargetEnemy()const{return m_bTargetEnemy;};
	BOOL				IsTargetAlly()const{return m_bTargetAlly;};
	BOOL				IsTargetObj()const{return m_bTargetObj;};
	BOOL				IsNeedShadow()const{	return m_bNeedShadow;};
	int					GetMaxTimes()const{  return m_nMaxTimes;};
	static	int			GetSkillIdFromName(char * szSkillName);
	unsigned short		GetSkillReqLevel()const{	return m_usReqLevel;};

#ifdef _SERVER
	unsigned long		GetSkillLevelUpScriptId()const{return m_dwSkillLevelUpScriptID;};
	KMissleMagicAttribsData* CreateMissleMagicAttribsData(int nLauncher)const;
#endif

#ifndef _SERVER
	eSkillLRInfo		GetSkillLRInfo() const{ return m_eLRSkillInfo;	};
	const char *		GetPreCastEffectFile() const{	 return m_szPreCastEffectFile;};
	const char *		GetPreCastSoundFile(BOOL bIsFeMale) const{ return bIsFeMale? m_szFMPreCastSoundFile: m_szManPreCastSoundFile;};
#endif

	const char * GetSkillName(){return m_szName;};
	int					GetSkillStyle(){ return m_eSkillStyle;};
	BOOL				GetInfoFromTabFile(int nCol);
	BOOL				GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow);
	BOOL				Load(int nCol);
	BOOL				Cast(int nLauncher,   int nParam1, int nParam2, int nWaitTime = 0, eSkillLauncherType eLauncherType = SKILL_SLT_Npc) const;	//发出时调用
	int					GetChildSkillNum(int nLevel)const;
	int					GetAttackRadius() const{	return m_nAttackRadius;};
	int					CalculateDamage()const;
	void				LoadSkillLevelData(unsigned long  ulLevel , int nParam); //从技能升级文件中获得nLevel等级的属性变动，并设置
	friend	class		KMissle;
	int					GetSkillCost(void *)const{return m_nCost;};
	NPCATTRIB			GetSkillCostType()const{return m_nSkillCostType;};
	BOOL				GetItemLimit(int)const;
	CLIENTACTION		GetActionType()const{return m_nCharActionId;};
	int					GetDelayPerCast()const{return m_nMinTimePerCast;	};
	unsigned int		GetMissleGenerateTime(int nNo) const ;		
	int					GetChildSkillNum() const {return m_nChildSkillNum;};
	int					GetChildSkillId() const {return m_nChildSkillId;};
	int					GetChildSkillLevel() const {return m_nChildSkillLevel;};
	
	int					GetSkillId() {return m_nId;};
	void				SetSkillId(int nId) {m_nId = nId;};		
	void				SetSkillLevel(unsigned long ulLevel){m_ulLevel = ulLevel;};

	BOOL				IsAura() const { return m_bIsAura;};
	BOOL				IsPhysical() const {return m_bIsPhysical;};
	int					GetMeleeType() const { return m_eMisslesForm;};
	int					GetStateSpecailId() const {return m_nStateSpecialId;		};
	BOOL				CanCastSkill  (int nLauncher, int &nParam1, int &nParam2)  const ;
	void				PlayPreCastSound  (BOOL bIsFeMale , int nX, int nY) const ;
	KMagicAttrib*		GetDamageAttribs()  { return m_DamageAttribs;};	
	
private:
	void				Vanish(KMissle *) const;
	void				FlyEvent(KMissle *)  const ;
	void				Collidsion(KMissle *) const ;
	BOOL				CastMissles(int nLauncher,	int nParam1, int nParam2, int nWaitTime = 0 , eSkillLauncherType eLauncherType = SKILL_SLT_Npc) const ;
	BOOL				CastInitiativeSkill(int nLauncher, int nParam1, int nParam2, int nWaitTime) const ;
	BOOL				CastPassivitySkill(int nLauncher, int nParam1, int nParam2, int nWaitTime) const ;
	
	int					CastWall					(TOrdinSkillParam * pSkillParam, int nDir, int nRefPX, int nRefPY) const ;
	int					CastLine					(TOrdinSkillParam * pSkillParam, int nDir, int nRefPX, int nRefPY) const ;
	int					CastSpread					(TOrdinSkillParam * pSkillParam, int nDir, int nRefPX, int nRefPY) const ;
	int					CastCircle					(TOrdinSkillParam * pSkillParam, int nDir, int nRefPX, int nRefPY) const ;
	int					CastZone					(TOrdinSkillParam * pSkillParam, int nDir, int nRefPX, int nRefPY) const ;
	int					CastExtractiveLineMissle	(TOrdinSkillParam * pSkillParam, int nDir, int nSrcX,  int nSrcY, int nXOffset, int nYOffset, int nDesX, int nDesY) const ;
	int					Param2PCoordinate			(int nLauncher, int nParam1, int nParam2 , int *npPX, int *npPY, eSkillLauncherType eLauncherType = SKILL_SLT_Npc) const ;

	
	void				CreateMissle(int nLauncher, int ChildSkillId,  int nMissleIndex) const ;
	
	BOOL				ParseString2MagicAttrib(unsigned long ulLevel, char * szMagicAttribName, char * szValue) ;
	inline const char *		MagicAttrib2String(int MagicAttrib) const ;

#ifndef _SERVER
public:
	void				DrawSkillIcon(int x, int y, int Width, int Height);
	KRUImage			m_RUIconImage;
	char				m_szSkillDesc[300];//技能的描述
	static void			GetDesc(unsigned long ulSkillId, unsigned long ulCurLevel, char * pszMsg, int nOwnerIndex,  bool bGetNextLevelDesc);
	void				GetDescAboutLevel(char * pszMsg);
#endif

};

#define MAX_MELEEWEAPON_PARTICULARTYPE_NUM 100
#define MAX_RANGEWEAPON_PARTICULARTYPE_NUM 100
extern int		g_nMeleeWeaponSkill[MAX_MELEEWEAPON_PARTICULARTYPE_NUM];
extern int		g_nRangeWeaponSkill[MAX_RANGEWEAPON_PARTICULARTYPE_NUM];	
extern int		g_nHandSkill;

#endif
