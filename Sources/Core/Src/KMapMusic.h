//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KMapMusic.h
// Date:	2003-5-18 17:22
// Code:	边城浪子
// 说明：游戏世界的音乐，具体策划需求设定请参阅
//			sourcesafe策划库\Setting\GAMESETTING\音乐系统设定.doc
//---------------------------------------------------------------------------

#pragma once

#ifndef _SERVER

#define		defONE_MAP_MAX_MUSIC			4				// 每张地图最大可能音乐数
#define		defGAME_TIME_ONE_DAY			(1440 * 120)	// 游戏每天时间的长度
#define		defMUSIC_MAX_STAGE				32
#define		defMUSIC_STATE_MAX_MAP			64
#define		defMUSIC_STATE_MAX_MUSIC		16

enum
{
	enumMUSIC_STATE_STOP = 0,
	enumMUSIC_STATE_AUTO,
	enumMUSIC_STATE_SCRIPT,
	enumMUSIC_STATE_NUM,
};

// 每首音乐的参数
struct	KMapMusicInfo
{
	int		m_nVolume;						// 音乐声音大小
	int		m_nStartTime;					// 音乐在游戏时间的什么阶段播放：起始时间
	int		m_nEndTime;						// 音乐在游戏时间的什么阶段播放：结束时间
	char	m_szFileName[FILE_NAME_LENGTH];	// 音乐文件名(含从游戏跟目录开始的完整游戏路径)
};

// 每张地图的音乐参数
struct	KMapAllMusic
{
	int				m_nMapID;				// 地图id
	KMapMusicInfo	m_sMusic[defONE_MAP_MAX_MUSIC];	// 本地图所有音乐的参数数据

	KMapAllMusic() {m_nMapID = 0; ZeroMemory(m_sMusic, sizeof(m_sMusic));};	// 构造函数，初始化
};

struct	KMapFightMusic
{
	int		m_nMusicNum;
	int		m_nMapID[defMUSIC_STATE_MAX_MAP];
	char	m_szMusicName[defMUSIC_STATE_MAX_MUSIC][FILE_NAME_LENGTH];
};

// 游戏世界的音乐
class	CORE_API KMapMusic
{
private:
	int				m_nState;			// 当前状态

	int				m_nCurMapID;		// 当前地图id
	int				m_nCurInfoNo;		// 当前地图id在信息数据的位置
	int				m_nCurMusicNo;		// 当前正在播放本地图第几首音乐
	int				m_nGameVolume;		// 当前游戏设定音乐声音大小(0(无声) -- 100(正常))
	int				m_nWeatherVolume;	// 当前天气导致的音乐声音大小的变化(0(无声) -- 100(正常))
	char			m_szCurName[FILE_NAME_LENGTH];	// 当前音乐文件名

	KMapAllMusic	*m_pMusicInfo;		// 所有地图的音乐信息
	int				m_nInfoSize;		// 所有地图音乐信息数据的大小(单位:sizeof(KMapAllMusic))
	int				m_nLoadFlag;		// 游戏音乐信息是否已经载入

	KMapFightMusic	m_sFightMusic[defMUSIC_MAX_STAGE];

	BOOL			m_bFightMode;
	int				m_nCurStage;

public:
	KMapMusic();
	~KMapMusic();
	void			LoadSetFile();		// 载入地图音乐信息
	void			Init();				// 初始化，载入地图音乐信息

	// 游戏世界音乐播放
	// 参数：nMapID 地图id
    //		nGameTime 游戏时间(0 -- 1440)
	void			Play(int nMapID, int nGameTime, BOOL bFightMode);
	void			Stop();

	void			Start(int nMapID, int nGameTime, BOOL bFightMode);// 进入游戏或者音乐设定打开的时候，播放音乐
	void			SetGameVolume(int nVolume);	// 设定游戏音乐总体音量大小(0 -- 100)
	void			SetWeatherVolume(int nVolume);	// 设定天气对音乐音量大小的影响(0 -- 100)
	void			ScriptPlay(char *lpszMusicName);// 用脚本播放音乐

private:
	void			Release();			// 清空

};


#endif