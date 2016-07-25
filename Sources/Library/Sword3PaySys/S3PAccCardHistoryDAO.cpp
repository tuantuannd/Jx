//-----------------------------------------//
//                                         //
//  File		: S3PAccCardHistoryDAO.cpp //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#include "S3PAccCardHistoryDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PAccCardHistoryDAO::S3PAccCardHistoryDAO( S3PDBConnection* pConn )
{
	m_pConn = pConn;
}

S3PAccCardHistoryDAO::~S3PAccCardHistoryDAO()
{

}

std::string S3PAccCardHistoryDAO::GetTableName()
{
	return "AccCard_History";
}

S3PDBConnection* S3PAccCardHistoryDAO::GetConnection()
{
	return m_pConn;
}