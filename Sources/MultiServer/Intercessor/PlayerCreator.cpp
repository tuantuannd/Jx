#include "stdafx.h"
#include "PlayerCreator.h"
#include "inifile.h"
#include "utils.h"
#include "tstring.h"
#include "../../Core/src/GameDatadef.h"

using OnlineGameLib::Win32::CIniFile;
using OnlineGameLib::Win32::GetAppFullPath;
using OnlineGameLib::Win32::_tstring;

CPlayerCreator::CPlayerCreator()
{
	for (int i = 0; i < MAX_PLAYERTYPE_VALUE; i++)
	{
		m_pRoleData[i] = NULL;
	}
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
}

bool CPlayerCreator::Init()
{
	CIniFile theIniFile;

	_tstring sIniFilePathName;

	sIniFilePathName = GetAppFullPath( NULL );

	char	szFileName[32];

	for (int i = 0; i < MAX_PLAYERTYPE_VALUE; i++)
	{
		sprintf(szFileName, PLAYERCREATOR_FILE, i);
		_tstring sIniFileName = sIniFilePathName + szFileName;

		if (!m_pRoleData[i])
			m_pRoleData[i] = new BYTE[MAX_NEWPLAYER_BUFFER];
		ZeroMemory(m_pRoleData[i], MAX_NEWPLAYER_BUFFER * sizeof(BYTE));
		GetRoleDataFromIni(m_pRoleData[i], sIniFileName.c_str());
	}

	return false;
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
		const TRoleData*	pData = (const TRoleData*)m_pRoleData[nIndex];
		uDataLength = pData->dwDataLen;
		return pData;
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
	//玩家信息
	pRoleData->BaseInfo.ileftprop	= cFile.ReadInteger("ROLE", "ileftprop", 0);
	pRoleData->BaseInfo.ileftfight	= cFile.ReadInteger("ROLE", "ileftfight", 0);
	pRoleData->BaseInfo.ipower		= cFile.ReadInteger("ROLE", "ipower", 0);
	pRoleData->BaseInfo.iagility	= cFile.ReadInteger("ROLE", "iagility", 0);
	pRoleData->BaseInfo.iouter		= cFile.ReadInteger("ROLE", "iouter", 0);
	pRoleData->BaseInfo.iinside		= cFile.ReadInteger("ROLE", "iinside", 0);
	pRoleData->BaseInfo.iluck		= cFile.ReadInteger("ROLE", "iluck", 0);

	//玩家的显示信息
	pRoleData->BaseInfo.ihelmres	= cFile.ReadInteger("ROLE", "ihelmres", 0);
	pRoleData->BaseInfo.iarmorres	= cFile.ReadInteger("ROLE", "iarmorres", 0);
	pRoleData->BaseInfo.iweaponres	= cFile.ReadInteger("ROLE", "iweaponres", 0);
	
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

	//登入点
	pRoleData->BaseInfo.irevivalid	= cFile.ReadInteger("ROLE", "irevivalid", 0);
	pRoleData->BaseInfo.irevivalx	= cFile.ReadInteger("ROLE", "irevivalx", 0);
	pRoleData->BaseInfo.irevivaly	= cFile.ReadInteger("ROLE", "irevivaly", 0);
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
	
//Item
	TDBItemData * pItemData = (TDBItemData *)pSkillData;
	int nItemCount = 0;
	nItemCount						= cFile.ReadInteger(SECTION_ITEMLIST, KEY_COUNT, 0);
	
	char szSection[100];
	
	for ( i = 0 ; i < nItemCount; i ++)
	{
		sprintf(szSection, "%s%d", SECTION_ITEM, i + 1 );
		pItemData->cusercode[0] = 0;
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

		pItemData ++;
	}

	pRoleData->dwFriendOffset = (BYTE *)pItemData - pData;
	pRoleData->nItemCount = nItemCount;
	pRoleData->nFriendCount = 0;
	pRoleData->dwDataLen = (BYTE *)pItemData - pData;

	return true;
}