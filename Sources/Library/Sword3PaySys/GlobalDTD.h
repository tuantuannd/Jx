#ifndef _GLOBALDTD_H_
#define _GLOBALDTD_H_

#include "windows.h"
#include "tchar.h"
//typedef char*					LPTSTR;
//typedef const char*				LPCTSTR;

typedef char					IBYTE;

#define def_DEFAULT_PORT			7800

#define def_MAX_PATH			260

// Describes databases preferences initialization file
//------>BEGIN
// Sectiong Name
#define def_SECTIONNAMELEN			256
#define def_CARDSECTIONNAME			_T("card")
#define def_ACCOUNTSECTIONNAME		_T("account")
#define def_ROLESECTIONNAME			_T("role")
// Database Identifier
#define def_CARDDB					1
#define def_ACCOUNTDB				2
#define def_ROLEDB					3
// Key Name
#define def_KEYNAMELEN				256
#define def_SERVERKEYNAME			_T("Server")
#define def_PORTKEYNAME				_T("Port")
#define def_DATABASEKEYNAME			_T("DataBase")
#define def_USERKEYNAME				_T("User")
#define def_PASSWORDKEYNAME			_T("PassWord")
//<------END

#define def_UDPSIZE					2048

#define def_ENABLEPOOLEVENTNAME		_T("EnablePool")
#define def_RELOCKEVENTNAME			_T("EnableRelock")

#endif	// _GLOBALDTD_H_