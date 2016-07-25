/**************************************************/
/*                                                */
/*  文件名:    S3PTaskListDAO                */
/*  描述    :  Task_List表的维护类            */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/26/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PTaskListDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTaskListDAO::S3PTaskListDAO(S3PDBConnection * pConn)
{
	m_pConn =pConn;
}

S3PTaskListDAO::~S3PTaskListDAO()
{
}

S3PDBConnection * S3PTaskListDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PTaskListDAO::GetTableName()
{
	return "Task_List";
}