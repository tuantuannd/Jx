//-----------------------------------------//
//                                         //
//  File		: S3PDBConnection.cpp      //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBConnection.h"

S3PDB_mySQL_Connection::S3PDB_mySQL_Connection():
	m_pRes(NULL), m_pCon(NULL)
{
}

S3PDB_mySQL_Connection::~S3PDB_mySQL_Connection()
{
	assert(m_pCon == NULL);
	assert(m_pRes == NULL);
}

void S3PDB_mySQL_Connection::CloseConnect()
{
	if (NULL != m_pCon)
	{
		m_pCon = NULL;
	}
	m_pRes = NULL;
}

bool S3PDB_mySQL_Connection::OpenConnect(_LPDATABASEINFO lpDBIdentifier)
{
	assert(lpDBIdentifier);
	assert(NULL == m_pCon);
	BOOL bRet = FALSE;
	if (NULL == m_pCon)
	{
		Connection* pCon = (Connection*)(malloc(sizeof(Connection)));
		if (NULL != pCon)
		{
			if (pCon->connect(lpDBIdentifier->strDataBase.c_str(),
								lpDBIdentifier->strServer.c_str(),
								lpDBIdentifier->strUser.c_str(),
								lpDBIdentifier->strPassword.c_str()))
			{
				m_pCon = pCon;
				bRet = TRUE;
			}
			else
			{
				free(pCon);
				pCon = NULL;
			}
		}
	}
	return bRet;
}

bool S3PDB_mySQL_Connection::QuerySql(const char* lpszSql, S3PResultVBC** ppResult)
{
	BOOL bRet = FALSE;
	m_pRes = NULL;
	if ( ( NULL != lpszSql )
		&& ( NULL != m_pCon ) )
	{
		try
		{
			Query query = m_pCon->query();
			query << lpszSql;
			m_res = query.store();
			m_pRes = (DWORD)(&m_res);
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
	if (bRet)
	{
	}
	return bRet;
}

bool S3PDB_mySQL_Connection::Do(const char* lpszSql)
{
	BOOL bRet = FALSE;
	m_pRes = NULL;
	if ( ( NULL != lpszSql )
		&& ( NULL != m_pCon ) )
	{
		try
		{
			m_res2 = m_pCon->execute( lpszSql );
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

std::string S3PDB_mySQL_Connection::GetDate()
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

std::string S3PDB_mySQL_Connection::GetDateTime()
{
	std::string strRet("");
	if ( NULL != m_pCon )
	{
		try
		{
			Query query = m_pCon->query();
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
