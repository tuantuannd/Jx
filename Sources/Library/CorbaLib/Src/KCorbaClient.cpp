#include "KCorbaClient.h"

extern CORBA::Object_ptr  GetObjectReference(char * szObjName, char * szObjType, CORBA::ORB_ptr orb);

BOOL KCorbaClient::CreateClient(CORBA::String_var sior)
{
	
	m_ObjVar = m_OrbVar->string_to_object(sior);
	if (m_ObjVar)
		return TRUE;
	else 
		return FALSE;

}


BOOL KCorbaClient::CreateClient(char * ServerName,char * ServerType)
{
	
	m_ObjVar = GetObjectReference(ServerName,ServerType,m_OrbVar);
	
	if (m_ObjVar)
		return TRUE;
	else 
		return FALSE;
	
}