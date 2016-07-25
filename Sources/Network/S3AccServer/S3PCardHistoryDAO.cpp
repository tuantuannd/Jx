// S3PCardHistoryDAO.cpp: implementation of the S3PCardHistoryDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PCardHistoryDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PCardHistoryDAO::S3PCardHistoryDAO(S3PDBConnection * pConn)
{
	m_pConn = pConn;
}

S3PCardHistoryDAO::~S3PCardHistoryDAO()
{

}

std::string S3PCardHistoryDAO::GetTableName()
{
	return "card_history";
}

S3PDBConnection * S3PCardHistoryDAO::GetConnection()
{
	return m_pConn;
}
