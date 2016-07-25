// S3PLifeSkill.cpp: implementation of the S3PLifeSkill class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PLifeSkill.h"
#include "S3PDBConnector.h"
#include "S3PLifeSkillDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PLifeSkill::S3PLifeSkill(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PLifeSkill::S3PLifeSkill()
{
	Init();
}

S3PLifeSkill::~S3PLifeSkill()
{
	Clear();
}

S3PDBConnection	* S3PLifeSkill::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PLifeSkill::GetTableDAO()
{
	return new S3PLifeSkillDAO(m_pConn);
}

std::string S3PLifeSkill::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PLifeSkill::GetMandatoryField()
{
	return "iLifeSkill";
}