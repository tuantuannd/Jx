// RootClient.h: interface for the CRootClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROOTCLIENT_H__ED4A7208_6DDB_442A_B974_B34DF73F99A5__INCLUDED_)
#define AFX_ROOTCLIENT_H__ED4A7208_6DDB_442A_B974_B34DF73F99A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetClient.h"

class CRootClient : public CNetClient  
{
public:
	CRootClient(class CRootCenter* pRootCenter, BOOL bAutoFree);
	virtual ~CRootClient();

protected:
	virtual void RecvPackage(const void* pData, size_t size);
	virtual void OnServerEventCreate();
	virtual void OnServerEventClose();
	virtual void BeginRoute();
	virtual void EndRoute();

	BOOL m_isStartAutoTime;
	DWORD m_LastRevPingTime;
	DWORD m_LastSendPingTime;
	BOOL IsStartAutoTime();
	BOOL StartAutoTime();
	BOOL StopAutoTime();

	void AutoTime();

private:
	void Proc0_Normal(const void* pData, size_t size);
	void Proc0_Relay(const void* pData, size_t size);
	void Proc1_Relay_Data(const void* pData, size_t size);
	void Proc1_Relay_AskWay(const void* pData, size_t size);
	void Proc1_Relay_LoseWay(const void* pData, size_t size);
	void Proc2_Relay_AskWay_AccountRoleID(const void* pData, size_t size, BOOL acc);
	void Proc2_Relay_Data_Here(const void* pData, size_t size);
	void Proc0_PlayerCommunity(const void* pData, size_t size);
};

#endif // !defined(AFX_ROOTCLIENT_H__ED4A7208_6DDB_442A_B974_B34DF73F99A5__INCLUDED_)
