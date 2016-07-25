// KRoleBlockProcess.cpp: implementation of the KRoleBlockProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "RoleDBServer.h"
#include "KRoleBlockProcess.h"
#include "memory.h"
#include "windows.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/
#define NULL 0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KRoleBlockProcess::KRoleBlockProcess()
{
	m_pProcessData = 0;
	m_nStreamSize = 0;
	m_nCurDataLen = 0;
}

KRoleBlockProcess::~KRoleBlockProcess()
{
	if (m_pProcessData)
		delete m_pProcessData;

}

//获得每块长度为nBlockSize大小的，第nNo块的KBlock
size_t KRoleBlockProcess::GetBlockPtr(KBlock * pBlock, unsigned long nNo, size_t nBlockSize)
{
	if (!pBlock || nNo < 0 || nBlockSize <= sizeof(KBlock) - 1 ) return 0;
	if (!m_pProcessData || m_pProcessData->nDataLen < 0) return 0;
	size_t nMemSize = 0;
	if (nNo == 0)
	{
		pBlock->bDataState = 1;
		pBlock->nOffset =  m_pProcessData->nDataLen;
		pBlock->nProtoId = m_pProcessData->nProtoId;
		
		if (m_pProcessData->nDataLen + sizeof(KBlock) - 1 <=  nBlockSize)
		{
			nMemSize = m_pProcessData->nDataLen;
		}
		else 
			nMemSize = nBlockSize - sizeof(KBlock) + 1;
	}
	else 
	{
		pBlock->bDataState = 0;
		pBlock->nOffset =  nNo * (nBlockSize - sizeof(KBlock) + 1) ;
		pBlock->nProtoId = m_pProcessData->nProtoId;


		if (m_pProcessData->nDataLen < pBlock->nOffset + nBlockSize - sizeof(KBlock) + 1)
		{
			nMemSize =  m_pProcessData->nDataLen - pBlock->nOffset;
		}
		else
		{
			nMemSize = nBlockSize - sizeof(KBlock) + 1;
		}
			
	}
	
	pBlock->nDataLen = nMemSize;
	if (!nNo)
		memcpy(&pBlock->pDataBuffer[0], m_pProcessData->pDataBuffer, nMemSize);
	else 
		memcpy(&pBlock->pDataBuffer[0], m_pProcessData->pDataBuffer + pBlock->nOffset, nMemSize );
	
	return nMemSize + sizeof(KBlock) - 1;

}

	//生成Steam
unsigned long KRoleBlockProcess::SetStream(TStreamData * pData, size_t nStreamSize)
{
	if (!m_pProcessData || m_nStreamSize < nStreamSize )
	{
		GenStream(nStreamSize);
	}
	memcpy(m_pProcessData , pData, nStreamSize);
	return nStreamSize;
}

unsigned long KRoleBlockProcess::GenStream(size_t nStreamSize)
{
	if (m_pProcessData)
	{
		delete []m_pProcessData;
		m_pProcessData = NULL;
	}
	m_pProcessData = (TStreamData*) new char [nStreamSize];
	m_nStreamSize = nStreamSize;
	return nStreamSize;
}
	
	//设置某一块
unsigned long KRoleBlockProcess::SetBlock(KBlock * pBlock, unsigned long nNo, size_t nBlockSize)
{
	if (!pBlock || nNo < 0 || nBlockSize <= sizeof(KBlock) - 1 ) return 0;
	int nOffset = 0;
	if (pBlock->nDataLen > nBlockSize - sizeof(KBlock) + 1 )  return 0;
	if (pBlock->bDataState == 1)
	{
		m_pProcessData->nDataLen = pBlock->nOffset ;
		m_pProcessData->nProtoId = pBlock->nProtoId;
		nOffset = 0;
	}

	//if (nNo * nBlockSize > m_pProcessData->nDataLen + sizeof)
	
	memcpy(&m_pProcessData->pDataBuffer[0] + (nBlockSize - sizeof(KBlock) + 1) * nNo, &pBlock->pDataBuffer[0] , pBlock->nDataLen);
	m_nCurDataLen  += pBlock->nDataLen;
	return pBlock->nDataLen;
}

unsigned long KRoleBlockProcess::SetBlock(KBlock * pBlock)
{
	if (!pBlock ) return 0;
	int nOffset = 0;
	if (pBlock->bDataState == 1)
	{
		m_pProcessData->nDataLen = pBlock->nOffset ;
		m_pProcessData->nProtoId = pBlock->nProtoId;
		nOffset = 0;
	}
	else 
		nOffset = pBlock->nOffset;
		
	memcpy(&m_pProcessData->pDataBuffer[0] + nOffset , &pBlock->pDataBuffer[0] , pBlock->nDataLen);
	m_nCurDataLen  += pBlock->nDataLen;
	return pBlock->nDataLen;
}