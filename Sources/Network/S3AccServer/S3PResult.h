// S3PResult.h: interface for the S3PResult class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PRESULT_H__2696807C_7E47_40E4_88EC_144021C8AB59__INCLUDED_)
#define AFX_S3PRESULT_H__2696807C_7E47_40E4_88EC_144021C8AB59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include "S3PResultVBC.h"
#include "S3P_MSSQLServer_Result.h"

#ifdef def_MYSQL_DB
#include <sqlplus.hh>
#include <assert.h>
#include "S3PRow.h"

class S3P_mySQL_Result : public S3PResultVBC
{
protected:

public:
	S3P_mySQL_Result(){m_pResult = NULL;}
	S3P_mySQL_Result(S3PRawResult * pResult) { m_pResult = pResult;}
	virtual ~S3P_mySQL_Result();

	virtual void SetResult(S3PRawResult * pResult){m_pResult = pResult;}
	virtual int num_rows() const {assert(m_pResult!=NULL); return m_pResult->num_rows();}
	virtual int num_fields() const { assert( m_pResult != NULL ); return m_pResult->num_fields(); }
	virtual void  data_seek (uint offset) const
	{
		assert(m_pResult!=NULL); 
		m_pResult->data_seek(offset);
	}

	virtual int size() const {return num_rows();}
	virtual int rows() const {return num_rows();}

	const ColumnAndValue operator [] (int i) const
	{
		assert(m_pResult!=NULL); 

		ColumnAndValue cav;
		Row row = (*m_pResult)[i];
		int col_count = m_pResult->num_fields();
		if (col_count>0)
		{
			for (int j = 0; j < col_count; j++)
			{
				std::string column = m_pResult->names(j);
				std::string value = row[j];
				cav[column]=value;
			}
		}
		return cav;
	}
};
#endif
#endif // !defined(AFX_S3PRESULT_H__2696807C_7E47_40E4_88EC_144021C8AB59__INCLUDED_)
