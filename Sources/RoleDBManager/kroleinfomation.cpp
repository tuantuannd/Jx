// KRoleInfomation.cpp: implementation of the KRoleInfomation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KRoleInfomation.h"
#include "kdbprocessthread.h"
#include "kroledbheader.h"
#include "KDBProcessThread.h"
#include <objbase.h>
#include <initguid.h>
#include <list>
#include <map>

extern char * GetRoleNameFromRoleBuffer(char * pRoleBuffer);
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif
using namespace std;
using OnlineGameLib::Win32::CCriticalSection;

//按照角色登入的顺序存入的玩家列表，用于自动保存的顺序
list<KRoleInfomation * > g_RoleInfoList;

//以角色姓名作为关键名，存放玩家的列表，用于在客户端读取档快速检索
map<OnlineGameLib::Win32::_tstring, KRoleInfomation * > g_RoleInfoMap;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KRoleInfomation::KRoleInfomation()
{
	Clear();
}

KRoleInfomation::~KRoleInfomation()
{
	Clear();
}

void KRoleInfomation::Clear()
{
	CCriticalSection::Owner locker( m_cs);
	m_nRoleInfoSize = 0;
	m_nSaveCmdCount = m_nLoadCmdCount = m_nSaveDBCount = m_bModify = 0;
	m_nLastSaveTime = 0;	
	m_nUnModifyTime = 0;
	m_bModify = false;
}
BOOL	KRoleInfomation::IsValid()
{
	CCriticalSection::Owner locker( m_cs);
	if (m_nRoleInfoSize == 0) return FALSE;
	return TRUE;
}

TCmd *	KRoleInfomation::CreateRoleInfoTCmdData(size_t &nRoleInfoSize)
{
	CCriticalSection::Owner locker( m_cs);
	if (m_nRoleInfoSize == 0 ) return NULL;
	
	TCmd * pNewCmd = (TCmd *)new BYTE[sizeof(TCmd) + m_nRoleInfoSize - 1];
	pNewCmd->ProcessData.nDataLen = m_nRoleInfoSize;
	nRoleInfoSize = m_nRoleInfoSize;
	memcpy(&pNewCmd->ProcessData.pDataBuffer[0], m_RoleInfoBuffer, m_nRoleInfoSize);
	return pNewCmd;
}

BYTE *	KRoleInfomation::GetRoleInfoBuffer(size_t & nRoleInfoSize)
{
	CCriticalSection::Owner locker( m_cs);
	nRoleInfoSize = 0;
	if (m_nRoleInfoSize == 0) return NULL;
	nRoleInfoSize = m_nRoleInfoSize;
	return m_RoleInfoBuffer ;
}

BOOL KRoleInfomation::GetRoleName(char * pName, size_t& nLen)
{
	CCriticalSection::Owner locker(m_cs);
	if (m_nRoleInfoSize == 0) return FALSE;
	size_t nCurLen = strlen(GetRoleNameFromRoleBuffer((char *)m_RoleInfoBuffer));
	
	if (nCurLen >= nLen)
		return FALSE;
	nLen = nCurLen;
	strcpy(pName, GetRoleNameFromRoleBuffer((char *)m_RoleInfoBuffer));
	return TRUE;
}

BOOL KRoleInfomation::CopyRoleInfoBuffer(BYTE * pBuffer ,size_t & nRoleInfoSize)
{
	CCriticalSection::Owner locker( m_cs);
	if (nRoleInfoSize < m_nRoleInfoSize) return FALSE;
	nRoleInfoSize = 0;
	if (m_nRoleInfoSize == 0) return FALSE;
	nRoleInfoSize = m_nRoleInfoSize;
	memcpy(pBuffer, m_RoleInfoBuffer, m_nRoleInfoSize);
	return TRUE ;
}

BOOL	KRoleInfomation::GetBaseRoleInfo(TRoleBaseInfo * pRoleBaseInfo)
{
	CCriticalSection::Owner locker(m_cs);
	if (!pRoleBaseInfo ) return FALSE;
	if (m_nRoleInfoSize == 0) return FALSE;
	TRoleBaseInfo * pInfo = (TRoleBaseInfo *) m_RoleInfoBuffer;
	*pRoleBaseInfo = *pInfo;
	return TRUE;
}

size_t KRoleInfomation::GetBufferSize()
{
	CCriticalSection::Owner locker( m_cs);
	return m_nRoleInfoSize;
}

BOOL	KRoleInfomation::SetRoleInfoBuffer(BYTE * pRoleInfoBuffer, size_t nRoleInfoSize)
{
	CCriticalSection::Owner locker( m_cs);
	if (pRoleInfoBuffer == NULL || nRoleInfoSize == 0 ) return FALSE;
	if (nRoleInfoSize > MAX_ROLEINFORMATION_SIZE) return FALSE;
	memcpy((BYTE*)m_RoleInfoBuffer, pRoleInfoBuffer, nRoleInfoSize);
	m_nRoleInfoSize = nRoleInfoSize;
	m_bModify = true;
	return TRUE;
}

BOOL	KRoleInfomation::CheckRoleInfoValid(const char * szRoleName)	//验证Buffer数据是否为RoleName的数据
{
	return TRUE;
}

BOOL	KRoleInfomation::CheckRoleInfoValid()	//验证Buffer数据是否为RoleName的数据
{
	if (m_nRoleInfoSize <= 0) return FALSE;
	return TRUE;
}
 