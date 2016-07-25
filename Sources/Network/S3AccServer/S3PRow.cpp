/**************************************************/
/*                                                */
/*  文件名:    S3PRow.cpp                         */
/*  描述    :  它接受一个hash表作为每个行的数据， */
/*             协助S3PTableDAO生成sql语句         */
/*                                                */
/*	作者	 : Liu Wansong                        */
/*	创建日期 : 8/22/2002                          */
/*  修改日期 : 8/26/2002                          */
/**************************************************/

#include "S3PRow.h"
#include "regexpr2.h"
#include "S3PTableInfoCatch.h"

using namespace std;
using namespace regex;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PRow::S3PRow(const std::string & tableName, ColumnAndValue * pCAV, S3PDBConnection * pConn)
{
	m_tableName = tableName;
	m_pCAV = pCAV;
	m_pConn = pConn;

	Init();
}

S3PRow::~S3PRow()
{
	m_pCAV = NULL;
	m_pConn = NULL;
}

int S3PRow::GetExpLikeInsert(std::string & exp)
{
	if (NULL == m_pCAV)
	{
		return -1;
	}

	std::string keys;
	std::string values;
	
	ColumnAndValue::iterator i;
	for(i=m_pCAV->begin(); i!=m_pCAV->end(); i++)
	{
		std::string key   = i->first;
		std::string value = i->second;
		keys += key + ",";

		std::string value_q = "";
		if ( FixValue(key, value, value_q) > 0 )
		{
			values += value_q + ",";
		}
		else
		{
			// 错误，不能修正字段的值
			return -2;
		}
	}

	subst_results results;
	rpattern pat(",$", "");
	
	pat.substitute(keys, results );
	pat.substitute(values, results);

	exp = "(" + keys + ") values(" + values + ")";
	return 1;
}

int S3PRow::FixValue(const std::string & key, const std::string & value, std::string & outValue)
{
	if (NULL == m_columnInfoMap)
		return -1;
	
	BOOL bFound = FALSE;
	std::string strType = "";

	outValue = value;

	std::map<string,string>::iterator i;
	for(i=m_columnInfoMap->begin(); i!=m_columnInfoMap->end(); i++)
	{
		std::string strKey = i->first;

		match_results results;
		rpattern pat("^\\s*" + key + "\\s*$", NOCASE);
		
		match_results::backref_type br = pat.match( strKey, results );
		if ( br.matched )
		{
			bFound = TRUE;
			strType = i->second;
			break;
		}
	}

	if ( !bFound )
	{
		//这个表中没有这个字段
		return -1;
	}

	if ( IsNeedQuote(strType) )
	{
		if ( Quote(value, outValue) <= 0 )
		{
			//给数据去掉敏感字符时出错
			return -2;
		}
	}

	return 1;
}

int S3PRow::Quote(const std::string &value, std::string &outValue)
{
	outValue = value;

	subst_results results;

	rpattern pat("'", "''", GLOBAL);
	
	pat.substitute(outValue, results );
	
	outValue = "'" + outValue + "'";

	return 1;
}

BOOL S3PRow::IsNeedQuote(const std::string &column)
{
	match_results results;

    rpattern pat("^\\s*(bigint|decimal|double|enum|float|int|mediumint|numeric|real|set|smallint|tinyint)",NOCASE);

    match_results::backref_type br = pat.match( column, results );

    if( br.matched )
	{
		return FALSE;
    }

	return TRUE;
}

BOOL S3PRow::Init()
{
	if (NULL == m_pCAV)
		return FALSE;

	if (NULL == m_pConn)
		return FALSE;

	m_columnInfoMap = 
		S3PTableInfoCatch::Instance()->GetColumnInfo(m_tableName, m_pConn);
	if (NULL == m_columnInfoMap)
		return FALSE;

	return TRUE;
}

int S3PRow::GetExpLikeUpdate(std::string &exp)
{
	if (NULL == m_pCAV)
	{
		return -1;
	}

	ColumnAndValue::iterator i;
	for(i=m_pCAV->begin(); i!=m_pCAV->end(); i++)
	{
		std::string key   = i->first;
		std::string value = i->second;

		exp += key + "=";

		std::string value_q = "";
		if ( FixValue(key, value, value_q) > 0 )
		{
			exp += value_q + ",";
		}
		else
		{
			// 错误，不能修正字段的值
			return -1;
		}
	}

	subst_results results;
	rpattern pat(",$", "");
	
	pat.substitute(exp, results );
	return 1;
}

int S3PRow::GetExpLikeWhereAnd(std::string &exp)
{
	if (NULL == m_pCAV)
	{
		return -1;
	}

	ColumnAndValue::iterator i;
	for(i=m_pCAV->begin(); i!=m_pCAV->end(); i++)
	{
		std::string key   = i->first;
		std::string value = i->second;

		exp += key + "=";

		std::string value_q = "";
		if ( FixValue(key, value, value_q) > 0 )
		{
			exp += value_q + " and ";
		}
		else
		{
			// 错误，不能修正字段的值
			return -1;
		}
	}

	subst_results results;
	rpattern pat("and\\s*$", "");
	
	pat.substitute(exp, results );
	return 1;
}

int S3PRow::GetExpLikeInsertKey(std::string &exp)
{
	if (NULL == m_pCAV)
	{
		return -1;
	}

	std::string keys;
	
	ColumnAndValue::iterator i;
	for(i=m_pCAV->begin(); i!=m_pCAV->end(); i++)
	{
		std::string key   = i->first;
		keys += key + ",";
	}

	subst_results results;
	rpattern pat(",$", "");
	
	pat.substitute(keys, results );

	exp = "(" + keys + ")";
	return 1;
}

int S3PRow::GetExpLikeInsertValue(std::string &exp)
{
	if (NULL == m_pCAV)
	{
		return -1;
	}

	std::string values;
	
	ColumnAndValue::iterator i;
	for(i=m_pCAV->begin(); i!=m_pCAV->end(); i++)
	{
		std::string key   = i->first;
		std::string value = i->second;

		std::string value_q = "";
		if ( FixValue(key, value, value_q) > 0 )
		{
			values += value_q + ",";
		}
		else
		{
			// 错误，不能修正字段的值
			return -2;
		}
	}

	subst_results results;
	rpattern pat(",$", "");
	
	pat.substitute(values, results);

	exp = "(" + values + ")";
	return 1;
}
