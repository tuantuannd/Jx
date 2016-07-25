// ChatConnect.h: interface for the CChatConnect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATCONNECT_H__5ED21755_43E7_4141_A572_2506BF7C31BA__INCLUDED_)
#define AFX_CHATCONNECT_H__5ED21755_43E7_4141_A572_2506BF7C31BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetConnect.h"

class CChatConnect : public CNetConnect  
{
public:
	CChatConnect(class CChatServer* pHostServer, unsigned long id);
	virtual ~CChatConnect();

protected:
	virtual void OnClientConnectCreate();
	virtual void OnClientConnectClose();

	virtual void RecvPackage(const void* pData, size_t size);

private:
	void Proc0_SomeoneChat(const void* pData, size_t size);
	void Proc0_ChannelChat(const void* pData, size_t size);
	void Proc0_GroupMan(const void* pData, size_t size);

};

#endif // !defined(AFX_CHATCONNECT_H__5ED21755_43E7_4141_A572_2506BF7C31BA__INCLUDED_)
