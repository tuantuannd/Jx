// S3PEquipment.cpp: implementation of the S3PEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PEquipment.h"
#include "S3PDBConnector.h"
#include "S3PEquipmentsDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PEquipment::S3PEquipment(int iid)
{
	m_pEquipRequireInfoList = NULL;
	m_pEquipBaseInfoList = NULL;	
	m_pEquipEfficInfoList = NULL;	
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PEquipment::S3PEquipment()
{
	m_pEquipRequireInfoList = NULL;
	m_pEquipBaseInfoList = NULL;	
	m_pEquipEfficInfoList = NULL;	
	Init();
}

S3PEquipment::~S3PEquipment()
{
	if (m_pEquipRequireInfoList)
		delete m_pEquipRequireInfoList;
	if (m_pEquipBaseInfoList)
		delete m_pEquipBaseInfoList;	
	if (m_pEquipEfficInfoList)
		delete m_pEquipEfficInfoList;
	Clear();
}

S3PDBConnection	* S3PEquipment::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PEquipment::GetTableDAO()
{
	return new S3PEquipmentsDAO(m_pConn);
}

std::string S3PEquipment::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PEquipment::GetMandatoryField()
{
	return "iEquipCode";
}

S3PTableObjList<S3PEquipRequireInfo> * S3PEquipment::GetEquipRequireInfoList()
{
	if (!m_bInit)
		return NULL;

	if (!m_pEquipRequireInfoList)
	{
		m_pEquipRequireInfoList = new S3PTableObjList<S3PEquipRequireInfo>("EquipRequireInfo", "iid", "iMainID", m_properties["iid"]);
	}
	return m_pEquipRequireInfoList;
}

S3PTableObjList<S3PEquipBaseInfo> * S3PEquipment::GetEquipBaseInfoList()
{
	if (!m_bInit)
		return NULL;

	if (!m_pEquipBaseInfoList)
	{
		m_pEquipBaseInfoList = new S3PTableObjList<S3PEquipBaseInfo>("EquipBaseInfo", "iid", "iMainID", m_properties["iid"]);
	}
	return m_pEquipBaseInfoList;
}

S3PTableObjList<S3PEquipEfficInfo> * S3PEquipment::GetEquipEfficInfoList()
{
	if (!m_bInit)
		return NULL;

	if (!m_pEquipEfficInfoList)
	{
		m_pEquipEfficInfoList = new S3PTableObjList<S3PEquipEfficInfo>("EquipEfficInfo", "iid", "iMainID", m_properties["iid"]);
	}
	return m_pEquipEfficInfoList;
}
