//-----------------------------------------//
//                                         //
//  File		: S3PDBConnection.cpp      //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBConnection.h"


S3PDBConnection::S3PDBConnection():
m_pRes( NULL )
{
	Init();
}

S3PDBConnection::~S3PDBConnection()
{
}

void S3PDBConnection::Close()
{
	S3PDBConnectionPool::Instance()->SetDBConFree( m_info.iDBIdentifier, m_info.dwConID );

	if ( NULL != m_info.pCon )
	{
		Init();
	}
	m_pRes = NULL;

	delete this;
}

DWORD S3PDBConnection::GetRes()
{
	return m_pRes;
}

BOOL S3PDBConnection::QueryBySql(LPCTSTR lpszSql)
{
	BOOL bRet = FALSE;
	m_pRes = NULL;
	if ( ( NULL != lpszSql )
		&& ( NULL != m_info.pCon ) )
	{
		try
		{
			Query query = m_info.pCon->query();
			query << lpszSql;
			m_res = query.store();
			m_pRes = ( DWORD )( &m_res );
			bRet = TRUE;
		}
		catch (BadQuery er)
		{
			cerr << "Error: " << er.error << endl;
			bRet = FALSE;
		}
		catch (BadConversion er)
		{
			cerr << "Error: Tried to convert \"" << er.data << "\" to a \""
				<< er.type_name << "\"." << endl;
			bRet = FALSE;
		}
	}
	return bRet;
}
#ifdef _DEBUG
FILE * g_pOutFile = NULL;
#endif

BOOL S3PDBConnection::Do( LPCTSTR lpszSql )
{

/*#ifdef _DEBUG 
	if (strstr(lpszSql, "Role_Info"))
	{
		fwrite(lpszSql, 1, strlen(lpszSql), g_pOutFile);
		fwrite("\n", 1,1, g_pOutFile);
		fflush(g_pOutFile);
	}
#endif
*/
	BOOL bRet = FALSE;
	m_pRes = NULL;
	if ( ( NULL != lpszSql )
		&& ( NULL != m_info.pCon ) )
	{
		try
		{
			m_res2 = m_info.pCon->execute( lpszSql );
			m_pRes = ( DWORD )( &m_res2 );
			bRet = TRUE;
		}
		catch (BadQuery er)
		{
			cerr << "Error: " << er.error << endl;
			bRet = FALSE;
		}
		catch (BadConversion er)
		{
			cerr << "Error: Tried to convert \"" << er.data << "\" to a \""
				<< er.type_name << "\"." << endl;
			bRet = FALSE;
		}
	}
	return bRet;
}

void S3PDBConnection::Init()
{
	m_info.dwConID = def_ERRORCONID;
	m_info.iDBIdentifier = -1;
	m_info.pCon = NULL;
}

BOOL S3PDBConnection::Connect(int iDBIdentifier)
{
	BOOL bRet = FALSE;
	if ( NULL == m_info.pCon )
	{
		_CONNECTION_ID con_ID =
			S3PDBConnectionPool::Instance()->ApplyDBConnection( iDBIdentifier );
		if ( ( def_ERRORCONID != con_ID.dwID )
			&& ( NULL != con_ID.pCon ) )
		{
			m_info.dwConID = con_ID.dwID;
			m_info.pCon = con_ID.pCon;
			m_info.iDBIdentifier = iDBIdentifier;
			bRet = TRUE;
		}
	}
	return bRet;
}

std::string S3PDBConnection::GetDate()
{
	std::string strRet("");
	std::string strDateTime = GetDateTime();
	if ( "" != strDateTime )
	{
		const char cSpace = 0x20;
		std::string::size_type index = strDateTime.find( cSpace );
		if ( std::string::npos != index )
		{
			strRet = strDateTime.substr( 0, index );
		}
	}
	return strRet;
}

std::string S3PDBConnection::GetDateTime()
{
	std::string strRet("");
	if ( NULL != m_info.pCon )
	{
		try
		{
			Query query = m_info.pCon->query();
			query << "Select NOW()";
			Result res = query.store();
			if ( res.empty() )
			{
				throw BadQuery("Failed to get time.");
			}
			Row row = res[0];
			strRet = row[0];
		}
		catch ( BadQuery er )
		{ 
			cerr << "Error: " << er.error << endl;
		}
		catch ( BadConversion er )
		{ 
			cerr << "Error: Tried to convert \"" << er.data << "\" to a \"" 
				<< er.type_name << "\"." << endl;
		}
	}
	return strRet;
}