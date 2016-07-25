/**************************************************/
/*                                                */
/*  文件名:    S3PFriendListDAO                */
/*  描述    :  Friend_List表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PFriendListDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PFriendListDAO::S3PFriendListDAO(S3PDBConnection * pConn)
{
	m_pConn=pConn;
}

S3PFriendListDAO::~S3PFriendListDAO()
{

}

S3PDBConnection * S3PFriendListDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PFriendListDAO::GetTableName()
{
	return "Friend_List";
}