//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KMapMusic.cpp
// Date:	2003-5-18 17:22
// Code:	边城浪子
//---------------------------------------------------------------------------
#include "KCore.h"

#ifndef _SERVER

#include	"KMapMusic.h"

#define	GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS		\
	(-10000 + (m_nGameVolume * m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_nVolume * m_nWeatherVolume) / 100)

//---------------------------------------------------------------------------
//	功能：构造函数
//---------------------------------------------------------------------------
KMapMusic::KMapMusic()
{
	m_nState			= enumMUSIC_STATE_STOP;

	m_nCurMapID			= -1;
	m_nCurInfoNo		= 0;
	m_nCurMusicNo		= 0;
	m_nGameVolume		= 100;
	m_nWeatherVolume	= 100;
	m_szCurName[0]		= 0;

	m_pMusicInfo		= NULL;
	m_nInfoSize			= 0;
	m_nLoadFlag			= FALSE;

	m_bFightMode		= FALSE;
	m_nCurStage			= -1;

	memset(m_sFightMusic, 0, sizeof(m_sFightMusic));
	int		i, j;
	for (i = 0; i < defMUSIC_MAX_STAGE; i++)
	{
		for (j = 0; j < defMUSIC_STATE_MAX_MAP; j++)
			this->m_sFightMusic[i].m_nMapID[j] = -1;
	}
}

//---------------------------------------------------------------------------
//	功能：析构函数
//---------------------------------------------------------------------------
KMapMusic::~KMapMusic()
{
	Release();
}

//---------------------------------------------------------------------------
//	功能：清空
//---------------------------------------------------------------------------
void	KMapMusic::Release()
{
	m_nState			= enumMUSIC_STATE_STOP;

	m_nCurMapID			= -1;
	m_nCurMusicNo		= 0;
	m_nGameVolume		= 100;
	m_nWeatherVolume	= 100;
	m_szCurName[0]		= 0;

	if (m_pMusicInfo)
	{
		delete []m_pMusicInfo;
		m_pMusicInfo = NULL;
	}
	m_nInfoSize = 0;
	m_nLoadFlag = FALSE;

	memset(m_sFightMusic, 0, sizeof(m_sFightMusic));
}

//---------------------------------------------------------------------------
//	功能：载入地图音乐信息
//---------------------------------------------------------------------------
void	KMapMusic::LoadSetFile()
{
	KTabFile	cFile;
	int			nHeight, i, j;

	Release();

//	g_SetFilePath("\\");
	if ( !cFile.Load(defMUSIC_SET_FILE) )
		return;
	nHeight = cFile.GetHeight();
	if (nHeight <= 1)
		return;
	m_nInfoSize = nHeight - 1;
	m_pMusicInfo = (KMapAllMusic*)new KMapAllMusic[m_nInfoSize];
	for (i = 0; i < m_nInfoSize; i++)
	{
		cFile.GetInteger(i + 2, 1, 0, &m_pMusicInfo[i].m_nMapID);
		for (j = 0; j < defONE_MAP_MAX_MUSIC; j++)
		{
			cFile.GetString(i + 2, 2 + j * 5, "", m_pMusicInfo[i].m_sMusic[j].m_szFileName, FILE_NAME_LENGTH);
			cFile.GetInteger(i + 2, 2 + j * 5 + 1, 100, &m_pMusicInfo[i].m_sMusic[j].m_nVolume);
			cFile.GetInteger(i + 2, 2 + j * 5 + 2, 0, &m_pMusicInfo[i].m_sMusic[j].m_nStartTime);
			cFile.GetInteger(i + 2, 2 + j * 5 + 3, defGAME_TIME_ONE_DAY - 1, &m_pMusicInfo[i].m_sMusic[j].m_nEndTime);
		}
	}

	int			nStage, nMapNum, nMusicNum;
	char		szSection[32], szKey[32];
	KIniFile	cFightFile;
//	g_SetFilePath("\\");
	if ( !cFightFile.Load(defMUSIC_FIGHT_SET_FILE) )
		return;
	cFightFile.GetInteger("Stage", "StageNum", 0, &nStage);
	if (nStage > defMUSIC_MAX_STAGE)
		nStage = defMUSIC_MAX_STAGE;
	for (i = 0; i < nStage; i++)
	{
		sprintf(szSection, "Stage%d", i);

		cFightFile.GetInteger(szSection, "MapNum", 0, &nMapNum);
		if (nMapNum > defMUSIC_STATE_MAX_MAP)
			nMapNum = defMUSIC_STATE_MAX_MAP;
		for (j = 0; j < nMapNum; j++)
		{
			sprintf(szKey, "Map%03d", j);
			cFightFile.GetInteger(szSection, szKey, 0, &m_sFightMusic[i].m_nMapID[j]);
		}

		cFightFile.GetInteger(szSection, "MusicNum", 0, &nMusicNum);
		if (nMusicNum > defMUSIC_STATE_MAX_MUSIC)
			nMusicNum = defMUSIC_STATE_MAX_MUSIC;
		m_sFightMusic[i].m_nMusicNum = nMusicNum;
		for (j = 0; j < nMusicNum; j++)
		{
			sprintf(szKey, "Music%03d", j);
			cFightFile.GetString(szSection, szKey, NULL, m_sFightMusic[i].m_szMusicName[j], FILE_NAME_LENGTH);
			if (!m_sFightMusic[i].m_szMusicName[j][0])
			{
				m_sFightMusic[i].m_nMusicNum = j;
				break;
			}
		}
	}

	m_nLoadFlag = TRUE;
}

