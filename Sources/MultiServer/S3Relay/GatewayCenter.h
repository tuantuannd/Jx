// GatewayCenter.h: interface for the CGatewayCenter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GATEWAYCENTER_H__3E880C9D_E2EE_4AF4_9D8C_E479417554D6__INCLUDED_)
#define AFX_GATEWAYCENTER_H__3E880C9D_E2EE_4AF4_9D8C_E479417554D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetCenter.h"

class CGatewayCenter : public CNetCenter  
{
public:
	CGatewayCenter();
	virtual ~CGatewayCenter();

public:
	BOOL TraceInfo();
};

#endif // !defined(AFX_GATEWAYCENTER_H__3E880C9D_E2EE_4AF4_9D8C_E479417554D6__INCLUDED_)
