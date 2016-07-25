/*******************************************************************************
// FileName			:	S3DBInterface.cpp
// FileAuthor		:	RomanDou
// FileCreateDate	:	2002-10-16 15:32:22
// FileDescription	:	
// Revision Count	:	
*******************************************************************************/
#include "Kstdafx.h"
#include "S3DBInterface.h"
#include "GlobalDTD.h"
#include "s3ptableinfocatch.h"
#include "s3pdbconnectionpool.h"
#include "s3pdbconnector.h"
#include "s3pManipulator.h"
#include "S3pEquipBaseInfo.h"
#include "S3pEquipEfficInfo.h"
#include "s3pEquipRequireInfo.h"
#include "s3pEquipment.h"
#include "s3pFightSkill.h"
#include "s3pFriend.h"
#include "s3pLifeSkill.h"
#include "s3pRole.h"
#include "s3pTask.h"
#include "s3pTaskList.h"
#include "S3PTableObjList.h"
#include "kengine.h"
#include "ktabfileCtrl.h"
#include "KIniFile.h"
#include "s3pRoleList.h"
#include "stdio.h"
#pragma warning(disable:4786)
#define MAX_FRIENDNAMELEN 100

//DBTableKeyMap  g_RoleTableKeyMap;
//DBTableKeyMap  g_ItemTableKeyMap;

S3PManipulator * GetTableFromString(S3_DB_TABLENAME tablename, std::string strUser);
S3PManipulator * GetTableFromID(S3_DB_TABLENAME tablename, int nId);

#ifdef _DEBUG
extern FILE * g_pOutFile;
#endif

S3DBI_API BOOL S3DBI_InitDBInterface()
{
#ifdef _DEBUG
	g_pOutFile = fopen("c:\\swsql.txt", "wa");
#endif
	int iRet = 0;
	TCHAR szINIPath[MAX_PATH+1];
	KPIGetExePath( szINIPath, MAX_PATH );
	_tcscat( szINIPath, "DataBase.ini" );
	
	S3PTableInfoCatch::Instance();
	S3PDBConnectionPool::Init( szINIPath );
	S3PDBConnectionPool::Instance();
	S3PDBConnector::Instance();

	return TRUE;
}

S3DBI_API void S3DBI_ReleaseDBInterface()
{
	S3PDBConnectionPool::ReleaseInstance();
	S3PDBConnector::ReleaseInstance();
	S3PTableInfoCatch::Release();
}

BOOL Cav2TableFile(ColumnAndValue *pCav, KTabFileCtrl * pTabFile)
{
	if ((!pCav) || (!pTabFile)) return FALSE;
	if (pCav->size() == 0) return FALSE;
	
	//	pTabFile->Clear();
	
	int nt = 1;
	ColumnAndValue::iterator i;
	for(i=pCav->begin(); i!=pCav->end(); i++)
	{
		std::string key   = i->first;
		std::string value = i->second;
		
		pTabFile->WriteString(1, nt, (char *)key.c_str() );
		pTabFile->WriteString(2, nt++, (char *)value.c_str());
	}
	return TRUE;
}

BOOL TabFile2Cav(KTabFileCtrl * pTabFile, ColumnAndValue * pCav)
{
	if ((!pCav) || (!pTabFile)) return FALSE;
	
	int nRow = pTabFile->GetHeight();
	
	int nWidth = pTabFile->GetWidth();
	for (int i =  0; i < nWidth; i ++)
	{
		char szValue[200];
		pTabFile->GetString(nRow, i + 1 , "", szValue, 200);
		(*pCav)[pTabFile->GetColName(i + 1)] = szValue;
		
	}
	return TRUE;
}

/*
[ROLE]
Name= DOUHAO
OLD= SJDLJFSDJF

  [ITEMS]
  COUNT=10
  
	[ITEM1]
	...
	...
	
	  [ITEMN]
	  BLISTS=1
	  ELISTS=3
	  RLISTS=4
	  
		[IBN_1]
		..
		[IBN_N]
		
		  
			[FRIENDS]
			COUNT=10
			F1=
			F2=
			F3=
			
			  [TASKS]
			  T1=
			  TV1=
			  T2=
			  TV2=
			  
				[FSKILLS]
				COUNT=10
				S1=
				L1=
				S2=
				L2
				
*/

void Item_BaseDataConversion(IN TDBItemData * pItemData, ColumnAndValue &ItemValue)
{
	
	pItemData->iid			= atoi(	ItemValue["iid"].c_str());				  
//	strcpy(pItemData->cusercode,   ItemValue["cusercode"].c_str());             
	pItemData->iequipclasscode = atoi(   ItemValue["iequipclasscode"].c_str());       
	pItemData->ilocal = atoi( ItemValue["ilocal"].c_str());                
	pItemData->ix = atoi(   ItemValue["ix"].c_str());                    
	pItemData->iy = atoi(   ItemValue["iy"].c_str());                    
	pItemData->iequipcode = atoi(   ItemValue["iequipcode"].c_str());            
	pItemData->idetailtype = atoi(   ItemValue["idetailtype"].c_str());           
	pItemData->iparticulartype = atoi(   ItemValue["iparticulartype"].c_str());       
	pItemData->ilevel = atoi(   ItemValue["ilevel"].c_str());                
	pItemData->iseries = atoi(   ItemValue["iseries"].c_str());               
	pItemData->iequipversion = atoi(   ItemValue["iequipversion"].c_str());         
	pItemData->irandseed = atoi(   ItemValue["irandseed"].c_str());             
	pItemData->iparam2 = atoi(   ItemValue["iparam2"].c_str());               
	pItemData->iparam3 = atoi(   ItemValue["iparam3"].c_str());               
	pItemData->iparam5 = atoi(   ItemValue["iparam5"].c_str());               
	pItemData->iparam4 = atoi(   ItemValue["iparam4"].c_str());               
	pItemData->iparam6 = atoi(   ItemValue["iparam6"].c_str());               
	pItemData->iparam1 = atoi(   ItemValue["iparam1"].c_str());               
	pItemData->ilucky = atoi(   ItemValue["ilucky"].c_str());                
}

