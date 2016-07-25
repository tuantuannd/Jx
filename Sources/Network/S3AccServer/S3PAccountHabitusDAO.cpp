//-----------------------------------------//
//                                         //
//  File		: S3PAccountHabitusDAO.cpp //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//

#include "S3PAccountHabitusDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PAccountHabitusDAO::S3PAccountHabitusDAO( S3PDBConnection* pConn )
{
	m_pConn = pConn;
}

S3PAccountHabitusDAO::~S3PAccountHabitusDAO()
{

}

std::string S3PAccountHabitusDAO::GetTableName()
{
	return "Account_Habitus";
}

S3PDBConnection* S3PAccountHabitusDAO::GetConnection()
{
	return m_pConn;
}