//-----------------------------------------//
//                                         //
//  File		: S3PGamerIDDAO.cpp		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 3/21/2003                //
//                                         //
//-----------------------------------------//

#include "S3PGamerIDDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PGamerIDDAO::S3PGamerIDDAO( S3PDBConnection * pConn )
{
	m_pConn = pConn;
}

S3PGamerIDDAO::~S3PGamerIDDAO()
{

}

S3PDBConnection* S3PGamerIDDAO::GetConnection()
{
	return m_pConn;
}

std::string S3PGamerIDDAO::GetTableName()
{
	return "gamerid";
}
