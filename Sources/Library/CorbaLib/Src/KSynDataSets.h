#ifndef KSynDataSets_H
#define KSynDataSets_H
#include "SynDataSets.h"
#include "KSynDataSet.h"

#define MAXSETSNUM 100

class KSynDataSets :public POA_SynDataSets,
public PortableServer::RefCountServantBase
{
public:
	KSynDataSets();
	KSynDataSet * m_Sets[MAXSETSNUM];
	
	
    CORBA::Long Lock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const char* ObjName);
    CORBA::Long UnLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long index);
    inline CORBA::Any* GetData(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id) {return	SearchSet(MapId,StyleId)->GetData(Id);};
	CORBA::Any* GetDataMember(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId) ;
    CORBA::Long SetData(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const CORBA::Any& data) ;
    CORBA::Long SetDataMember(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data) ;
    CORBA::Any* GetDataWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Any* GetDataMemberWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Long SetDataWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Long SetDataMemberWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Any* GetDataWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Any* GetDataMemberWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Long SetDataWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
    CORBA::Long SetDataMemberWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) ;
    virtual KSynDataSet * SearchSet(long MapId, long StyleId) = 0;
    KSynData* GetDataPtr(long MapId, long StyleId, long id ){return SearchSet(MapId,StyleId)->m_Sets[id];};
	virtual DataSeq* SaveDataSets(CORBA::Long MapId, CORBA::Long StyleId) {return SearchSet(MapId,StyleId)->SaveData();};

	
};
#endif