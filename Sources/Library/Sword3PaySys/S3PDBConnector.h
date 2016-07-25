//-----------------------------------------//
//                                         //
//  File		: S3PDBConnector.h		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/25/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBCONNECTOR_H_
#define _S3PDBCONNECTOR_H_

#pragma warning( disable : 4786 4800 4355 )

#include "KStdAfx.h"
#include "S3PDBConnection.h"
#include "S3PDBConnectionPool.h"
#include <string>

class S3PDBConnector  
{
public:
	static S3PDBConnector * Instance();
	static void             ReleaseInstance();

	virtual ~S3PDBConnector();

	virtual S3PDBConnection* ApplyDBConnection( LPCTSTR lpszDBSection );
	virtual S3PDBConnection* ApplyDBConnection( int iDBIdentifier );

protected:
	S3PDBConnector();
protected:
	static S3PDBConnector * m_pInstance;
};

#endif	// _S3PDBCONNECTOR_H_
