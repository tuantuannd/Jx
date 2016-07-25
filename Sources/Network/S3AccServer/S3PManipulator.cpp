// S3PManipulator.cpp: implementation of the S3PManipulator class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PManipulator.h"
#include "regexpr2.h"

using namespace std;
using namespace regex;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PManipulator::S3PManipulator()
{
	m_pTableDAO = NULL;
	m_pConn = NULL;
}

S3PManipulator::~S3PManipulator()
{
}

ColumnAndValue & S3PManipulator::GetProperties()
{
	return m_properties;
}

int S3PManipulator::Load()
{
	if (!m_bInit)
		return -1;

	S3PRow where(m_pTableDAO->GetTableName(), 
					&m_primaryFields,
					m_pConn);

	std::string strWhere;
	std::string strQuery;
	if ( where.GetExpLikeWhereAnd(strWhere) > 0 )
	{
		strQuery = "select * from " + m_pTableDAO->GetTableName() + " where " + strWhere;
		S3PResult result;
		if ( m_pTableDAO->Query(strQuery, result) > 0 )
		{
			if (result.size()==1)
			{
				m_properties = result[0];
			}
			else
			{
				//没有结果或者结果数量大于1
				return -4;
			}
		}
		else
		{
			//执行query语句出错
			return -3;
		}
	}
	else
	{
		//得不到where子语句
		return -2;
	}

	return 1;
}

int S3PManipulator::Save()
{
	if (!m_bInit)
		return -1;

	if (ValidateData(m_properties, update)<=0)
	{
		return -2;
	}

	S3PRow rowProp(m_pTableDAO->GetTableName(), &m_properties, m_pConn);
	S3PRow rowWhere(m_pTableDAO->GetTableName(), &m_primaryFields, m_pConn);
	
	if ( m_pTableDAO->Update(&rowProp, &rowWhere) > 0 )
	{
		//Success
	}
	else
	{
		//更新失败
		return -3;
	}
	return 1;
}

int S3PManipulator::Add(ColumnAndValue & cav)
{
	if (!m_bInit)
		return -1;

	if (ValidateData(cav, add) <= 0)
	{
		return -2;
	}

	S3PRow rowCav(m_pTableDAO->GetTableName(), &cav, m_pConn);

	if ( m_pTableDAO->Add(&rowCav) > 0 )
	{
		//Success
		int ins_id = m_pTableDAO->GetInsertID();
		std::string field = GetAutoIncrementField();
		char buf[255];
		sprintf(buf, "%d", ins_id);
		m_primaryFields[field] = buf;
		if ( Load() > 0 )
		{
			//Success
		}
		else
		{
			//load更新失败
		}
	}
	else
	{
		//更新失败
		return -2;
	}
	return 1;
}

int S3PManipulator::Init()
{
	m_bInit = FALSE;
	if (m_pConn)
	{
		m_pConn->Close();
		m_pConn=NULL;
	}
	if (m_pTableDAO)
	{
		delete m_pTableDAO;
		m_pTableDAO = NULL;
	}
	m_pConn = GetConn();
	if (!m_pConn)
		return -1;

	m_pTableDAO = GetTableDAO();
	if(!m_pTableDAO)
	{
		m_pConn->Close();
		m_pConn=NULL;
		return -2;
	}

	m_bInit = TRUE;
	return 1;
}

int S3PManipulator::Delete()
{
	if (!m_bInit)
		return -1;

	if (ValidateData(m_primaryFields, del)<=0)
	{
		return -2;
	}

	S3PRow rowWhere(m_pTableDAO->GetTableName(), &m_primaryFields, m_pConn);
	
	if ( m_pTableDAO->Delete(&rowWhere) > 0 )
	{
		//Success
		m_properties.clear();
	}
	else
	{
		//删除失败
		return -3;
	}
	return 1;
}

void S3PManipulator::Clear()
{
	if (m_pConn)
	{
		m_pConn->Close();
		m_pConn=NULL;
	}
	if (m_pTableDAO)
	{
		delete m_pTableDAO;
		m_pTableDAO = NULL;
	}
}

int S3PManipulator::ValidateData(ColumnAndValue &cav, DataMrgContext dmc)
{
	if (!m_bInit)
		return -1;

	if (dmc == add)
	{
		std::string mkey = GetMandatoryField();

		if (mkey != "")
		{
			std::string mvalue;
			subst_results results;
			rpattern pat("^\\s*" + mkey + "\\s*$", NOCASE);

			ColumnAndValue::iterator i;
			BOOL bFound = FALSE;
			for(i=cav.begin(); i!=cav.end() && !bFound; i++)
			{
				std::string key = i->first;
				
				rpattern_c::backref_type br = pat.match(key, results );
				if( br.matched )
				{
					bFound = TRUE;
					mvalue = i->second;
				}
			}
			if (!bFound)
			{
				//主键不存在
				return -2;
			}

			if (mvalue == "")
			{
				return -3;
			}

			ColumnAndValue w;
			w[mkey]=mvalue;
			S3PRow rowW(m_pTableDAO->GetTableName(), &w, m_pConn);
			if ( m_pTableDAO->HasItem(&rowW) )
			{
				//数据重复
				return -4;
			}
		}
	}
	return 1;
}
