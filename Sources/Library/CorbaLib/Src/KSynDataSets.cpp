#include "KSynDataSets.h" 
#include "KSynDataSet.h"


KSynDataSets::KSynDataSets()
{
	for(int i = 0;i<MAXSETSNUM;i++)
		m_Sets[i] = NULL;
}

CORBA::Long KSynDataSets::Lock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const char* ObjName)
{
	
	return SearchSet(MapId,StyleId)->Lock(Id,ObjName);
	
	
}
CORBA::Long KSynDataSets::UnLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long index)
{
	
	return (SearchSet(MapId,StyleId))->UnLock(Id,index);
	
}

CORBA::Any* KSynDataSets::GetDataMember(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId) 
{
	return SearchSet(MapId,StyleId)->GetDataMember(Id,MemberId);
	
}

CORBA::Long KSynDataSets::SetData(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const CORBA::Any& data) 
{
	return SearchSet(MapId,StyleId)->SetData(Id,data);
}
CORBA::Long KSynDataSets::SetDataMember(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data) 
{
	
	return SearchSet(MapId,StyleId)->SetDataMember(Id,MemberId,data);
}
CORBA::Any* KSynDataSets::GetDataWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long& Result, const char* ObjName)
{
	
	return SearchSet(MapId,StyleId)->GetDataWithLock(Id,Result, ObjName);
}

CORBA::Any* KSynDataSets::GetDataMemberWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	return SearchSet(MapId,StyleId)->GetDataMemberWithLock(Id, MemberId, Result, ObjName)	;
	
}

CORBA::Long KSynDataSets::SetDataWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	
	return SearchSet(MapId,StyleId)->SetDataWithLock(Id,data,Result,ObjName);
	
}
CORBA::Long KSynDataSets::SetDataMemberWithLock(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	return	SearchSet(MapId,StyleId)->SetDataMemberWithLock(Id, MemberId, data,Result,ObjName);
	
}
CORBA::Any* KSynDataSets::GetDataWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long& Result, const char* ObjName) 
{
	return  SearchSet(MapId,StyleId)->GetDataWithOnce(Id,Result, ObjName);
}
CORBA::Any* KSynDataSets::GetDataMemberWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, CORBA::Long& Result, const char* ObjName) 
{
	return 	SearchSet(MapId,StyleId)->GetDataMemberWithOnce(Id,MemberId,Result,ObjName);
}
CORBA::Long KSynDataSets::SetDataWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	return  SearchSet(MapId,StyleId)->SetDataWithOnce(Id, data,Result,ObjName);
}
CORBA::Long KSynDataSets::SetDataMemberWithOnce(CORBA::Long MapId, CORBA::Long StyleId, CORBA::Long Id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	return SearchSet(MapId,StyleId)->SetDataMemberWithOnce(Id,MemberId,data,Result,ObjName);
}

