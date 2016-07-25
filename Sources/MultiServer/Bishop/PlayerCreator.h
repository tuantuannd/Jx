#ifndef CPlayerCreatorH
#define	CPlayerCreatorH

#include "S3DBInterface.h"

#include <map>
#include <vector>

#ifndef NAME_LEN
	#define NAME_LEN 32
#endif

class CPlayerCreator
{
public:
	
#define	MAX_PLAYERTYPE_VALUE	10
#define	MAX_NEWPLAYER_BUFFER	5000
#define	PLAYERCREATOR_FILE		"settings\\player\\newplayerini%02d.ini"
#define REVIVALID_FILENAME		"settings\\player\\revivalid.ini"

	typedef struct tagRoleGeneratorParam
	{
		char	szAccName[NAME_LEN];
		char	szName[NAME_LEN];
		int		nSeries;
		int		nSex;
		int		nMapID;
		
	} ROLEPARAM, NEAR *PROLEPARAM, FAR *LPROLEPARAM;

private:

	BYTE*		m_pRoleData[MAX_PLAYERTYPE_VALUE];
	
	typedef std::vector< unsigned >					stdRevivalID;
	typedef std::map< size_t, stdRevivalID >		stdMapID2RID;

	stdMapID2RID			m_theMapID2RID;

public:

	CPlayerCreator();
	~CPlayerCreator();

	enum enumSearchType
	{
		enumRandom
	};
	
	const TRoleData*	GetRoleData( unsigned int &uDataLength, LPROLEPARAM lpParam );
	unsigned			GetRevivalID( size_t nMapID, UINT nType = enumRandom );
	
private:

	bool				Init();

	bool				GetRoleDataFromIni( BYTE* pData, const char* szFileName );

};
#endif