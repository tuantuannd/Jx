//-----------------------------------------//
//                                         //
//  File		: S3PAccountInfoDAO.cpp    //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#include "S3PAccountInfoDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PAccountInfoDAO::S3PAccountInfoDAO( S3PDBConnection * pConn )
{
	m_pConn = pConn;
}

S3PAccountInfoDAO::~S3PAccountInfoDAO()
{

}

std::string S3PAccountInfoDAO::GetTableName()
{
	return "Account_Info";
}

S3PDBConnection * S3PAccountInfoDAO::GetConnection()
{
	return m_pConn;
}