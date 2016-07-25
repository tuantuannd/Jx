
#include "KCore.h"
#ifndef _SERVER
#include "../KSubWorldSet.h"
#include "KWeather.h"
#include "..\kWeatherMgr.h"
#include "KOption.h"

KWeather::KWeather()
{
	m_bIsShutingDown = false;
	m_nStartTime = g_SubWorldSet.GetGameTime();
	m_nLifeTime = 30*20;
	m_nParticleNum = 2000;
	m_nDownSpeed = 800;
	m_nWindSpeed = 50;
}

KWeather::~KWeather()
{
	m_ParticleList.clear();
}

void KWeather::Breath()
{
	// 如果超出声明期则关闭天气效果
	int nCurTime = g_SubWorldSet.GetGameTime();
	if(nCurTime - m_nStartTime > m_nLifeTime)
		ShutDown();
}

//*********************************************************************************************
//KWeatherRain的实现
//*********************************************************************************************
KWeatherRain::KWeatherRain()
{
	m_nLenRate = 100;
	m_bIsShutDown = false;
	m_nFlareTime = 10 * 20;
	m_nLastFlareTime = g_SubWorldSet.GetGameTime();
	m_nFlareDelay = g_Random(m_nFlareTime);
	m_nFlareState = -1;
	m_fAmbientFactor = 1.0f;
	m_szRainSound[0] = 0;
	m_szFlareSounds[0][0] = 0;
	m_szFlareSounds[1][0] = 0;
	m_szFlareSounds[2][0] = 0;
	m_pSound = NULL;
	m_nSoundVolume = -10000;
}

KWeatherRain::~KWeatherRain()
{
	if(m_pSound)
	{
		m_pSound->Stop();
		m_pSound = NULL;
	}
}

bool KWeatherRain::ReadInfoFromIniFile(int nWeatherID)
{
	if(nWeatherID < 0 || nWeatherID >= WEATHERID_NOTHING)
		return false;

	KIniFile	IniFile;
	if(!IniFile.Load("\\settings\\Weather\\Weather.ini"))
		return false;

	char szTagName[40];
	sprintf(szTagName, "Weather%.3d", nWeatherID);
	
	int n, n1, n2;
	IniFile.GetInteger(szTagName, "LifeTimeMin", 200, &n1);
	IniFile.GetInteger(szTagName, "LifeTimeMax", 600, &n2);
	SetLifeTime(n1 + g_Random(n2 - n1));
	IniFile.GetInteger(szTagName, "ParticleNum", 1000, &n);
	SetParticleNum(n);
	IniFile.GetInteger(szTagName, "DownSpeed", 500, &n);
	SetDownSpeed(n);
	IniFile.GetInteger(szTagName, "WindSpeed", 60, &n);
	SetWindSpeed(n);
	IniFile.GetInteger(szTagName, "FlareTime", 30, &n);
	SetFlareTime(n);
	IniFile.GetInteger(szTagName, "LenRate", 100, &m_nLenRate);
	if(m_nLenRate < 0)
		m_nLenRate = 0;
	if(m_nLenRate > 400)
		m_nLenRate = 400;
	m_nFlareDelay = g_Random(m_nFlareTime);
	IniFile.GetString(szTagName, "RainSound", "", m_szRainSound, 79);
	IniFile.GetString(szTagName, "FlareSound1", "", m_szFlareSounds[0], 79);
	IniFile.GetString(szTagName, "FlareSound2", "", m_szFlareSounds[1], 79);
	IniFile.GetString(szTagName, "FlareSound3", "", m_szFlareSounds[2], 79);

	g_SetFilePath("\\");
	KCacheNode *pSndNode = NULL;
	pSndNode = (KCacheNode*) g_SoundCache.GetNode(m_szRainSound, pSndNode);
	m_pSound = (KWavSound*) pSndNode->m_lpData;
	if (m_pSound)
	{
		m_pSound->Play(0, -10000 + Option.GetSndVolume() * 100, true);
	}
	return true;
}

