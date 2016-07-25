// S3PFightSkillDAO.h: interface for the S3PFightSkillDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PFIGHTSKILLDAO_H__1E0A554C_20C3_491E_B5F9_31B7F8375C92__INCLUDED_)
#define AFX_S3PFIGHTSKILLDAO_H__1E0A554C_20C3_491E_B5F9_31B7F8375C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PFightSkillDAO : public S3PTableDAO  
{
public:
	S3PFightSkillDAO(S3PDBConnection * pConn);
	virtual ~S3PFightSkillDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();
protected:
	S3PDBConnection * m_pConn;
};

#endif // !defined(AFX_S3PFIGHTSKILLDAO_H__1E0A554C_20C3_491E_B5F9_31B7F8375C92__INCLUDED_)
