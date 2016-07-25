#pragma once
#include "KCore.h"
#include "kMissle.h"
#include "KObj.h"
#include "KNpc.h"
#include "SkillDef.h"

class ISkill
{
public:
	//char m_szName[100];
	//unsigned long m_nID;
	//unsigned long m_nLevel;
	//char m_szSkillIcon[80];
	//char m_szSkillDesc[100];
	int					m_nLevel;				//	
	virtual BOOL Cast(TSkillParam * pSkillParam);
	virtual void DrawSkillIcon(int nX, int nY, int nWidth, int nHeight);
	void GetDesc(char * pszSkillDesc, TGameActorInfo nOwner);
	virtual int	 GetSkillStyle();
	virtual const char GetName();
	static	int	 GetSkillIdFromName(char * szSkillName);
	static	int	 Param2PCoordinate(int nLauncher, int nParam1, int nParam2 ,  int *nPX, int *nPY, eSkillLauncherType = SKILL_SLT_Npc);
	virtual GetCost() = 0;
	virtual OnEvent(eSkillEvent Event, int nParam1, int nParam2);
	int					m_nRowIdInSkillTabFile;
	void	SetRowIdInSkillTabFile(int nRowId){ m_nRowIdInSkillTabFile	= nRowId;	};
	int		GetRowIdInSkillTabFile(){ return m_nRowIdInSkillTabFile;};
};


extern ISkill * g_pSkill[MAX_SKILL][MAX_SKILLLEVEL];

extern int	g_CastSkill(TSkillParam * pSkillParam);
extern BOOL	g_GenSkillData(unsigned long ulSkillID, unsigned long ulSkillLevel);
extern ISkill * g_GetSkill(unsigned long ulSkillID, unsigned long ulSkillLevel);