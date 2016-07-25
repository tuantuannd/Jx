// S3PRole.cpp: implementation of the S3PRole class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PRole.h"
#include "S3PDBConnector.h"
#include "S3PRoleInfoDAO.h"
#include "S3PTaskList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PRole::S3PRole()
{
	m_pTaskList = NULL;
	m_pFriendList = NULL;
	m_pFightSkillList = NULL;
	m_pLifeSkillList = NULL;
	m_pEquipmentList = NULL;

	Init();
}

S3PRole::S3PRole(std::string userCode)
{
	m_pTaskList = NULL;
	m_pFriendList = NULL;
	m_pFightSkillList = NULL;
	m_pLifeSkillList = NULL;
	m_pEquipmentList = NULL;

	m_primaryFields["cUserCode"] = userCode;
	Init();
	Load();
}

S3PRole::S3PRole(int iid)
{
	m_pTaskList = NULL;
	char buf[255];
	sprintf(buf, "%d", iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PRole::~S3PRole()
{
	Clear();
	if (m_pTaskList)
	{
		delete m_pTaskList;
	}
	if (m_pFriendList == NULL)
	{
		delete m_pFriendList;
	}
	if (m_pFightSkillList == NULL)
	{
		delete m_pFightSkillList;
	}
	if (m_pLifeSkillList == NULL)
	{
		delete m_pLifeSkillList;
	}
}

S3PDBConnection * S3PRole::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PRole::GetTableDAO()
{
	return new S3PRoleInfoDAO(m_pConn);
}

std::string S3PRole::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PRole::GetMandatoryField()
{
	return "cusercode";
}

S3PTableObjList<S3PTask>* S3PRole::GetTaskList()
{
	if (!m_bInit)
		return NULL;

	if (m_pTaskList==NULL)
	{
		m_pTaskList = new S3PTableObjList<S3PTask>("Task_List", "iid", "cusercode", m_properties["cusercode"]);
	}
	return m_pTaskList;
}

S3PTableObjList<S3PFriend>* S3PRole::GetFriendList()
{
	if (!m_bInit)
		return NULL;

	if (m_pFriendList == NULL)
	{
		m_pFriendList = new S3PTableObjList<S3PFriend>("Friend_List","iid", "cusercode", m_properties["cusercode"]);
	}
	return m_pFriendList;
}

S3PTableObjList<S3PFightSkill>* S3PRole::GetFightSkillList()
{
	if (!m_bInit)
		return NULL;

	if (m_pFightSkillList == NULL)
	{
		m_pFightSkillList = new S3PTableObjList<S3PFightSkill>("FightSkill","iid", "cusercode", m_properties["cusercode"]);
	}
	return m_pFightSkillList;
}

S3PTableObjList<S3PLifeSkill>* S3PRole::GetLifeSkillList()
{
	if (!m_bInit)
		return NULL;

	if (m_pLifeSkillList == NULL)
	{
		m_pLifeSkillList = new S3PTableObjList<S3PLifeSkill>("LifeSkill","iid", "cusercode", m_properties["cusercode"]);
	}
	return m_pLifeSkillList;
}

S3PTableObjList<S3PEquipment> * S3PRole::GetEquipmentList()
{
	if (!m_bInit)
		return NULL;

	if (m_pEquipmentList == NULL)
	{
		m_pEquipmentList = new S3PTableObjList<S3PEquipment>("Equipments","iid", "cusercode", m_properties["cusercode"]);
	}
	return m_pEquipmentList;
}