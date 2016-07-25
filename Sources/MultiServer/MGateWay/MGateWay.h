
#if !defined(AFX_MGATEWAY_H__1850D008_6D4D_4D4B_B8E2_7C0B8BE17F2E__INCLUDED_)
#define AFX_MGATEWAY_H__1850D008_6D4D_4D4B_B8E2_7C0B8BE17F2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#include "Environment.h"

enum ShowInfoFormat
{
	SIF_NONE = 0,

	SIF_SPACE,
	SIF_COLON
};

void SetShowInfo( _tstring sTitle, _tstring sContent, int nFormat = SIF_NONE );

#endif // !defined(AFX_MGATEWAY_H__1850D008_6D4D_4D4B_B8E2_7C0B8BE17F2E__INCLUDED_)
