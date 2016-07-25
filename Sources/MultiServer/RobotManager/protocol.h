/********************************************************************
	created:	2003/05/14
	file base:	protocol
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_PROTOCOL_H__
#define __INCLUDE_PROTOCOL_H__

enum
{
	enumProtocolBegin = 0,

	enumDispatchTask,				// tagDspT_INFO
	enumRequireTask,				// tagReqT_CMD

	enumProtocolEnd
};

#endif // __INCLUDE_PROTOCOL_H__