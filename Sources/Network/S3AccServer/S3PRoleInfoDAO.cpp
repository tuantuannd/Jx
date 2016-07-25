/**************************************************/
/*                                                */
/*  文件名:    S3PRoleInfoDAO                */
/*  描述    :  Role_Info表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/22/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PRoleInfoDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PRoleInfoDAO::S3PRoleInfoDAO(S3PDBConnection * pConn)
{
	m_pConn = pConn;
}

S3PRoleInfoDAO::~S3PRoleInfoDAO()
{

}

std::string S3PRoleInfoDAO::GetTableName()
{
	return "Role_Info";
}

S3PDBConnection * S3PRoleInfoDAO::GetConnection()
{
	return m_pConn;
}