void Role_BaseDataConversion(IN TRoleBaseInfo * pRoleData, IN ColumnAndValue &RoleValue)
{
		pRoleData->dwId		=		atoi(RoleValue["iid"].c_str());				
		strcpy(pRoleData->szName,		RoleValue["cusercode"].c_str());			
		const char * pSex = RoleValue["bsex"].c_str();
		if (!strcmp(pSex , "男")) pRoleData->bSex = 0;
		else pRoleData->bSex = 1;
		//pRoleData->bSex =		atoi(RoleValue["bsex"].c_str());				
		strcpy(pRoleData->szAlias, RoleValue["calias"].c_str());				
		pRoleData->nFirstSect =		atoi(RoleValue["ifirstsect"].c_str());			
		pRoleData->nSect =		atoi(RoleValue["isect"].c_str());				
		pRoleData->ijoincount =		atoi(RoleValue["ijoincount"].c_str());			
		pRoleData->isectrole =		atoi(RoleValue["isectrole"].c_str());			
		pRoleData->igrouprole =		atoi(RoleValue["igroupcode"].c_str());			
		pRoleData->igrouprole =		atoi(RoleValue["igrouprole"].c_str());			
		pRoleData->irevivalid =		atoi(RoleValue["irevivalid"].c_str());			
		strcpy(pRoleData->cpartnercode , RoleValue["cpartnercode"].c_str());		
		pRoleData->isavemoney =		atoi(RoleValue["isavemoney"].c_str());			
		pRoleData->imoney =		atoi(RoleValue["imoney"].c_str());				
		pRoleData->ifiveprop =		atoi(RoleValue["ifiveprop"].c_str());			
		pRoleData->iteam =		atoi(RoleValue["iteam"].c_str());				
		pRoleData->ifightlevel =		atoi(RoleValue["ifightlevel"].c_str());		
		pRoleData->fightexp =		atoi(RoleValue["ifightexp"].c_str());			
		pRoleData->ileadlevel =		atoi(RoleValue["ileadlevel"].c_str());			
		pRoleData->ileadexp =		atoi(RoleValue["ileadexp"].c_str());			
		pRoleData->iliveexp =		atoi(RoleValue["iliveexp"].c_str());			
		pRoleData->ipower =		atoi(RoleValue["ipower"].c_str());				
		pRoleData->iagility =		atoi(RoleValue["iagility"].c_str());			
		pRoleData->iouter =		atoi(RoleValue["iouter"].c_str());				
		pRoleData->iinside =		atoi(RoleValue["iinside"].c_str());			
		pRoleData->iluck =		atoi(RoleValue["iluck"].c_str());				
		pRoleData->imaxlife =		atoi(RoleValue["imaxlife"].c_str());			
		pRoleData->imaxstamina =		atoi(RoleValue["imaxstamina"].c_str());		
		pRoleData->imaxinner =		atoi(RoleValue["imaxinner"].c_str());			
		pRoleData->ileftprop =		atoi(RoleValue["ileftprop"].c_str());			
		pRoleData->ileftfight =		atoi(RoleValue["ileftfight"].c_str());			
		pRoleData->ileftlife =		atoi(RoleValue["ileftlife"].c_str());			
		strcpy(pRoleData->caccname ,RoleValue["caccname"].c_str());			
		pRoleData->ifinishgame =		atoi(RoleValue["ifinishgame"].c_str());		
		pRoleData->irevivalx =		atoi(RoleValue["irevivalx"].c_str());			
		pRoleData->irevivaly =		atoi(RoleValue["irevivaly"].c_str());			
		pRoleData->iarmorres =		atoi(RoleValue["iarmorres"].c_str());			
		pRoleData->iweaponres =		atoi(RoleValue["iweaponres"].c_str());			
		pRoleData->ihelmres =		atoi(RoleValue["ihelmres"].c_str());			
}

