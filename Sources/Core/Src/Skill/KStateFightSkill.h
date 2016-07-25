#pragma once
#include "KFightSkill.h"

class KStateFightSkill : public KFightSkill
{
public:
	BOOL	Cast(TSkillParam * pSkillParam);
	BOOL	CastInitiativeSkill(TSkillParam * pSkillParam);
	BOOL	CastPassivitySkill(TSkillParam * pSkillParam);
	int		m_nStateSpecialId; //状态性魔法光环id号
	int		GetStateSpecailId(){CheckAndGenSkillData();return m_nStateSpecialId;		};
	BOOL	GetInfoFromTabFile(KITabFile *pSkillsSettingFile, int nRow);
};

