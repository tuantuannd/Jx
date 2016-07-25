// S3PRole.h: interface for the S3PRole class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PROLE_H__76A9489F_EC9F_44EB_8B92_1BA17BD3FD07__INCLUDED_)
#define AFX_S3PROLE_H__76A9489F_EC9F_44EB_8B92_1BA17BD3FD07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"

#include "S3PTask.h"
#include "S3PFriend.h"
#include "S3PFightSkill.h"
#include "S3PLifeSkill.h"
#include "S3PEquipment.h"
#include "S3PTableObjList.h"

class S3PRole : public S3PManipulator  
{
public:
	S3PRole(std::string userCode);
	S3PRole(int iid);
	S3PRole();
	virtual ~S3PRole();

	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();

	S3PTableObjList<S3PTask> * GetTaskList();
	S3PTableObjList<S3PFriend> * GetFriendList();
	S3PTableObjList<S3PFightSkill> * GetFightSkillList();
	S3PTableObjList<S3PLifeSkill> * GetLifeSkillList();
	S3PTableObjList<S3PEquipment> * GetEquipmentList();

protected:
	S3PTableObjList<S3PTask> *m_pTaskList;
	S3PTableObjList<S3PFriend> *m_pFriendList;
	S3PTableObjList<S3PFightSkill> *m_pFightSkillList;
	S3PTableObjList<S3PLifeSkill> *m_pLifeSkillList;
	S3PTableObjList<S3PEquipment> *m_pEquipmentList;
};

#endif // !defined(AFX_S3PROLE_H__76A9489F_EC9F_44EB_8B92_1BA17BD3FD07__INCLUDED_)
