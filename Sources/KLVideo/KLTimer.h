#ifndef __KLTIMER_H__
#define __KLTIMER_H__

#include <Mmsystem.h>

enum EKLTimerState
{
	KLTIMER_RUNING,
	KLTIMER_PAUSE,
	KLTIMER_STOPED,
};

class KLTimer
{
public:
	KLTimer()
	{
		m_eState = KLTIMER_STOPED;
		m_tmElapsed = 0;
		m_tmLast = 0;
	}
	void Start()
	{
		m_tmElapsed = 0;
		m_tmLast = timeGetTime();
		m_eState = KLTIMER_RUNING;
	}
	void Pause()
	{
		if(m_eState == KLTIMER_RUNING)
		{
			m_tmElapsed += timeGetTime() - m_tmLast;
			m_eState = KLTIMER_PAUSE;
		}
	}
	void GoOn()
	{
		if(m_eState == KLTIMER_PAUSE)
		{
			m_tmLast = timeGetTime();
			m_eState = KLTIMER_RUNING;
		}
	}
	void Stop()
	{
		m_eState = KLTIMER_STOPED;
	}
	DWORD GetElapse()
	{
		if(m_eState == KLTIMER_RUNING)
		{
			return m_tmElapsed + (timeGetTime() - m_tmLast);
		}
		else if(m_eState == KLTIMER_PAUSE)
		{
			return m_tmElapsed;
		}
		else
			return 0;
	}
protected:
	EKLTimerState	m_eState; 
	DWORD			m_tmLast;
	DWORD			m_tmElapsed;
};

#endif