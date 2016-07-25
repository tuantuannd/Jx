#ifndef SKILL_H
#define SKILL_H

#include "SkillDef.h"


class ISkill
{
public:
    virtual ~ISkill() {};
	virtual int				GetSkillId() = 0;
	virtual const char *	GetSkillName() = 0;
	virtual int				GetSkillStyle() =0;
	virtual void			LoadSkillLevelData(unsigned long  ulLevel, int nParam) = 0;
	virtual BOOL			CanCastSkill  (int nLauncher, int &nParam1, int &nParam2)  const = 0;
	virtual NPCATTRIB		GetSkillCostType() const= 0;
	virtual int				GetSkillCost(void *) const = 0;
	virtual BOOL			IsTargetOnly()const = 0;
	virtual BOOL			IsTargetEnemy()const = 0;
	virtual BOOL			IsTargetAlly()const = 0;
	virtual BOOL			IsTargetObj()const = 0;
	virtual int				GetAttackRadius() const = 0;
	virtual BOOL			IsAura()const{return FALSE;	};
	virtual BOOL			IsPhysical()const{return FALSE;};
	virtual unsigned long	GetSkillLevelUpScriptId()const{return 0;} ;
#ifndef _SERVER
	virtual void			DrawSkillIcon(int x, int y, int Width, int Height) = 0; 		
#endif
};

#endif
