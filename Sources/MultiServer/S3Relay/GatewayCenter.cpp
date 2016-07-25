// GatewayCenter.cpp: implementation of the CGatewayCenter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GatewayCenter.h"
#include "S3Relay.h"
#include "Global.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGatewayCenter::CGatewayCenter()
{

}

CGatewayCenter::~CGatewayCenter()
{

}


BOOL CGatewayCenter::TraceInfo()
{
	std::_tstring info("message: [GatewayCenter] ");
	char buffer[_MAX_PATH];

	BOOL ready = g_GatewayClient.IsReady();

	sprintf(buffer, "<total: %d> : ", ready ? 1 : 0);
	info.append(buffer);

	if (ready)
	{
		sprintf(buffer, "%08X", g_GatewayClient.GetSvrIP());
		info.append(buffer);
	}

	rTRACE(info.c_str());

	return TRUE;
}
