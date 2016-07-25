#include "KAssistantServer.h"
#include "iostream.h"
#include "Stdio.h"

/*

CORBA::Long KAssistantServer::ExecuteSeq(const MessageSeq& MsgSeq, const char* DataServerName)
{
	long len = MsgSeq.length();
 
	for(long i = 0; i < len; i ++)
	{
	 Execute( MsgSeq[i] ,DataServerName);
	}
	
	return 1;
}
*/
extern CORBA::Boolean BindObjectToName(CORBA::ORB_ptr orb, CORBA::Object_ptr objref, char * ObjName, char * ObjType);
BOOL KAssistantServer::ServerInit(int argc = 0, char **argv = NULL)
{
	try
	{
	 m_OrbVar  = CORBA::ORB_init(argc, argv, "omniORB3");
     m_ObjVar = m_OrbVar->resolve_initial_references("RootPOA");
     m_PoaVar = PortableServer::POA::_narrow(m_ObjVar);
	}
  
	catch(CORBA::SystemException&) {
    cerr << "Caught CORBA::SystemException6." << endl;
	return FALSE;
  }
  catch(CORBA::Exception&) {
    cerr << "Caught CORBA::Exception." << endl;
	return FALSE;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
	return FALSE;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
	return FALSE;
  }
	return TRUE;
}


BOOL KAssistantServer::CreateServer(char * ServerName ,char * ServerType)
{
	try
	{
	strcpy(m_ServerName, ServerName);
	strcpy(m_ServerType, ServerType);
	
	PortableServer::ObjectId_var ObjId = m_PoaVar->activate_object(this);

    // Obtain a reference to the object, and register it in
    // the naming service.
    m_ObjVar = this->_this();
	
  if (!BindObjectToName(m_OrbVar,m_ObjVar,m_ServerName,m_ServerType))
      return FALSE;
	
    this->_remove_ref();
	
	m_Sior=(m_OrbVar->object_to_string(m_ObjVar));
    


    m_PoaManager = m_PoaVar->the_POAManager();
    m_PoaManager->activate();
	printf("KAssistantServer[%s.%s] Have Started!.........\nsior is:%s\n",m_ServerName,m_ServerType,(char*)m_Sior);

	m_OrbVar->run();
    m_OrbVar->destroy();
	}

	catch(CORBA::SystemException&)  
	{
		cerr << "Caught CORBA::SystemException6." << endl;
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


BOOL KAssistantServer::ShutDownServer()
{
	return TRUE;
}
