#include "KReqServer.h"
#include "stdio.h"
//#define printf ;

void KReqServer::SendResult(CORBA::Long id, CORBA::Long index, CORBA::Long reserve) 
{
	char e_name[100];
	sprintf(e_name, "%d_%d",id,index);
	HANDLE handle;
	printf("Open Event:%s\n",e_name);
	handle = OpenEvent(EVENT_ALL_ACCESS, FALSE, e_name);
	if (!handle)
		printf("Error To Get The Event Handle:%s\n", e_name);
	SetEvent(handle);
	
}

char * KReqServer::GetServerInfo() 
{
	return NULL;
}
