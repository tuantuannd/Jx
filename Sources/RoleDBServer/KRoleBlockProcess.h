// KRoleBlockProcess.h: interface for the KRoleBlockProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KROLEBLOCKPROCESS_H__880E1044_4D49_4BDC_BA62_6ABFCA94EAED__INCLUDED_)
#define AFX_KROLEBLOCKPROCESS_H__880E1044_4D49_4BDC_BA62_6ABFCA94EAED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#include "KRoleDBServer.h"
#include <math.h>
#include "KRoleDBHeader.h"

class KRoleBlockProcess  
{
public:
	KRoleBlockProcess();
	KRoleBlockProcess(TStreamData * pData, size_t nStreamSize)
	{
		m_pProcessData = 0;
		m_nCurDataLen = 0;
		m_nStreamSize = 0;
		SetStream(pData, nStreamSize);
	}
	KRoleBlockProcess(size_t nStreamSize)
	{
		m_pProcessData = 0;
		m_nCurDataLen = 0;
		m_nStreamSize = 0;
		GenStream(nStreamSize);
	}
	virtual ~KRoleBlockProcess();
	unsigned long m_nStreamSize;
	
	//获得每块长度为nBlockSize大小的，第nNo块的KBlock
	size_t GetBlockPtr(KBlock * pBlock, unsigned long nNo, size_t nBlockSize);
	
	unsigned long GetBlockCount(size_t nBlockSize)
	{

		double fCount = m_nStreamSize* 1.0 / (nBlockSize - sizeof(KBlock) + 1);
		return (unsigned long )ceil(fCount);
	}

	//生成Stream
	unsigned long SetStream(TStreamData * pData, size_t nStreamSize);
	
	size_t	GetStreamSize(){return m_nStreamSize;};
	
	unsigned long GenStream(size_t nStreamSize);
	//设置某一块
	unsigned long SetBlock(KBlock * pBlock, unsigned long nNo, size_t nBlockSize);
	unsigned long SetBlock(KBlock * pBlock);
	
	bool	IsComplete()
	{
		if (!m_pProcessData) return false;
		if (m_pProcessData->nDataLen == m_nCurDataLen) return true;
		return false;
	}

	void * GetProcessData()
	{
		if (!m_pProcessData) return 0;
		if (!IsComplete()) return 0;
		return &m_pProcessData->pDataBuffer[0];
	}
	TStreamData * m_pProcessData;
private:
	size_t		  m_nCurDataLen;
	

};

#endif // !defined(AFX_KROLEBLOCKPROCESS_H__880E1044_4D49_4BDC_BA62_6ABFCA94EAED__INCLUDED_)
