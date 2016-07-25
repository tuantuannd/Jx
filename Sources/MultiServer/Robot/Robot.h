
#if !defined(AFX_ROBOT_H__F0EC2C95_6B06_4165_9880_0F8A494BDD86__INCLUDED_)
#define AFX_ROBOT_H__F0EC2C95_6B06_4165_9880_0F8A494BDD86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include <string>
#include <list>

#include "IClient.h"
extern IClient *g_pClientToManager;

typedef std::list< IPlayer * >	PLAYER_LIST;
extern  PLAYER_LIST				g_thePlayers;

#endif // !defined(AFX_ROBOT_H__F0EC2C95_6B06_4165_9880_0F8A494BDD86__INCLUDED_)
