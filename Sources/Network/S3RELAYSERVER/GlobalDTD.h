#ifndef _GLOBALDTD_H_
#define _GLOBALDTD_H_

typedef char					IBYTE;

#define def_DEFAULT_PORT			7800

#define def_MAX_PATH			260

// Describes databases preferences initialization file
//------>BEGIN
// Sectiong Name
#define def_SECTIONNAMELEN			256
// Key Name
#define def_KEYNAMELEN				256
#define def_SERVERKEYNAME			"Server"
#define def_PORTKEYNAME				"Port"
#define def_DATABASEKEYNAME			"DataBase"
#define def_USERKEYNAME				"User"
#define def_PASSWORDKEYNAME			"PassWord"
//<------END

#define def_UDPSIZE					2048

#endif	// _GLOBALDTD_H_