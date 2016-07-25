// S3PCardInfoDAO.cpp: implementation of the S3PCardInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PCardInfoDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PCardInfoDAO::S3PCardInfoDAO(S3PDBConnection * pConn)
{
	m_pConn = pConn;
}

S3PCardInfoDAO::~S3PCardInfoDAO()
{

}

std::string S3PCardInfoDAO::GetTableName()
{
	return "cardInfo";
}

S3PDBConnection * S3PCardInfoDAO::GetConnection()
{
	return m_pConn;
}
