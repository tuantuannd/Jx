// DBClient.h: interface for the CDBClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBCONNECT_H__ED4A7208_6DDB_442A_B974_B34DF73F99A5__INCLUDED_)
#define AFX_DBCONNECT_H__ED4A7208_6DDB_442A_B974_B34DF73F99A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetClient.h"
#include "NetCenter.h"

class CDBCenter : public CNetCenter  
{
public:
	CDBCenter();
	virtual ~CDBCenter();

public:
	BOOL TraceInfo();
};

class CDBClient : public CNetClient  
{
public:
	CDBClient(class CDBCenter* pRootCenter, BOOL bAutoFree);
	virtual ~CDBClient();

protected:
	virtual void RecvPackage(const void* pData, size_t size);
	virtual void OnServerEventCreate();
	virtual void OnServerEventClose();

private:
};

#endif // !defined(AFX_DBCONNECT_H__ED4A7208_6DDB_442A_B974_B34DF73F99A5__INCLUDED_)