S3DBI_API void *	S3DBI_GetRoleInfo(IN OUT BYTE * pRoleBuffer, IN char * strUser, IN OUT int &nBufLen)
{
	
	int	 nFriendBufferLen;
	TRoleData*	pRoleData = (TRoleData*) pRoleBuffer;
	
	//*************************************************
	// 加载玩家的基本信息//
	S3PRole Role(strUser);
	if (Role.GetLastResult()<0) return NULL;
	ColumnAndValue &RoleInfo  = Role.GetProperties();
	pRoleData->bBaseNeedUpdate = 1;
	Role_BaseDataConversion((TRoleBaseInfo*)pRoleBuffer,RoleInfo);
	
	//*************************************************


	pRoleData->dwFSkillOffset = (BYTE*)&pRoleData->pBuffer - (BYTE*)pRoleData;
	//*************************************************
	// 加载玩家的战斗技能列表
	S3PTableObjList<S3PFightSkill> * pFightSkillList = Role.GetFightSkillList();
	
	int nFightSkillCount =  pFightSkillList->Size();
	pRoleData->nFightSkillCount = nFightSkillCount;
	int nFightSkillBufferLen = nFightSkillCount * sizeof(TDBSkillData);
	BYTE * pFightSkillBuffer = (BYTE*)pRoleData + pRoleData->dwFSkillOffset;
	TDBSkillData * pFSkillData = (TDBSkillData * )pFightSkillBuffer;
	for (int t = 0; t < nFightSkillCount; t++, pFSkillData ++)
	{	
		ColumnAndValue &FightSkillValue = ((*pFightSkillList)[t])->GetProperties();
		pFSkillData->m_nSkillId			= atoi(FightSkillValue["ifightskill"].c_str());
		pFSkillData->m_nSkillLevel		= atoi(FightSkillValue["ifightskilllevel"].c_str());
	}
	//*************************************************
	
	
	pRoleData->dwLSkillOffset = pRoleData->dwFSkillOffset + nFightSkillBufferLen;
	//*************************************************
	// 加载玩家的生活技能列表
	S3PTableObjList<S3PLifeSkill> * pLifeSkillList = Role.GetLifeSkillList();
	int nLifeSkillCount =  pLifeSkillList->Size();
	pRoleData->nLiveSkillCount = nLifeSkillCount;
	int nLifeSkillBufferLen = nLifeSkillCount * sizeof(TDBSkillData);
	BYTE * pLifeSkillBuffer = (BYTE*)pRoleData + pRoleData->dwLSkillOffset;
	TDBSkillData * pLSkillData = (TDBSkillData * )pLifeSkillBuffer;
	for (t = 0; t < nLifeSkillCount; t++, pLSkillData ++)
	{
		ColumnAndValue &LifeSkillValue = ((*pLifeSkillList)[t])->GetProperties();
		pLSkillData->m_nSkillId			= atoi(LifeSkillValue["ilifeskill"].c_str());
		pLSkillData->m_nSkillLevel		= atoi(LifeSkillValue["ilifeskilllevel"].c_str());
	}
	//*************************************************


	pRoleData->dwTaskOffset = pRoleData->dwLSkillOffset + nLifeSkillBufferLen;
	//*************************************************
	// 加载玩家的任务列表
	S3PTableObjList<S3PTask> * pTaskList = Role.GetTaskList();
	int nTaskCount = pTaskList->Size();
	pRoleData->nTaskCount = nTaskCount;
	int nTaskBufferLen = nTaskCount * sizeof(TDBTaskData);
	TDBTaskData * pTaskData = (TDBTaskData*)((BYTE *)pRoleData + pRoleData->dwTaskOffset);
	for (t= 0; t < nTaskCount; t++, pTaskData ++)
	{
		ColumnAndValue &TaskValue	= (*pTaskList)[t]->GetProperties();
		pTaskData->m_nTaskId		= atoi(TaskValue["itaskcode"].c_str());
		pTaskData->m_nTaskValue		= atoi(TaskValue["idegree"].c_str());
	}
	//*************************************************

	
	pRoleData->dwItemOffset = pRoleData->dwTaskOffset + nTaskBufferLen;
	//*************************************************
	// 加载玩家的物品列表
	S3PTableObjList<S3PEquipment> *EquipList =  Role.GetEquipmentList();
	
	int nEquiptListCount = EquipList->Size();
	pRoleData->nItemCount = nEquiptListCount;
	TDBItemData * pItemData = (TDBItemData*)((BYTE*)pRoleData + pRoleData->dwItemOffset);
	KMagicAttrib * pMagicAttrib = NULL;
	//
	for (int j = 0 ; j < nEquiptListCount; j ++)
	{
		
		//基本信息加载
		ColumnAndValue &BaseInfoValue = (*EquipList)[j]->GetProperties();
		Item_BaseDataConversion(pItemData, BaseInfoValue);
		
#ifdef SAVE_ITEM_MAGICATTRIB_TO_DB
		//基本属性加载
		S3PTableObjList<S3PEquipBaseInfo> *EquipBaseInfoList = (*EquipList)[j]->GetEquipBaseInfoList();
		int nEquipBaseInfoListCount = EquipBaseInfoList->Size();
		pItemData->BaseInfoCount = nEquipBaseInfoListCount;
		pMagicAttrib = (KMagicAttrib*) &pItemData->pBuffer;
		for (int k = 0; k < nEquipBaseInfoListCount; k ++, pMagicAttrib ++)
		{
			ColumnAndValue &BaseValue = (*EquipBaseInfoList)[k]->GetProperties();
			pMagicAttrib->nAttribType = atoi(BaseValue["iclass"].c_str());
			pMagicAttrib->nValue[0] = atoi(BaseValue["ivalue1"].c_str());
			pMagicAttrib->nValue[1] = atoi(BaseValue["ivalue2"].c_str());
			pMagicAttrib->nValue[2] = atoi(BaseValue["ivalue3"].c_str());
		}

		//	效果数据加载
		S3PTableObjList<S3PEquipEfficInfo> *EquipEfficInfoList = (*EquipList)[j]->GetEquipEfficInfoList();
		int nEquipEfficInfoListCount = EquipEfficInfoList->Size();
		pItemData->EfficCount = nEquipEfficInfoListCount;
		
		
		for ( k = 0; k < nEquipEfficInfoListCount; k ++, pMagicAttrib ++)
		{
			ColumnAndValue &EfficValue = (*EquipEfficInfoList)[k]->GetProperties();
			pMagicAttrib->nAttribType = atoi(EfficValue["iclass"].c_str());
			pMagicAttrib->nValue[0] = atoi(EfficValue["ivalue1"].c_str());
			pMagicAttrib->nValue[1] = atoi(EfficValue["ivalue2"].c_str());
			pMagicAttrib->nValue[2] = atoi(EfficValue["ivalue3"].c_str());
		}
		//需求属性加载
		S3PTableObjList<S3PEquipRequireInfo> *EquipRequireInfoList = (*EquipList)[j]->GetEquipRequireInfoList();
		int nEquipRequireInfoListCount = EquipRequireInfoList->Size();
		pItemData->RequireInfoCount = nEquipRequireInfoListCount;
		
		for (k = 0; k < nEquipRequireInfoListCount; k ++, pMagicAttrib ++)
		{
			ColumnAndValue &RequireValue = (*EquipRequireInfoList)[k]->GetProperties();
			pMagicAttrib->nAttribType = atoi(RequireValue["iclass"].c_str());
			pMagicAttrib->nValue[0] = atoi(RequireValue["ivalue1"].c_str());
			pMagicAttrib->nValue[1] = atoi(RequireValue["ivalue2"].c_str());
			pMagicAttrib->nValue[2] = atoi(RequireValue["ivalue3"].c_str());
		}
		pItemData = (TDBItemData *)pMagicAttrib;
#else
		pItemData ++;
#endif
	}
	//*************************************************	
	
	
	//*************************************************
	// 加载玩家的好友列表//
	// 玩家的姓名列表放在唯一的一块Buffer上，通过0标计间隔开
	if (pItemData)
		pRoleData->dwFriendOffset = (BYTE*)pItemData - (BYTE*)pRoleData;
	else
		pRoleData->dwFriendOffset = pRoleData->dwItemOffset;
	S3PTableObjList<S3PFriend> * pFriendList = Role.GetFriendList();
	int nFriendCount =  pFriendList->Size();
	pRoleData->nFriendCount = nFriendCount;

	void * pFriendBuffer = (BYTE *)pRoleData + pRoleData->dwFriendOffset;
	BYTE * pCurFriend = (BYTE*)pFriendBuffer;
	
	for (t = 0; t < nFriendCount; t++)
	{
		ColumnAndValue &FriendValue = ((*pFriendList)[t])->GetProperties();
		const char * pFriendName = FriendValue["cfriendcode"].c_str();
		int nCurNameLen = strlen(pFriendName);
		memcpy(pCurFriend, pFriendName, nCurNameLen);
		pCurFriend = (BYTE *)((char *)pCurFriend + nCurNameLen);
		*(pCurFriend++) = 0;
	}
	nFriendBufferLen = pCurFriend - (BYTE*)pFriendBuffer;
	//*************************************************
	
	nBufLen = pRoleData->dwFriendOffset + nFriendBufferLen;
	return pRoleBuffer;
}

