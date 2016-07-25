/**************************************************/
/*                                                */
/*  文件名:    S3PTableDAO.cpp                    */
/*  描述    :  一个表维护基类， 它分析每个        */
/*             字段的类型，然后根据不同的功能     */
/*             组织成相应的sql语句                */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/22/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

//////////////////////////////////////////////////////////////////////

#include "S3PTableDAO.h"
#include "regexpr2.h"
#ifdef def_MYSQL_DB
using namespace std;
using namespace regex;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3P_mySQL_TableDAO::S3P_mySQL_TableDAO()
{

}

S3P_mySQL_TableDAO::~S3P_mySQL_TableDAO()
{

}

int S3P_mySQL_TableDAO::Add(S3PRow * row)
{
	std::string strQuery;
	std::string tableName = GetTableName();

	std::string strInsert;
	if ( row->GetExpLikeInsert(strInsert) > 0 )
	{
		strQuery = "insert into " + tableName + strInsert;
		S3PDBConnection * pConn = GetConnection();
		if(pConn)
		{
			if ( pConn->Do(strQuery.c_str()) )
			{
				// Success
				m_resAdd = *((ResNSel *)pConn->GetRes());
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

int S3P_mySQL_TableDAO::Update(S3PRow * row, S3PRow * where)
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
		}
	}
	else
	{
		return -2;
	}

	return 1;
}

int S3P_mySQL_TableDAO::Delete(S3PRow * where)
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

bool S3P_mySQL_TableDAO::HasItem( S3PRow* where )
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

int S3P_mySQL_TableDAO::Query(std::string q, S3PResult & result)
{
	S3PDBConnection * pConn = GetConnection();
	if(pConn)
	{
		if(pConn->QueryBySql(q.c_str()))
		{
			//Success
			S3PRawResult * pResult = (S3PRawResult*)(pConn->GetRes());
			if (pResult)
			{
				result.SetResult(pResult);
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

int S3P_mySQL_TableDAO::AddGroup(const std::list<ColumnAndValue*> & group)
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

int S3P_mySQL_TableDAO::GetInsertID()
{
	return m_resAdd.insert_id;
}

int S3P_mySQL_TableDAO::Query(S3PDBConnection *pConn, string q, S3PResult &result)
{
	if(pConn)
	{
		if(pConn->QueryBySql(q.c_str()))
		{
			//Success
			S3PRawResult * pResult = (S3PRawResult*)(pConn->GetRes());
			if (pResult)
			{
				result.SetResult(pResult);
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
#endif