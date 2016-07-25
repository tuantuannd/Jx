/**************************************************/
/*                                                */
/*  文件名:    S3PEquipRequireInfoDAO             */
/*  描述    :  EquipRequireInfo表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PEquipRequireInfoDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipRequireInfoDAO::S3PEquipRequireInfoDAO(S3PDBConnection *pConn)
{
	m_pConn=pConn;
}

S3PEquipRequireInfoDAO::~S3PEquipRequireInfoDAO()
{

}

S3PDBConnection * S3PEquipRequireInfoDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PEquipRequireInfoDAO::GetTableName()
{
	return "EquipRequireInfo";
}