void	Item_SaveBaseInfo(ColumnAndValue &ItemValue, TDBItemData * pItemData)
{
	char szValue[100];
	//ItemValue["iid"]				   = itoa(pItemData->iid, szValue, 10);
   //ItemValue["cusercode"]              = pItemData->cusercode;
   ItemValue["iequipclasscode"]        = itoa(pItemData->iequipclasscode, szValue, 10);
   ItemValue["ilocal"]                 = itoa(pItemData->ilocal, szValue, 10);
   ItemValue["ix"]                     = itoa(pItemData->ix, szValue, 10);
   ItemValue["iy"]                     = itoa(pItemData->iy, szValue, 10);
   ItemValue["iequipcode"]             = itoa(pItemData->iequipcode, szValue, 10);
   ItemValue["idetailtype"]            = itoa(pItemData->idetailtype, szValue, 10);
   ItemValue["iparticulartype"]        = itoa(pItemData->iparticulartype, szValue, 10);
   ItemValue["ilevel"]                 = itoa(pItemData->ilevel, szValue, 10);
   ItemValue["iseries"]                = itoa(pItemData->iseries, szValue, 10);
   ItemValue["iequipversion"]          = itoa(pItemData->iequipversion, szValue, 10);
   ItemValue["irandseed"]              = itoa(pItemData->irandseed, szValue, 10);
   ItemValue["iparam2"]                = itoa(pItemData->iparam2, szValue, 10);
   ItemValue["iparam3"]                = itoa(pItemData->iparam3, szValue, 10);
   ItemValue["iparam5"]                = itoa(pItemData->iparam5, szValue, 10);
   ItemValue["iparam4"]                = itoa(pItemData->iparam4, szValue, 10);
   ItemValue["iparam6"]                = itoa(pItemData->iparam6, szValue, 10);
   ItemValue["iparam1"]                = itoa(pItemData->iparam1, szValue, 10);
   ItemValue["ilucky"]                 = itoa(pItemData->ilucky, szValue, 10);
}



