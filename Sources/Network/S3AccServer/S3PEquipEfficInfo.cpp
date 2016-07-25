// S3PEquipEfficInfo.cpp: implementation of the S3PEquipEfficInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PEquipEfficInfo.h"
#include "S3PDBConnector.h"
#include "S3PEquipEfficInfoDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipEfficInfo::S3PEquipEfficInfo(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PEquipEfficInfo::S3PEquipEfficInfo()
{
	Init();
}

S3PEquipEfficInfo::~S3PEquipEfficInfo()
{
	Clear();
}

S3PDBConnection	* S3PEquipEfficInfo::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PEquipEfficInfo::GetTableDAO()
{
	return new S3PEquipEfficInfoDAO(m_pConn);
}

std::string S3PEquipEfficInfo::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PEquipEfficInfo::GetMandatoryField()
{
	return "cInfoText";
}
