/********************************************************************
	created:	2003/05/30
	file base:	Msg_define
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_MSG_DEFINE_H__
#define __INCLUDE_MSG_DEFINE_H__

#define WM_ENTER_AFFIRM				( WM_USER + 0x100 )
#define WM_LEAVE_AFFIRM				( WM_USER + 0x200 )
#define WM_SERVER_LOGIN_SUCCESSFUL	( WM_USER + 0x300 )
#define WM_SERVER_LOGIN_FAILED		( WM_USER + 0x400 )

#define DBROLESERVER_NOTIFY			0x1
#define ACCOUNTSERVER_NOTIFY		0x2

#define NAME_PWD_LEN				64
#define NAME_PWD_EX_LEN				64

#define CONNECTED	TRUE
#define DICONNECTED	FALSE

#define WM_SERVER_STATUS			( WM_USER + 0x500 )

#define ADD_GAMESERVER_ACTION	0x1
#define DEL_GAMESERVER_ACTION	0x2

#define WM_GAMESERVER_EXCHANGE		( WM_USER + 0x600 )

#endif // __INCLUDE_MSG_DEFINE_H__