void	Role_SaveBaseInfo(ColumnAndValue &RoleValue, TRoleData * pRoleData )
{
	char szValue[100];
			
	RoleValue["cusercode"]			= pRoleData->BaseInfo.szName;
	
	if (pRoleData->BaseInfo.bSex) RoleValue["bsex"] = "女";
	else
		RoleValue["bsex"] = "男";
	RoleValue["calias"]				= pRoleData->BaseInfo.szAlias;
	RoleValue["ifirstsect"]			= itoa(pRoleData->BaseInfo.nFirstSect , szValue, 10);
	RoleValue["isect"]				= itoa(pRoleData->BaseInfo.nSect , szValue, 10);
	RoleValue["ijoincount"]			= itoa(pRoleData->BaseInfo.ijoincount , szValue, 10);
	RoleValue["isectrole"]			= itoa(pRoleData->BaseInfo.isectrole , szValue, 10);
	RoleValue["igroupcode"]			= itoa(pRoleData->BaseInfo.igrouprole , szValue, 10);
	RoleValue["igrouprole"]			= itoa(pRoleData->BaseInfo.igrouprole , szValue, 10);
	RoleValue["irevivalid"]			= itoa(pRoleData->BaseInfo.irevivalid , szValue, 10);
	RoleValue["cpartnercode"]		= pRoleData->BaseInfo.cpartnercode;
	RoleValue["isavemoney"]			= itoa(pRoleData->BaseInfo.isavemoney , szValue, 10);
	RoleValue["imoney"]				= itoa(pRoleData->BaseInfo.imoney , szValue, 10);
	RoleValue["ifiveprop"]			= itoa(pRoleData->BaseInfo.ifiveprop , szValue, 10);
	RoleValue["iteam"]				= itoa(pRoleData->BaseInfo.iteam , szValue, 10);
	RoleValue["ifightlevel"]		= itoa(pRoleData->BaseInfo.ifightlevel , szValue, 10);
	RoleValue["ifightexp"]			= itoa(pRoleData->BaseInfo.fightexp , szValue, 10);
	RoleValue["ileadlevel"]			= itoa(pRoleData->BaseInfo.ileadlevel , szValue, 10);
	RoleValue["ileadexp"]			= itoa(pRoleData->BaseInfo.ileadexp , szValue, 10);
	RoleValue["iliveexp"]			= itoa(pRoleData->BaseInfo.iliveexp , szValue, 10);
	RoleValue["ipower"]				= itoa(pRoleData->BaseInfo.ipower , szValue, 10);
	RoleValue["iagility"]			= itoa(pRoleData->BaseInfo.iagility , szValue, 10);
	RoleValue["iouter"]				= itoa(pRoleData->BaseInfo.iouter , szValue, 10);
	RoleValue["iinside"]			= itoa(pRoleData->BaseInfo.iinside , szValue, 10);
	RoleValue["iluck"]				= itoa(pRoleData->BaseInfo.iluck , szValue, 10);
	RoleValue["imaxlife"]			= itoa(pRoleData->BaseInfo.imaxlife , szValue, 10);
	RoleValue["imaxstamina"]		= itoa(pRoleData->BaseInfo.imaxstamina , szValue, 10);
	RoleValue["imaxinner"]			= itoa(pRoleData->BaseInfo.imaxinner , szValue, 10);
	RoleValue["ileftprop"]			= itoa(pRoleData->BaseInfo.ileftprop , szValue, 10);
	RoleValue["ileftfight"]			= itoa(pRoleData->BaseInfo.ileftfight , szValue, 10);
	RoleValue["ileftlife"]			= itoa(pRoleData->BaseInfo.ileftlife , szValue, 10);
	RoleValue["caccname"]			= pRoleData->BaseInfo.caccname;
	RoleValue["ifinishgame"]		= itoa(pRoleData->BaseInfo.ifinishgame , szValue, 10);
	RoleValue["irevivalx"]			= itoa(pRoleData->BaseInfo.irevivalx , szValue, 10);
	RoleValue["irevivaly"]			= itoa(pRoleData->BaseInfo.irevivaly , szValue, 10);
	RoleValue["iarmorres"]			= itoa(pRoleData->BaseInfo.iarmorres , szValue, 10);
	RoleValue["iweaponres"]			= itoa(pRoleData->BaseInfo.iweaponres , szValue, 10);
	RoleValue["ihelmres"]			= itoa(pRoleData->BaseInfo.ihelmres , szValue, 10);
			
}

#ifdef SHOWDEBUGINFO
			{
	
			}
#endif

