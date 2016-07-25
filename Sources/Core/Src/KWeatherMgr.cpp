#include "KCore.h"
#include "KSubWorldSet.h"
#include "KWeatherMgr.h"

#ifdef _SERVER

KWeatherMgr::KWeatherMgr()
{
	m_bInWeather = false;
	m_nWeatherNum = 0;
	m_nFullOdds = 0;
	m_nCurWeather = WEATHERID_NOTHING;
	m_nHappenTimeMin = 300 * 20;
	m_nHappenTimeMax = 1000 * 20;
	m_nLastHappentime = g_SubWorldSet.GetGameTime();
	m_nNextHappentime = m_nLastHappentime + 500 * 20;
	m_nWeatherDuration = 0;
}

// IniFile:已经打开的.wor文件
bool KWeatherMgr::InitFromIni(KIniFile &IniFile)
{
	KIniFile IniFileWeather;
	// 打开游戏中所有天气定义的ini文件
	if(!IniFileWeather.Load("\\settings\\Weather\\Weather.ini"))
		return false;

	IniFile.GetInteger("Weather", "WeatherNum", 0, &m_nWeatherNum);
	if(m_nWeatherNum > 4)
		m_nWeatherNum = 4;

	m_nFullOdds = 0;
	char cc[40];
	for(int i=0; i<m_nWeatherNum; i++)
	{
		sprintf(cc, "Weather%d",i);
		IniFile.GetInteger("Weather", cc, 0, &m_weatherInfo[i].m_weatherID);
		sprintf(cc, "Odds%d",i);
		IniFile.GetInteger("Weather", cc, 0, &m_weatherInfo[i].m_nOdds);
		m_nFullOdds += m_weatherInfo[i].m_nOdds;
		sprintf(cc, "Weather%.3d",i);
		IniFileWeather.GetInteger(cc, "LifeTimeMin", 200, &m_weatherInfo[i].m_nLifeTimeMin);
		IniFileWeather.GetInteger(cc, "LifeTimeMax", 600, &m_weatherInfo[i].m_nLifeTimeMax);
		m_weatherInfo[i].m_nLifeTimeMin *= 20;
		m_weatherInfo[i].m_nLifeTimeMax *= 20;
	}

	if(m_nFullOdds < 100)
		m_nFullOdds = 100;

	IniFile.GetInteger("Weather", "HappenTimeMin", 300, &m_nHappenTimeMin);
	IniFile.GetInteger("Weather", "HappenTimeMax", 1000, &m_nHappenTimeMax);
	if(m_nHappenTimeMax < m_nHappenTimeMin)
	{
		int n = m_nHappenTimeMax;
		m_nHappenTimeMax = m_nHappenTimeMin;
		m_nHappenTimeMin = n;
	}
	m_nHappenTimeMin *= 20;
	m_nHappenTimeMax *= 20;

	m_nNextHappentime = m_nLastHappentime + m_nHappenTimeMin + g_Random(m_nHappenTimeMax - m_nHappenTimeMin);

	return true;
}

int KWeatherMgr::Activate()
{
	int tmCur = g_SubWorldSet.GetGameTime();
	int i;
	if(!m_bInWeather)
	{
		if(tmCur > m_nNextHappentime)
		{
			// 时间到，发生一次天气
			int nOdds = g_Random(m_nFullOdds);
			m_nWeatherDuration = 0;
			for(i=0; i<m_nWeatherNum; i++)
			{
				if(nOdds < m_weatherInfo[i].m_nOdds)
				{
					m_nCurWeather = m_weatherInfo[i].m_weatherID;
					m_nWeatherDuration = m_weatherInfo[i].m_nLifeTimeMin 
										+ g_Random(m_weatherInfo[i].m_nLifeTimeMax - m_weatherInfo[i].m_nLifeTimeMin);
					break;
				}
				else
					nOdds -= m_weatherInfo[i].m_nOdds;
			}

			m_nLastHappentime = tmCur;
			m_nNextHappentime = m_nLastHappentime + m_nWeatherDuration + m_nHappenTimeMin + g_Random(m_nHappenTimeMax - m_nHappenTimeMin);
			m_bInWeather = true;
		}
	}
	else
	{
		if(tmCur - m_nLastHappentime > m_nWeatherDuration)
		{
			// 时间到，天气停止
			m_nCurWeather = WEATHERID_NOTHING;
			m_bInWeather = false;
		}
	}

	return m_nCurWeather;
}

#endif
