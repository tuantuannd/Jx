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
	virtual int  Run() = 0;
	
	virtual bool Create() = 0;
	virtual bool Destroy() = 0;

	virtual ~IGServer(){};
};

#endif // __INCLUDE_IGSERVER_H__