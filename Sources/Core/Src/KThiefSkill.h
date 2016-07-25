#pragma once
#include "KCore.h"
#include "Skill.h"
#include "KNpc.h"
#include "KPlayer.h"
#define THIEFSKILL_SETTINGFILE "\\SETTINGS\\THIEFSKILL.TXT"	

enum 
{
	thief_redress, //劫富济贫
	thief_showlife,
	thief_showmana,
	thief_showmoney,
	thief_runscript,	//使玩家执行某个脚本
};

//----------------------------------------------------------------------
/*
							劫富济贫
  一、技能系统：
  1、  技能名称：劫富济贫；
  2、  技能作用：有一定几率偷盗别人身上锁带的钱财的Y％，被偷的钱被系统吃掉；
  3、  技能范围：新手无法学习和使用，技能不能升级，白名以外的人之间可以相互作用，不受战斗关系限制；
  4、  技能消耗：使用此技能消耗角色的内力X点；
  5、  技能命中：此技能命中率由主动方与被动方等级差决定，命中率＝30－|Lv1－Lv2|×0.3。
  二、任务系统：
  1、  与乞丐对话，选择是否给乞丐钱；
  2、  乞丐有N％的概率教角色此技能，N应该为一个很小的比例。
  三、系统消息：
  1、  被偷成功：％％％从你身上偷到＃＃＃两银子捐给前线；
  2、  被偷未遂：％％％试图偷你的钱；
  3、  偷窃成功：你成功的从％％％那里偷到＃＃＃两银子捐给前线；
  4、  偷窃未遂：你偷％％％失败。
*/
//----------------------------------------------------------------------
class KThiefSkill :public ISkill
{
public:
	KThiefSkill();
    virtual ~KThiefSkill();
	int				GetSkillId();
	const char *	GetSkillName();
	int				GetSkillStyle();
	void			LoadSkillLevelData(unsigned long  ulLevel, int nParam);
	int				DoSkill(KNpc * pNpc, int nX, int nY);
	int				OnSkill(KNpc * pNpc);
	BOOL			CanCastSkill  (int nLauncher, int &nParam1, int &nParam2)  const ;
	unsigned long	GetDelayPerCast(){return m_ulDelayPerCast;};
	int				Cast(KPlayer * pLauncherPlayer, KPlayer * pTargetPlayer);
	NPCATTRIB		GetSkillCostType()const{return m_nSkillCostType;};
	int				GetSkillCost(void * pParam)const;
	int				GetAttackRadius() const{	return m_ulAttackRadius;};
	
	BOOL			m_bTargetEnemy;
	BOOL			m_bTargetAlly;
	BOOL			m_bTargetObj;
	char			m_szSkillName[50];
	BOOL			m_bUseCostPercent;//使用该技能时，消耗值按百分比计算/或者绝对数值

	BOOL			IsTargetOnly()const{return TRUE;};
	BOOL			IsTargetEnemy()const{return TRUE;};
	BOOL			IsTargetAlly()const{return TRUE;};
	BOOL			IsTargetObj()const{return FALSE;};
	BOOL			LoadSetting(char * szSettingFile);
	
#ifndef _SERVER
	char			m_szSkillIcon[100];
	KRUImage		m_RUIconImage;
	char			m_szSkillDesc[300];//技能的描述
	char			m_szTargetMovie[200];
	char			m_szTargetMovieParam[20];
	char			m_szSkillSound[200];
	void			DrawSkillIcon(int x, int y, int Width, int Height)  ;
	char			m_szSkillSpecialFile[200];
	void			GetDesc(unsigned long ulSkillId, unsigned long ulCurLevel, char * pszMsg, int nOwnerIndex,  bool bGetNextLevelDesc);
#endif
	
	unsigned long	m_ulAttackRadius;
	unsigned long	m_ulDelayPerCast;
	NPCATTRIB		m_nSkillCostType;
	unsigned long	m_nThiefPercent;
	int				m_nCost;
private:
	int				m_nThiefStyle;
	DWORD			m_dwParam1;
	DWORD			m_dwParam2;
#ifdef _SERVER
	int				Cast_Redress(KPlayer * pLauncherPlayer, KPlayer * pTargetPlayer);
	int				Cast_ShowLife(KPlayer * pLauncherPlayer, KPlayer * pTargetPlayer);
	int				Cast_ShowMana(KPlayer * pLauncherPlayer, KPlayer * pTargetPlayer);
	int				Cast_ShowMoney(KPlayer * pLauncherPlayer, KPlayer * pTargetPlayer);
	int				Cast_RunScript(KPlayer * pLauncherPlayer, KPlayer * pTargetPlayer);
#endif
};

