// Package.cpp: implementation of the Package class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Package.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace PackageCQ
{
	
	Package::~Package()
	{
		m_DataStream.clear(); 

		m_DataStream.close();
		
	}
	
	Package::Package(string PackageName)
	{
		try
		{
		
			
			m_DataStream.open(PackageName.c_str(),ios::in|ios::binary );
			if(!m_DataStream.is_open())
			{
				throw exception("File Open Error!!");
				
			}
			
			
			
		}
		catch(exception& error)
		{
			strstream ErrorInfo;
			ErrorInfo<<"Open Package File Error! System Error Info :"<< error.what()<<endl<<ends;
			throw exception(ErrorInfo.str()  );
			
			
			
		}
		
		
		
		
		PackageFileHead FileHead;
		
		FileHead<<m_DataStream;
		
		
		if(!FileHeadCheck(FileHead))
		{
			throw (exception("FILE Type Error:File isn't Packge Type"));
		}
		
		unsigned long  BlockCount = FileHead.GetBlockCount();
		

		unsigned long  IndexOffset = FileHead.GetIndexOffset();

		m_DataStream.seekg(IndexOffset,ios::beg);
		


		BlockSource Block; 
		
		for(size_t i=0;i<BlockCount;i++)
		{
			Block<<m_DataStream;

			int Pos = m_DataStream.tellg(); 
			
			
			m_DataBlock.InsertBlock(Block);
			
		}
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	}
	
	bool Package::FileHeadCheck(const PackageFileHead& FileHead) const
	{
		
		if(FileHead.GetFileType() == string(FileID))
		{
			return true;
			
			
		}
		else
		{
			return false;
			
		}
		
	}
	
	DataSource& Package::GetDataSource()
	{
		return m_DataBlock;
		
	}
	
	Package::Package(DataSource &Source,const string FileName)
	{
		

		
		m_DataStream.open(FileName.c_str(),ios::out|ios::binary );
	
        

		if(!m_DataStream.is_open())
		{
			throw exception("File Open Error!!");

		}
	
		Interval Interval1;
        
		Interval1.StartRecord (); 

		
		unsigned long  BlockStartPosition = sizeof(pack_header);
        
 		
		
		ConstructionDataBlock(Source,BlockStartPosition);
		
		
		
		Interval1.EndRecord();

		int IntervalSecond = Interval1.GetIntervalBySecond(); 

		unsigned long IndexOffset = m_DataStream.tellg();
		
		m_DataBlock >> m_DataStream;


		m_DataStream.seekg(0,ios::beg);

		unsigned long CRC32value = 0;
		
		PackageFileHead FileHead(Source.GetBlockCount(), IndexOffset,FileID,BlockStartPosition,CRC32value) ;
		
		FileHead >> m_DataStream;

	    


		
		
	
		m_DataStream.clear(); 
		m_DataStream.close(); 

		m_DataStream.seekg(0,ios::beg ); 

		m_DataStream.open(FileName.c_str(),ios::in|ios::binary ); 
		



		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	}
	
	void Package::ConstructionDataBlock(DataSource &Source,unsigned long ConstructionPosition)
	{
		int BlockCount = Source.GetBlockCount();
		
		
		
		int CurrentPoint = ConstructionPosition;

	/*	Interval Interval1;

		Interval1.StartRecord (); 
	*/	
		
		
		for( int i=0;i<BlockCount;i++)
		{
			DataPointFromStream FilePoint(m_DataStream,CurrentPoint);
			
			BlockSource         Block(FilePoint);
			

	//		Interval Interval2;
			
	//		Interval2.StartRecord ();
			
			Block.CloneBlockSourceToMe(Source[i]) ;
			
	//		Interval2.EndRecord();
			
	//		cout <<"部分耗时"<<(Interval2.GetInterval() )<<endl; 
			
			
			

			m_DataBlock.InsertBlock(Block);

			 
			
			CurrentPoint = CurrentPoint + Block.GetLength(); 
		}
		
	//	Interval1.EndRecord();

	//	cout <<"总循环耗时"<<(Interval1.GetInterval() )<<endl; 

		
		
		
	}
}
