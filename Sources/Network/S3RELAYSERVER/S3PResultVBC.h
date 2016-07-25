// S3PResultVBC.h: interface for the S3PResultVBC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PRESULTVBC_H__D0D0FB5C_3D6A_4C6F_8F43_2AAFCD93ADF2__INCLUDED_)
#define AFX_S3PRESULTVBC_H__D0D0FB5C_3D6A_4C6F_8F43_2AAFCD93ADF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdafx.h"

class S3PResultVBC
{
protected:
	virtual ~S3PResultVBC();
	S3PResultVBC();
public:
	virtual int num_rows() const = 0;
	virtual int num_fields() const = 0;
	enum
	{
		begin,
		next,
		previous,
		end
	};
	virtual void data_seek(unsigned int offset, int nType) const = 0;
	virtual bool get_field_data(unsigned int nfieldindex, void* pData, unsigned long nsize) = 0;
	virtual int unuse() = 0;
};

#endif // !defined(AFX_S3PRESULTVBC_H__D0D0FB5C_3D6A_4C6F_8F43_2AAFCD93ADF2__INCLUDED_)
