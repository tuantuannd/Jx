#pragma once
#include "Skill.h"
#define MAX_MISSLE_DAMAGEATTRIB 8
class KFightSkill:public ISkill
{
public:
	char				m_szName[32];			//	技能名称
	DWORD				m_nId;					//	技能Id
#ifndef _SERVER
	char				m_szSkillIcon[80];
#endif
	BOOL				m_bIsAura;				//	是否为光环技能
	BOOL				IsAura(){CheckAndGenSkillData(); return m_bIsAura;};
	CLIENTACTION		m_nCharActionId;		//	发这个技能时人物做什么动作
	BOOL				m_bTargetOnly;			//	只有选中目标的时候才能发出
	BOOL				m_bTargetEnemy;			//	技能的目标是否为敌人
	BOOL				m_bTargetAlly;			//	技能的目标是否为同伴
	BOOL				m_bTargetObj;			//	技能的目标是否为物件
	BOOL				m_bTargetSelf;			//	技能的目标是否可以为自已

	NPCATTRIB			m_nSkillCostType;		//	技能使用时所需要花费的内力、体力、精力、金钱的类型
	int					m_nCost;				//	发该技能所需的内力、体力等的消耗
	int					m_nMinTimePerCast;		//	发该技能的最小间阁时间
	BOOL				m_bMustBeHit;			//	是否为必中技能，即对其处理简单化以便降低服务器负荷

	int					m_nAttackRadius;		// 射程
	int					m_nWaitTime;			//	该技能正常情况下真正产生的时间
	KMagicAttrib		m_DamageAttribs[MAX_MISSLE_DAMAGEATTRIB];	//应用到非状态相关的数据
	int					m_nDamageAttribsNum;
	BOOL				m_bClientSend;			//  该技能是否对服务器来说有效
	KMagicAttrib		m_ImmediateAttribs[MAXSKILLLEVELSETTINGNUM];//非状态类数据,立即伤害，技能发的子弹打中对方就立即生效的伤害,不随技能等级变化而改变.
	int					m_nImmediateAttribsNum;
	
	KMagicAttrib		m_StateAttribs[MAXSKILLLEVELSETTINGNUM];     //应用到状态相关的数据
	int					m_nStateAttribsNum;		//个数，最大10
	int					m_nChildSkillNum;	//	同时发射子技能的数量	
	int					m_nSkillTime;			//	该技能的作用时间
	
	int					m_nChildSkillId;		//	技能引发的子技能Id;	//当该技能为基本技能时，这项无用
	int					m_nChildSkillLevel;
	
	int					m_nMaxTimes;			//某些如陷阱魔法，最大同时出现多少个陷阱
	eSKillStyle			m_eSkillStyle;			//  当前的技能类型
		
#ifndef _SERVER
	eSkillLRInfo		m_eLRSkillInfo;			//0 表示左右键皆可，1表示只可以作右键技能，2表示左右键都不可作
	char				m_szPreCastEffectFile[100];
#endif
public:
#ifndef _SERVER
	eSkillLRInfo		GetSkillLRInfo(){CheckAndGenSkillData(); return m_eLRSkillInfo;	};
#endif

	BOOL				IsTargetOnly(){CheckAndGenSkillData();return m_bTargetOnly;};
	BOOL				IsTargetEnemy(){CheckAndGenSkillData();return m_bTargetEnemy;};
	BOOL				IsTargetAlly(){CheckAndGenSkillData();return m_bTargetAlly;};
	BOOL				IsTargetObj(){CheckAndGenSkillData();return m_bTargetObj;};

	BOOL				GetInfoFromTabFile(int nCol);
	BOOL				GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow);
	BOOL				Load(int nCol);
	int					GetAttackRadius() {CheckAndGenSkillData();	return m_nAttackRadius;};

	int					GetSkillCost(){CheckAndGenSkillData();return m_nCost;};
	NPCATTRIB			GetSkillCostType(){CheckAndGenSkillData();return m_nSkillCostType;};
	CLIENTACTION		GetActionType(){CheckAndGenSkillData();return m_nCharActionId;};
	int					GetDelayPerCast(){CheckAndGenSkillData();return m_nMinTimePerCast;	};

	
#ifndef _SERVER
	const char *		GetPreCastEffectFile(){	CheckAndGenSkillData(); return m_szPreCastEffectFile;};
#endif
	

#ifdef _SERVER
	KMissleMagicAttribsData* CreateMissleMagicAttribsData(TGameActorInfo GameActor);
	KMissleMagicAttribsData* CreateMissleMagicAttribsData(TSkillParam * pSkillParam);
#endif 


#ifndef _SERVER
public:
	void				DrawSkillIcon(int x, int y, int Width, int Height);
	KRUImage			m_RUIconImage;
	char				m_szSkillDesc[100];//技能的描述
	void				GetDesc(char * pszSkillDesc, int nOwnerIndex);
#endif
	virtual BOOL	ParseString2MagicAttrib(MAGIC_ATTRIB MagicAttrib, int nValue1, int nValue2, int nValue3);
	virtual void	SetSkillLevelData(int nLevel /* =0*/);
	virtual BOOL	ParseString2MagicAttrib(char * szMagicAttribName, char * szValue);
	inline const char *		MagicAttrib2String(int MagicAttrib);
	void CheckAndGenSkillData() {};
};