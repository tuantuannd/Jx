//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KNpcResNode.cpp
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Obj Class
//---------------------------------------------------------------------------
#include "KCore.h"
#include "../../Engine/Src/KSG_StringProcess.h"

#ifndef _SERVER

#include	"KSprControl.h"
#include	"KTabFile.h"
#include	"KIniFile.h"
#include	"KFilePath.h"
//#include	"KStrBase.h"
#include	"KNpcResNode.h"
#include	"CoreUseNameDef.h"

#define	MAX_SORT_DIR			16


KNpcResNode::KNpcResNode()
{
	m_nNpcKind = NPC_RES_NORMAL;
	m_nPartNum = 1;
	m_szNpcName[0] = 0;
	m_szResPath[0] = 0;
	memset(m_szSoundName, 0, sizeof(m_szSoundName));
}

KNpcResNode::~KNpcResNode()
{
}

BOOL	KNpcResNode::Init(char *lpszNpcName, CActionName *cActionName, CActionName *cNpcAction)
{
	if (lpszNpcName == NULL || lpszNpcName[0] == 0)
		return FALSE;

	KTabFile	KindFile;
	int			i, j, k, nFindNo;
	char		szBuffer[FILE_NAME_LENGTH], szTemp[FILE_NAME_LENGTH], szBuf[FILE_NAME_LENGTH];
    const char *pcszTemp = NULL;

	strcpy(m_szNpcName, lpszNpcName);
	// 载入文件 人物类型.txt
//	g_SetFilePath(RES_INI_FILE_PATH);
	if ( !KindFile.Load(NPC_RES_KIND_FILE_NAME) )
		return FALSE;
	nFindNo = KindFile.FindRow(lpszNpcName);
	if (nFindNo < 0)
		return FALSE;
	KindFile.GetString(nFindNo, KIND_NAME_SECT, "", szBuffer, sizeof(szBuffer));

	// 判断npc类型
	if (strcmp(szBuffer, KIND_NAME_SPECIAL) == 0)
		m_nNpcKind = NPC_RES_SPECIAL;
	else
		m_nNpcKind = NPC_RES_NORMAL;
	// 得到资源文件路径
	KindFile.GetString(nFindNo, KIND_FILE_SECT5, "", m_szResPath, sizeof(m_szResPath));

	// 特殊npc
	if (m_nNpcKind == NPC_RES_SPECIAL)
	{
		KTabFile	PartFile, SoundName, ShadowName;
		// 得到部件说明文件名
		KindFile.GetString(nFindNo, KIND_FILE_SECT1, "", szBuffer, sizeof(szBuffer));
		if ( !szBuffer[0] )
			return FALSE;
//		g_SetFilePath(RES_INI_FILE_PATH);
		g_UnitePathAndName(RES_INI_FILE_PATH, szBuffer, szBuf);
		if ( !PartFile.Load(szBuf) )
			return FALSE;
		// 得到部件组成信息
		m_nPartNum = 0;
		for (i = 0; i < MAX_BODY_PART; i++)
		{
			for (j = 0; j < MAX_BODY_PART_SECT; j++)
			{
				m_nSectInfo[i * MAX_BODY_PART_SECT + j].Clear();
				PartFile.GetString(
					i + 2,
					j + 3,
					"",
					m_nSectInfo[i * MAX_BODY_PART_SECT + j].szSectName,
					sizeof(m_nSectInfo[i * MAX_BODY_PART_SECT + j].szSectName));
				if ( m_nSectInfo[i * MAX_BODY_PART_SECT + j].szSectName[0] )
				{
					m_nSectInfo[i * MAX_BODY_PART_SECT + j].nFlag = 1;
					m_nPartNum++;
				}
			}
		}
		// 得到每个部件的资源说明文件名
		for (i = 0; i < MAX_PART; i++)
		{
			if (m_nSectInfo[i].nFlag)
			{
				KindFile.GetString(
					nFindNo,
					m_nSectInfo[i].szSectName,
					"",
					m_nSectInfo[i].szSectResName,
					sizeof(m_nSectInfo[i].szSectResName));
				// 资源信息说明文件名是资源说明文件名加上SPR_INFO_NAME（“信息”）
				if (m_nSectInfo[i].szSectResName[0])
				{
					g_StrCpyLen(
						m_nSectInfo[i].szSectSprInfoName, 
						m_nSectInfo[i].szSectResName, 
						g_StrLen(m_nSectInfo[i].szSectResName) - 3
						);
					g_StrCat(m_nSectInfo[i].szSectSprInfoName, SPR_INFO_NAME);
					g_StrCat(m_nSectInfo[i].szSectSprInfoName, ".txt");
				}
			}
		}
		// 得到每个部件的具体的资源文件名
		KTabFile	SectFile, SectInfoFile;
		int			nGetEquipNo, nActionCount;
		for (i = 0; i < MAX_PART; i++)
		{
			m_cResInfo[i].AutoDelete();
			if (m_nSectInfo[i].nFlag)
			{
//				g_SetFilePath(RES_INI_FILE_PATH);

#ifdef TOOLVERSION					
				g_UnitePathAndName(RES_INI_FILE_PATH, m_nSectInfo[i].szSectResName, szBuf);
				if (!SectFile.Load(szBuf))
					continue;
				else
				{
					g_UnitePathAndName(RES_INI_FILE_PATH, m_nSectInfo[i].szSectSprInfoName, szBuf);
					SectInfoFile.Load(szBuf);
				}
#else
				g_UnitePathAndName(RES_INI_FILE_PATH, m_nSectInfo[i].szSectResName, szBuf);
				if ( !SectFile.Load(szBuf) )
					continue;
				g_UnitePathAndName(RES_INI_FILE_PATH, m_nSectInfo[i].szSectSprInfoName, szBuf);
				if ( !SectInfoFile.Load(szBuf))
					continue;
#endif

				nGetEquipNo = SectFile.GetHeight() - 1;
				if (nGetEquipNo <= 0)
					continue;
				nActionCount = cActionName->GetActionCount();
				if (nActionCount <= 0)
					continue;
				m_cResInfo[i].AutoNew(nActionCount, nGetEquipNo);
				for (j = 0; j < nGetEquipNo; j++)
				{
					for (k = 0; k < nActionCount; k++)
					{
						// 用字符串比较太慢，所以直接用编号，但是表格必须保证不出错
//						cActionName->GetActionName(k, szBuffer, sizeof(szBuffer));
//						SectFile.GetString(
//							j + 2,
//							szBuffer,
//							"",
//							szTemp,
//							80);
						SectFile.GetString(
							j + 2,
							k + 2,
							"",
							szTemp,
							sizeof(szTemp));
						ComposePathAndName(m_cResInfo[i].m_cSprInfo[j * nActionCount + k].szFileName, m_szResPath, szTemp);
						SectInfoFile.GetString(
							j + 2,
							k + 2,
							"16,16,0",
							szTemp,
							sizeof(szTemp)
                        );

                        pcszTemp = szTemp;
                        m_cResInfo[i].m_cSprInfo[j * nActionCount + k].nTotalFrames = KSG_StringGetInt(&pcszTemp, 16);
                        KSG_StringSkipSymbol(&pcszTemp, ',');
                        m_cResInfo[i].m_cSprInfo[j * nActionCount + k].nTotalDirs = KSG_StringGetInt(&pcszTemp, 16);
                        KSG_StringSkipSymbol(&pcszTemp, ',');
                        m_cResInfo[i].m_cSprInfo[j * nActionCount + k].nInterval = KSG_StringGetInt(&pcszTemp, 0);
						//sscanf(szTemp, "%d,%d,%d", 
						//	&m_cResInfo[i].m_cSprInfo[j * nActionCount + k].nTotalFrames,
						//	&m_cResInfo[i].m_cSprInfo[j * nActionCount + k].nTotalDirs,
						//	&m_cResInfo[i].m_cSprInfo[j * nActionCount + k].nInterval
                        //);
					}
				}
			}
		}
		// 得到武器行为关联表文件名
		KTabFile	NoHorseFile, OnHorseFile;
		int			nTableWidth, nTableHeight, nGetNo;
		char	szNoHorseTableName[80], szOnHorseTableName[80];
		KindFile.GetString(nFindNo, KIND_FILE_SECT2, "", szNoHorseTableName, sizeof(szNoHorseTableName));
		KindFile.GetString(nFindNo, KIND_FILE_SECT3, "", szOnHorseTableName, sizeof(szOnHorseTableName));
		// 未骑马对应表
		if (szNoHorseTableName[0])
		{
//			g_SetFilePath(RES_INI_FILE_PATH);
			g_UnitePathAndName(RES_INI_FILE_PATH, szNoHorseTableName, szBuf);
			if ( NoHorseFile.Load(szBuf) )
			{
				nTableWidth = NoHorseFile.GetWidth() - 1;
				nTableHeight = NoHorseFile.GetHeight() - 1;
				m_NoHorseTable.AutoNew(nTableWidth, nTableHeight);
				for (i = 0; i < nTableHeight; i++)
				{
					for (j = 0; j < nTableWidth; j++)
					{
						NoHorseFile.GetString(i + 2, j + 2, "", szBuffer, sizeof(szBuffer));
						nGetNo = cActionName->GetActionNo(szBuffer);
						m_NoHorseTable.SetValue(j, i, nGetNo);
					}
				}
			}
		}
		// 骑马对应表
		if (szOnHorseTableName[0])
		{
//			g_SetFilePath(RES_INI_FILE_PATH);
			g_UnitePathAndName(RES_INI_FILE_PATH, szOnHorseTableName, szBuf);
			if ( OnHorseFile.Load(szBuf) )
			{
				nTableWidth = OnHorseFile.GetWidth() - 1;
				nTableHeight = OnHorseFile.GetHeight() - 1;
				m_OnHorseTable.AutoNew(nTableWidth, nTableHeight);
				for (i = 0; i < nTableHeight; i++)
				{
					for (j = 0; j < nTableWidth; j++)
					{
						OnHorseFile.GetString(i + 2, j + 2, "", szBuffer, sizeof(szBuffer));
						nGetNo = cActionName->GetActionNo(szBuffer);
						m_OnHorseTable.SetValue(j, i, nGetNo);
					}
				}
			}
		}
		// 得到贴图顺序表文件名
		if ( KindFile.GetString(nFindNo, KIND_FILE_SECT4, "", szBuffer, sizeof(szBuffer)) )
		{
			strcpy(m_cSortTable.m_sSortTableFileName, szBuffer);
			m_cSortTable.GetTable(szBuffer, cActionName, m_nPartNum);
		}

		// 获得动作阴影文件信息
		int		nFindNo;
//		g_SetFilePath(RES_INI_FILE_PATH);
		nActionCount = cActionName->GetActionCount();
		this->m_cShadowInfo.Init(nActionCount);
		ShadowName.Load(PLAYER_RES_SHADOW_FILE);
		nFindNo = ShadowName.FindRow(lpszNpcName);
		if (nFindNo > 0 && nActionCount > 0)
		{
			for (i = 0; i < nActionCount; i++)
			{
				ShadowName.GetString(nFindNo, 2 + i * 2, "", szTemp, sizeof(szTemp));
				if (szTemp[0])
					this->ComposePathAndName(m_cShadowInfo.m_psInfo[i].m_szName, m_szResPath, szTemp);
				ShadowName.GetString(nFindNo, 2 + i * 2 + 1, "16,8,1", szTemp, sizeof(szTemp));
				
                pcszTemp = szTemp;
                m_cShadowInfo.m_psInfo[i].m_nTotalFrame = KSG_StringGetInt(&pcszTemp, 16);
                KSG_StringSkipSymbol(&pcszTemp, ',');
                m_cShadowInfo.m_psInfo[i].m_nTotalDir = KSG_StringGetInt(&pcszTemp, 8);
                KSG_StringSkipSymbol(&pcszTemp, ',');
                m_cShadowInfo.m_psInfo[i].m_nInterval = KSG_StringGetInt(&pcszTemp, 1);
                //sscanf(szTemp, "%d,%d,%d", &m_cShadowInfo.m_psInfo[i].m_nTotalFrame, &m_cShadowInfo.m_psInfo[i].m_nTotalDir, &m_cShadowInfo.m_psInfo[i].m_nInterval);

				m_cShadowInfo.m_psInfo[i].m_nCgX = SPR_X_OFF;
				m_cShadowInfo.m_psInfo[i].m_nCgY = SPR_Y_OFF;
			}
		}

		// 获得动作音效文件名
//		g_SetFilePath(RES_INI_FILE_PATH);
		SoundName.Load(PLAYER_SOUND_FILE);
		nFindNo = SoundName.FindColumn(lpszNpcName);
		nActionCount = cActionName->GetActionCount();
		if (nFindNo > 0 && nActionCount > 0)
		{
			for (i = 0; i < nActionCount; i++)
			{
				SoundName.GetString(i + 2, nFindNo, "", szTemp, sizeof(szTemp));
				if (szTemp[0])
					this->ComposePathAndName(this->m_szSoundName[i], RES_SOUND_FILE_PATH, szTemp);
			}
		}

	}
	// 普通npc
	else if (m_nNpcKind == NPC_RES_NORMAL)
	{
		int			nActionCount;
		KTabFile	NormalNpc, NormalNpcSprInfo, SoundName;

		for (i = 0; i < MAX_PART; i++)
			m_nSectInfo[i].nFlag = 0;
		m_nSectInfo[NORMAL_NPC_PART_NO].nFlag = 1;
		m_nPartNum = 1;

		for (i = 0; i < MAX_PART; i++)
			m_cResInfo[i].AutoDelete();
		nActionCount = cNpcAction->GetActionCount();
		if (nActionCount < 0)
			nActionCount = 0;
		m_cResInfo[NORMAL_NPC_PART_NO].AutoNew(nActionCount, 1);
		this->m_cShadowInfo.Init(nActionCount);

//		g_SetFilePath(RES_INI_FILE_PATH);
		if ( !NormalNpc.Load(NPC_NORMAL_RES_FILE))
			return FALSE;
		// 动画信息文件没有没有必要初始化不成功，所以没有If(!) return FALSE;
		NormalNpcSprInfo.Load(NPC_NORMAL_SPRINFO_FILE);
		SoundName.Load(NPC_SOUND_FILE);
		nFindNo = NormalNpc.FindRow(lpszNpcName);
		if (nFindNo < 0)
			return FALSE;

		for (i = 0; i < nActionCount; i++)
		{
			cNpcAction->GetActionName(i, szBuffer, sizeof(szBuffer));
			NormalNpc.GetString(nFindNo, szBuffer, "", szTemp, sizeof(szTemp));
			ComposePathAndName(m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].szFileName, m_szResPath, szTemp);
			NormalNpcSprInfo.GetString(nFindNo, szBuffer, "16,8,0", szTemp, sizeof(szTemp));

            pcszTemp = szTemp;
            m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nTotalFrames = KSG_StringGetInt(&pcszTemp, 16);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nTotalDirs = KSG_StringGetInt(&pcszTemp, 8);
            KSG_StringSkipSymbol(&pcszTemp, ',');
            m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nInterval = KSG_StringGetInt(&pcszTemp, 0);
            //sscanf(szTemp, "%d,%d,%d", 
			//	&m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nTotalFrames,
			//	&m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nTotalDirs,
			//	&m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nInterval
            //);

			KNpcResNode::GetShadowName(m_cShadowInfo.m_psInfo[i].m_szName, m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].szFileName);
			m_cShadowInfo.m_psInfo[i].m_nTotalFrame = m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nTotalFrames;
			m_cShadowInfo.m_psInfo[i].m_nTotalDir = m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nTotalDirs;
			m_cShadowInfo.m_psInfo[i].m_nInterval = m_cResInfo[NORMAL_NPC_PART_NO].m_cSprInfo[i].nInterval;
			m_cShadowInfo.m_psInfo[i].m_nCgX = SPR_X_OFF;
			m_cShadowInfo.m_psInfo[i].m_nCgY = SPR_Y_OFF;

			SoundName.GetString(nFindNo, szBuffer, "", szTemp, sizeof(szTemp));
			if (szTemp[0])
				ComposePathAndName(m_szSoundName[i], RES_SOUND_FILE_PATH, szTemp);
		}
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// 功能:	把路经和文件名合成在一起成为带路径的文件名
//---------------------------------------------------------------------------
void	KNpcResNode::ComposePathAndName(char *lpszGet, char *lpszPath, char *lpszName)
{
	if (strlen(lpszName) == 0)
	{
		lpszGet[0] = 0;
		return;
	}
	if (strlen(lpszPath) == 0)
	{
		strcpy(lpszGet, lpszName);
		return;
	}
	if (lpszPath[0] != '\\')
	{
		lpszGet[0] = '\\';
		lpszGet[1] = 0;
	}
	else
	{
		lpszGet[0] = 0;
	}
	strcat(lpszGet, lpszPath);
	if (lpszGet[strlen(lpszGet) - 1] != '\\')
		strcat(lpszGet, "\\");
	strcat(lpszGet, lpszName);
}

