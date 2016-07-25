// KSkills.h: interface for the KSkills class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "KMissle.h"
#include "skilldef.h"
#include "KWorldMsg.h"
#include "KNpc.h"
#include "KNode.h"
#include "KMissleMagicAttribsData.h"
#include "KFightSkill.h"


//Skill[0]无用，每个skill的[0]无用!!注意！
#ifdef TOOLVERSION
class CORE_API KMissleSkill  :public KFightSkill
#else
class KMissleSkill	:public KFightSkill
#endif
{
private:
	int (* CastMissleFunc[SKILL_MF_COUNT])(int nSrcPX, int nSrcPY, int nDesPX , int nDesPY, TSkillParam * pSkillParam);
public:
private:
#ifndef _SERVER
	char				m_szSkillIcon[80];
#endif

	eMisslesForm		m_eMisslesForm;			//	多个子弹的起始格式
	//技能限制项
	//BOOL				m_bIsPassivity;			//	是否为被动技能
	BOOL				m_bIsMelee;				//	是否为近身攻击，近身攻击时
	BOOL				m_bIsPhysical;			//	是否为物理技能

	BOOL				m_bUseAttackRate;		//	是否考虑命中率
	
	
	BOOL				m_bBaseSkill;			//	是否为最基本技能
	BOOL				m_bByMissle;			//	当由父技能产生时，是否是根据玩家为基点还是以当前的子弹为基点

	TGameActorInfo		m_RefGameActor;
		
	
	
	BOOL				m_bFlyingEvent;			//	是否需要在飞行过程消息发生是，调用相关回调函数
	BOOL				m_bStartEvent;			//	是否需要在技能第一次Active时，调用相关回调函数
	BOOL				m_bCollideEvent;		//	是否需要在子技能魔法碰撞时，调用相关回调函数
	BOOL				m_bVanishedEvent;		//	是否需要在子技能消亡时，调用相关的回调函数
	
	int					m_nFlySkillId;			//	整个飞行的相关技能
	int					m_nFlyEventTime;		//	每多少帧回调FlyEvent;
	int					m_nStartSkillId;		//	技能刚刚才发出时所引发的事件时，所需要的相关技能id
	int					m_nVanishedSkillId;		//	技能发出的子弹结束时引发的技能Id;
	int					m_nCollideSkillId;		//	技能发出的子弹碰撞到物件引发的技能Id;
	int					m_nEventSkillLevel;
	
		
	int					m_nValue1;				//	附加整形数据1
	int					m_nValue2;				//	附加整形数据2
	
	eMisslesGenerateStyle m_eMisslesGenerateStyle;//同时生成的多个子弹，DoWait的时间顺序	
	int					m_nMisslesGenerateData;	//相关数据
	int					m_nMaxShadowNum;		//	最大保留的残影量
	BOOL				m_bNeedShadow;

	KList				m_MissleLevelUpSettingList;//记录有关该等级子弹升级数据信息	

	KMagicAttrib		m_MissleAttribs[MAXSKILLLEVELSETTINGNUM];//生成子弹的相关数据。不同等级的技能，子弹的相关属性可以变化。
	int					m_nMissleAttribsNum;
	
public:
	KMissleSkill();
	virtual ~KMissleSkill();
	
	
	BOOL ParseString2MagicAttrib(MAGIC_ATTRIB MagicAttrib, int nValue1, int nValue2, int nValue3);
	BOOL				IsNeedShadow(){	CheckAndGenSkillData();return m_bNeedShadow;};
	int					GetMaxTimes(){ CheckAndGenSkillData(); return m_nMaxTimes;};

	BOOL				Cast(TSkillParam * pSkillParam);	//发出时调用
	
	inline	BOOL		IsNpcExist(int nLauncher);
	
	int					GetChildSkillNum(int nLevel);


	friend	class		KMissle;
	
	unsigned int		GetMissleGenerateTime(int nNo);		
	int					GetChildSkillNum(){CheckAndGenSkillData();return m_nChildSkillNum;};
	int					GetChildSkillId(){CheckAndGenSkillData();return m_nChildSkillId;};
	int					GetChildSkillLevel(){CheckAndGenSkillData();return m_nChildSkillLevel;};
	void				SetSkillId(int nId){m_nId = nId;};		

	BOOL				IsPhysical(){CheckAndGenSkillData();return m_bIsPhysical;};
	void				SetSkillLevel(int nLevel){	m_nLevel = nLevel;	};
	
private:
	void				Vanish(int	nMissleId);
	void				FlyEvent(int nMissleId);
	void				Collidsion(int nMissleId);
	void				TrigerMissleEventSkill(int nMissleId, int nSkillId, int nSkillLevel);

	BOOL				CastMissles(TSkillParam *pSkillParam);
	int					CastWall(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastLine(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastSpread(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastCircle(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastAtTarget(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastAtFirer(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastZone(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	int					CastExtractiveLineMissle(int nSrcPX, int nSrcPY, int nRefPX, int nRefPY, TSkillParam * pSkillParam);
	void				CreateMissle(TSkillParam * pSkillParam, int ChildSkillId,  int nMissleIndex);
	BOOL				GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow);
	BOOL				ParseString2MagicAttrib(char * szMagicAttribName, char * szValue);
	
#ifndef		_SERVER
	BOOL				PrepareCast(TSkillParam * pSkillParam, int nX, int nY);
#endif
};

#define MAX_MELEEWEAPON_PARTICULARTYPE_NUM 100
#define MAX_RANGEWEAPON_PARTICULARTYPE_NUM 100
extern int		g_nMeleeWeaponSkill[MAX_MELEEWEAPON_PARTICULARTYPE_NUM];
extern int		g_nRangeWeaponSkill[MAX_RANGEWEAPON_PARTICULARTYPE_NUM];	
extern int		g_nHandSkill;
