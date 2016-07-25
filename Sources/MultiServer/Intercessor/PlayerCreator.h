#ifndef CPlayerCreatorH
#define	CPlayerCreatorH

#include "S3DBInterface.h"

class CPlayerCreator
{
public:
	
#define	MAX_PLAYERTYPE_VALUE	10
#define	MAX_NEWPLAYER_BUFFER	5000
#define	PLAYERCREATOR_FILE		"settings\\Player\\NewPlayer%02d.ini"

	typedef struct tagRoleGeneratorParam
	{
		char	szName[32];
		int		nSeries;
		int		nSex;
		
	} ROLEPARAM, NEAR *PROLEPARAM, FAR *LPROLEPARAM;

private:

	BYTE*		m_pRoleData[MAX_PLAYERTYPE_VALUE];

public:

	CPlayerCreator();
	~CPlayerCreator();

	bool				Init();
	const TRoleData*	GetRoleData( unsigned int &uDataLength, LPROLEPARAM lpParam );
	
private:

	bool				GetRoleDataFromIni( BYTE* pData, const char* szFileName );

};
#endif