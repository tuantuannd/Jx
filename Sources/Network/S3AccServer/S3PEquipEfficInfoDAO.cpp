/**************************************************/
/*                                                */
/*  文件名:    S3PEquipEfficInfoDAO                */
/*  描述    :  EquipEfficInfo表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PEquipEfficInfoDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipEfficInfoDAO::S3PEquipEfficInfoDAO(S3PDBConnection *pConn)
{
	m_pConn=pConn;
}

S3PEquipEfficInfoDAO::~S3PEquipEfficInfoDAO()
{

}

S3PDBConnection * S3PEquipEfficInfoDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PEquipEfficInfoDAO::GetTableName()
{
	return "EquipEfficInfo";
}