void KWeatherRain::Breath()
{
	if(m_bIsShutDown)
		return;

	if(m_bIsShutingDown)
	{
		// 雨势减小，慢慢停止
		m_flareColorAdd.SetColor(0, 0, 0);
		if(m_fAmbientFactor < 1.0f)
		{
			m_fAmbientFactor += 0.005f;
			m_nParticleNum = m_nParticleNum * 24 / 25;
			m_nSoundVolume = (10000 + m_nSoundVolume) * 199 / 200 - 10000;
		}
		else
		{
			if(m_pSound)
			{
				m_pSound->Stop();
				m_pSound = NULL;
			}
			m_bIsShutDown = true;
		}
	}
	else
	{
		if(m_fAmbientFactor > 0.7f)
			m_fAmbientFactor -= 0.005f;
		if(m_nSoundVolume < 0)
		{
			m_nSoundVolume += 200;
			if(m_nSoundVolume > 0)
				m_nSoundVolume = 0;
		}
	}

	if(m_pSound)
		m_pSound->SetVolume(m_nSoundVolume);

	// 要生成的粒子数目
	int nCreateNum = m_nParticleNum / 20;
	// 生成粒子
	int nZOff = m_nDownSpeed / 20;
	KParticle particle;
	for(int j=0; j<nCreateNum; j++)
	{
		particle.m_vPos.nX = m_nLeftTopX + g_Random(WEATHER_AREA_WIDTH);
		particle.m_vPos.nY = m_nLeftTopY + g_Random(WEATHER_AREA_HEIGHT);
		particle.m_vPos.nZ = 500.0f + (particle.m_vPos.nY - m_nFocusPosY) / 3 - g_Random(nZOff);
		particle.m_vSpeed.nX = m_nWindSpeed / 20;
		particle.m_vSpeed.nZ = m_nDownSpeed / 20;
		particle.m_color.Color_b.a = 0x20 + g_Random(10);
		particle.m_color.Color_b.r = particle.m_color.Color_b.g = particle.m_color.Color_b.b = 0xff;
		m_ParticleList.push_back(particle);
	}

	//更新所有粒子，已落地的则删除
	list<KParticle>::iterator i;
	for (i = m_ParticleList.begin(); i != m_ParticleList.end(); )
	{
		i->m_vPos.nX += i->m_vSpeed.nX;
		i->m_vPos.nZ -= i->m_vSpeed.nZ;

		if(i->m_vPos.nZ < 0)
		{
			i = m_ParticleList.erase(i);
			continue;
		}

		++i;
	}

	if(m_nFlareTime > 0 && !m_bIsShutingDown)
	{
		// 处理闪电
		int tmCur = g_SubWorldSet.GetGameTime();
		switch (m_nFlareState)
		{
		case -1:
			if(tmCur - m_nLastFlareTime > m_nFlareDelay)
			{
				// 时间到，处理闪电
				m_nFlareStateStart = tmCur;
				m_nFlareStateDelay = 2 + g_Random(3);
				m_nFlareState = 0;
				m_nFlareDelay = g_Random(m_nFlareTime);
				m_nLastFlareTime = tmCur;
				int n = 0x40 + g_Random(0x20);
				m_flareColorAdd.SetColor(n, n, n);
			}
			break;
		case 0:
			if(tmCur - m_nFlareStateStart > m_nFlareStateDelay)
			{
				// 阶段0结束
				m_nFlareStateStart = tmCur;
				m_nFlareStateDelay = 2 + g_Random(2);
				m_nFlareState = 1;
				int n = 0x10 + g_Random(0x30);
				m_flareColorAdd.SetColor(n, n, n);
			}
			break;
		case 1:
			if(tmCur - m_nFlareStateStart > m_nFlareStateDelay)
			{
				// 阶段1结束
				m_nFlareStateStart = tmCur;
				m_nFlareStateDelay = 2 + g_Random(2);
				m_nFlareState = 2;
				int n = 0x10 + g_Random(0x40);
				m_flareColorAdd.SetColor(n, n, n);
			}
			break;
		case 2:
			if(tmCur - m_nFlareStateStart > m_nFlareStateDelay)
			{
				// 阶段2结束
				m_nFlareStateStart = tmCur;
				m_nFlareStateDelay = 8 + g_Random(10);
				m_nFlareState = 3;
				m_flareColorAdd.SetColor(0, 0, 0);
			}
			break;
		case 3:
			if(tmCur - m_nFlareStateStart > m_nFlareStateDelay)
			{
				// 阶段3结束
				KWavSound * pSound = NULL;
				KCacheNode *pSndNode = NULL;
				g_SetFilePath("\\");
				pSndNode = (KCacheNode*) g_SoundCache.GetNode(m_szFlareSounds[g_Random(3)], pSndNode);
				pSound = (KWavSound*) pSndNode->m_lpData;
				if (pSound)
				{
					pSound->Play(0, -10000 + Option.GetSndVolume() * 100, false);
				}
				m_nFlareState = -1;
			}
			break;
		default:
			m_nFlareState = -1;
		}
	}

	// 这个要在函数最后调用，因为内部修改了m_nLastBreathTime
	KWeather::Breath();
}

