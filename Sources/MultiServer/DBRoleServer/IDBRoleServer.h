/********************************************************************
	created:	2003/06/03
	file base:	IDBRoleServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_IDBROLESERVER_H__
#define __INCLUDE_IDBROLESERVER_H__

#include "S3DBInterface.h"

BOOL InitDBInterface();

void ReleaseDBInterface();

void *GetRoleInfo( char * pRoleBuffer, char * strUser, int &nBufLen );

int	SaveRoleInfo( char * pRoleBuffer, const char * strUser, BOOL bAutoInsertWhenNoExistUser );

int GetRoleListOfAccount( char * szAccountName, S3DBI_RoleBaseInfo * RoleBaseList, int nMaxCount );

bool DeleteRole( const char * strUser );

#endif // __INCLUDE_IDBROLESERVER_H__