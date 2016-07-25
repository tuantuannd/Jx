//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KClientNpc.h
// Date:	2003.03.06
// Code:	±ß³ÇÀË×Ó
// Desc:	ClientNpc Class
//---------------------------------------------------------------------------
#pragma once

#ifndef _SERVER

#include "KLinkArray.h"

#define		CLIENT_NPC_REGION_WIDTH		5
#define		CLIENT_NPC_REGION_HEIGHT	5
#define		CLIENT_NPC_REGION_AREA		(CLIENT_NPC_REGION_WIDTH * CLIENT_NPC_REGION_HEIGHT)

class KClientNpc
{
	friend class	KSubWorld;
};

#endif