void KWeatherRain::Render(iRepresentShell *pRepresent)
{
	if(!pRepresent)
		return;

	if(m_bIsShutDown)
		return;

	int nPrimitiveCount = 0;
	KRULine Primitives[100];

	list<KParticle>::iterator i;
	for (i = m_ParticleList.begin(); i != m_ParticleList.end(); i++)
	{
		// 计算雨点的长度
		int nLen = 35 + (i->m_vPos.nY - m_nFocusPosY) / 60;
		nLen = nLen * m_nLenRate / 100;
		if(nLen < 0)
			continue;

		if(g_Random(3) == 0)
			continue;

		Primitives[nPrimitiveCount].Color.Color_dw = i->m_color.Color_dw;
		Primitives[nPrimitiveCount].oPosition.nX = i->m_vPos.nX;
		Primitives[nPrimitiveCount].oPosition.nY = i->m_vPos.nY;
		Primitives[nPrimitiveCount].oPosition.nZ = i->m_vPos.nZ;
		Primitives[nPrimitiveCount].oEndPos.nX = i->m_vPos.nX + (m_nWindSpeed * nLen) / m_nDownSpeed;
		Primitives[nPrimitiveCount].oEndPos.nY = i->m_vPos.nY;
		Primitives[nPrimitiveCount].oEndPos.nZ = i->m_vPos.nZ - nLen;
		nPrimitiveCount++;
		if(nPrimitiveCount >= 100)
		{
			pRepresent->DrawPrimitives(nPrimitiveCount, Primitives, RU_T_LINE, false);
			nPrimitiveCount = 0;
		}
	}

	if(nPrimitiveCount)
		pRepresent->DrawPrimitives(nPrimitiveCount, Primitives, RU_T_LINE, false);
}

void KWeatherRain::FilterAmbient(DWORD &dwLight)
{
	KLColor color;
	color.SetColor(dwLight);
	color.Scale(m_fAmbientFactor);
	if(m_nFlareDelay > 0 && m_nFlareState != -1)
		color += m_flareColorAdd; 
	dwLight = color.GetColor();
}

//*********************************************************************************************
//KWeatherSnow的实现
//*********************************************************************************************
bool KWeatherSnow::ReadInfoFromIniFile(int nWeatherID)
{
	KIniFile	IniFile;
	g_SetRootPath(NULL);
	g_SetFilePath("\\settings\\Weather");
	if(!IniFile.Load("Weather.ini"))
		return false;

	char szTagName[40];
	sprintf(szTagName, "Weather%3d", nWeatherID);

	int n;
	IniFile.GetInteger(szTagName, "LifeTime", 600, &n);
	SetLifeTime(n);
	IniFile.GetInteger(szTagName, "ParticleNum", 1000, &n);
	SetParticleNum(n);
	IniFile.GetInteger(szTagName, "DownSpeed", 500, &n);
	SetDownSpeed(n);
	IniFile.GetInteger(szTagName, "WindSpeed", 60, &n);
	SetWindSpeed(n);
	
	return true;
}

