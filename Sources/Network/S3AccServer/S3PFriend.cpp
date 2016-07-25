// S3PFriend.cpp: implementation of the S3PFriend class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PFriend.h"
#include "S3PDBConnector.h"
#include "S3PFriendListDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PFriend::S3PFriend(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PFriend::S3PFriend()
{
	Init();
}

S3PFriend::~S3PFriend()
{
	Clear();
}

S3PDBConnection	* S3PFriend::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PFriend::GetTableDAO()
{
	return new S3PFriendListDAO(m_pConn);
}

std::string S3PFriend::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PFriend::GetMandatoryField()
{
	return "fUserCode";
}
