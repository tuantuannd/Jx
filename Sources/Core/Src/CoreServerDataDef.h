#ifndef CORESERVERDATADEF_H
#define CORESERVERDATADEF_H

#ifndef _STANDALONE 
#include "s3dbinterface.h"
#else
#include "S3DBInterface.h"
#endif

#include "CoreUseNameDef.h"

class KMutex;

struct KCoreConnectInfo
{
	int	nNumPlayer;
};

typedef struct
{
	char				szAccount[32];
	int					nRoleCount;
	S3DBI_RoleBaseInfo	PlayerBaseInfo[MAX_PLAYER_IN_ACCOUNT];
	int					nStep;
	unsigned int		nParam;
	DWORD				dwKey;
} LoginData;

#include "KPlayerDef.h"
#endif
