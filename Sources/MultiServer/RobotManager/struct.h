/********************************************************************
	created:	2003/05/14
	file base:	struct
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_STRUCT_H__
#define __INCLUDE_STRUCT_H__

#pragma warning( disable : 4200 )

#pragma pack( push, 1 )

struct tagRobotProtocolHeader
{
	BYTE cProtocol;
};

struct tagServerInfo
{
	/*
	 * IPv4 : 255.255.255.255
	 */
	char szIP[16];

	unsigned short nPort;
};

/*
 * enumDispatchTask
 */
struct tagDspT_INFO : public tagRobotProtocolHeader
{
	tagServerInfo theGameSvrInfo;

	size_t	nAccCount;

	size_t	nNameLen;

	DWORD	dwVersion;

	/*
	 * ASSERT( sizeof( szAccName ) == ( nAccCount * nNameLen ) )
	 */
	char	szAccName[0];

};

/*
 * enumRequireTask
 */
struct tagReqT_CMD : public tagRobotProtocolHeader
{

};

#pragma pack( pop )

#endif // __INCLUDE_STRUCT_H__