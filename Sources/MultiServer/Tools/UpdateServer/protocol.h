/********************************************************************
	created:	2003/06/23
	file base:	protocol
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_PROTOCOL_H__
#define __INCLUDE_PROTOCOL_H__

enum enumProtocol
{
	enumBegin = 0,

	enumWork,

	enumEnd
};

/*
 * enumWork
 */
struct tagHead
{
	BYTE cProtocol;
};

struct tagWork : tagHead
{
	UINT nAction;
};

#endif // __INCLUDE_PROTOCOL_H__