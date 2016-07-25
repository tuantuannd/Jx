
#ifndef _SERVER

#ifndef __KWEATHER_H__
#define __KWEATHER_H__

#include "..\GameDataDef.h"
#include "../../Represent/iRepresent/KRepresentUnit.h"
#include "SceneDataDef.h"
#include "../../Represent/iRepresent/iRepresentShell.h"

#pragma warning(disable : 4786)
#include <list>
#ifndef __CODEWARRIOR
using namespace std;
#endif

#define WEATHER_AREA_WIDTH		1200
#define WEATHER_AREA_HEIGHT		2000

struct KParticle
{
	KPosition3 m_vPos;				// 粒子位置
	KPosition3 m_vSpeed;			// 粒子速度
	KRColor	m_color;				// 粒子颜色
};

class KWeather
{
public:

public:
	KWeather();
	virtual ~KWeather();
	void SetParticleNum(int nNum)
	{
		if(nNum < 0)
			return;
		m_nParticleNum = nNum;
	}

	void SetDownSpeed(float nSpeed)
	{
		m_nDownSpeed = nSpeed;
	}

	void SetWindSpeed(float nSpeed)
	{
		m_nWindSpeed = nSpeed;
	}

	void SetLifeTime(int nTime)
	{
		if(nTime < 0)
			return;
		m_nLifeTime = nTime * 20;
	}

	void SetFocusPos(int nX, int nY)
	{
		m_nFocusPosX = nX;
		m_nFocusPosY = nY;

		m_nLeftTopX = nX - WEATHER_AREA_WIDTH / 2;
		m_nLeftTopY = nY - WEATHER_AREA_HEIGHT / 2;
	}

	void ShutDown()
	{
		m_bIsShutingDown = true;
	}

	virtual bool IsShutDown() = 0;
	virtual bool ReadInfoFromIniFile(int nWeather) = 0;
	virtual void FilterAmbient(DWORD &dwLight){}
	virtual void Breath();
	virtual void Render(iRepresentShell *pRepresent) = 0;

protected:
	bool	m_bIsShutingDown;			// 是否正在关闭
	int		m_nStartTime;				// 天气开始时间,游戏帧计数
	int		m_nLifeTime;				// 天气持续的时间，秒
	int		m_nParticleNum;				// 每秒产生的粒子数目
	float	m_nDownSpeed;				// 下落速度，每秒游戏单位数
	float	m_nWindSpeed;				// 横向风俗，每秒游戏单位数

	int		m_nLeftTopX;				// 九个焦点区域左上角X坐标
	int		m_nLeftTopY;				// 九个焦点区域左上角Y坐标

	int		m_nFocusPosX;				// 焦点坐标X
	int		m_nFocusPosY;				// 焦点坐标Y

	list<KParticle> m_ParticleList;		// 粒子链表
private:

};

class KWeatherRain : public KWeather
{
public:
	KWeatherRain();
	~KWeatherRain();

	void SetFlareTime(int nTime)
	{
		m_nFlareTime = nTime * 2 * 20;
	}
public:
	virtual bool IsShutDown()
	{
		return m_bIsShutDown;
	}
	virtual bool ReadInfoFromIniFile(int nWeatherID);
	virtual void FilterAmbient(DWORD &dwLight);
	virtual void Breath();
	virtual void Render(iRepresentShell *pRepresent);
private:
	int		m_nLenRate;					// 雨点长度为最大长度的百分比
	bool	m_bIsShutDown;				// 是否已经关闭
	int		m_nFlareTime;				// 平均多少时间一次闪电，单位为游戏帧数
	int		m_nLastFlareTime;			// 上次闪电时间，单位为游戏帧数
	int		m_nFlareDelay;				// 下一次闪电距上一次闪电的时间，单位为游戏帧数
	int		m_nFlareState;				// 闪电阶段，为－1时为不闪电
	int		m_nFlareStateStart;			// 闪电阶段起始时间，单位为游戏帧数
	int		m_nFlareStateDelay;			// 闪电阶段延续时间，单位为游戏帧数
	KLColor m_flareColorAdd;			// 闪电各阶段叠加在环境光上的亮度
	float	m_fAmbientFactor;			// 环境光比例因子

	char	m_szRainSound[80];			// 下雨声
	char	m_szFlareSounds[3][80];		// 打雷声
	KWavSound *m_pSound;
	int		m_nSoundVolume;				// 下雨声音量

private:
};

class KWeatherSnow : public KWeather
{
public:
	KWeatherSnow()
	{
		m_bIsShutDown = false;
	}

public:
	virtual bool IsShutDown()
	{
		return m_bIsShutDown;
	}
	virtual bool ReadInfoFromIniFile(int nWeatherID);
	virtual void Breath();
	virtual void Render(iRepresentShell *pRepresent);
private:
	bool	m_bIsShutDown;				// 是否已经关闭

private:
};

#endif

#endif