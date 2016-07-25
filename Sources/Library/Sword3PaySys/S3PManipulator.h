// S3PManipulator.h: interface for the S3PManipulator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PMANIPULATOR_H__9DFA310E_2CBD_49EF_8528_93F0FE913FDA__INCLUDED_)
#define AFX_S3PMANIPULATOR_H__9DFA310E_2CBD_49EF_8528_93F0FE913FDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "S3PRow.h"

class S3PDBConnection;
class S3PTableDAO;

enum DataMrgContext
{
	add,
	update,
	del
};

class  S3PManipulator  
{
public:
	ColumnAndValue m_primaryFields;
	S3PManipulator();
	virtual ~S3PManipulator();
	virtual ColumnAndValue & GetProperties();
	virtual int Add(ColumnAndValue & cav);
	virtual int Load();
	virtual int Load(ColumnAndValue &cav);
	virtual int Save();
	virtual int Delete();
	virtual int ValidateData(ColumnAndValue & cav, DataMrgContext dmc);
	virtual int GetLastResult(){ return m_bLastResult;};
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
public:
	ColumnAndValue m_properties;
	BOOL  m_bLastResult; //上一次执行指令的结果
	
};

#endif // !defined(AFX_S3PMANIPULATOR_H__9DFA310E_2CBD_49EF_8528_93F0FE913FDA__INCLUDED_)
