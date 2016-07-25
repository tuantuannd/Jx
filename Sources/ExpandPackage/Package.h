// Package.h: interface for the Package class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PACKAGE_H__B0CA3A89_0B86_4056_ADBB_F52E1051B45C__INCLUDED_)
#define AFX_PACKAGE_H__B0CA3A89_0B86_4056_ADBB_F52E1051B45C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<string>
#include<fstream>
#include<strstream>
using namespace std;
#include"DataSource.h"
#include"PackageFileHead.h"
#include"Interval.h"

using namespace CQ;

namespace PackageCQ
{
#define FILEHEADSIZE 8 + 4 +4 
#define INDEXSIZE    4 + 4 +4
	
	class Package  
	{
	public:
		Package(DataSource& Source,const string FileName);
		DataSource& GetDataSource();
		
		Package(string PackageName);
		virtual ~Package();
	private:
		void ConstructionDataBlock(DataSource &Source,unsigned long ConstructionPosition);
		
		bool FileHeadCheck(const PackageFileHead& FileHead) const;
		
		fstream m_DataStream;
		
		DataSource m_DataBlock;
		
		
	};
}

#endif // !defined(AFX_PACKAGE_H__B0CA3A89_0B86_4056_ADBB_F52E1051B45C__INCLUDED_)
