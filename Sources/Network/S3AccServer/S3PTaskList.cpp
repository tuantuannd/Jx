// S3PTaskList.cpp: implementation of the S3PTaskList class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTaskList.h"
#include "S3PTask.h"
#include "S3PResult.h"
#include "S3PTaskListDAO.h"
#include "S3PDBConnector.h"
#include "regexpr2.h"

using namespace std;
using namespace regex;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTaskList::~S3PTaskList()
{
	Clear();
}

S3PTaskList::S3PTaskList(std::string cUserCode)
{
	m_cUserCode = cUserCode;
	if ( Init() > 0 )
	{
		m_bInit = TRUE;
	}
	else
	{
		m_bInit = FALSE;
	}
}

int S3PTaskList::Init()
{
	S3PDBConnection * pConn = S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
	if (NULL == pConn)
	{
		return -1;
	}
	std::string strQuery = "select * from Task_List where ";
	
	ColumnAndValue w;
	w["cUserCode"] = m_cUserCode;
	S3PRow row("Task_List", &w, pConn);
	
	std::string strWhere;
	if (row.GetExpLikeWhereAnd(strWhere)<=0)
	{
		pConn->Close();
		return -2;
	}

	strQuery += strWhere + " order by iid";

	S3PTaskListDAO tasks(pConn);
	S3PResult result;
	
	if (tasks.Query(strQuery, result)<=0)
	{
		pConn->Close();
		return -3;
	}

	for(int i=0; i<result.size(); i++)
	{
		ColumnAndValue cav = result[i];
		S3PTask * pTask = new S3PTask(atoi(cav["iid"].c_str()));
		m_list.push_back(pTask);
	}

	pConn->Close();
	return 1;
}

S3PTask * S3PTaskList::operator [] (int i) const
{
	if (!m_bInit)
	{
		return NULL;
	}
	return m_list[i];
}

int S3PTaskList::Size()
{
	if (!m_bInit)
	{
		return 0;
	}
	return m_list.size();
}

int S3PTaskList::Reload()
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

void S3PTaskList::Clear()
{
	std::vector<S3PTask*>::iterator i;
	for(i=m_list.begin(); i!=m_list.end(); i++)
	{
		delete (*i);
	}
	m_list.clear();
}

int S3PTaskList::Add(ColumnAndValue &cav)
{
	if (!m_bInit)
	{
		return 0;
	}

	S3PTask * pTask = new S3PTask();
	if (pTask)
	{
		subst_results results;
		rpattern pat("^\\s*cusercode\\s*$", NOCASE);

		ColumnAndValue::iterator i;
		BOOL bFound = FALSE;
		for(i=cav.begin(); i!=cav.end() && !bFound; i++)
		{
			std::string key = i->first;
			
			rpattern_c::backref_type br = pat.match(key, results );
			if( br.matched )
			{
				bFound = TRUE;
				cav[key]=m_cUserCode;
			}
		}
		if (!bFound)
		{
			cav["cUserCode"] = m_cUserCode;
		}

		if (pTask->Add(cav)>0)
		{
			m_list.push_back(pTask);
		}
		else
		{
			delete pTask;
			return -2;
		}
	}
	else
	{
		if(pTask)
		{
			delete pTask;
		}
		return -1;
	}
	return 1;
}

int S3PTaskList::Delete(int idx)
{
	if (!m_bInit)
	{
		return 0;
	}

	S3PTask * pTask = m_list[idx];
	pTask->Delete();
	delete pTask;

	m_list.erase(m_list.begin()+idx);
	return 1;
}


int S3PTaskList::DeleteAll()
{
	if (!m_bInit)
	{
		return 0;
	}

	std::vector<S3PTask*>::iterator i;
	for(i=m_list.begin(); i!=m_list.end(); i++)
	{
		S3PTask *pTask = (*i);
		pTask->Delete();
		delete pTask;
	}
	m_list.clear();
	return 1;
}
