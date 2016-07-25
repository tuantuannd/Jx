#include "stdafx.h"
#include "PlayerCreator.h"
#include "inifile.h"
#include "utils.h"
#include "tstring.h"
#include "Macro.h"
#include "GameDatadef.h"

using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;
using OnlineGameLib::Win32::ToBool;


CPlayerCreator::CPlayerCreator()
{
	for (int i = 0; i < MAX_PLAYERTYPE_VALUE; i++)
	{
		m_pRoleData[i] = NULL;
	}
	
	Init();
}

CPlayerCreator::~CPlayerCreator()
{
	for (int i = 0; i < MAX_PLAYERTYPE_VALUE; i++)
	{
		if (!m_pRoleData[i])
		{
			delete [] m_pRoleData[i];
			m_pRoleData[i] = NULL;
		}
	}

	stdMapID2RID::iterator it;	
	for ( it = m_theMapID2RID.begin(); it != m_theMapID2RID.end(); it ++ )
	{
		stdRevivalID &SL = ( *it ).second;
		
		SL.clear();
	}
	
	m_theMapID2RID.erase( m_theMapID2RID.begin(), m_theMapID2RID.end() );
}

bool CPlayerCreator::Init()
{
	CIniFile theIniFile;

	_tstring sIniFilePathName;

	sIniFilePathName = GetAppFullPath( NULL );

	char	szFileName[MAX_PATH];

	for (int i = 0; i < MAX_PLAYERTYPE_VALUE; i++)
	{
		sprintf(szFileName, PLAYERCREATOR_FILE, i);
		_tstring sIniFileName = sIniFilePathName + szFileName;

		if (!m_pRoleData[i])
			m_pRoleData[i] = new BYTE[MAX_NEWPLAYER_BUFFER];
		ZeroMemory(m_pRoleData[i], MAX_NEWPLAYER_BUFFER * sizeof(BYTE));
		GetRoleDataFromIni(m_pRoleData[i], sIniFileName.c_str());
	}
	
	_tstring sRevivalFileName = sIniFilePathName + REVIVALID_FILENAME;

	CIniFile	cFile;
	cFile.SetFile( sRevivalFileName.c_str() );

	CIniFile::_VETSTR theIniVetstr;
	cFile.ReadSections( theIniVetstr );

	char szBuffer[64];

	CIniFile::_VETSTR::iterator it;
	for ( it = theIniVetstr.begin(); it != theIniVetstr.end(); it ++ )
	{
		_tstring info = ( *it );

		if ( info.empty() )
		{
			continue;
		}

		int nSection = atoi( info.c_str() );
		int nCount = cFile.ReadInteger( info.c_str(), "Count", 0 );

		if ( nCount > 0 )
		{
			stdRevivalID rid;
			
			for ( int i=0; i<nCount; i++ )
			{
				sprintf( szBuffer, "RevivalId%2.2d", i );
				
				int nID = cFile.ReadInteger( info.c_str(), szBuffer, 0 );
				
				rid.push_back( nID );
			}
			
			m_theMapID2RID.insert( stdMapID2RID::value_type( nSection, rid ) );
		}
	}

	return false;
}

unsigned CPlayerCreator::GetRevivalID( size_t nMapID, UINT nType /*= enumRandom*/ )
{
	stdMapID2RID::iterator it;
	
	if ( m_theMapID2RID.end() != ( it = m_theMapID2RID.find( nMapID ) ) )
	{
		stdRevivalID& sl = ( *it ).second;

		/*
		 * TODO : Don't get the server when it can't carry anyone
		 */
		if ( !sl.empty() )
		{
			int nCount = sl.size();

			ASSERT( nCount > 0 );

			int nIndex = rand() % nCount;
			nIndex = ( nIndex >= 0 && nIndex < nCount ) ? nIndex : 0;

			return sl[nIndex];
		}
	}

	return 0;
}

const TRoleData* CPlayerCreator::GetRoleData(unsigned int &uDataLength, LPROLEPARAM lpParam)
{
	int nIndex = lpParam->nSeries * ROLE_NO + lpParam->nSex;
	if (nIndex < 0 || nIndex >= MAX_PLAYERTYPE_VALUE)
	{
		uDataLength = 0;
		return NULL;
	}

	if (m_pRoleData[nIndex])
	{
		TRoleData *pData = ( TRoleData * )m_pRoleData[nIndex];

		ASSERT( pData );

		strncpy(pData->BaseInfo.szName, lpParam->szName, sizeof(pData->BaseInfo.szName));
        pData->BaseInfo.szName[sizeof(pData->BaseInfo.szName) - 1] = '\0';

		strncpy(pData->BaseInfo.caccname, lpParam->szAccName,  sizeof(pData->BaseInfo.caccname));
        pData->BaseInfo.caccname[sizeof(pData->BaseInfo.caccname) - 1] = '\0';

		pData->BaseInfo.bSex = ( bool )( 0 != lpParam->nSex );
		pData->BaseInfo.ifiveprop = lpParam->nSeries;

		unsigned nNativeID = GetRevivalID( lpParam->nMapID );

		pData->BaseInfo.irevivalid = lpParam->nMapID;
		pData->BaseInfo.irevivalx = nNativeID;
		pData->BaseInfo.irevivaly = 0;
		
		uDataLength = pData->dwDataLen;
		return ( const TRoleData * )pData;
	}

	uDataLength = 0;
	return NULL;
}

