// KSynLock.cpp: implementation of the KSynLock class.
//
//////////////////////////////////////////////////////////////////////

#include "KSynLock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

long KSynLock::Lock()	 //检查是否数据已经被锁定，如果是的话返回当前的值
{
	if (m_bLock)
		++m_nCount ;
	else 
	{
		if (m_nCurId == 0)//未曾使用
		{
			m_bLock = TRUE;
			m_nCurId = 1;
			m_nCount  = 1;
			return m_nCurId;
		}
		else 
			return -1*(++m_nCount) ;
	}
	
	return -1 * m_nCount;
}

BOOL  KSynLock::UnLock(long id)
{
	//要求反锁者为当前使用者，可以反锁
	if (id == m_nCurId)
	{
		m_bLock = FALSE;
		if (m_nCount <= m_nCurId)  
		{
			m_nCount = m_nCurId = 0;
			printf("\n\n重置Index为零！！！\n\n");
		}
		else 
			m_nCurId ++;
		return TRUE;
	}
	
	return FALSE;
}