//---------------------------------------------------------------------------
//	功能：初始化，载入地图音乐信息
//---------------------------------------------------------------------------
void	KMapMusic::Init()
{
	LoadSetFile();

	SetGameVolume(100);
	SetWeatherVolume(100);
}

//---------------------------------------------------------------------------
//	功能：游戏世界音乐播放
//	参数：nMapID 地图id    nGameTime 游戏时间(0 -- 1440)
//---------------------------------------------------------------------------
void	KMapMusic::Play(int nMapID, int nGameTime, BOOL bFightMode)
{
	if (m_nState == enumMUSIC_STATE_STOP)
		return;

	if ( !m_pMusicInfo || m_nInfoSize <= 0)
		return;
	if (nMapID <= 0)
		return;
	if (!g_pMusic)
		return;

	int		i, j;

	if (m_nState == enumMUSIC_STATE_SCRIPT)
	{
		if (g_pMusic->IsPlaying())
		{
			g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
		}
		else
		{
			g_pMusic->Stop();
			m_nState = enumMUSIC_STATE_AUTO;
			Start(nMapID, nGameTime, bFightMode);
		}
		return;
	}

	// 	if (m_nState == enumMUSIC_STATE_AUTO)

	// 非战斗模式
	if (!bFightMode)
	{
		nGameTime %= defGAME_TIME_ONE_DAY;
		// 如果换了地图，或者从战斗模式变成非战斗模式
		if (nMapID != m_nCurMapID || m_bFightMode)
		{
			m_nCurMapID = nMapID;
			m_bFightMode = bFightMode;
			// 查找当前应该播放哪一首音乐
			for (i = 0; i < m_nInfoSize; i++)
			{
				if (this->m_pMusicInfo[i].m_nMapID == nMapID)
					break;
			}
			if (i >= m_nInfoSize)
				return;
			m_nCurInfoNo = i;
			m_nCurMusicNo = 0;
			for (i = 0; i < defONE_MAP_MAX_MUSIC; i++)
			{
				if (m_pMusicInfo[m_nCurInfoNo].m_sMusic[i].m_nStartTime <= nGameTime &&
					nGameTime <= m_pMusicInfo[m_nCurInfoNo].m_sMusic[i].m_nEndTime)
				{
					this->m_nCurMusicNo = i;
					break;
				}
			}

			// 找到了当前应该播放哪一首音乐，处理之
			// 如果当前正在播放应该播放的音乐，继续
			if (g_pMusic->IsPlaying() && 
				strcmp(m_szCurName, m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_szFileName) == 0)
			{
				g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
				return;
			}
			// 播放应该播放的音乐
			strcpy(m_szCurName, m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_szFileName);
			if (!m_szCurName[0])
				return;
			g_pMusic->Stop();
//			g_SetFilePath("\\");
			if (m_szCurName[0] && g_pMusic->Open(m_szCurName))
			{
				g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
				g_pMusic->Play(FALSE);
			}

			return;
		}

		// 正常播放，循环之
		if (m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_nStartTime <= nGameTime &&
			nGameTime <= m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_nEndTime)
		{
			if (g_pMusic->IsPlaying())
			{
				g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
			}
			else
			{
				g_pMusic->Stop();
				if (m_szCurName[0] && g_pMusic->Open(m_szCurName))
				{
					g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
					g_pMusic->Play(FALSE);
				}
			}
			return;
		}

		// 时间区域变了，换音乐
		m_nCurMusicNo = 0;
		for (i = 0; i < defONE_MAP_MAX_MUSIC; i++)
		{
			if (m_pMusicInfo[m_nCurInfoNo].m_sMusic[i].m_nStartTime <= nGameTime &&
				nGameTime <= m_pMusicInfo[m_nCurInfoNo].m_sMusic[i].m_nEndTime)
			{
				this->m_nCurMusicNo = i;
				break;
			}
		}

		// 如果同名，继续播放
		if (strcmp(m_szCurName, m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_szFileName) == 0)
		{
			g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
			return;
		}

		// 不同名，换音乐
		strcpy(m_szCurName, m_pMusicInfo[m_nCurInfoNo].m_sMusic[m_nCurMusicNo].m_szFileName);
		g_pMusic->Stop();
//		g_SetFilePath("\\");
		if (m_szCurName[0] && g_pMusic->Open(m_szCurName))
		{
			g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
			g_pMusic->Play(FALSE);
		}
	}
	// 战斗模式
	else
	{
		int		nFind;
		// 如果换了地图，或者从非战斗模式变成战斗模式
		if (nMapID != m_nCurMapID || !m_bFightMode)
		{
			m_bFightMode = bFightMode;
			m_nCurMapID = nMapID;
			// 查找当前地图在哪个区域
			nFind = 0;
			for (i = 0; i < defMUSIC_MAX_STAGE; i++)
			{
				if (m_sFightMusic[i].m_nMapID[0] == -1)
					break;
				for (j = 0; j < defMUSIC_STATE_MAX_MAP; j++)
				{
					if (m_sFightMusic[i].m_nMapID[j] == -1)
						break;
					if (m_sFightMusic[i].m_nMapID[j] == nMapID)
					{
						nFind = 1;
						break;
					}
				}
				if (nFind)
					break;
			}
			// 找到了当前地图在哪个区域，处理之
			if (nFind)
			{
				m_nCurStage = i;
				// 判断当前播放的音乐是否是本区域的，如果是，不中断，否则重新找一首音乐
				nFind = 0;
				for (i = 0; i < defMUSIC_STATE_MAX_MUSIC; i++)
				{
					if (m_sFightMusic[m_nCurStage].m_szMusicName[i][0] &&
						strcmp(m_sFightMusic[m_nCurStage].m_szMusicName[i], m_szCurName) == 0)
					{
						nFind = 1;
						m_nCurMusicNo = i;
						break;
					}
				}
				// 当前播放的音乐正是本区域的，不处理了
				if (nFind)
					return;
				// 当前播放的音乐不是本区域的，中断，重新找一首音乐
				g_pMusic->Stop();
				m_nCurMusicNo = g_Random(m_sFightMusic[m_nCurStage].m_nMusicNum);
				strcpy(m_szCurName, m_sFightMusic[m_nCurStage].m_szMusicName[m_nCurMusicNo]);
//				g_SetFilePath("\\");
				if (m_szCurName[0] && g_pMusic->Open(m_szCurName))
				{
					g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
					g_pMusic->Play(FALSE);
				}
			}
			return;
		}

		// 如果音乐正在播放，只需要设定音量大小
		if (g_pMusic->IsPlaying())
		{
			g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
			return;
		}
		// 如果音乐播放结束，重新找一首音乐，播放之
		else
		{
			if (m_nCurStage < 0 || m_sFightMusic[m_nCurStage].m_nMusicNum <= 0)
				return;
			m_nCurMusicNo = g_Random(m_sFightMusic[m_nCurStage].m_nMusicNum);
			if (!m_sFightMusic[m_nCurStage].m_szMusicName[m_nCurMusicNo][0])
				return;
			strcpy(m_szCurName, m_sFightMusic[m_nCurStage].m_szMusicName[m_nCurMusicNo]);
			g_pMusic->Stop();
//			g_SetFilePath("\\");
			if (m_szCurName[0] && g_pMusic->Open(m_szCurName))
			{
				g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
				g_pMusic->Play(FALSE);
			}
			return;
		}
	}
}

