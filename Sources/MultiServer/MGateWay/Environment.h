/********************************************************************
	created:	2003/04/09
	file base:	Environment
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_ENVIRONMENT_H__
#define __INCLUDE_ENVIRONMENT_H__

#pragma warning( disable:4786 )

#include "tstring.h"

#include "S3DBInterface.h"
#define NEW_PLAYER_KIND		10

using OnlineGameLib::Win32::_tstring;

#include <string>
#include <map>

using namespace std;

typedef map< _tstring, _tstring > _tssmap;
typedef _tssmap::iterator	_tssmap_it;

typedef struct tagEnvironment
{
	long	lnPlayerMaxCount;
	long	lnPrecision;

	_tstring	sLocalName;
	_tstring	sLocalAddress0;
	_tstring	sLocalAddress1;

	_tstring	sApplicationPath;
	
	/*
	 * Draw
	 */
	_tssmap		mShowInfo;

	/*
	 * Network
	 */
	_tstring	sAccountServerIP;
	_tstring	sRoleDBServerIP;

	unsigned short	usAccSvrPort;
	unsigned short	usRoleDBSvrPort;
	unsigned short	usClientOpenPort;
	unsigned short	usGameSvrOpenPort;
	
	/*
	 * Setting
	 */
	TRoleData	theRoleData[NEW_PLAYER_KIND];
	
	/*
	 * Config file
	 */
	char	szConfigFileName[MAX_PATH];
	
}ENVIRONMENT, NEAR *PENVIRONMENT, FAR *LPENVIRONMENT;

extern ENVIRONMENT	g_theEnviroment;

bool LoadEnvironmentVariable( HINSTANCE hParentInst );
bool SaveEnvironmentVariable();

#endif // __INCLUDE_ENVIRONMENT_H__