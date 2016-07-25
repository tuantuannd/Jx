#ifndef KCORBADATASERVER_H
#define KCORBADATASERVER_H

#include <omniORB3/CORBA.h>
#include <omniORB3/callDescriptor.h>
#include "KSynDataSets.h"
class KCorbaDataServer :public KSynDataSets
{
public:
	CORBA::ORB_var m_OrbVar;
	PortableServer::POA_var m_PoaVar;
	CORBA::Object_var m_ObjVar;
	PortableServer::POAManager_var m_PoaManager;
	
	KCorbaDataServer():KSynDataSets(){};
	~KCorbaDataServer(){};
	
	BOOL ServerInit(int argc , char **argv );
	
	BOOL StartServer(char *ServerName,char *ServerType);
	BOOL ShutDownServer();
	
	virtual BOOL InitWorldData() = 0;
	virtual BOOL LoadWorldData(char * FileName) = 0;
	
	char  m_ServerName[20];
	char  m_ServerType[20];
	
	CORBA::String_var m_Sior;//用一个字符串唯一的指向一个Corba Server
	
	
};
#endif