//-----------------------------------------//
//                                         //
//  File		: S3PServerListDAO.cpp	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#include "S3PServerListDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PServerListDAO::S3PServerListDAO( S3PDBConnection* pConn )
{
	m_pConn = pConn;
}

S3PServerListDAO::~S3PServerListDAO()
{

}

std::string S3PServerListDAO::GetTableName()
{
	return "ServerList";
}

S3PDBConnection* S3PServerListDAO::GetConnection()
{
	return m_pConn;
}