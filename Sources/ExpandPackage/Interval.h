// Interval.h: interface for the Interval class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERVAL_H__4F3C9057_E924_4A77_A322_B70FE038DE40__INCLUDED_)
#define AFX_INTERVAL_H__4F3C9057_E924_4A77_A322_B70FE038DE40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include"time.h"
namespace CQ
{
	
	class Interval  
	{
	public:
		int GetIntervalBySecond();
		double GetInterval();
		void EndRecord();
		void StartRecord();
		Interval();
		virtual ~Interval();
		
	private:
		clock_t m_EndTime;
		clock_t m_StartTime;
	};
}
#endif // !defined(AFX_INTERVAL_H__4F3C9057_E924_4A77_A322_B70FE038DE40__INCLUDED_)
