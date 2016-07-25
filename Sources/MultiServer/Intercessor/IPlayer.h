/********************************************************************
	created:	2003/04/08
	file base:	IPlayer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_IPLAYER_H__
#define __INCLUDE_IPLAYER_H__

class IPlayer
{
public:
	/*
	 * Interface
	 */
	virtual bool DispatchTask( UINT nTaskID ) = 0;
	virtual bool IsWorking() = 0;
	virtual int  Run() = 0;
	virtual bool AppendData( UINT nOwner, const void *pData, size_t dataLength ) = 0;
	
	virtual bool Create( const char * const pName, const char * const pPassword ) = 0;
	virtual bool Destroy( const char * const /*pPassword*/ ) = 0;
	
	virtual ~IPlayer(){};
};

#endif // __INCLUDE_IPLAYER_H__