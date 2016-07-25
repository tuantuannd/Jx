#pragma  once 

class KPartnerSkill:public ISkill
{
private:
	unsigned long  m_ulNpcTemplateId;

private:
	unsigned long GetPartnerLevel();

public:
	int				GetSkillId();
	const char *	GetSkillName();
	int				GetSkillStyle();
	void			LoadSkillLevelData(unsigned long  ulLevel, int nParam);
	BOOL			Cast(int nLauncher,  int nParam1, int nParam2);
	BOOL			InitSkill();
	static void		GetDesc(unsigned long ulSkillId, unsigned long ulCurLevel, char * pszMsg, int nOwnerIndex,  bool bGetNextLevelDesc);
}