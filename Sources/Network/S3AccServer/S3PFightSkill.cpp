// S3PFightSkill.cpp: implementation of the S3PFightSkill class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PFightSkill.h"
#include "S3PDBConnector.h"
#include "S3PFightSkillDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PFightSkill::S3PFightSkill()
{
	Init();
}

S3PFightSkill::S3PFightSkill(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PFightSkill::~S3PFightSkill()
{
	Clear();
}


S3PDBConnection	* S3PFightSkill::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PFightSkill::GetTableDAO()
{
	return new S3PFightSkillDAO(m_pConn);
}

std::string S3PFightSkill::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PFightSkill::GetMandatoryField()
{
	return "iFightSkill";
}
