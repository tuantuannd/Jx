// KSynDataSet.cpp: implementation of the KSynDataSet class.
//
//////////////////////////////////////////////////////////////////////

#include "KSynDataSet.h"
#include "KSynDataSets.h"
#include "iostream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//#define printf ;
extern FILE * stream;
extern FILE * stream1;


extern CORBA::ORB_var g_Orb;

CORBA::Object_ptr  GetObjectReference(char * szObjName, char * szObjType, CORBA::ORB_ptr orb);
CORBA::Boolean BindObjectToName(CORBA::ORB_ptr orb, CORBA::Object_ptr objref, char * ObjName, char * ObjType);
KSynDataSet::KSynDataSet()
{
	m_nTopIndex = 0;
	for(long i = 0; i < MAX_UNITNUM; i++)
	{
		
		m_Sets[i] = NULL;
		
	}
	
}

KSynDataSet::~KSynDataSet()
{
	
}


//
CORBA::Long KSynDataSet::Lock(CORBA::Long id, const char* ObjName)
{
	long lockid = 0;
	
	HANDLE hevent;
	char e_name[20];
	
	itoa(id, e_name,10);
	//同时只能有一个线程申请Lock服务。
	hevent = OpenEvent(EVENT_ALL_ACCESS, FALSE, e_name);
	if (!hevent)
		hevent = CreateEvent(NULL,TRUE,TRUE,e_name);
	
	if (!hevent)
		hevent = OpenEvent(EVENT_ALL_ACCESS, FALSE, e_name);
	
	WaitForSingleObject(hevent,INFINITE);
	
	if ((lockid = m_LockSets[id].Lock()) < 0)
	{
		LockInfoNode * pNode = new LockInfoNode;
		pNode->id = id;
		pNode->index =  -1 * lockid;
		pNode->pServer = NULL;
		
		long ServerId = g_FileName2Id((char *)ObjName);
		
		TServerNode *pServerData = NULL; 
		
		
		if(!m_ServerList.IsEmpty())
		{
			//在服务对象列表中查找是否有一致id的服务器对象
			for( pServerData = (TServerNode*) m_ServerList.GetHead(); pServerData != NULL; )
			{
				if (pServerData->ServerId == ServerId)
				{
					
					pNode->pServer = pServerData->pServer;
					break;
					
				}
				pServerData = (TServerNode *) pServerData->GetNext();
			}
			if (!pNode->pServer)
			{
			}
		}
		else 
		{
			
		}
		
		//未找到的话
		if (!pNode->pServer)//Server  is Not Exist ,Then Build a New Server Connection
		{
			char cString[100];
			strcpy(cString,ObjName); 
			char * sName = cString;
			char * sType = strstr(cString, ".");
			sType ++;
			cString[sType - sName - 1] = '\0';
			
			CORBA::Object_ptr obj_ptr = GetObjectReference(sName,sType,g_Orb);
			//	printf("Servername is %s,%s and %s\n",ObjName,sName,sType);
			
			if (obj_ptr)
			{
				TServerNode * pNewServer = new TServerNode;
				pNewServer->pServer = ReqServer::_narrow(obj_ptr);
				pNewServer->ServerId = ServerId;
				pNode->pServer = pNewServer->pServer;
				m_ServerList.AddTail((KNode*) pNewServer);
				
			}
			else
			{
				
				SetEvent(hevent);
				return -100000;
			}
			
		}
		
		m_ReqLockList.AddTail((KNode*)pNode);
		
		
	}
	
	SetEvent(hevent);
	return lockid;
}


CORBA::Long KSynDataSet::UnLock(CORBA::Long id, CORBA::Long index)  
{
	
	HANDLE hevent;
	char e_name[20];
	e_name[0]='U';
	
	itoa(id, e_name+1,10);
	
	
	//同时只能有一个线程申请UnLock服务
	hevent = OpenEvent(EVENT_ALL_ACCESS, FALSE, e_name);
	if (!hevent)
		hevent = CreateEvent(NULL,TRUE,TRUE,e_name);
	WaitForSingleObject(hevent,INFINITE);
	
	
	LockInfoNode *pNode;
	
	
	fflush(stream);
	fflush(stream1);
	
	
	if (m_LockSets[id].UnLock(index) == TRUE)//如果本次解锁成功
	{
		
		
		if(!m_ReqLockList.IsEmpty())
		{
			///在数据锁定请求列表中找到下一个请求相关的对象，并调用通知其。
			
			
			if (m_LockSets[id].GetCurId() > 0)
			{
				for( pNode = (LockInfoNode*) m_ReqLockList.GetHead(); pNode != NULL; )
				{
					if (pNode->id == id && pNode->index == m_LockSets[id].GetCurId()) 
					{
						
						pNode->pServer->SendResult(id, m_LockSets[id].GetCurId(),0);
						
						pNode->Remove();
						SetEvent(hevent);
						return 1;
					}
					
					pNode = (LockInfoNode *) pNode->GetNext();
				}
				
				
				
			}
			
			
		}
		else
		{
			
		}
		
		
		SetEvent(hevent);
		return 1;
		
	}
	else 
	{
		
		SetEvent(hevent);
		return -1;
	}
	SetEvent(hevent);
	
	
}





