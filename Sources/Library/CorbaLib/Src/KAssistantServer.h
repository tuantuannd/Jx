#ifndef KAssistantServer_H
#define KAssistantServer_H
#include "AssistantServer.h"
#include "SynDataSets.h"
#include "KEngine.h"


struct TDataServerNode:public KNode
{
public:
	SynDataSets_ptr pDataServer;
	long ServerId;
	char ServerName[30];
};

class KAssistantServer:public POA_AssistantServer,
public PortableServer::RefCountServantBase
{
public:	
	virtual CORBA::Long ExecuteSeq(const MessageSeq& MsgSeq, const char* DataServerName) = 0;
	virtual CORBA::Long Execute(const Message& Msg, const char* DataServerName) =0 ;

	BOOL CreateServer(char * ServerName, char * ServerType);
	BOOL ServerInit(int argc ,char ** argv);
	BOOL ShutDownServer();
	void SetORB(CORBA::ORB_var Orb){m_OrbVar = Orb;};
	void SetPOA(PortableServer::POA_var Poa){m_PoaVar = Poa;	};
	
	
	CORBA::ORB_var m_OrbVar;
	PortableServer::POA_var m_PoaVar;
	CORBA::Object_var m_ObjVar;
	PortableServer::POAManager_var m_PoaManager;
	
	char  m_ServerName[20];
	char  m_ServerType[20];
	
	KList m_DataServerList;
	
	CORBA::String_var m_Sior;//用一个字符串唯一的指向一个Corba Server
	
	
};
#endif