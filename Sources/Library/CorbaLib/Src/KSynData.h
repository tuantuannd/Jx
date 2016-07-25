#ifndef KSynData_H
#define KSynData_H
#include <omniORB3/CORBA.h>

class KSynData 
{
public:
	KSynData(){m_bExist = FALSE;};
	virtual	long SetData(CORBA::Any Data) = 0;
	virtual CORBA::Any  GetData() = 0;
	KSynData* GetDataPtr() {return this;};
	virtual CORBA::Any GetDataMember(long MemberId) = 0;
	virtual long SetDataMember(long MemberId, CORBA::Any Data) = 0;
	
	BOOL m_bExist;
	long m_DataType;
	
};
#endif //KSynData_H