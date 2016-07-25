#include "KDataManage.h"
#include "iostream.h"

KDataManage::KDataManage()
{
	
	
}

KDataManage::~KDataManage()
{
}


long KDataManage::Lock(CORBA::Long MapId, CORBA::Long StyleId, long id)
{

	
	try{
		
		long lockid = 0;
		if ((lockid = m_pCorbaDataSets->Lock(MapId,StyleId,id,m_ServerObjName)) > 0 )
		{
			return lockid;
		}
		else 
		{
			char e_name[100];
			sprintf(e_name,"%d_%d",id,lockid);
			HANDLE hevent;
			hevent = CreateEvent(NULL,TRUE,FALSE,e_name);
			WaitForSingleObject(hevent,INFINITE);
			CloseHandle(hevent);
			return lockid;
		}
		
	}
	
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "object." <<id<< endl;
		
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught CORBA::SystemException1." << endl;
	}
	catch(CORBA::Exception&) {
		cerr << "Caught CORBA::Exception." << endl;
	}
	catch(omniORB::fatalException& fe) {
		cerr << "Caught omniORB::fatalException:" << endl;
		cerr << "  file: " << fe.file() << endl;
		cerr << "  line: " << fe.line() << endl;
		cerr << "  mesg: " << fe.errmsg() << endl;
	}
	catch(...) {
		cerr << "Caught unknown exception." << endl;
	}
	
		return -9999;
}

long KDataManage::UnLock(CORBA::Long MapId, CORBA::Long StyleId, long id, long index)
{
	return m_pCorbaDataSets->UnLock(MapId,StyleId,id,index);
}

CORBA::Any* KDataManage::GetData(CORBA::Long MapId, CORBA::Long StyleId, long id)
{
	return m_pCorbaDataSets->GetData(MapId,StyleId,id);
	
}

long KDataManage::SetData(CORBA::Long MapId, CORBA::Long StyleId, long id, const CORBA::Any& data) 
{
	return m_pCorbaDataSets->SetData(MapId,StyleId,id,data);
	
}
//获得数据，并锁住数据
CORBA::Any* KDataManage::GetDataWithLock(CORBA::Long MapId, CORBA::Long StyleId, long id) 
{
	
	long lockid = 0;
	CORBA::Long Result;
	CORBA::Any * pAnyResult;
	try
	{
		
		pAnyResult  = m_pCorbaDataSets->GetDataWithLock(MapId,StyleId,id,Result,m_ServerObjName);
		if ( Result == 1)
		{
			return pAnyResult;
		}
		else
		{
			char e_name[100];
			sprintf(e_name,"%d_%d",id,-1*Result);
			
			HANDLE hevent;
			hevent = CreateEvent(NULL,TRUE,FALSE,e_name);
			WaitForSingleObject(hevent,INFINITE);
			CloseHandle(hevent);
			return m_pCorbaDataSets->GetData(MapId,StyleId,id);
			
			
			
			
			
		}   
		
		
	}
	
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "object." << endl;
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught CORBA::SystemException2." << endl;
	}
	catch(CORBA::Exception&) {
		cerr << "Caught CORBA::Exception." << endl;
	}
	catch(omniORB::fatalException& fe) {
		cerr << "Caught omniORB::fatalException:" << endl;
		cerr << "  file: " << fe.file() << endl;
		cerr << "  line: " << fe.line() << endl;
		cerr << "  mesg: " << fe.errmsg() << endl;
	}
	catch(...) {
		cerr << "Caught unknown exception." << endl;
	}
		return	NULL;
}