S3DBI_API void S3DBI_ShowDebugInfo(IN BYTE * pRoleBuffer, char * pShowString)
{
	if (!pShowString) return;
	//检查入口有效性
	
	if (!pRoleBuffer) return ;
	
	//获得数据库中玩家基本信息表
	
	TRoleData *pRoleData = (TRoleData*)pRoleBuffer;
			
	
	ColumnAndValue RoleValue ;//= Role.GetProperties();
	Role_SaveBaseInfo(RoleValue, pRoleData);
	ColumnAndValue::iterator II;
	sprintf(pShowString, "---------------RoleBaseINfo-------------\n");
	for (II = RoleValue.begin(); II != RoleValue.end(); II ++)
	{
		sprintf(pShowString , "%s %s = %s\n",  pShowString, II->first.c_str(), II->second.c_str());
	}

	//*************************************************
	//更新玩家的战斗技能列表
	sprintf(pShowString, "%s---------------FightSkill-------------\n", pShowString);
	if (pRoleData->dwFSkillOffset > 0)
	{
		int nFightCount = pRoleData->nFightSkillCount;
		
		TDBSkillData * pSkillData = (TDBSkillData *)((BYTE *)pRoleData + pRoleData->dwFSkillOffset);
		for (int nT = 0; nT < nFightCount; nT ++, pSkillData ++)
		{

			sprintf(pShowString, "%sFight SkillId = %d, SkillLevel = %d\n",pShowString, pSkillData->m_nSkillId, pSkillData->m_nSkillLevel);
		}
	}
	
	//*************************************************
	
	
	//*************************************************
	// 更新玩家的生活技能列表
	sprintf(pShowString, "%s---------------LifeSkill-------------\n",pShowString);
	if (pRoleData->dwLSkillOffset > 0)
	{
		TDBSkillData * pSkillData = (TDBSkillData *)((BYTE *) pRoleData + pRoleData->dwLSkillOffset);
		int nLifeCount = pRoleData->nLiveSkillCount;
				
		for (int nT = 0; nT < nLifeCount; nT ++, pSkillData ++)
		{

			sprintf(pShowString,"%sLive SkillId = %d, SkillLevel = %d\n",pShowString, pSkillData->m_nSkillId, pSkillData->m_nSkillLevel);
		}
	}
	
	
	
	//*************************************************

		
	//*************************************************
	// 更新玩家的任务列表
	sprintf(pShowString, "%s---------------Task-------------\n", pShowString);
	if (pRoleData->dwTaskOffset > 0)
	{
		int nTaskCount = pRoleData->nTaskCount;
		TDBTaskData * pTaskData = (TDBTaskData *)((BYTE*)pRoleData + pRoleData->dwTaskOffset);
		
		for (int nT = 0; nT < nTaskCount; nT ++, pTaskData ++)
		{
			sprintf(pShowString, "%sTask%d Value%d\n", pShowString, pTaskData->m_nTaskId, pTaskData->m_nTaskValue);
		}
	}
	
	
	//*************************************************

	
	
	//*************************************************
	// 更新玩家的物品列表
	//存在物品列表则更新物品列表
	sprintf(pShowString, "%s---------------ItemList-------------\n", pShowString);
	if (pRoleData->dwItemOffset > 0)
	{
		int nEquipListCount = pRoleData->nItemCount;
		TDBItemData * pItemData = (TDBItemData *)((BYTE * ) pRoleData + pRoleData->dwItemOffset);
		
		
		{
								
			//依次保存每一个物件
			sprintf(pShowString, "%s---------------ItemBase-------------\n", pShowString);
			for (int j = 0; j < nEquipListCount; j ++)
			{
				sprintf(pShowString, "%s\n", pShowString);
				int nEquipBaseInfoListCount  = 0;//某个物件的基本属性列表数目
				ColumnAndValue InfoValue;

				Item_SaveBaseInfo(InfoValue, pItemData);
				ColumnAndValue::iterator II;
				for (II = InfoValue.begin(); II != InfoValue.end(); II ++)
				{
					sprintf(pShowString, "%s%s = %s\n", pShowString, II->first.c_str(), II->second.c_str());
				}

#ifdef SAVE_ITEM_MAGICATTRIB_TO_DB
				//基本属性更新
				
				nEquipBaseInfoListCount = pItemData->BaseInfoCount;

				KMagicAttrib * pMagicAttrib = (KMagicAttrib *)&pItemData->pBuffer;
				
				//保存物件基本属性
				for (int  t = 0; t < nEquipBaseInfoListCount; t++, pMagicAttrib ++)
				{
					//物件列表中第j+1物件的基本信息的t+1项
					sprintf(pShowString, "%sBaseInfo[%d] %d, %d, %d, %d\n",pShowString, t, pMagicAttrib->nAttribType, pMagicAttrib->nValue[0], pMagicAttrib->nValue[1], pMagicAttrib->nValue[2] );
				}
				
				
				//更新效果属性
				int nEquipEfficInfoListCount = pItemData->EfficCount;
				
				//保存物件
				for ( t = 0; t < nEquipEfficInfoListCount; t++, pMagicAttrib ++)
				{
					//物件列表中第j+1物件的基本信息的t+1项
					sprintf(pShowString, "%sEfficInfo[%d] %d, %d, %d, %d\n", pShowString, t, pMagicAttrib->nAttribType, pMagicAttrib->nValue[0], pMagicAttrib->nValue[1], pMagicAttrib->nValue[2] );
				}
				
				//更新需求属性
				int nEquipRequireInfoListCount = pItemData->RequireInfoCount;
							
				//保存物件
				for ( t = 0; t < nEquipRequireInfoListCount; t++, pMagicAttrib ++)
				{
					//物件列表中第j+1物件的基本信息的t+1项
					sprintf(pShowString, "%sRequireInfo[%d] %d, %d, %d, %d\n",pShowString, t, pMagicAttrib->nAttribType, pMagicAttrib->nValue[0], pMagicAttrib->nValue[1], pMagicAttrib->nValue[2] );
					
				}
				pItemData = (TDBItemData*)pMagicAttrib;
#else
				pItemData ++;
#endif
			}
			
			
		}
	}
	//*************************************************	
	

	
	//************************************************
	// 更新玩家的好友列表//
	sprintf(pShowString, "%s---------------Friend-------------\n", pShowString);
	if (pRoleData->dwFriendOffset > 0)
	{
		int nFriendCount = pRoleData->nFriendCount;
		char * pFriendBuffer = (char *)((BYTE *)pRoleData + pRoleData->dwFriendOffset);
		
		for (int nT = 0; nT < nFriendCount; nT ++)
		{
			
			sprintf(pShowString, "%sFriend%d is %s\n", pShowString, nT, pFriendBuffer);
			pFriendBuffer += strlen(pFriendBuffer) + 1;
		}
	}

	//*************************************************
	
}

// result -1 othererror
// -4 have or no user exist
// 

