/**************************************************/
/*                                                */
/*  文件名:    S3PEquipBaseInfoDAO                */
/*  描述    :  EquipBaseInfo表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PEquipBaseInfoDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipBaseInfoDAO::S3PEquipBaseInfoDAO(S3PDBConnection *pConn)
{
	m_pConn=pConn;
}

S3PEquipBaseInfoDAO::~S3PEquipBaseInfoDAO()
{

}

S3PDBConnection * S3PEquipBaseInfoDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PEquipBaseInfoDAO::GetTableName()
{
	return "EquipBaseInfo";
}