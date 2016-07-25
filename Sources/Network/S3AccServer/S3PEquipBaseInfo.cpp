// S3PEquipBaseInfo.cpp: implementation of the S3PEquipBaseInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PEquipBaseInfo.h"
#include "S3PDBConnector.h"
#include "S3PEquipBaseInfoDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipBaseInfo::S3PEquipBaseInfo(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PEquipBaseInfo::S3PEquipBaseInfo()
{
	Init();
}

S3PEquipBaseInfo::~S3PEquipBaseInfo()
{
	Clear();
}

S3PDBConnection	* S3PEquipBaseInfo::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PEquipBaseInfo::GetTableDAO()
{
	return new S3PEquipBaseInfoDAO(m_pConn);
}

std::string S3PEquipBaseInfo::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PEquipBaseInfo::GetMandatoryField()
{
	return "cInfoText";
}
