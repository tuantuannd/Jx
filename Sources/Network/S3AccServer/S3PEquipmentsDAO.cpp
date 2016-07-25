/**************************************************/
/*                                                */
/*  文件名:    S3PEquipmentsDAO                */
/*  描述    :  Equipments表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PEquipmentsDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipmentsDAO::S3PEquipmentsDAO(S3PDBConnection *pConn)
{
	m_pConn=pConn;
}

S3PEquipmentsDAO::~S3PEquipmentsDAO()
{

}

S3PDBConnection * S3PEquipmentsDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PEquipmentsDAO::GetTableName()
{
	return "Equipments";
}