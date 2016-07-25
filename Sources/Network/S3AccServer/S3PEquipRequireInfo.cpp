// S3PEquipRequireInfo.cpp: implementation of the S3PEquipRequireInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PEquipRequireInfo.h"
#include "S3PDBConnector.h"
#include "S3PEquipRequireInfoDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipRequireInfo::S3PEquipRequireInfo(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PEquipRequireInfo::S3PEquipRequireInfo()
{
	Init();
}

S3PEquipRequireInfo::~S3PEquipRequireInfo()
{
	Clear();
}

S3PDBConnection	* S3PEquipRequireInfo::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PEquipRequireInfo::GetTableDAO()
{
	return new S3PEquipRequireInfoDAO(m_pConn);
}

std::string S3PEquipRequireInfo::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PEquipRequireInfo::GetMandatoryField()
{
	return "cInfoText";
}
