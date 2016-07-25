/**************************************************/
/*                                                */
/*  文件名:    S3PFightSkillDAO                   */
/*  描述    :  FightSkill表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PFightSkillDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PFightSkillDAO::S3PFightSkillDAO(S3PDBConnection * pConn)
{
	m_pConn = pConn;
}

S3PFightSkillDAO::~S3PFightSkillDAO()
{

}

S3PDBConnection * S3PFightSkillDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PFightSkillDAO::GetTableName()
{
	return "FightSkill";
}