// S3PManipulator.h: interface for the S3PManipulator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PMANIPULATOR_H__9DFA310E_2CBD_49EF_8528_93F0FE913FDA__INCLUDED_)
#define AFX_S3PMANIPULATOR_H__9DFA310E_2CBD_49EF_8528_93F0FE913FDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PTableDAO.h"
#include "S3PResult.h"
#include "S3PRow.h"

class S3PDBConnection;
//class S3PTableDAO;

enum DataMrgContext
{
	add,
	update,
	del
};

class S3PManipulator  
{
public:
	S3PManipulator();
	virtual ~S3PManipulator();
	virtual ColumnAndValue & GetProperties();
	virtual int Add(ColumnAndValue & cav);
	virtual int Load();
	virtual int Save();
	virtual int Delete();
	virtual int ValidateData(ColumnAndValue & cav, DataMrgContext dmc);
protected:
	virtual int Init();
	virtual S3PDBConnection	* GetConn() = 0;
	virtual S3PTableDAO * GetTableDAO() = 0;
	virtual std::string GetAutoIncrementField() = 0;
	virtual std::string GetMandatoryField() = 0;

	virtual void Clear();
protected:
	BOOL  m_bInit;
	S3PDBConnection * m_pConn;
	S3PTableDAO*   m_pTableDAO;
	ColumnAndValue m_properties;
	ColumnAndValue m_primaryFields;
};

#endif // !defined(AFX_S3PMANIPULATOR_H__9DFA310E_2CBD_49EF_8528_93F0FE913FDA__INCLUDED_)
