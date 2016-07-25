// S3PResult.h: interface for the S3PResult class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PRESULT_H__2696807C_7E47_40E4_88EC_144021C8AB59__INCLUDED_)
#define AFX_S3PRESULT_H__2696807C_7E47_40E4_88EC_144021C8AB59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include <sqlplus.hh>
#include <assert.h>
#include "S3PRow.h"

class S3PResult  
{
protected:
	Result * m_pResult;

protected:

public:
	S3PResult(){m_pResult = NULL;}
	S3PResult(Result * pResult) { m_pResult = pResult;}
	virtual ~S3PResult();

	void SetResult(Result * pResult){m_pResult = pResult;}
	int num_rows() const {assert(m_pResult!=NULL); return m_pResult->num_rows();}
	int num_fields() const { assert( m_pResult != NULL ); return m_pResult->num_fields(); }
	void  data_seek (uint offset) const
	{
		assert(m_pResult!=NULL); 
		m_pResult->data_seek(offset);
	}

	int size() const {return num_rows();}
	int rows() const {return num_rows();}

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

#endif // !defined(AFX_S3PRESULT_H__2696807C_7E47_40E4_88EC_144021C8AB59__INCLUDED_)
