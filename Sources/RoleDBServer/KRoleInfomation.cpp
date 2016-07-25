// KRoleInfomation.cpp: implementation of the KRoleInfomation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoleDBServer.h"
#include "KRoleInfomation.h"
#include <objbase.h>
#include <initguid.h>
#include <list>
#include <map>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
using namespace std;

//按照角色登入的顺序存入的玩家列表，用于自动保存的顺序
list<KRoleInfomation * > g_RoleInfoList;

//以角色姓名作为关键名，存放玩家的列表，用于在客户端读取档快速检索
map<std::string , KRoleInfomation * > g_RoleInfoMap;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KRoleInfomation::KRoleInfomation()
{
	Clear();
}

KRoleInfomation::~KRoleInfomation()
{
	
}

void KRoleInfomation::Clear()
{
	m_RoleInfoBuffer[0] = 0;
	m_nRoleInfoSize = 0;
	m_nSaveCmdCount = m_nLoadCmdCount = m_nSaveDBCount = m_bModify = 0;
	m_nLastSaveTime = 0;	
	m_nUnModifyTime = 0;
}

BYTE *	KRoleInfomation::GetRoleInfoBuffer(size_t & nRoleInfoSize)
{
	nRoleInfoSize = 0;
	if (m_RoleInfoBuffer[0] == 0) return NULL;
	nRoleInfoSize = m_nRoleInfoSize;
	return m_RoleInfoBuffer ;
}

BOOL	KRoleInfomation::SetRoleInfoBuffer(BYTE * pRoleInfoBuffer, size_t nRoleInfoSize)
{
	if (pRoleInfoBuffer == NULL || nRoleInfoSize == 0 ) return FALSE;
	memcpy(&m_RoleInfoBuffer, pRoleInfoBuffer, nRoleInfoSize);
	m_nRoleInfoSize = nRoleInfoSize;
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
