// S3PTableInfoCatch.cpp: implementation of the S3PTableInfoCatch class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTableInfoCatch.h"
#include "S3PDBConnection.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
S3PTableInfoCatch * S3PTableInfoCatch::m_pInstance = NULL;

S3PTableInfoCatch::S3PTableInfoCatch()
{
}

S3PTableInfoCatch::~S3PTableInfoCatch()
{
	std::map<std::string,std::map<std::string, std::string> *>::iterator i;
	for(i=m_initedMap.begin(); i!=m_initedMap.end(); i++)
	{
		delete i->second;
	}
	m_initedMap.clear();

}

S3PTableInfoCatch * S3PTableInfoCatch::Instance()
{
	if (m_pInstance==NULL)
	{
		m_pInstance = new S3PTableInfoCatch;
	}
	return m_pInstance;
}

void S3PTableInfoCatch::Release()
{
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
std::map<std::string, std::string> * 
	S3PTableInfoCatch::GetColumnInfo(std::string tableName, S3PDBConnection * pConn)
{
	std::map<std::string,std::map<std::string, std::string> *>::iterator i;
	i = m_initedMap.find(tableName);
    if(i != m_initedMap.end() )
	{
		return i->second;
	}
	else
	{
		std::string strQuery;
		strQuery = "select * from " + tableName + " where 1<>1";
		
		if ( pConn->QueryBySql(strQuery.c_str()) )
		{
			Result * pResult = (Result*)(pConn->GetRes());
			if (pResult)
			{
				int col_count = pResult->num_fields();
				if (col_count>0)
				{
					std::map<std::string, std::string> * columnInfoMap = 
						new std::map<std::string, std::string>;
					//Question no delete up! must delete in somewhere ,romandou
					for (unsigned int i = 0; i < col_count; i++)
					{
						std::string column = pResult->names(i);
						std::string type = pResult->types(i).sql_name();
						columnInfoMap->insert(std::map<std::string, std::string>::value_type(column,type));
					}
					m_initedMap[tableName] = columnInfoMap;
					return columnInfoMap;
				}
				else
				{
					return NULL;
				}
			}
			else
			{
				// Error
				return NULL;
			}
		}
		else
		{
			//Ê§°Ü
			return NULL;
		}
	}
	return NULL;
}
