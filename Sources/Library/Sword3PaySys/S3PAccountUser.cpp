//------------------------------------------//
//											//
//  File		: S3PAccountUser.cpp		//
//	Author		: Yang Xiaodong				//
//	Modified	: 12/19/2002				//
//											//
//------------------------------------------//

#include "S3PAccountUser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PAccountUserDAO::S3PAccountUserDAO( S3PDBConnection* pConn )
{
	m_pConn = pConn;
}

S3PAccountUserDAO::~S3PAccountUserDAO()
{

}

S3PDBConnection* S3PAccountUserDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PAccountUserDAO::GetTableName()
{
	return "admins";
	return "user";
}