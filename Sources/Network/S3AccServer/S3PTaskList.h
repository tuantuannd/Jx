// S3PTaskList.h: interface for the S3PTaskList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_S3PTASKLIST_H__1D926926_41D8_4B70_9D5D_240F1AEFBA49__INCLUDED_)
#define AFX_S3PTASKLIST_H__1D926926_41D8_4B70_9D5D_240F1AEFBA49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KStdAfx.h"
#include <string>
#include <vector>
#include "S3PRow.h"

class S3PTask;


class S3PTaskList  
{
public:
	int DeleteAll();
	int Delete(int idx);
	int Add(ColumnAndValue & cav);
	void Clear();
	virtual int Reload();
	S3PTaskList(std::string cUserCode);
	virtual ~S3PTaskList();


	S3PTask * operator [] (int i) const;
	int Size();
protected:
	int Init();

protected:
	BOOL m_bInit;
	std::vector<S3PTask*> m_list;
	std::string m_cUserCode;
};

#endif // !defined(AFX_S3PTASKLIST_H__1D926926_41D8_4B70_9D5D_240F1AEFBA49__INCLUDED_)
