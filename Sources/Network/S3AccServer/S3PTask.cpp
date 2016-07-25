// S3PTask.cpp: implementation of the S3PTask class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTask.h"
#include "S3PDBConnector.h"
#include "S3PTaskListDAO.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTask::S3PTask(int iid)
{
	char buf[200];
	sprintf(buf,"%d",iid);
	m_primaryFields["iid"] = buf;
	Init();
	Load();
}

S3PTask::S3PTask()
{
	Init();
}

S3PTask::~S3PTask()
{
	Clear();
}

S3PDBConnection	* S3PTask::GetConn()
{
	return S3PDBConnector::Instance()->ApplyDBConnection(def_ROLEDB);
}

S3PTableDAO * S3PTask::GetTableDAO()
{
	return new S3PTaskListDAO(m_pConn);
}

std::string S3PTask::GetAutoIncrementField()
{
	return "iid";
}

std::string S3PTask::GetMandatoryField()
{
	return "iTaskCode";
}
