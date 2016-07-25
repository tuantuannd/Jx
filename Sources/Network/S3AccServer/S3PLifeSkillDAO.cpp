/**************************************************/
/*                                                */
/*  文件名:    S3PLifeSkillDAO                */
/*  描述    :  LifeSkill表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PLifeSkillDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PLifeSkillDAO::S3PLifeSkillDAO(S3PDBConnection *pConn)
{
	m_pConn=pConn;
}

S3PLifeSkillDAO::~S3PLifeSkillDAO()
{

}

S3PDBConnection * S3PLifeSkillDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PLifeSkillDAO::GetTableName()
{
	return "LifeSkill";
}