S3DBI_API int	S3DBI_SaveRoleInfo(IN BYTE * pRoleBuffer, IN char * strUser, BOOL bAutoInsertWhenNoExistUser)
{
	//检查入口有效性
	char strRoleName[50];
	if (!pRoleBuffer) return -1;
	
	if ((!strUser )|| (!strUser[0])) 
	{
		TRoleData * ptmpRoleData = (TRoleData*) pRoleBuffer;
		strcpy(strRoleName, ptmpRoleData->BaseInfo.szName);
	}
	else
		strcpy(strRoleName, strUser);

	int bSaveRole = FALSE;//保存角色基本信息是否成功
	int nLastResult = 0;
	BOOL bNewUser = FALSE;//是否是新创建该玩家

	
	//获得数据库中玩家基本信息表
	S3PRole Role(strRoleName);

	if ((nLastResult = Role.GetLastResult()) < 0 )	
	{
		//-4表示不存在该用户，需增加
		if (nLastResult == -4)
		{
			if (bAutoInsertWhenNoExistUser)//当已设定成角色在数据库中不存在自动增加时，增加。否则返回
				bNewUser = TRUE;
			else 
				return -1;
		}
		else
			return -1;//其它错误返回
	}


	TRoleData *pRoleData = (TRoleData*)pRoleBuffer;

	if (pRoleData->bBaseNeedUpdate)
	{
		if (!bNewUser) //非新用户，直接更新!
		{
			char szCurKey[100] = "";
			char szPrevKey[100] = "";
			
			ColumnAndValue &RoleValue = Role.GetProperties();
			RoleValue.clear();
			Role_SaveBaseInfo(RoleValue, pRoleData);
			bSaveRole = Role.Save();
			if (bSaveRole < 0 )	return -1;
		}
		else //新用户，增加数据！
		{
			char szCurKey[100] = "";
			char szPrevKey[100] = "";
						
			//S3PRole NewRole;
			ColumnAndValue RoleValue ;//= Role.GetProperties();
			Role_SaveBaseInfo(RoleValue, pRoleData);
			bSaveRole = Role.Add(RoleValue);
			if (bSaveRole < 0 ) return -1;
			//Role = NewRole;
		}
	}
	else
	{
		if (bNewUser)		return -1;//新用户必须含有基本信息!
	}
		

	//*************************************************
	//更新玩家的战斗技能列表
	if (pRoleData->dwFSkillOffset > 0)
	{
		int nFightCount = pRoleData->nFightSkillCount;

		Role.GetFightSkillList()->DeleteAll();
		TDBSkillData * pSkillData = (TDBSkillData *)((BYTE *)pRoleData + pRoleData->dwFSkillOffset);
		for (int nT = 0; nT < nFightCount; nT ++, pSkillData ++)
		{
			char szValue[100];
			ColumnAndValue FightSkillValue;
			FightSkillValue["ifightskill"] = itoa(pSkillData->m_nSkillId, szValue, 10);
			FightSkillValue["ifightskilllevel"] = itoa(pSkillData->m_nSkillLevel, szValue, 10);
			Role.GetFightSkillList()->Add(FightSkillValue);
		}
	}
	
	//*************************************************
	
	
	//*************************************************
	// 更新玩家的生活技能列表
	if (pRoleData->dwLSkillOffset > 0)
	{
		TDBSkillData * pSkillData = (TDBSkillData *)((BYTE *) pRoleData + pRoleData->dwLSkillOffset);
		int nLifeCount = pRoleData->nLiveSkillCount;
		Role.GetLifeSkillList()->DeleteAll();
		
		for (int nT = 0; nT < nLifeCount; nT ++, pSkillData ++)
		{
			char szValue[100];
			ColumnAndValue LifeSkillValue;
			LifeSkillValue["ilifeskill"] = itoa(pSkillData->m_nSkillId, szValue, 10);
			LifeSkillValue["ilifeskilllevel"] = itoa(pSkillData->m_nSkillLevel, szValue, 10);
			Role.GetLifeSkillList()->Add(LifeSkillValue);
		}
	}
	
	
	
	//*************************************************

		
	//*************************************************
	// 更新玩家的任务列表
	if (pRoleData->dwTaskOffset > 0)
	{
		int nTaskCount = pRoleData->nTaskCount;
		Role.GetTaskList()->DeleteAll();
		TDBTaskData * pTaskData = (TDBTaskData *)((BYTE*)pRoleData + pRoleData->dwTaskOffset);
		
		for (int nT = 0; nT < nTaskCount; nT ++, pTaskData ++)
		{
			char szValue[200] ;
			ColumnAndValue TaskValue;
			TaskValue["itaskcode"] = itoa(pTaskData->m_nTaskId, szValue, 10);
			TaskValue["idegree"] = itoa(pTaskData->m_nTaskValue, szValue, 10);
			Role.GetTaskList()->Add(TaskValue);
		}
	}
	
	
	//*************************************************

	
	
	//*************************************************
	// 更新玩家的物品列表
	//存在物品列表则更新物品列表
	if (pRoleData->dwItemOffset > 0)
	{
		int nEquipListCount = pRoleData->nItemCount;
		S3PTableObjList<S3PEquipment> * pEquipList = Role.GetEquipmentList();
		TDBItemData * pItemData = (TDBItemData *)((BYTE * ) pRoleData + pRoleData->dwItemOffset);
		
		if (pEquipList)
		{
			pEquipList->DeleteAll();//清除所有物件
						
			//依次保存每一个物件
			for (int j = 0; j < nEquipListCount; j ++)
			{
				int nEquipBaseInfoListCount  = 0;//某个物件的基本属性列表数目
				//基本信息更新

				ColumnAndValue InfoValue;

				Item_SaveBaseInfo(InfoValue, pItemData);
				if (pEquipList->Add(InfoValue) < 0 ) continue; //如果保存失败，则该物件的其它数据不用再更新了
				std::string  szIid = (*pEquipList)[pEquipList->Size() - 1]->GetProperties()["iid"];
				
#ifdef SAVE_ITEM_MAGICATTRIB_TO_DB

				//基本属性更新
				
				nEquipBaseInfoListCount = pItemData->BaseInfoCount;

				char szValue[200];
				KMagicAttrib * pMagicAttrib = (KMagicAttrib *)&pItemData->pBuffer;
				
				//保存物件基本属性
				for (int  t = 0; t < nEquipBaseInfoListCount; t++, pMagicAttrib ++)
				{
					//物件列表中第j+1物件的基本信息的t+1项
				
					ColumnAndValue BaseValue;
					BaseValue["iclass"] = itoa(pMagicAttrib->nAttribType, szValue, 10);
					BaseValue["ivalue1"] = itoa(pMagicAttrib->nValue[0], szValue, 10);
					BaseValue["ivalue2"] = itoa(pMagicAttrib->nValue[1], szValue, 10);
					BaseValue["ivalue3"] = itoa(pMagicAttrib->nValue[2], szValue, 10);
					BaseValue["imainid"] = szIid;//不一定要写
					(*pEquipList)[pEquipList->Size() - 1]->GetEquipBaseInfoList()->Add(BaseValue);
					
				}
				
				
				//更新效果属性
				int nEquipEfficInfoListCount = pItemData->EfficCount;
				
				//保存物件
				for ( t = 0; t < nEquipEfficInfoListCount; t++, pMagicAttrib ++)
				{
					//物件列表中第j+1物件的基本信息的t+1项

					ColumnAndValue EfficValue;
					EfficValue["iclass"] = itoa(pMagicAttrib->nAttribType, szValue, 10);
					EfficValue["ivalue1"] = itoa(pMagicAttrib->nValue[0], szValue, 10);
					EfficValue["ivalue2"] = itoa(pMagicAttrib->nValue[1], szValue, 10);
					EfficValue["ivalue3"] = itoa(pMagicAttrib->nValue[2], szValue, 10);
					EfficValue["imainid"] = szIid;//不一定要写
					(*pEquipList)[pEquipList->Size() - 1]->GetEquipEfficInfoList()->Add(EfficValue);
				}
				
				//更新需求属性
				int nEquipRequireInfoListCount = pItemData->RequireInfoCount;
							
				//保存物件
				for ( t = 0; t < nEquipRequireInfoListCount; t++, pMagicAttrib ++)
				{
					//物件列表中第j+1物件的基本信息的t+1项
					ColumnAndValue RequireValue;
					RequireValue["iclass"] = itoa(pMagicAttrib->nAttribType, szValue, 10);
					RequireValue["ivalue1"] = itoa(pMagicAttrib->nValue[0], szValue, 10);
					RequireValue["ivalue2"] = itoa(pMagicAttrib->nValue[1], szValue, 10);
					RequireValue["ivalue3"] = itoa(pMagicAttrib->nValue[2], szValue, 10);
					RequireValue["imainid"] = szIid;//不一定要写
					(*pEquipList)[pEquipList->Size() - 1]->GetEquipRequireInfoList()->Add(RequireValue);
					
				}
				pItemData = (TDBItemData *) pMagicAttrib;
#else
				pItemData ++;
#endif
			}
			
		}
	}
	//*************************************************	
	

	
	//************************************************
	// 更新玩家的好友列表//
	if (pRoleData->dwFriendOffset > 0)
	{
		int nFriendCount = pRoleData->nFriendCount;
		Role.GetFriendList()->DeleteAll();
		char * pFriendBuffer = (char *)((BYTE *)pRoleData + pRoleData->dwFriendOffset);
		
		for (int nT = 0; nT < nFriendCount; nT ++)
		{
			
			ColumnAndValue FriendValue;
			FriendValue["cfriendcode"] = pFriendBuffer;
			FriendValue["cusercode"] = strRoleName;
			pFriendBuffer += strlen(pFriendBuffer) + 1;
			Role.GetFriendList()->Add(FriendValue);
		}
	}

	//*************************************************
	
	return 1;
}


