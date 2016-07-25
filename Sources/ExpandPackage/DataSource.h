// DataSource.h: interface for the DataSource class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)

#if !defined(AFX_DATASOURCE_H__66EB8B17_9AB6_422C_8513_995F837DE45E__INCLUDED_)
#define AFX_DATASOURCE_H__66EB8B17_9AB6_422C_8513_995F837DE45E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include<set>

#include "BlockSource.h"	// Added by ClassView


using namespace  std;

namespace PackageCQ
{
	class DataSource  
	{
	public:
		const DataSource operator+ (DataSource &Source1) const;
		int GetBlockCount() const;
		void InsertBlock(const BlockSource & Block );
		DataSource();
		BlockSource& operator [](size_t Index);
		virtual ~DataSource();
		const DataSource& operator =(const DataSource &Source) ;
		DataSource& operator >>(iostream &DataStream);
	private:
	
		set<BlockSource  > m_BlockSet;
		
		
	};
}
#endif // !defined(AFX_DATASOURCE_H__66EB8B17_9AB6_422C_8513_995F837DE45E__INCLUDED_)
