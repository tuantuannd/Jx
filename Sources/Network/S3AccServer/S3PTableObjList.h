// S3PTableObjList.h: interface for the S3PTableObjList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTABLEOBJLIST_H__0F1D2380_72D6_4A8B_8D68_2A937A1748E7__INCLUDED_)
#define AFX_S3PTABLEOBJLIST_H__0F1D2380_72D6_4A8B_8D68_2A937A1748E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include <string>
#include <vector>
#include "S3PRow.h"
#include "S3PResult.h"
#include "S3PTableDAO.h"
#include "S3PDBConnector.h"
#include "regexpr2.h"

using namespace std;
using namespace regex;

template <class T>
class S3PTableObjList  
{
protected:
	BOOL m_bInit;
	std::vector<T*> m_list;
	std::string m_foreignValue;
	std::string m_foreignKey;
	std::string m_tableName;
	std::string m_orderBy;

public:
	S3PTableObjList(std::string tableName, 
					std::string orderBy,
					std::string foreignKey,
					std::string foreignValue)
	{
		m_foreignValue = foreignValue;
		m_foreignKey   = foreignKey;
		m_tableName = tableName;
		m_orderBy = orderBy;
		if ( Init() > 0 )
		{
			m_bInit = TRUE;
		}
		else
		{
			m_bInit = FALSE;
		}
	}
	
	virtual ~S3PTableObjList()
	{
		Clear();
	}

	int DeleteAll()
	{
		if (!m_bInit)
		{
			return 0;
		}

		std::vector<T*>::iterator i;
		for(i=m_list.begin(); i!=m_list.end(); i++)
		{
			T *pT = (*i);
			pT->Delete();
			delete pT;
		}
		m_list.clear();
		return 1;
	}

	int Delete(int idx)
	{
		if (!m_bInit)
		{
			return 0;
		}

		T * pT = m_list[idx];
		pT->Delete();
		delete pT;

		m_list.erase(m_list.begin()+idx);
		return 1;
	}

	int Add(ColumnAndValue & cav)
	{
		if (!m_bInit)
		{
			return 0;
		}

		T *pT = new T();
		if (pT)
		{
			subst_results results;
			rpattern pat("^\\s*"+ m_foreignKey +"\\s*$", NOCASE);

			ColumnAndValue::iterator i;
			BOOL bFound = FALSE;
			for(i=cav.begin(); i!=cav.end() && !bFound; i++)
			{
				std::string key = i->first;
				
				rpattern_c::backref_type br = pat.match(key, results );
				if( br.matched )
				{
					bFound = TRUE;
					cav[key]=m_foreignValue;
				}
			}
			if (!bFound)
			{
				cav[m_foreignKey] = m_foreignValue;
			}

			if (pT->Add(cav)>0)
			{
				m_list.push_back(pT);
			}
			else
			{
				delete pT;
				return -2;
			}
		}
		else
		{
			if(pT)
			{
				delete pT;
			}
			return -1;
		}
		return 1;
	}
	void Clear()
	{
		std::vector<T*>::iterator i;
		for(i=m_list.begin(); i!=m_list.end(); i++)
		{
			delete (*i);
		}
		m_list.clear();
	}

	int Reload()
	{
		Clear();
		if ( Init() > 0 )
		{
			m_bInit = TRUE;
		}
		else
		{
			m_bInit = FALSE;
		}

		return m_bInit;
	}

	T * operator [] (int i) const
	{
		if (!m_bInit)
		{
			return NULL;
		}
		return m_list[i];
	}

	int Size()
	{
		if (!m_bInit)
		{
			return 0;
		}
		return m_list.size();
	}
protected:
	int Init()
	{
		S3PDBConnection * pConn = S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
		if (NULL == pConn)
		{
			return -1;
		}
		std::string strQuery = "select * from " + m_tableName + " where ";
		
		ColumnAndValue w;
		w[m_foreignKey] = m_foreignValue;
		S3PRow row(m_tableName, &w, pConn);
		
		std::string strWhere;
		if (row.GetExpLikeWhereAnd(strWhere)<=0)
		{
			pConn->Close();
			return -2;
		}

		strQuery += strWhere + " order by " + m_orderBy;

		S3PResult result;
		
		if (S3PTableDAO::Query(pConn, strQuery, result)<=0)
		{
			pConn->Close();
			return -3;
		}

		for(int i=0; i<result.size(); i++)
		{
			ColumnAndValue cav = result[i];
			T * pT = new T(atoi(cav["iid"].c_str()));
			m_list.push_back(pT);
		}

		pConn->Close();
		return 1;
	}
};



#endif // !defined(AFX_S3PTABLEOBJLIST_H__0F1D2380_72D6_4A8B_8D68_2A937A1748E7__INCLUDED_)