S3DBI_API int S3DBI_DeleteRole(char * strUser)
{
	if (!strUser || !strUser[0])  return -1;
	S3PRole Role(strUser);
	int nLastResult = 0;
	
	//查询不到就返回!
	if ((nLastResult = Role.GetLastResult()) < 0 ) return -1;
	
//************************************************
// 删除玩家的好友列表//
	nLastResult = Role.GetFriendList()->DeleteAll();
	if (nLastResult < 0)	g_DebugLog("Delete[%s]Player FriendList Error!,Result = %d", strUser, nLastResult);
//*************************************************


//*************************************************
// 玩家的战斗技能列表
	nLastResult = Role.GetFightSkillList()->DeleteAll();
	if (nLastResult < 0)	g_DebugLog("Delete[%s]Player FightSkillList Error!,Result = %d", strUser, nLastResult);
//*************************************************
	

//*************************************************
// 玩家的生活技能列表
	nLastResult = Role.GetLifeSkillList()->DeleteAll();
	if (nLastResult < 0)	g_DebugLog("Delete[%s]Player LifeSkillList Error!,Result = %d", strUser, nLastResult);

//*************************************************


//*************************************************
// 玩家的物品列表
	nLastResult = Role.GetEquipmentList()->DeleteAll();
	if (nLastResult < 0)	g_DebugLog("Delete[%s]Player Item List Error!,Result = %d", strUser, nLastResult);

//*************************************************	

//*************************************************
// 玩家的任务列表
	nLastResult = Role.GetTaskList()->DeleteAll();
	if (nLastResult < 0)	g_DebugLog("Delete[%s]Player TaskList Error!,Result = %d", strUser, nLastResult);
//*************************************************

//数据库中玩家基本信息表
	nLastResult = Role.Delete();
	if (nLastResult < 0)	g_DebugLog("Delete[%s]Player InfoData Error!,Result = %d", strUser, nLastResult);
	return nLastResult;
}

S3DBI_API int S3DBI_GetRoleListOfAccount(char * szAccountName, S3DBI_RoleBaseInfo * RoleBaseList, int nMaxCount)
{
	if (!szAccountName || (!szAccountName[0])) return 0;
	S3PRoleList RoleList(szAccountName);
	int nCount = RoleList.Size();
	if (nCount <= 0) return 0;
	if (nCount > nMaxCount) nCount = nMaxCount;
	
	for (int i  = 0; i < nCount; i ++)
	{
		ColumnAndValue &RoleValue = (*RoleList[i]).GetProperties();
		strcpy((RoleBaseList + i)->szName,RoleValue["cusercode"].c_str()); 
		(RoleBaseList + i)->Level		= atoi(RoleValue["ifightlevel"].c_str());
		(RoleBaseList + i)->ArmorType	= atoi(RoleValue["iarmorres"].c_str());
		(RoleBaseList + i)->WeaponType = atoi(RoleValue["iweaponres"].c_str());
		(RoleBaseList + i)->HelmType	= atoi(RoleValue["ihelmres"].c_str());
		
		if (!strcmp(RoleValue["bsex"].c_str(), "男")) 
			(RoleBaseList + i)->Sex		= 0;
		else
			(RoleBaseList + i)->Sex		= 1;
	}

	return nCount;
}

S3DBI_API BOOL S3DBI_IsRoleNameExisted(char * strUser)
{
	if ((!strUser) || (!strUser[0])) return FALSE;
	//获得数据库中玩家基本信息表
	S3PRole Role(strUser);
	int nLastResult = Role.GetLastResult();
	if (nLastResult > 0)
		 return TRUE;
	return FALSE;
}

//获得数据库中玩家基本信息表

	
//************************************************
// 更新玩家的好友列表//


//*************************************************


//*************************************************
// 加载玩家的战斗技能列表
	
//*************************************************
	

//*************************************************
// 加载玩家的生活技能列表
	



//*************************************************


//*************************************************
// 加载玩家的物品列表


//*************************************************	


//*************************************************
// 加载玩家的任务列表


//*************************************************