CORBA::Any* KSynDataSet::GetData(CORBA::Long id) 
{
	CORBA::Any* result = new CORBA::Any;
	*result = m_Sets[id]->GetData();
	return result;
	
}

CORBA::Any* KSynDataSet::GetDataWithLock(CORBA::Long id, CORBA::Long& Result, const char* ObjName) 
{
	
	
	if ((Result = Lock(id,ObjName)) > 0)
	{
		return GetData(id);
	}
	else
	{
		return NULL;
	}
	
}

CORBA::Long KSynDataSet::SetData(CORBA::Long id, const CORBA::Any& data) 
{
	m_Sets[id]->SetData(data);
	return 1;
}

CORBA::Long KSynDataSet::SetDataWithLock(CORBA::Long id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	
	
	if ( (Result = Lock(id,ObjName)) > 0)
	{
		
		SetData(id,data);
		return Result;
	}
	else
		return Result;
	
	
}

CORBA::Long KSynDataSet::SetDataWithOnce(CORBA::Long id, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	
	long nLockId;
	if ((nLockId = Lock(id,ObjName)) > 0)
	{
		SetData(id,data);
		UnLock(id, nLockId);
		Result = nLockId;
		return Result;
	}
	
	else 
	{
		if (nLockId < 0){};
		Result = nLockId;
		return nLockId;
	}
	
}

CORBA::Any* KSynDataSet::GetDataWithOnce(CORBA::Long id, CORBA::Long& Result, const char* ObjName) 
{
	CORBA::Any *result;
	
	if ((Result = Lock(id,ObjName)) > 0)
	{
		result = GetData(id);
		UnLock(id,Result);
		return result;
	}
	else
		return NULL;
	
}





CORBA::Object_ptr  GetObjectReference(char * szObjName, char * szObjType, CORBA::ORB_ptr orb)
{
	CosNaming::NamingContext_var rootContext;
	
	try {
		// Obtain a reference to the root context of the Name service:
		CORBA::Object_var obj;
		obj = orb->resolve_initial_references("NameService");
		
		// Narrow the reference returned.
		rootContext = CosNaming::NamingContext::_narrow(obj);
		if( CORBA::is_nil(rootContext) ) {
			cerr << "Failed to narrow the root naming context." << endl;
			return CORBA::Object::_nil();
		}
	}
	catch(CORBA::ORB::InvalidName& ex) {
		// This should not happen!
		cerr << "Service required is invalid [does not exist]." << endl;
		return CORBA::Object::_nil();
	}
	
	
	// Create a name object, containing the name test/context:
	CosNaming::Name name;
	name.length(2);
	
	name[0].id   = (const char*) "test";       // string copied
	name[0].kind = (const char*) "my_context"; // string copied
	name[1].id   = (const char*) szObjName;//"CorbaPartical";
	name[1].kind = (const char*) szObjType;//"Object";
	// Note on kind: The kind field is used to indicate the type
	// of the object. This is to avoid conventions such as that used
	// by files (name.type -- e.g. test.ps = postscript etc.)
	
	
	try {
		// Resolve the name to an object reference.
		return rootContext->resolve(name);
	}
	catch(CosNaming::NamingContext::NotFound& ex) {
		// This exception is thrown if any of the components of the
		// path [contexts or the object] aren't found:
		cerr << "Context not found." << endl;
	}
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "naming service." << endl;
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught a CORBA::SystemException while using the naming service."
			<< endl;
	}
}



