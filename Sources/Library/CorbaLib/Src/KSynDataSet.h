// KSynDataSet.h: interface for the KSynDataSet class.
//
//////////////////////////////////////////////////////////////////////
#ifndef KSYNDATASET_H
#define KSYNDATASET_H

#include "SynDataSet.h"
#include "KSynDataSet.h"
#include "KSynData.h"
#include "KSynLock.h"
#include "KNode.h"
#include "Klist.h"
class DataSeq;

#include "ReqServer.h"
#define MAX_UNITNUM 4000
class LockInfoNode:public KNode
{
public:
	long id;
	long index;
	ReqServer_ptr  pServer;
};

struct TServerNode:public KNode
{
public:
	ReqServer_ptr  pServer;
	long ServerId;
};

class KSynDataSet 
{
public:
	
	KSynDataSet();
	virtual ~KSynDataSet();
	
	
	CORBA::Long Lock(CORBA::Long id, const char* ObjName) ;
	CORBA::Long UnLock(CORBA::Long id, CORBA::Long index) ;
	CORBA::Any* GetData(CORBA::Long id) ;
	KSynData* GetDataPtr(long id){ return m_Sets[id]; };
	CORBA::Any* GetDataMember(CORBA::Long id,CORBA::Long MemberId);
	CORBA::Long SetData(CORBA::Long id, const CORBA::Any& data) ;
	CORBA::Long SetDataMember(CORBA::Long id, CORBA::Long MemberId, const CORBA::Any& data) ;
	CORBA::Any* GetDataWithLock(CORBA::Long id, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Any* GetDataMemberWithLock(CORBA::Long id, CORBA::Long MemberId, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Long SetDataWithLock(CORBA::Long id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Long SetDataMemberWithLock(CORBA::Long id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Any* GetDataWithOnce(CORBA::Long id, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Any* GetDataMemberWithOnce(CORBA::Long id, CORBA::Long MemberId, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Long SetDataWithOnce(CORBA::Long id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
	CORBA::Long SetDataMemberWithOnce(CORBA::Long id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
	
	DataSeq* SaveData();
	
	//存放KSynData数据的指针数组
	KSynData *  m_Sets[MAX_UNITNUM];
	
	//存放KSynLock锁表的数组
	KSynLock m_LockSets[MAX_UNITNUM];
	
	//增加数据至列表中
	BOOL AddData(KSynData * pData, long id);
//	BOOL DelData(id);
	long m_nTopIndex;
	KList m_ReqLockList;//请求锁定列表
	KList m_ServerList;//服务器列表
	
};

#endif //KSYNDATASET_H
