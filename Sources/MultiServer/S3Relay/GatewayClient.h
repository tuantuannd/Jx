// -------------------------------------------------------------------------
//	文件名		：	GatewayClient.h
//	创建者		：	万里
//	创建时间	：	2003-6-6 20:41:41
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __GATEWAYCLIENT_H__
#define __GATEWAYCLIENT_H__

#include "NetClient.h"
#include <list>

class CGatewayClient : public CNetClient 
{
public:
	CGatewayClient(class CGatewayCenter* pGatewayCenter, BOOL bAutoFree);
	virtual ~CGatewayClient();

protected:
	virtual void RecvPackage(const void* pData, size_t size);

protected:
	virtual void OnServerEventCreate();
	virtual void OnServerEventClose();

private:
	void Proc0_NotifySvrIP(const void* pData, size_t size);

public:
	BOOL PassAskWayMap(DWORD fromIP, const RELAY_ASKWAY_DATA* pAskWayMap);

private:
	enum {MAX_PACKAGECOUNT = 65536};	//by type of WORD
	WORD m_lastPckgID;

	struct ASKWAYMAP
	{
		DWORD tick;

		DWORD fromIP;
		RELAY_ASKWAY_DATA* pAskWayMap;
	};
	typedef std::map<WORD, ASKWAYMAP>	ID2ASKWAYMAPMAP;
	ID2ASKWAYMAPMAP m_mapAskWayMaps;

	CLockSRSW m_lockAskWayMap;

};

#endif // __GATEWAYCLIENT_H__
