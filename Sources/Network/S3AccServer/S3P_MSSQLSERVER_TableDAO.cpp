// S3P_MSSQLSERVER_TableDAO.cpp: implementation of the S3P_MSSQLSERVER_TableDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3P_MSSQLSERVER_TableDAO.h"
#include "regexpr2.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#ifdef def_MSSQLSERVER_DB
using namespace std;
using namespace regex;

S3P_MSSQLServer_TableDAO::S3P_MSSQLServer_TableDAO()
{

}

S3P_MSSQLServer_TableDAO::~S3P_MSSQLServer_TableDAO()
{

}

int S3P_MSSQLServer_TableDAO::Add( S3PRow * row )
{
	std::string strQuery;
	std::string tableName = GetTableName();

	std::string strInsert;
	if ( row->GetExpLikeInsert(strInsert) > 0 )
	{
		strQuery = "insert into " + tableName + strInsert;
		S3PDBConnection * pConn = GetConnection();
		if( pConn )
		{
			if ( pConn->Do( strQuery.c_str() ) )
			{
				// Success
				m_resAdd = ( _RecordsetPtr )( ( IDispatch* )( pConn->GetRes() ) );
			}
			else
			{
				return -3;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -2;
	}

	return 1;
}

int S3P_MSSQLServer_TableDAO::AddGroup( const std::list<ColumnAndValue*> & group )
{
	BOOL bFirst = TRUE;
	std::string strQuery;

	std::list<ColumnAndValue*>::const_iterator i;
	int errCode = 0;
	try
	{
		for(i=group.begin();i!=group.end();i++)
		{
			S3PRow row(GetTableName(), (*i), GetConnection());
			if(bFirst)
			{
				std::string keys;
				if (row.GetExpLikeInsertKey(keys)>0)
				{
					strQuery = "insert into " + GetTableName() + keys + " values";
				}
				else
				{
					errCode = -1;
					throw(errCode);
				}
				bFirst = FALSE;
			}
			std::string values;
			if (row.GetExpLikeInsertValue(values)>0)
			{
				strQuery += values + ",";
			}
			else
			{
				errCode = -2;
				throw(errCode);
			}
		}

		subst_results results;
		rpattern pat(",$", "");
		
		pat.substitute(strQuery, results );
		
		S3PDBConnection * pConn = GetConnection();
		if(pConn)
		{
			if ( pConn->Do(strQuery.c_str()) )
			{
				// Success
			}
			else
			{
				errCode = -3;
				throw(errCode);
			}
		}
		else
		{
			errCode = -4;
			throw(errCode);
		}
	}
	catch(...)
	{
		return errCode;
	}

	return 1;
}

int S3P_MSSQLServer_TableDAO::Delete( S3PRow * where /* = NULL */ )
{
	std::string strQuery;
	std::string tableName = GetTableName();

	strQuery = "delete from " + tableName;

	std::string strWhere;
	if (where && where->GetExpLikeWhereAnd(strWhere) > 0 )
	{
		strQuery += " where " + strWhere;
	}

	S3PDBConnection * pConn = GetConnection();
	if(pConn)
	{
		if(pConn->Do(strQuery.c_str()))
		{
			//Success
		}
		else
		{
			return -3;
		}
	}
	else
	{
		return -1;
	}
	return 1;
}

int S3P_MSSQLServer_TableDAO::GetInsertID()
{
	int iRet = 0;
	S3PDBConnection* pCon = GetConnection();
	if ( NULL != pCon )
	{
		if ( NULL != pCon->m_info.pCon )
		{
			try
			{
				
				_RecordsetPtr pRes;
				HRESULT hr =
					pRes.CreateInstance( "ADODB.Recordset" );
				if ( SUCCEEDED(hr) )
				{
					std::string strTableName = GetTableName();
					char szSource[256];
					sprintf( szSource, "SELECT IDENT_CURRENT('%s')", strTableName.c_str() );
					hr =
						pRes->Open( szSource,
						( IDispatch* )( pCon->m_info.pCon ),
						adOpenDynamic,
						adLockOptimistic,
						adCmdText );
					if ( SUCCEEDED(hr) )
					{
						_variant_t IDEntityValue;
						IDEntityValue = pRes->GetCollect("");
						iRet = IDEntityValue.iVal;
					}
					pRes->Close();
				}
			}
			catch( _com_error e )
			{
				printf( "[S3P_MSSQLServer_TableDAO::GetInsertID]COM error: %s\r\n", e.ErrorMessage() );
			}
		}
	}
	return iRet;
}

bool S3P_MSSQLServer_TableDAO::HasItem( S3PRow* where )
{
	bool bRet = false;

	std::string strQuery;
	std::string tableName = GetTableName();

	strQuery = "select * from " + tableName;

	std::string strWhere;
	if ( where && where->GetExpLikeWhereAnd(strWhere) > 0 )
	{
		strQuery += " where " + strWhere;
	}

	S3PDBConnection * pConn = GetConnection();
	if( pConn )
	{
		S3PResult result;
		if ( Query( pConn, strQuery, result ) > 0 )
		{
			if ( result.size() > 0 )
			{
				bRet = true;
			}
		}
	}

	return bRet;
}

int S3P_MSSQLServer_TableDAO::Query( std::string q, S3PResult & result )
{
	int iRet = 0;
	S3PDBConnection* pConn = GetConnection();
	iRet = Query( pConn, q, result );
	return iRet;
}

int S3P_MSSQLServer_TableDAO::Query( S3PDBConnection* pConn, string q, 
									S3PResult& result )
{
	if( pConn )
	{
		if( pConn->QueryBySql( q.c_str() ) )
		{
			S3PRawResult pResult =
				( S3PRawResult )( ( IDispatch* )( pConn->GetRes() ) );
			if ( NULL != pResult )
			{
				//Success
				result.SetResult( pResult );
			}
			else
			{
				return -4;
			}
		}
		else
		{
			return -3;
		}
	}
	else
	{
		return -1;
	}
	return 1;
}

int S3P_MSSQLServer_TableDAO::Update( S3PRow * row, S3PRow * where /* = NULL */ )
{
	std::string strQuery;
	std::string tableName = GetTableName();

	std::string strWhere;
	std::string strUpdate;
	if ( row->GetExpLikeUpdate(strUpdate) > 0 )
	{

		strQuery = "update " + tableName + " set " + strUpdate;
		if (where && where->GetExpLikeWhereAnd(strWhere) >0 )
		{
			strQuery += " where " + strWhere;
			S3PDBConnection * pConn = GetConnection();
			if(pConn)
			{
				if( pConn->Do( strQuery.c_str() ) )
				{
					//Success
				}
				else
				{
					return -3;
				}
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		return -2;
	}

	return 1;
}
#endif