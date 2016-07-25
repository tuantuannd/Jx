//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KEngine.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KEngine_H
#define KEngine_H
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#ifndef _SERVER
#include "KDDraw.h"
#include "KDSound.h"
#include "KDInput.h"
#endif
#include "KFile.h"
#include "KFilePath.h"
#ifndef _SERVER
#include "KFileDialog.h"
#endif
#include "KTimer.h"
#include "KMemBase.h"
#include "KRandom.h"
//#include "KPakList.h"
#include "KPakFile.h"
#include "KIniFile.h"
#ifndef _SERVER
#include "KCanvas.h"
#include "KSprite.h"
#include "KFont.h"
#include "KWin32App.h"
#endif
#include "KList.h"
//---------------------------------------------------------------------------
ENGINE_API BOOL	g_InitEngine();
ENGINE_API void	g_ExitEngine();
//---------------------------------------------------------------------------
#endif
