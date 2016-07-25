//-----------------------------------------//
//                                         //
//  File		: S3PDBConnector.cpp	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/25/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBConnector.h"

S3PDBConnector * S3PDBConnector::m_pInstance = NULL;

S3PDBConnector * S3PDBConnector::Instance()
{
	if (m_pInstance == NULL)
	{
		 m_pInstance = new S3PDBConnector();
	}
	return m_pInstance;
}

void S3PDBConnector::ReleaseInstance()
{
	if ( NULL != m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

S3PDBConnector::S3PDBConnector()
{
}

S3PDBConnector::~S3PDBConnector()
{

}

/*
S3PDBConnection* S3PDBConnector::ApplyDBConnection( LPCTSTR lpszDBSection )
{
	int iDBIdentifier = def_ACCOUNTDB;
	S3PDBConnection* pRet = NULL;
	S3PDBConnection* pConnection = new S3PDBConnection;
	if ( NULL != pConnection )
	{
		if ( TRUE == pConnection->Connect( iDBIdentifier ) )
		{
			pRet = pConnection;
		}
		else
		{
			delete pConnection;
			pConnection = NULL;
		}
	}
	return pRet;
}
*/
S3PDBConnection* S3PDBConnector::ApplyDBConnection( int iDBIdentifier )
{
	S3PDBConnection* pRet = NULL;
	S3PDBConnection* pConnection = new S3PDBConnection;
	if ( NULL != pConnection )
	{
		if ( TRUE == pConnection->Connect( iDBIdentifier ) )
		{
			pRet = pConnection;
		}
		else
		{
			delete pConnection;
			pConnection = NULL;
		}
	}
	return pRet;
}