#ifndef KReqServer_H
#define KReqServer_H
#include "ReqServer.h"


class KReqServer :public POA_ReqServer,
public PortableServer::RefCountServantBase
{
public:
	void SendResult(CORBA::Long id, CORBA::Long index, CORBA::Long reserve) ;
	char* GetServerInfo() ;
};

#endif// KReqServer_H