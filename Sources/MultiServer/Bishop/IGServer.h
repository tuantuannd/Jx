/********************************************************************
	created:	2003/05/11
	file base:	IGServer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_IGSERVER_H__
#define __INCLUDE_IGSERVER_H__

class IGServer
{
public:
	/*
	 * Interface
	 */
	virtual bool Create() = 0;
	virtual bool Destroy() = 0;
	
	virtual bool Attach( const char *pAccountName ) = 0;
	
	virtual void SendText( const char *pText, int nLength, UINT uOption ) = 0;
	
	virtual size_t GetID() = 0;
	virtual size_t GetContent() = 0;
	virtual size_t GetCapability() = 0;
	virtual size_t GetIP( BYTE cType ) = 0;
	virtual size_t GetIndentify() = 0;
	
	virtual bool AnalyzeRequire( const void *pData, size_t datalength ) = 0;

	virtual bool DispatchTask( UINT nTask, const void *pData, size_t datalength, WORD nData) = 0;

	virtual ~IGServer(){};
};

#endif // __INCLUDE_IGSERVER_H__