//---------------------------------------------------------------------------
// 功能:	判断某个部件是否存在
//---------------------------------------------------------------------------
BOOL	KNpcResNode::CheckPartExist(int nPartNo)
{
	if (nPartNo < 0 || nPartNo >= MAX_PART)
		return FALSE;
	if (m_nSectInfo[nPartNo].nFlag)
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
// 功能:	获得某个部件的某个装备在某个动作下的 spr 文件名
//---------------------------------------------------------------------------
BOOL	KNpcResNode::GetFileName(int nPartNo, int nActionNo, int nEquipNo, char *lpszDefault, char *lpszGetName, int nStrLen)
{
	if (nPartNo < 0 || nPartNo >= MAX_PART)
		return FALSE;
	if ( m_cResInfo[nPartNo].GetName(nActionNo, nEquipNo, "", lpszGetName, nStrLen) )
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
// 功能:	获得某个部件的某个装备在某个动作下的 spr 文件信息
//---------------------------------------------------------------------------
int		KNpcResNode::GetInterval(int nPartNo, int nActionNo, int nEquipNo, int nDefault)
{
	if (nPartNo < 0 || nPartNo >= MAX_PART)
		return nDefault;

	return m_cResInfo[nPartNo].GetInterval(nActionNo, nEquipNo, nDefault);
}

//---------------------------------------------------------------------------
// 功能:	获得某个部件的某个装备在某个动作下的 spr 文件信息
//---------------------------------------------------------------------------
int		KNpcResNode::GetTotalFrames(int nPartNo, int nActionNo, int nEquipNo, int nDefault)
{
	if (nPartNo < 0 || nPartNo >= MAX_PART)
		return nDefault;

	return m_cResInfo[nPartNo].GetTotalFrames(nActionNo, nEquipNo, nDefault);
}

//---------------------------------------------------------------------------
// 功能:	获得某个部件的某个装备在某个动作下的 spr 文件信息
//---------------------------------------------------------------------------
int		KNpcResNode::GetTotalDirs(int nPartNo, int nActionNo, int nEquipNo, int nDefault)
{
	if (nPartNo < 0 || nPartNo >= MAX_PART)
		return nDefault;

	return m_cResInfo[nPartNo].GetTotalDirs(nActionNo, nEquipNo, nDefault);
}

//---------------------------------------------------------------------------
// 功能:	获得某武器某状态下的动作编号
//---------------------------------------------------------------------------
int		KNpcResNode::GetActNo(int nDoing, int nEquipNo, BOOL bRideHorse)
{
	// 普通npc Doing 与 Action 相同
	if (m_nNpcKind == NPC_RES_NORMAL)
		return nDoing;

	// 特殊 npc， 查表
	if (bRideHorse == FALSE)
	{
		return m_NoHorseTable.GetValue(nEquipNo, nDoing);
	}
	else
	{
		return m_OnHorseTable.GetValue(nEquipNo, nDoing);
	}
}

//---------------------------------------------------------------------------
// 功能:	从表格中得到一组排序信息
//---------------------------------------------------------------------------
BOOL	KNpcResNode::GetSort(int nActNo, int nDir, int nFrameNo, int *lpnTable, int nTableLen)
{
	// 普通npc
	if (m_nNpcKind == NPC_RES_NORMAL)
	{
		if (!lpnTable || nTableLen <= 0)
			return FALSE;
		for (int i = 0; i < nTableLen; i++)
			lpnTable[i] = -1;
		lpnTable[0] = NORMAL_NPC_PART_NO;
		return TRUE;
	}

	// 特殊npc
	if ( m_cSortTable.GetSort(nActNo, nDir, nFrameNo, lpnTable, nTableLen) )
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
// 功能：获得某个动作对应的音效文件名
//---------------------------------------------------------------------------
void	KNpcResNode::GetActionSoundName(int nAction, char *lpszSoundName)
{
	if (nAction < 0 || nAction >= MAX_ACTION)
		return;
	if (!lpszSoundName)
		return;
	g_StrCpy(lpszSoundName, m_szSoundName[nAction]);
}

//---------------------------------------------------------------------------
// 功能：按默认规则把一个npc的spr图文件名转换成相应的阴影文件名
//---------------------------------------------------------------------------
void	KNpcResNode::GetShadowName(char *lpszShadow, char *lpszSprName)
{
	if (lpszSprName[0] == 0)
	{
		lpszShadow[0] = 0;
		return;
	}
	strcpy(lpszShadow, lpszSprName);
	for (int i = strlen(lpszShadow) - 1; i >= 0; i--)
	{
		if (lpszShadow[i] == '.')
		{
			lpszShadow[i] = 0;
			break;
		}
	}
	strcat(lpszShadow, "b.spr");
}

//---------------------------- class CActionName ----------------------------
//---------------------------------------------------------------------------
// 功能:	构造函数
//---------------------------------------------------------------------------
CActionName::CActionName()
{
	m_nMaxAction = MAX_ACTION;
	m_nCurActionNo = 0;
	for (int i = 0; i < MAX_ACTION; i++)
		m_szName[i][0] = 0;
}

//---------------------------------------------------------------------------
// 功能:	获取动作种类、名称等信息
//---------------------------------------------------------------------------
BOOL	CActionName::Init(char *lpszFileName)
{
	if (!lpszFileName || !lpszFileName[0])
		return FALSE;

	char		szBuf[FILE_NAME_LENGTH];
	KTabFile	cTabFile;
	int			i;

//	g_SetFilePath(RES_INI_FILE_PATH);
	g_UnitePathAndName(RES_INI_FILE_PATH, lpszFileName, szBuf);
	if ( !cTabFile.Load(szBuf) )
		return FALSE;
	m_nCurActionNo = cTabFile.GetHeight() - 1;
	if (m_nCurActionNo <= 0)
		return FALSE;
	if (m_nCurActionNo > m_nMaxAction)
		m_nCurActionNo = m_nMaxAction;
	for (i = 0; i < m_nCurActionNo; i++)
	{
		cTabFile.GetString(i + 2, 1, "", m_szName[i], sizeof(m_szName[i]));
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// 功能:	由动作名称得到动作编号
//---------------------------------------------------------------------------
int		CActionName::GetActionNo(char *lpszName)
{
	if ( !lpszName || !lpszName[0] )
		return -1;
	for (int i = 0; i < m_nCurActionNo; i++)
	{
		if (strcmp(lpszName, m_szName[i]) == 0)
			return i;
	}
	return -1;
}

//---------------------------------------------------------------------------
// 功能:	得到动作种类数
//---------------------------------------------------------------------------
int		CActionName::GetActionCount()
{
	return m_nCurActionNo;
}

//---------------------------------------------------------------------------
// 功能:	由动作编号得到动作名称
//---------------------------------------------------------------------------
BOOL	CActionName::GetActionName(int nNo, char *lpszName, int nSize)
{
	if (!lpszName)
		return FALSE;
	if (nNo < 0 || nNo >= m_nCurActionNo)
		return FALSE;
	if (strlen(m_szName[nNo]) >= (DWORD)nSize)
		return FALSE;
	strcpy(lpszName, m_szName[nNo]);

	return TRUE;
}
//-------------------------- class CActionName end --------------------------


//-------------------------- class CShadowFileName --------------------------

//---------------------------------------------------------------------------
// 功能：构造函数
//---------------------------------------------------------------------------
CShadowFileName::CShadowFileName()
{
	m_psInfo = NULL;
	m_nInfoLength = 0;
}

//---------------------------------------------------------------------------
// 功能：析构函数
//---------------------------------------------------------------------------
CShadowFileName::~CShadowFileName()
{
	if (m_psInfo)
		delete []m_psInfo;
	m_psInfo = NULL;
	m_nInfoLength = 0;
}

void	CShadowFileName::Init(int nActionCount)
{
	if (m_psInfo)
		delete []m_psInfo;
	m_psInfo = NULL;
	m_nInfoLength = 0;

	if (nActionCount <= 0)
		return;

	m_psInfo = (SShadowFileInfo*)new SShadowFileInfo[nActionCount];
	memset(m_psInfo, 0, sizeof(SShadowFileInfo) * nActionCount);
	m_nInfoLength = nActionCount;
}

//---------------------------------------------------------------------------
// 功能：得到某个动做的阴影文件名及其相关文件信息
//---------------------------------------------------------------------------
BOOL	CShadowFileName::GetFile(int nActionNo, int *pnFrame, int *pnDir, int *pnInterval, int *pnCgX, int *pnCgY, char *lpszName)
{
	if (nActionNo < 0 || nActionNo >= m_nInfoLength)
		return FALSE;
	if (!m_psInfo)
		return FALSE;

	*pnFrame	= m_psInfo[nActionNo].m_nTotalFrame;
	*pnDir		= m_psInfo[nActionNo].m_nTotalDir;
	*pnInterval	= m_psInfo[nActionNo].m_nInterval;
	*pnCgX		= m_psInfo[nActionNo].m_nCgX;
	*pnCgY		= m_psInfo[nActionNo].m_nCgY;
	strcpy(lpszName, m_psInfo[nActionNo].m_szName);

	return TRUE;
}

//------------------------ class CShadowFileName end ------------------------


//------------------------- class CStateMagicTable --------------------------
//---------------------------------------------------------------------------
// 功能:	构造函数
//---------------------------------------------------------------------------
CStateMagicTable::CStateMagicTable()
{
	for (int i = 0; i < MAX_RES_STATE; i++)
	{
		m_nType[i] = 0;
		m_nPlayType[i] = 0;
		m_nBackStart[i] = 0;
		m_nBackEnd[i] = 0;
		m_nTotalFrame[i] = 1;
		m_nTotalDir[i] = 1;
		m_nInterVal[i] = 1;
		m_szName[i][0] = 0;
	}
}

//---------------------------------------------------------------------------
// 功能:	初始化
//---------------------------------------------------------------------------
BOOL	CStateMagicTable::Init()
{
	KTabFile	cTabFile;
	int			i;
	char		szBuffer[80], szTemp[80];

//	g_SetFilePath(RES_INI_FILE_PATH);
	if ( !cTabFile.Load(STATE_MAGIC_TABLE_NAME) )
		return FALSE;

	//KNpcResNode		Temp;
	for (i = 0; i < MAX_RES_STATE; i++)
	{
		cTabFile.GetString(i + 2, 3, "", szBuffer, sizeof(szBuffer));
		if (strcmp(szBuffer, "头顶") == 0)
			m_nType[i] = STATE_MAGIC_HEAD;
		else if (strcmp(szBuffer, "脚底") == 0)
			m_nType[i] = STATE_MAGIC_FOOT;
		else
			m_nType[i] = STATE_MAGIC_BODY;
		cTabFile.GetString(i + 2, 4, "", szBuffer, sizeof(szBuffer));
		if (strcmp(szBuffer, "循环") == 0)
			m_nPlayType[i] = 0;
		else
			m_nPlayType[i] = 1;
		cTabFile.GetInteger(i + 2, 5, 0, &m_nBackStart[i]);
		cTabFile.GetInteger(i + 2, 6, 0, &m_nBackEnd[i]);
		cTabFile.GetInteger(i + 2, 7, 1, &m_nTotalFrame[i]);
		cTabFile.GetInteger(i + 2, 8, 1, &m_nTotalDir[i]);
		cTabFile.GetInteger(i + 2, 9, 1, &m_nInterVal[i]);
		cTabFile.GetString(i + 2, 2, "", szTemp, sizeof(szTemp));
		g_StrCpy(m_szName[i], szTemp);
//		Temp.ComposePathAndName(m_szName[i], RES_NPC_STYLE_PATH, szTemp);
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// 功能:	得到某种状态的光影效果的信息
// nNo 从 1 到 MAX_RES_STATE
//---------------------------------------------------------------------------
void	CStateMagicTable::GetInfo(int nNo, char *lpszGetName, int *pnType, int *pnPlayType, int *pnBackStart, int *pnBackEnd, int *pnTotalFrame, int *pnTotalDir, int *pnInterVal)
{
	if (!lpszGetName)
		return;
	if (nNo <= 0 || nNo > MAX_RES_STATE)
	{
		lpszGetName[0] = 0;
		*pnType = 0;
		*pnPlayType = 0;
		*pnBackStart = 0;
		*pnBackEnd = 0;
		*pnTotalFrame = 1;
		*pnTotalDir = 1;
		*pnInterVal = 1;
		return;
	}
	strcpy(lpszGetName, m_szName[nNo - 1]);
	*pnType = m_nType[nNo - 1];
	*pnPlayType = m_nPlayType[nNo - 1];
	*pnBackStart = m_nBackStart[nNo - 1];
	*pnBackEnd = m_nBackEnd[nNo - 1];
	*pnTotalFrame = m_nTotalFrame[nNo - 1];
	*pnTotalDir = m_nTotalDir[nNo - 1];
	*pnInterVal = m_nInterVal[nNo - 1];
}
//----------------------- class CStateMagicTable end ------------------------

//----------------------------- class CRESINFO ------------------------------
//---------------------------------------------------------------------------
// 功能:	构造函数
//---------------------------------------------------------------------------
CRESINFO::CRESINFO()
{
	m_nActionKind = 0;
	m_nEquipKind = 0;
	m_cSprInfo = NULL;
}

//---------------------------------------------------------------------------
// 功能:	析构函数
//---------------------------------------------------------------------------
CRESINFO::~CRESINFO()
{
	AutoDelete();
}

//---------------------------------------------------------------------------
// 功能:	清除数据
//---------------------------------------------------------------------------
void	CRESINFO::AutoDelete()
{
	if (m_cSprInfo)
		delete [] m_cSprInfo;
	m_cSprInfo = NULL;

	m_nActionKind = 0;
	m_nEquipKind = 0;
}

//---------------------------------------------------------------------------
// 功能:	设定动作种类数，装备种类数，分配表格内存缓冲区
//---------------------------------------------------------------------------
BOOL	CRESINFO::AutoNew(int nWidth, int nHeight)
{
	int		i;

	AutoDelete();

	m_nActionKind = nWidth;
	if (m_nActionKind < 0)
		m_nActionKind = 0;
	m_nEquipKind = nHeight;
	if (m_nEquipKind < 0)
		m_nEquipKind = 0;

	if (m_nActionKind == 0 || m_nEquipKind == 0)
		return FALSE;

	m_cSprInfo = (CSPR_INFO *)new CSPR_INFO[m_nActionKind * m_nEquipKind];
	if (m_cSprInfo == NULL)
		return FALSE;
	ZeroMemory(m_cSprInfo, sizeof(CSPR_INFO) * m_nActionKind * m_nEquipKind);
	
	for (i = 0; i < m_nActionKind * m_nEquipKind; i++)
	{
		m_cSprInfo[i].szFileName[0] = 0;
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// 功能:	根据动作编号、装备编号得到资源文件名
//---------------------------------------------------------------------------
BOOL	CRESINFO::GetName(int nActionNo, int nEquipNo, char *lpszDefault, char *lpszGetName, int nStrLen)
{
	if (!lpszGetName)
		return FALSE;
	if (m_cSprInfo == NULL)
		goto FALSE_LAB;
	if (nActionNo < 0 || nActionNo >= m_nActionKind || nEquipNo < 0 || nEquipNo >= m_nEquipKind)
		goto FALSE_LAB;

	strcpy(lpszGetName, m_cSprInfo[nEquipNo * m_nActionKind + nActionNo].szFileName);

	return TRUE;

FALSE_LAB:
	g_StrCpyLen(lpszGetName, lpszDefault, nStrLen);
	return FALSE;

}

int		CRESINFO::GetInterval(int nActionNo, int nEquipNo, int nDefault)
{
	if (nActionNo < 0 || nActionNo >= m_nActionKind || nEquipNo < 0 || nEquipNo >= m_nEquipKind)
		return nDefault;

	return m_cSprInfo[nEquipNo * m_nActionKind + nActionNo].nInterval;
}

int		CRESINFO::GetTotalFrames(int nActionNo, int nEquipNo, int nDefault)
{
	if (nActionNo < 0 || nActionNo >= m_nActionKind || nEquipNo < 0 || nEquipNo >= m_nEquipKind)
		return nDefault;

	return m_cSprInfo[nEquipNo * m_nActionKind + nActionNo].nTotalFrames;
}

int		CRESINFO::GetTotalDirs(int nActionNo, int nEquipNo, int nDefault)
{
	if (nActionNo < 0 || nActionNo >= m_nActionKind || nEquipNo < 0 || nEquipNo >= m_nEquipKind)
		return nDefault;

	return m_cSprInfo[nEquipNo * m_nActionKind + nActionNo].nTotalDirs;
}
//--------------------------- class CRESINFO end ----------------------------

//------------------------- class CEquipStyleTable --------------------------
//---------------------------------------------------------------------------
// 功能:	构造函数
//---------------------------------------------------------------------------
CEquipStyleTable::CEquipStyleTable()
{
	m_nStyleKind = 0;
	m_nEquipKind = 0;
	m_lpnActionNo = NULL;
}

//---------------------------------------------------------------------------
// 功能:	析构函数
//---------------------------------------------------------------------------
CEquipStyleTable::~CEquipStyleTable()
{
	AutoDelete();
}

//---------------------------------------------------------------------------
// 功能:	清除数据
//---------------------------------------------------------------------------
void	CEquipStyleTable::AutoDelete()
{
	if (m_lpnActionNo)
		delete [] m_lpnActionNo;
	m_lpnActionNo = NULL;
	m_nStyleKind = 0;
	m_nEquipKind = 0;
}

//---------------------------------------------------------------------------
// 功能:	设定表格长宽，初始化表格缓冲区
//---------------------------------------------------------------------------
BOOL	CEquipStyleTable::AutoNew(int nWidth, int nHeight)
{
	AutoDelete();

	if (nWidth < 0 || nHeight < 0)
		return FALSE;

	m_nStyleKind = nWidth;
	m_nEquipKind = nHeight;

	m_lpnActionNo = new int[nWidth * nHeight];
	if (m_lpnActionNo == NULL)
		return FALSE;
	for (int i = 0; i < nWidth * nHeight; i++)
		m_lpnActionNo[i] = -1;

	return TRUE;
}

//---------------------------------------------------------------------------
// 功能:	设定表格中某一项的值
//---------------------------------------------------------------------------
void	CEquipStyleTable::SetValue(int nXpos, int nYpos, int nValue)
{
	if (nXpos < 0 || nXpos >= m_nStyleKind || nYpos < 0 || nYpos >= m_nEquipKind)
		return;
	m_lpnActionNo[nYpos * m_nStyleKind + nXpos] = nValue;
}

//---------------------------------------------------------------------------
// 功能:	得到表格中某一项的值
// 参数:	nXpos : 装备编号  nYpos : 状态编号
// 返回:	动作编号
//---------------------------------------------------------------------------
int		CEquipStyleTable::GetValue(int nXpos, int nYpos)
{
	if (nXpos < 0 || nXpos >= m_nEquipKind || nYpos < 0 || nYpos >= m_nStyleKind)
		return -1;
	if (m_lpnActionNo == NULL)
		return -1;
	return m_lpnActionNo[nXpos * m_nStyleKind + nYpos];
}

//----------------------- class CEquipStyleTable end ------------------------

//---------------------------- class CSortTable -----------------------------
//---------------------------------------------------------------------------
// 功能:	构造函数
//---------------------------------------------------------------------------
CSortTable::CSortTable()
{
	m_lpnSortTable = NULL;
	m_nSortTableSize = 0;
	m_nPartNum = 1;
	strcpy(m_sSortTableFileName,"");
}

//---------------------------------------------------------------------------
// 功能:	析构函数
//---------------------------------------------------------------------------
CSortTable::~CSortTable()
{
	Release();
}

//---------------------------------------------------------------------------
// 功能:	清除数据
//---------------------------------------------------------------------------
void	CSortTable::Release()
{
	int		i;

	if (m_lpnSortTable)
	{
		delete [] m_lpnSortTable;
		m_lpnSortTable = NULL;
	}
	m_nSortTableSize = 0;

	for (i = 0; i < MAX_ACTION; i++)
		m_sActTableOff[i].Clear();
}

//---------------------------------------------------------------------------
// 功能:	从字符串中把一系列数字取出来
// 参数:	lpszStr : 字符串  lpnTable : 接受数字的数组  nTableSize : 数组的大小
//---------------------------------------------------------------------------
void	CSortTable::SortStrToNum(char *lpszStr, int *lpnTable, int nTableSize)
{
	if ( !lpszStr || !lpszStr[0] || !lpnTable || nTableSize < 1)
		return;
	if (nTableSize > m_nPartNum + 1)
		nTableSize = m_nPartNum + 1;

	int		nTablePos = 0, nNumPos = 0, nStrPos = 0;
	char	szNum[32];

	while (1)
	{
		if (lpszStr[nStrPos] == '-' || 
			('0' <= lpszStr[nStrPos] && lpszStr[nStrPos] <= '9'))
		{
			szNum[nNumPos] = lpszStr[nStrPos];
			nNumPos++;
		}
		else
		{
			if (nNumPos > 0)
			{
				szNum[nNumPos] = 0;
				nNumPos = 0;
				if (nTablePos < nTableSize)
				{
					lpnTable[nTablePos] = atoi(szNum);
					nTablePos++;
				}
			}
			if ( lpszStr[nStrPos] == 0 )
				break;
		}
		nStrPos ++;
	}
	for (int i = nTablePos; i < nTableSize; i++)
		lpnTable[i] = -1;
}

//---------------------------------------------------------------------------
// 功能:	从排序文件中把排序信息读取出来
// 参数:	
//			lpszFileName: 排序文件名
//			*cActionName: 用于获取动作的各种信息
//			nPartNum：部件的数量
// 返回:	读取成功与否
//---------------------------------------------------------------------------
BOOL	CSortTable::GetTable(char *lpszFileName, CActionName *cActionName, int nPartNum)
{
	if ( !lpszFileName || !lpszFileName[0])
		return FALSE;

	char		szBuf[FILE_NAME_LENGTH];
	KIniFile	SortIni;

//	g_SetFilePath(RES_INI_FILE_PATH);
	g_UnitePathAndName(RES_INI_FILE_PATH, lpszFileName, szBuf);
	if ( !SortIni.Load(szBuf) )
		return FALSE;
	return GetTable(&SortIni, cActionName, nPartNum);
}
BOOL	CSortTable::GetTable(KIniFile *pIni, CActionName *cActionName, int nPartNum)
{
//	if ( !lpszFileName || !lpszFileName[0])
//		return FALSE;
	if (nPartNum > 1)
		m_nPartNum = nPartNum;

//	KIniFile	SortIni;
	int			i, j, nActNo;
	char		szBuffer[256], szKey[80], szActName[80];

//	g_SetFilePath(RES_INI_FILE_PATH);
//	if ( !SortIni.Load(lpszFileName) )
//		return FALSE;
//	KIniFile	*pIni = &SortIni;

// ----------------------- 通过遍历文件计算 Table 的大小 --------------------------
	Release();
	// default 部分，16 个方向
	for (i = 0; i < MAX_SORT_DIR; i++)
	{
		sprintf(szKey, "Dir%d", i + 1);
		if( pIni->GetString("DEFAULT", szKey, "", szBuffer, sizeof(szBuffer)) )
			m_nSortTableSize ++;
	}
	// 按动作逐个计算
	nActNo = cActionName->GetActionCount();
	if (nActNo <= 0)
		return FALSE;
	for (i = 0; i < nActNo; i++)
	{
		// 得到动作名
		if ( !cActionName->GetActionName(i, szActName, sizeof(szActName)) )
			continue;
		if( !pIni->IsSectionExist(szActName) )
			continue;
		// 判断是否使用默认方向顺序
		for (j = 0; j < MAX_SORT_DIR; j++)
		{
			sprintf(szKey, "Dir%d", j + 1);
			if( pIni->GetString(szActName, szKey, "", szBuffer, sizeof(szBuffer)) )
			{
				m_sActTableOff[i].bUseDefault = FALSE;
				m_sActTableOff[i].nActOff = m_nSortTableSize;
				m_sActTableOff[i].nLineNum = MAX_SORT_DIR;
				break;
			}
		}
		// 判断是否有需要特殊排序的特殊桢
		j = 1;
		while (1)
		{
			sprintf(szKey, "Line%d", j);
			if( pIni->GetString(szActName, szKey, "", szBuffer, sizeof(szBuffer)) )
			{
				if (m_sActTableOff[i].nActOff == 0)
				{
					m_sActTableOff[i].nActOff = m_nSortTableSize;
					m_sActTableOff[i].nLineNum = 1;
				}
				else
				{
					m_sActTableOff[i].nLineNum++;
				}
			}
			else
			{
				break;
			}

			j++;
		}
		m_nSortTableSize += m_sActTableOff[i].nLineNum;
	}
	if (m_nSortTableSize < MAX_SORT_DIR)
		return FALSE;
// --------------------- 通过遍历文件计算 Table 的大小 end ------------------------

// ------------------------------ 读入具体的对应表数据 ----------------------------
	// 给 m_lpnSortTable 分配内存
	m_lpnSortTable = (int *)new int[m_nSortTableSize * (m_nPartNum + 1)];
	if (m_lpnSortTable == NULL)
	{
		Release();
		return FALSE;
	}
	for (i = 0; i < m_nSortTableSize * (m_nPartNum + 1); i++)
	{
		m_lpnSortTable[i] = -1;
	}

	int		nTablePos = 0;
	// default 部分，16 个方向
	for (i = 0; i < MAX_SORT_DIR; i++)
	{
		sprintf(szKey, "Dir%d", i + 1);
		if( pIni->GetString("DEFAULT", szKey, "", szBuffer, sizeof(szBuffer)) )
		{
			SortStrToNum(szBuffer, &m_lpnSortTable[nTablePos * (m_nPartNum + 1)], m_nPartNum + 1);
		}
		nTablePos++;
	}
	// 按动作逐个读入
	for (i = 0; i < nActNo; i++)
	{
		// 如果完全使用默认值，不用再处理
		if (m_sActTableOff[i].nActOff == 0)
			continue;
		// 得到动作名
		if ( !cActionName->GetActionName(i, szActName, sizeof(szActName)) )
			continue;
		if( !pIni->IsSectionExist(szActName) )
			continue;
		// 如果方向不使用默认值，读入方向顺序
		if (m_sActTableOff[i].bUseDefault == FALSE)
		{
			for (j = 0; j < MAX_SORT_DIR; j++)
			{
				sprintf(szKey, "Dir%d", j + 1);
				if ( pIni->GetString(szActName, szKey, "", szBuffer, sizeof(szBuffer)) )
				{
					SortStrToNum(szBuffer, &m_lpnSortTable[nTablePos * (m_nPartNum + 1)], m_nPartNum + 1);
				}
				else
				{
					memcpy(&m_lpnSortTable[nTablePos * (m_nPartNum + 1)], &m_lpnSortTable[j * (m_nPartNum + 1)], sizeof(int) * (m_nPartNum + 1));
				}
				nTablePos++;
			}
		}
		// 读入特殊行顺序
		if ((m_sActTableOff[i].bUseDefault == FALSE && m_sActTableOff[i].nLineNum > MAX_SORT_DIR) || 
			(m_sActTableOff[i].bUseDefault == TRUE && m_sActTableOff[i].nLineNum > 0))
		{
			j = 1;
			while (1)
			{
				sprintf(szKey, "Line%d", j);
				if( !pIni->GetString(szActName, szKey, "", szBuffer, sizeof(szBuffer)) )
					break;
				SortStrToNum(szBuffer, &m_lpnSortTable[nTablePos * (m_nPartNum + 1)], m_nPartNum + 1);
				nTablePos++;
				j++;
			}
		}
	}
// ---------------------------- 读入具体的对应表数据 end --------------------------

	return TRUE;
}

//---------------------------------------------------------------------------
// 功能:	从表格中得到一组排序信息
// 参数:	
//			nActNo : 动作编号
//			nDir : 当前方向 ( 0 -- 16 )
//			nFrameNo : 当前第几桢
//			lpnTable : 接收排序信息的数组
//			nTableLen : lpnTable的长度
// 返回:	读取成功与否
//---------------------------------------------------------------------------
BOOL	CSortTable::GetSort(int nActNo, int nDir, int nFrameNo, int *lpnTable, int nTableLen)
{
	if (!lpnTable)
		return FALSE;
	if (nActNo < 0 || nActNo >= MAX_ACTION)
		return FALSE;
	if (nDir < 0 || nDir >= MAX_SORT_DIR)
		return FALSE;
	if (!m_lpnSortTable)
		return FALSE;

	int		i;

	// 完全使用默认
	if (m_sActTableOff[nActNo].nActOff == 0)
	{
		if (nTableLen <= m_nPartNum)
			memcpy(lpnTable, &m_lpnSortTable[nDir * (m_nPartNum + 1) + 1], sizeof(int) * nTableLen);
		else
		{
			memcpy(lpnTable, &m_lpnSortTable[nDir * (m_nPartNum + 1) + 1], sizeof(int) * m_nPartNum);
			for (i = m_nPartNum; i < nTableLen; i++)
				lpnTable[i] = -1;
		}
	}
	else
	{
		// 方向使用默认，带特殊桢
		if (m_sActTableOff[nActNo].bUseDefault == TRUE)
		{
			// 判断是否是特殊桢
			for (i = 0; i < m_sActTableOff[nActNo].nLineNum; i++)
			{
				// 找到特殊桢
				if (nFrameNo == m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + i) * (m_nPartNum + 1)])
				{
					if (nTableLen <= m_nPartNum)
						memcpy(lpnTable, &m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + i) * (m_nPartNum + 1) + 1], sizeof(int) * nTableLen);
					else
					{
						memcpy(lpnTable, &m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + i) * (m_nPartNum + 1) + 1], sizeof(int) * m_nPartNum);
						for (int j = m_nPartNum; j < nTableLen; j++)
							lpnTable[j] = -1;
					}
					return TRUE;
				}
			}
			// 到此，没有找到特殊桢
			if (nTableLen <= m_nPartNum)
				memcpy(lpnTable, &m_lpnSortTable[nDir * (m_nPartNum + 1) + 1], sizeof(int) * nTableLen);
			else
			{
				memcpy(lpnTable, &m_lpnSortTable[nDir * (m_nPartNum + 1) + 1], sizeof(int) * m_nPartNum);
				for (i = m_nPartNum; i < nTableLen; i++)
					lpnTable[i] = -1;
			}
		}
		// 方向不使用默认
		else
		{
			// 判断是否是特殊桢
			for (i = MAX_SORT_DIR; i < m_sActTableOff[nActNo].nLineNum; i++)
			{
				// 找到特殊桢
				if (nFrameNo == m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + i) * (m_nPartNum + 1)])
				{
					if (nTableLen <= m_nPartNum)
						memcpy(lpnTable, &m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + i) * (m_nPartNum + 1) + 1], sizeof(int) * nTableLen);
					else
					{
						memcpy(lpnTable, &m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + i) * (m_nPartNum + 1) + 1], sizeof(int) * m_nPartNum);
						for (int j = m_nPartNum; j < nTableLen; j++)
							lpnTable[j] = -1;
					}
					return TRUE;
				}
			}
			// 到此，没有找到特殊桢
			if (nTableLen <= m_nPartNum)
				memcpy(lpnTable, &m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + nDir) * (m_nPartNum + 1) + 1], sizeof(int) * nTableLen);
			else
			{
				memcpy(lpnTable, &m_lpnSortTable[(m_sActTableOff[nActNo].nActOff + nDir) * (m_nPartNum + 1) + 1], sizeof(int) * m_nPartNum);
				for (i = m_nPartNum; i < nTableLen; i++)
					lpnTable[i] = -1;
			}
		}
	}

	return TRUE;
}
//-------------------------- class CSortTable end ---------------------------

#endif