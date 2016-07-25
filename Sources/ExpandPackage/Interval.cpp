// Interval.cpp: implementation of the Interval class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Interval.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace CQ
{
	
	Interval::Interval()
		:m_StartTime(0)
		,m_EndTime(0)
	{
		
	}
	
	Interval::~Interval()
	{
		
	}
	
	void Interval::StartRecord()
	{
		m_StartTime = clock();
		
	}
	
	void Interval::EndRecord()
	{
		m_EndTime = clock();
		
		
	}
	
	double Interval::GetInterval()
	{
		return m_EndTime - m_StartTime;
		
	}
	int Interval::GetIntervalBySecond()
	{
		return (m_EndTime - m_StartTime)/CLOCKS_PER_SEC;
		
	}

}

