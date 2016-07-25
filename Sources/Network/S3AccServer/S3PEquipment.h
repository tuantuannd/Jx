// S3PEquipment.h: interface for the S3PEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PEQUIPMENT_H__DC0FF1FC_9145_41BD_9446_96B3B4C0DE30__INCLUDED_)
#define AFX_S3PEQUIPMENT_H__DC0FF1FC_9145_41BD_9446_96B3B4C0DE30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PManipulator.h"
#include "S3PTableObjList.h"
#include "S3PEquipRequireInfo.h"
#include "S3PEquipBaseInfo.h"
#include "S3PEquipEfficInfo.h"

class S3PEquipment : public S3PManipulator  
{
public:
	S3PEquipment(int iid);
	virtual ~S3PEquipment();
	S3PEquipment();

public:
	virtual S3PDBConnection	* GetConn();
	virtual S3PTableDAO * GetTableDAO();
	virtual std::string GetAutoIncrementField();
	virtual std::string GetMandatoryField();
	virtual S3PTableObjList<S3PEquipRequireInfo> * GetEquipRequireInfoList();
	virtual S3PTableObjList<S3PEquipBaseInfo> * GetEquipBaseInfoList();
	virtual S3PTableObjList<S3PEquipEfficInfo> * GetEquipEfficInfoList();
protected:
	int m_iid;
	S3PTableObjList<S3PEquipRequireInfo>*  m_pEquipRequireInfoList;
	S3PTableObjList<S3PEquipBaseInfo>*     m_pEquipBaseInfoList;
	S3PTableObjList<S3PEquipEfficInfo>*    m_pEquipEfficInfoList;
};

#endif // !defined(AFX_S3PEQUIPMENT_H__DC0FF1FC_9145_41BD_9446_96B3B4C0DE30__INCLUDED_)
