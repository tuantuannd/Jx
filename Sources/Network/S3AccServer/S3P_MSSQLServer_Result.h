// S3P_MSSQLServer_Result.h: interface for the S3P_MSSQLServer_Result class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3P_MSSQLSERVER_RESULT_H__16A5506B_D906_41FD_91A2_9A87D4EB4E53__INCLUDED_)
#define AFX_S3P_MSSQLSERVER_RESULT_H__16A5506B_D906_41FD_91A2_9A87D4EB4E53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include "S3PResultVBC.h"

#include "msado15.tlh"

class S3P_MSSQLServer_Result : public S3PResultVBC  
{
public:
	_RecordsetPtr m_pResult;
	S3P_MSSQLServer_Result()
	{
		m_pResult = NULL;
	}
	virtual ~S3P_MSSQLServer_Result()
	{
		m_pResult = NULL;
	}

	void AttachResult(_RecordsetPtr& pResult)
	{
		m_pResult = pResult;
	}

	virtual int num_rows() const;
	virtual int num_fields() const;
	virtual void data_seek (unsigned int offset, int nType) const;
	virtual bool get_field_data(unsigned int nfieldindex, void* pData, unsigned long nsize);
	virtual int unuse();
};

#endif // !defined(AFX_S3P_MSSQLSERVER_RESULT_H__16A5506B_D906_41FD_91A2_9A87D4EB4E53__INCLUDED_)
