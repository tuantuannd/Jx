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
	virtual bool Run() = 0;
	virtual bool AppendData( UINT nOwner, const void *pData, size_t dataLength ) = 0;

	virtual bool Attach( const char *pRoleName ) = 0;

	virtual bool IsActive() = 0;
	virtual bool Active() = 0;
	virtual bool Inactive() = 0;
	
	virtual ~IPlayer(){};
};

#endif // __INCLUDE_IPLAYER_H__