bool CPlayerCreator::GetRoleDataFromIni(BYTE* pData, const char* szFileName)
{
	CIniFile	cFile;

	cFile.SetFile( szFileName );

	TRoleData*	pRoleData = (TRoleData *)pData;

	pRoleData->bBaseNeedUpdate = 1;
	pRoleData->dwVersion = 0;
	//玩家信息
	pRoleData->BaseInfo.ileftprop	= cFile.ReadInteger("ROLE", "ileftprop", 0);
	pRoleData->BaseInfo.ileftfight	= cFile.ReadInteger("ROLE", "ileftfight", 0);
	pRoleData->BaseInfo.ipower		= cFile.ReadInteger("ROLE", "ipower", 0);
	pRoleData->BaseInfo.iagility	= cFile.ReadInteger("ROLE", "iagility", 0);
	pRoleData->BaseInfo.iouter		= cFile.ReadInteger("ROLE", "iouter", 0);
	pRoleData->BaseInfo.iinside		= cFile.ReadInteger("ROLE", "iinside", 0);
	pRoleData->BaseInfo.iluck		= cFile.ReadInteger("ROLE", "iluck", 0);

	//玩家的显示信息	-- remark by spe because client ui display changed 2003/07/21
//	pRoleData->BaseInfo.ihelmres	= cFile.ReadInteger("ROLE", "ihelmres", 0);
//	pRoleData->BaseInfo.iarmorres	= cFile.ReadInteger("ROLE", "iarmorres", 0);
//	pRoleData->BaseInfo.iweaponres	= cFile.ReadInteger("ROLE", "iweaponres", 0);
	
	//玩家等级信息
	pRoleData->BaseInfo.fightexp	= cFile.ReadInteger("ROLE", "ifightexp", 0);
	pRoleData->BaseInfo.ifightlevel	= cFile.ReadInteger("ROLE", "ifightlevel", 0);
	pRoleData->BaseInfo.ileadlevel	= cFile.ReadInteger("ROLE", "ileadlevel", 0);
	pRoleData->BaseInfo.ileadexp	= cFile.ReadInteger("ROLE", "ileadexp", 0);

	//门派信息
	pRoleData->BaseInfo.nSect		= cFile.ReadInteger("ROLE", "isect", 0);
	pRoleData->BaseInfo.nFirstSect	= cFile.ReadInteger("ROLE", "ifirstsect", 0);
	pRoleData->BaseInfo.ijoincount	= cFile.ReadInteger("ROLE", "ijoincount", 0);

	//现金和贮物箱中的钱
	pRoleData->BaseInfo.imoney		= cFile.ReadInteger("ROLE", "imoney", 0);
	pRoleData->BaseInfo.isavemoney	= cFile.ReadInteger("ROLE", "isavemoney", 0);
	//

	pRoleData->BaseInfo.ifiveprop	= cFile.ReadInteger("ROLE", "ifiveprop", 0);
	pRoleData->BaseInfo.iteam		= cFile.ReadInteger("ROLE", "iteam", 0);
	pRoleData->BaseInfo.bSex		= ( 0 != cFile.ReadInteger("ROLE", "bsex", 0) );
	pRoleData->BaseInfo.imaxlife	= cFile.ReadInteger("ROLE", "imaxlife", 0);
	pRoleData->BaseInfo.imaxstamina	= cFile.ReadInteger("ROLE", "imaxstamina", 0);
	pRoleData->BaseInfo.imaxinner	= cFile.ReadInteger("ROLE", "imaxinner", 0);
	pRoleData->BaseInfo.icurlife	= pRoleData->BaseInfo.imaxlife;
	pRoleData->BaseInfo.icurinner	= pRoleData->BaseInfo.imaxinner;
	pRoleData->BaseInfo.icurstamina	= pRoleData->BaseInfo.imaxstamina;

	//登入点
	pRoleData->BaseInfo.irevivalid	= cFile.ReadInteger("ROLE", "irevivalid", 0);
	pRoleData->BaseInfo.irevivalx	= cFile.ReadInteger("ROLE", "irevivalx", 0);
	pRoleData->BaseInfo.irevivaly	= cFile.ReadInteger("ROLE", "irevivaly", 0);
	
	pRoleData->BaseInfo.cUseRevive = 1;
	pRoleData->BaseInfo.cFightMode = 0;
	pRoleData->BaseInfo.cIsExchange = 0;
	pRoleData->BaseInfo.ientergameid = 0;
	pRoleData->BaseInfo.ientergamex = 0;
	pRoleData->BaseInfo.ientergamey = 0;

	//PK相关
	pRoleData->BaseInfo.cPkStatus = 1;
	pRoleData->BaseInfo.ipkvalue = 0;
	
	//帮会相关
	pRoleData->BaseInfo.dwTongID = 0;

	pRoleData->dwFSkillOffset = (BYTE * )pRoleData->pBuffer - pData;

//  Skill
	int nSkillCount = 0;
	char szSkillId[100];
	char szSkillLevel[100];
	nSkillCount						= cFile.ReadInteger(SECTION_FIGHTSKILLLIST, KEY_COUNT, 0);
	TDBSkillData *pSkillData		= (TDBSkillData *)(pData + pRoleData->dwFSkillOffset);
	for (int i = 0 ; i < nSkillCount; i ++, pSkillData ++)
	{
		int nSkillId = 0;
		int nSkillLevel = 0;
		sprintf(szSkillId, "%s%d", KEY_FIGHTSKILL, i+1);	
		sprintf(szSkillLevel, "%s%d", KEY_FIGHTSKILLLEVEL, i+1);
		pSkillData->m_nSkillId		= cFile.ReadInteger(SECTION_FIGHTSKILLLIST, szSkillId, 0);
		pSkillData->m_nSkillLevel	= cFile.ReadInteger(SECTION_FIGHTSKILLLIST, szSkillLevel, 0);
	}
	pRoleData->nFightSkillCount		= nSkillCount;
	pRoleData->dwLSkillOffset		= (BYTE *)pSkillData - pData;

	nSkillCount						= cFile.ReadInteger(SECTION_LIFESKILLLIST, KEY_COUNT, 0);
	pSkillData						= (TDBSkillData *)(pData + pRoleData->dwLSkillOffset);
	
int i;
	for (i = 0 ; i < nSkillCount; i ++, pSkillData ++)
	{
		int nSkillId = 0;
		int nSkillLevel = 0;
		sprintf(szSkillId, "%s%d", KEY_FIGHTSKILL, i+1);	
		sprintf(szSkillLevel, "%s%d", KEY_FIGHTSKILLLEVEL, i+1);
		pSkillData->m_nSkillId		= cFile.ReadInteger(SECTION_FIGHTSKILLLIST, szSkillId, 0);
		pSkillData->m_nSkillLevel	= cFile.ReadInteger(SECTION_FIGHTSKILLLIST, szSkillLevel, 0);
	}
	pRoleData->nLiveSkillCount		= nSkillCount;
	pRoleData->dwTaskOffset			= (BYTE*)pSkillData - pData;
	pRoleData->nTaskCount			= 0;
	pRoleData->dwItemOffset			= pRoleData->dwTaskOffset;
	
//Item
	TDBItemData * pItemData = (TDBItemData *)pSkillData;
	int nItemCount = 0;
	nItemCount						= cFile.ReadInteger(SECTION_ITEMLIST, KEY_COUNT, 0);
	
	char szSection[100];
	
	for ( i = 0 ; i < nItemCount; i ++)
	{
		sprintf(szSection, "%s%d", SECTION_ITEM, i + 1 );
		//物件基本信息**************************************************************
		pItemData->iequipclasscode	= cFile.ReadInteger(szSection, "iequipclasscode", 0);
		pItemData->idetailtype		= cFile.ReadInteger(szSection, "idetailtype", 0);
		pItemData->iparticulartype	= cFile.ReadInteger(szSection, "iparticulartype", 0);
		pItemData->ilevel			= cFile.ReadInteger(szSection, "ilevel", 0);
		pItemData->iseries			= cFile.ReadInteger(szSection, "iseries", 0);
		pItemData->ix				= cFile.ReadInteger(szSection, "ix", 0);
		pItemData->iy				= cFile.ReadInteger(szSection, "iy", 0);
		pItemData->ilocal			= cFile.ReadInteger(szSection, "ilocal", 0);
		pItemData->iparam1			= cFile.ReadInteger(szSection, "iparam1", 0);
		pItemData->iparam2			= cFile.ReadInteger(szSection, "iparam2", 0);
		pItemData->iparam3			= cFile.ReadInteger(szSection, "iparam3", 0);
		pItemData->iparam4			= cFile.ReadInteger(szSection, "iparam4", 0);
		pItemData->iparam5			= cFile.ReadInteger(szSection, "iparam5", 0);
		pItemData->iparam6			= cFile.ReadInteger(szSection, "iparam6", 0);
		pItemData->iequipversion	= cFile.ReadInteger(szSection, "iequipversion", 0);
		pItemData->irandseed		= cFile.ReadInteger(szSection, "irandseed", 0);
		pItemData->ilucky			= cFile.ReadInteger(szSection, "ilucky", 0);
		pItemData->idurability		= 0;
		pItemData->iidentify		= 0;

		pItemData ++;
	}

	pRoleData->dwFriendOffset = (BYTE *)pItemData - pData;
	pRoleData->nItemCount = nItemCount;
	pRoleData->nFriendCount = 0;
	pRoleData->dwDataLen = (BYTE *)pItemData - pData;

	return true;
}