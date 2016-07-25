// S3PLifeSkillDAO.h: interface for the S3PLifeSkillDAO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PLIFESKILLDAO_H__656AF8D5_0A21_43B6_B6DC_BCFC901555C3__INCLUDED_)
#define AFX_S3PLIFESKILLDAO_H__656AF8D5_0A21_43B6_B6DC_BCFC901555C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"

class S3PLifeSkillDAO : public S3PTableDAO  
{
public:
	S3PLifeSkillDAO(S3PDBConnection *pConn);
	virtual ~S3PLifeSkillDAO();
	virtual S3PDBConnection * GetConnection();
	virtual std::string GetTableName();
protected:
	S3PDBConnection * m_pConn;

};

#endif // !defined(AFX_S3PLIFESKILLDAO_H__656AF8D5_0A21_43B6_B6DC_BCFC901555C3__INCLUDED_)