void	KMapMusic::Stop()
{
	if (g_pMusic)
		g_pMusic->Stop();
	m_nState = enumMUSIC_STATE_STOP;
}

//---------------------------------------------------------------------------
//	功能：设定游戏音乐总体音量大小(0(无声) -- 100(正常))
//---------------------------------------------------------------------------
void	KMapMusic::SetGameVolume(int nVolume)
{
	if (nVolume <= 0)
		nVolume = 0;

	if (nVolume > 100)
		nVolume = 100;

	m_nGameVolume = nVolume;

	if (g_pMusic)
		g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
}

//---------------------------------------------------------------------------
//	功能：设定天气对音乐音量大小的影响(0(无声) -- 100(正常))
//---------------------------------------------------------------------------
void	KMapMusic::SetWeatherVolume(int nVolume)
{
	if (nVolume < 0)
		m_nWeatherVolume = 0;
	if (nVolume > 100)
		m_nWeatherVolume = 100;
	m_nWeatherVolume = nVolume;
	if (g_pMusic)
		g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
}

//---------------------------------------------------------------------------
//	功能：进入游戏或者音乐设定打开的时候，播放音乐
//---------------------------------------------------------------------------
void	KMapMusic::Start(int nMapID, int nGameTime, BOOL bFightMode)
{
	this->m_nCurMapID = -1;
	m_nState = enumMUSIC_STATE_AUTO;
	Play(nMapID, nGameTime, bFightMode);
}

void	KMapMusic::ScriptPlay(char *lpszMusicName)
{
	if (!lpszMusicName || !lpszMusicName[0])
		return;

	m_nState = enumMUSIC_STATE_SCRIPT;
	strcpy(m_szCurName, lpszMusicName);
	g_pMusic->Stop();
	if (m_szCurName[0] && g_pMusic->Open(m_szCurName))
	{
		g_pMusic->SetVolume(GAMEVALUE_TO_HUNDREDTHS_OF_DECIBELS);
		g_pMusic->Play(FALSE);
	}
}

#endif