CORBA::Boolean BindObjectToName(CORBA::ORB_ptr orb, CORBA::Object_ptr objref, char * ObjName, char * ObjType)
{
	CosNaming::NamingContext_var rootContext;
	
	try {
		// Obtain a reference to the root context of the Name service:
		CORBA::Object_var obj;
		obj = orb->resolve_initial_references("NameService");
		
		// Narrow the reference returned.
		rootContext = CosNaming::NamingContext::_narrow(obj);
		if( CORBA::is_nil(rootContext) ) {
			cerr << "Failed to narrow the root naming context." << endl;
			return 0;
		}
	}
	catch(CORBA::ORB::InvalidName& ex) {
		// This should not happen!
		cerr << "Service required is invalid [does not exist]." << endl;
		return 0;
	}
	
	
	try {
		// Bind a context called "test" to the root context:
		
		
		CosNaming::Name contextName;
		contextName.length(1);
		contextName[0].id   = (const char*) "test";       // string copied
		contextName[0].kind = (const char*) "my_context"; // string copied
		
		// Note on kind: The kind field is used to indicate the type
		// of the object. This is to avoid conventions such as that used
		// by files (name.type -- e.g. test.ps = postscript etc.)
		
		CosNaming::NamingContext_var testContext;
		try {
			// Bind the context to root.
			testContext = rootContext->bind_new_context(contextName);
		}
		catch(CosNaming::NamingContext::AlreadyBound& ex) {
			// If the context already exists, this exception will be raised.
			// In this case, just resolve the name and assign testContext
			// to the object returned:
			CORBA::Object_var obj;
			obj = rootContext->resolve(contextName);
			testContext = CosNaming::NamingContext::_narrow(obj);
			if( CORBA::is_nil(testContext) ) {
				cerr << "Failed to narrow d context." << endl;
				return 0;
			}
		}
		
		
		// Bind objref with name Echo to the testContext:
		CosNaming::Name objectName;
		objectName.length(1);
		objectName[0].id   = (const char*) ObjName; //"CorbaPartical";   // string copied
		objectName[0].kind = (const char*) ObjType;//"Object"; // string copied
		
		try {
			testContext->bind(objectName, objref);
		}
		catch(CosNaming::NamingContext::AlreadyBound& ex) {
			testContext->rebind(objectName, objref);
		}
		// Note: Using rebind() will overwrite any Object previously bound
		//       to /test/Echo with obj.
		//       Alternatively, bind() can be used, which will raise a
		//       CosNaming::NamingContext::AlreadyBound exception if the name
		//       supplied is already bound to an object.
		
		// Amendment: When using OrbixNames, it is necessary to first try bind
		// and then rebind, as rebind on it's own will throw a NotFoundexception if
		// the Name has not already been bound. [This is incorrect behaviour -
		// it should just bind].
	}
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "naming service." << endl;
		return 0;
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught a CORBA::SystemException while using the naming service."
			<< endl;
		return 0;
	}
	
	return 1;
}


CORBA::Any* KSynDataSet::GetDataMember(CORBA::Long id,CORBA::Long MemberId)
{
	CORBA::Any* result = new CORBA::Any;
	
	*result = m_Sets[id]->GetDataMember(MemberId);
	
	return result;
	
}

CORBA::Long KSynDataSet::SetDataMember(CORBA::Long id, CORBA::Long MemberId, const CORBA::Any& data) 
{
	m_Sets[id]->SetDataMember(MemberId, data);
	
	return 1;
	
}

CORBA::Any* KSynDataSet::GetDataMemberWithLock(CORBA::Long id, CORBA::Long MemberId, CORBA::Long& Result, const char* ObjName) 
{
	if ((Result = Lock(id,ObjName)) > 0)
	{
		return GetDataMember(id,MemberId);
	}
	else
	{
		return NULL;
	}
	
}

CORBA::Long KSynDataSet::SetDataMemberWithLock(CORBA::Long id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	if ( (Result = Lock(id,ObjName)) > 0)
	{
		
		SetDataMember(id,MemberId,data);
		return Result;
	}
	else
		return Result;
}

CORBA::Any* KSynDataSet::GetDataMemberWithOnce(CORBA::Long id, CORBA::Long MemberId, CORBA::Long& Result, const char* ObjName) 
{
	CORBA::Any *result;
	
	if ((Result = Lock(id,ObjName)) > 0)
	{
		result = GetDataMember(id, MemberId);
		UnLock(id,Result);
		return result;
	}
	else
		return NULL;
	
	
}


CORBA::Long KSynDataSet::SetDataMemberWithOnce(CORBA::Long id, CORBA::Long MemberId, const CORBA::Any& data, CORBA::Long& Result, const char* ObjName) 
{
	
	long nLockId;
	if ((nLockId = Lock(id,ObjName)) > 0)
	{
		SetDataMember(id, MemberId, data);
		UnLock(id, nLockId);
		Result = nLockId;
		return Result;
	}
	
	else 
	{
		if (nLockId < 0){};
		
		Result = nLockId;
		return nLockId;
	}
	
	
}

BOOL KSynDataSet::AddData(KSynData * pData, long id)
{
	
	if(m_Sets[id])
	{
		if (m_Sets[id]->m_bExist == FALSE)
		{
			m_Sets[id] = pData;
			m_Sets[id]->m_bExist = TRUE;
			return TRUE;
		}
		else 
			return FALSE;
	}
	else 
	{
		m_Sets[id] = pData;
		m_Sets[id]->m_bExist = TRUE;
		if (id > m_nTopIndex) m_nTopIndex = id;
		return TRUE;
		
		
	}
	
}


DataSeq * KSynDataSet::SaveData()
{
	
	long Len = 0;
	
	for(long j = 0; j< m_nTopIndex ;j++)
	{
		if(m_Sets[j]->m_bExist)
			Len ++;
	}
	DataSeq*  pdataSeq = new DataSeq;
	pdataSeq->length(Len);
	
	for(long i= 0;i<Len;i++)
	{
		if (m_Sets[i]->m_bExist)
		{
			CORBA::Any * pAny;
			pAny = GetData(i);
			
			(*pdataSeq)[i] = *pAny;
		}
	}
	
	return pdataSeq;
}