void KWeatherSnow::Breath()
{
	if(m_bIsShutDown)
		return;

	if(m_bIsShutingDown)
	{
		m_bIsShutDown = true;
	}
	else
	{
	}

	// 要生成的粒子数目
	int nCreateNum = m_nParticleNum / 20;
	// 生成粒子
	int nZOff = m_nDownSpeed / 20;
	KParticle particle;
	for(int j=0; j<nCreateNum; j++)
	{
		particle.m_vPos.nX = m_nLeftTopX + g_Random(WEATHER_AREA_WIDTH);
		particle.m_vPos.nY = m_nLeftTopY + g_Random(WEATHER_AREA_HEIGHT);
		particle.m_vPos.nZ = 500.0f + (particle.m_vPos.nY - m_nFocusPosY) / 3 - g_Random(nZOff);
		particle.m_vSpeed.nX = m_nWindSpeed / 20;
		particle.m_vSpeed.nZ = m_nDownSpeed / 20;
		particle.m_vSpeed.nY = 0;
		particle.m_color.Color_b.a = (0xff - (0x80 + g_Random(50))) >> 3;
		particle.m_color.Color_b.r = particle.m_color.Color_b.g = particle.m_color.Color_b.b = 0xff;
		m_ParticleList.push_back(particle);
	}

	//更新所有粒子，已落地的则删除
	list<KParticle>::iterator i;
	for (i = m_ParticleList.begin(); i != m_ParticleList.end(); )
	{
		i->m_vPos.nX += i->m_vSpeed.nX;
		i->m_vPos.nY += i->m_vSpeed.nY;
		i->m_vPos.nZ -= i->m_vSpeed.nZ;

		if(g_Random(40) == 0)
		{
			if(i->m_vSpeed.nX >= m_nWindSpeed)
				i->m_vSpeed.nX = m_nWindSpeed * 2 / 4 / 20;
			else
				i->m_vSpeed.nX = m_nWindSpeed * 6 / 4 / 20;
		}

		if(i->m_vPos.nZ < 0)
		{
			i = m_ParticleList.erase(i);
			continue;
		}

		++i;
	}

	// 这个要在函数最后调用，因为内部修改了m_nLastBreathTime
	KWeather::Breath();
}

void KWeatherSnow::Render(iRepresentShell *pRepresent)
{
	if(!pRepresent)
		return;

	if(m_bIsShutDown)
		return;

	int nPrimitiveCount = 0;
	KRUShadow Primitives[100];

	list<KParticle>::iterator i;
	for (i = m_ParticleList.begin(); i != m_ParticleList.end(); i++)
	{
		int nLen = 30 + (i->m_vPos.nY - m_nFocusPosY) / 80;
		if(nLen < 0)
			continue;

//		if(g_Random(3) == 0)
//			continue;

		Primitives[nPrimitiveCount].Color.Color_dw = i->m_color.Color_dw;
		Primitives[nPrimitiveCount].oPosition.nX = i->m_vPos.nX;
		Primitives[nPrimitiveCount].oPosition.nY = i->m_vPos.nY;
		Primitives[nPrimitiveCount].oPosition.nZ = i->m_vPos.nZ;
		Primitives[nPrimitiveCount].oEndPos.nX = i->m_vPos.nX + 3;
		Primitives[nPrimitiveCount].oEndPos.nY = i->m_vPos.nY;
		Primitives[nPrimitiveCount].oEndPos.nZ = i->m_vPos.nZ - 3;
		nPrimitiveCount++;
		if(nPrimitiveCount >= 100)
		{
			pRepresent->DrawPrimitives(nPrimitiveCount, Primitives, RU_T_SHADOW, false);
			nPrimitiveCount = 0;
		}
	}

	if(nPrimitiveCount)
		pRepresent->DrawPrimitives(nPrimitiveCount, Primitives, RU_T_SHADOW, false);
}

#endif