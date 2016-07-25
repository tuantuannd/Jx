// RootCenter.cpp: implementation of the CRootCenter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RootCenter.h"
#include "S3Relay.h"
#include "Global.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRootCenter::CRootCenter()
{

}

CRootCenter::~CRootCenter()
{

}


BOOL CRootCenter::TraceInfo()
{
	std::_tstring info("message: [RootCenter] ");
	char buffer[_MAX_PATH];

	BOOL ready = g_RootClient.IsReady();

	sprintf(buffer, "<total: %d> : ", ready ? 1 : 0);
	info.append(buffer);

	if (ready)
	{
		sprintf(buffer, "%08X", g_RootClient.GetSvrIP());
		info.append(buffer);
	}

	rTRACE(info.c_str());

	return TRUE;
}
