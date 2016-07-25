// DataSource.cpp: implementation of the DataSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataSource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace PackageCQ
{
	DataSource::DataSource()
	
	{
		
	}
	
	DataSource::~DataSource()
	{
		
	}
	
	void DataSource::InsertBlock(const BlockSource & Block )
	{
		m_BlockSet.insert(Block) ;
		
		
	}
	
	int DataSource::GetBlockCount() const
	{
		return m_BlockSet.size() ;
		
	}
	BlockSource &  DataSource::operator [](size_t Index)
	{
		
		if(Index >= 0 && Index< GetBlockCount())
		{
			set<BlockSource>::iterator ContainerPointer;
			
			ContainerPointer = m_BlockSet.begin();
			
			for(size_t i = 0; i < Index ; i ++)
			{
				ContainerPointer++;
			}

		
			
			return  BlockSource(*ContainerPointer);
			
			
			
		}
		else
		{
			throw exception("DataSource  Index is  overflow!!");
			
		}
		
	}
	
	const DataSource DataSource::operator +(DataSource &Source) const
	{
	
		DataSource Result = * this;
		
		
		int SourceCount = Source.GetBlockCount();
		
		for(int i =0 ;i<SourceCount;i++)
		{
			BlockSource SourceElem = Source[i];
			
			set <BlockSource> :: iterator SourceElemInSelfPosition;
			
			SourceElemInSelfPosition = Result.m_BlockSet.find(SourceElem);
			
			if(SourceElemInSelfPosition == Result.m_BlockSet.end())
			{
				Result.m_BlockSet.insert(SourceElem) ;
				
				
				
			}
			else
			{
				Result.m_BlockSet.erase(SourceElemInSelfPosition) ;
				
				Result.m_BlockSet.insert(SourceElem) ;
				
				
				
			}
			
			
			
			
			
			
			
			
		}
		
		return Result;
		
		
		
		
		
		
		
		
		
	}
	
	const DataSource& DataSource::operator =(const DataSource &Source) 
	{
		m_BlockSet = Source.m_BlockSet ;
		
		return * this;
		
		
	}
	

	

	

	
	DataSource& DataSource::operator >>(iostream &DataStream)
	{
		for(int i=0;i<GetBlockCount();i++)
		{
			(* this)[i]>>DataStream;
			
			
		}
		
		return * this;
		
	}
}
