//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KTimer.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Timer Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KTimer.h"
// flying add this macro.
// must pay more attention in the platform porting job.
#ifdef __linux
#include <sys/time.h>
#endif

KTimer::KTimer()
{
#ifdef WIN32
	m_nFrequency.QuadPart = 200 * 1024 * 1024;
	m_nTimeStart.QuadPart = 0;
	m_nTimeStop.QuadPart = 0;
	m_nFPS = 0;
	QueryPerformanceFrequency(&m_nFrequency);
#else
    //m_nFrequency = CLOCKS_PER_SEC;
#endif
}
//---------------------------------------------------------------------------
// 函数:	Start
// 功能:	开始计时
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KTimer::Start()
{
#ifdef WIN32
	QueryPerformanceCounter(&m_nTimeStart);
#else
    gettimeofday(&m_nTimeStart, NULL);
#endif
}
//---------------------------------------------------------------------------
// 函数:	Stop
// 功能:	停止计时
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KTimer::Stop()
{
#ifdef WIN32
	QueryPerformanceCounter(&m_nTimeStop);
#else
	gettimeofday(&m_nTimeStop, NULL);
#endif
}
//---------------------------------------------------------------------------
// 函数:	GetElapse
// 功能:	计算从开始计时到现在已经过到时间
// 参数:	void
// 返回:	DWORD in ms
//---------------------------------------------------------------------------
DWORD KTimer::GetElapse()
{
#ifdef WIN32
	LARGE_INTEGER nTime;
	QueryPerformanceCounter(&nTime);
	return (DWORD)((nTime.QuadPart - m_nTimeStart.QuadPart) 
		* 1000 / m_nFrequency.QuadPart);
#else
	timeval tv;
	gettimeofday(&tv, NULL);
    return (tv.tv_sec - m_nTimeStart.tv_sec) * 1000 + tv.tv_usec / 1000;
#endif
}
//---------------------------------------------------------------------------
// 函数:	GetElapseFrequency
// 功能:	计算从开始计时到现在已经过到时间
// 参数:	void
// 返回:	DWORD in frequency
//---------------------------------------------------------------------------
DWORD KTimer::GetElapseFrequency()
{
#ifdef WIN32
	LARGE_INTEGER nTime;
	QueryPerformanceCounter(&nTime);
	return (DWORD)(nTime.QuadPart - m_nTimeStart.QuadPart);
#endif
	return 0;
}
//---------------------------------------------------------------------------
// 函数:	GetInterval
// 功能:	取得从开始到停止之间的时间间隔，以毫秒为单位
// 参数:	void
// 返回:	毫秒值
//---------------------------------------------------------------------------
DWORD KTimer::GetInterval()
{
#ifdef WIN32
	return (DWORD)((m_nTimeStop.QuadPart - m_nTimeStart.QuadPart) 
		* 1000 / m_nFrequency.QuadPart);
#endif
	return 0;
}
//---------------------------------------------------------------------------
// 函数:	Passed
// 功能:	看是否过了nTime毫秒
// 参数:	nTime	毫秒
// 返回:	TRUE	已经过了
//			FALSE	还没有过
//---------------------------------------------------------------------------
BOOL KTimer::Passed(int nTime)
{

	if (GetElapse() >= (DWORD)nTime)
	{
		Start();
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	GetFPS
// 功能:	取得帧速率
// 参数:	nFPS	帧速率
// 返回:	TRUE	成功
//			FALSE	失败
//---------------------------------------------------------------------------
BOOL KTimer::GetFPS(int *nFPS)
{
	if (GetElapse() >= 1000)
	{
		*nFPS = m_nFPS;
		m_nFPS = 0;
		Start();
		return TRUE;
	}
	m_nFPS++;
	return FALSE;
}