CORBA::Long KDataManage::SetDataWithLock(CORBA::Long MapId, CORBA::Long StyleId,CORBA::Long id, const CORBA::Any& data) 
{
	
	
	long lockid = 0;
	CORBA::Long Result;

	try
	{
		
		m_pCorbaDataSets->SetDataWithLock(MapId,StyleId,id,data ,Result,m_ServerObjName);
		
		if ( Result == 1)
		{
			return 1;
		}
		else
		{
			char e_name[100];
			sprintf(e_name,"%d_%d",id,-1 * Result);
			printf("Add Lock %s\n",e_name);
			
			HANDLE hevent;
			hevent = CreateEvent(NULL,TRUE,FALSE,e_name);
			WaitForSingleObject(hevent,INFINITE);
			
			CloseHandle(hevent);
			m_pCorbaDataSets->SetData(MapId,StyleId,id,data);
			return Result;
			
			
			
			
		}   
		
		
	}
	
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "object." << endl;
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught CORBA::SystemException3." << endl;
	}
	catch(CORBA::Exception&) {
		cerr << "Caught CORBA::Exception." << endl;
	}
	catch(omniORB::fatalException& fe) {
		cerr << "Caught omniORB::fatalException:" << endl;
		cerr << "  file: " << fe.file() << endl;
		cerr << "  line: " << fe.line() << endl;
		cerr << "  mesg: " << fe.errmsg() << endl;
	}
	catch(...) {
		cerr << "Caught unknown exception." << endl;
	}
	
	
	
	
}
CORBA::Any* KDataManage::GetDataWithOnce(CORBA::Long MapId, CORBA::Long StyleId,CORBA::Long id)
{
	long lockid = 0;
	CORBA::Long Result;
	CORBA::Any * pAnyResult;
	try
	{
		
		pAnyResult  = m_pCorbaDataSets->GetDataWithOnce(MapId,StyleId,id,Result,m_ServerObjName);
		if ( Result == 1)
		{
			return pAnyResult;
		}
		else
		{
			char e_name[100];
			sprintf(e_name,"%d_%d",id,-1*Result);
			
			HANDLE hevent;
			hevent = CreateEvent(NULL,TRUE,FALSE,e_name);
			WaitForSingleObject(hevent,INFINITE);
			
			CloseHandle(hevent);
			return m_pCorbaDataSets->GetData(MapId,StyleId,id);
			
			
			
		}   
		
		
	}
	
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "object." << endl;
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught CORBA::SystemException4." << endl;
	}
	catch(CORBA::Exception&) {
		cerr << "Caught CORBA::Exception." << endl;
	}
	catch(omniORB::fatalException& fe) {
		cerr << "Caught omniORB::fatalException:" << endl;
		cerr << "  file: " << fe.file() << endl;
		cerr << "  line: " << fe.line() << endl;
		cerr << "  mesg: " << fe.errmsg() << endl;
	}
	catch(...) {
		cerr << "Caught unknown exception." << endl;
	}
	return	NULL;
}

CORBA::Long KDataManage::SetDataWithOnce(CORBA::Long MapId, CORBA::Long StyleId,CORBA::Long id, const CORBA::Any& data) 
{
	
	long lockid = 0;
	CORBA::Long Result = 999 ;

	try
	{
		
		long tt = m_pCorbaDataSets->SetDataWithOnce(MapId,StyleId,id,data ,Result,m_ServerObjName);
		
		if ( Result == 1)
		{
			return 1;
		}
		else
		{
			char e_name[100];
			sprintf(e_name,"%d_%d",id,-1*Result);
			printf("tt is %d\n",tt);
			printf("Add Lock %s\n",e_name);
			
			HANDLE hevent;
			hevent = CreateEvent(NULL,TRUE,FALSE,e_name);
			WaitForSingleObject(hevent,INFINITE);
			
			CloseHandle(hevent);
			m_pCorbaDataSets->SetData(MapId,StyleId,id,data);
			printf("UnLock %d\n",-1*Result);
			UnLock(MapId, StyleId, id,-1*Result);
			return 1;
			
			
			
		}   
		
		
	}
	
	catch(CORBA::COMM_FAILURE& ex) {
		cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
			<< "object." << endl;
	}
	catch(CORBA::SystemException&) {
		cerr << "Caught CORBA::SystemException5." <<id<<Result<< endl;
	}
	catch(CORBA::Exception&) {
		cerr << "Caught CORBA::Exception." << endl;
	}
	catch(omniORB::fatalException& fe) {
		cerr << "Caught omniORB::fatalException:" << endl;
		cerr << "  file: " << fe.file() << endl;
		cerr << "  line: " << fe.line() << endl;
		cerr << "  mesg: " << fe.errmsg() << endl;
	}
	catch(...) {
		cerr << "Caught unknown exception." << endl;
	}
}
