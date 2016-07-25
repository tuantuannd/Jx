// BlockSource.cpp: implementation of the BlockSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BlockSource.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace PackageCQ
{	
	

	BlockSource::~BlockSource()
	{
		
	}
	
	int BlockSource::GetID() const
	{
		return m_BlockID;
		
	}
	
	int BlockSource::GetLength() const
	{
		return m_BlockLength;
		
		
	}
	
	iostream& BlockSource::GetBlockStream() 
	{
		return m_BlockPoint.GetStreamPointToData (); 
		
	}
	
	bool BlockSource::operator ==(const BlockSource &Source) const
	{
		if (m_BlockID == Source.m_BlockID)
		{
			return true;
			
			
			
		}
		else
		{
			return false;
			
		}
		
	}
	
	bool BlockSource::operator <(const BlockSource &Source) const
	{
		if (m_BlockID < Source.m_BlockID)
		{
			return true;
			
			
			
		}
		else
		{
			return false;
			
		}
		
	}
	bool BlockSource::operator >(const BlockSource &Source) const
	{
		if (m_BlockID < Source.m_BlockID)
		{
			return true;
			
			
			
		}
		else
		{
			return false;
			
		}
		
	}
	
	const BlockSource& BlockSource::operator <<(iostream &DataStream)
	{
		
		
		if(!DataStream.eof())
		{
			
			DataStream.read((char *) &m_BlockID,sizeof(m_BlockID));
		}
		else
		{
			throw exception("File is End when Get BlockSource");
		}
		
		if(DataStream.bad() )
		{
			throw exception("read error when Get BlockSource");
		}
		
		
		
		unsigned  DataOffset = 0;
		
		if(!DataStream.eof())
		{
			DataStream.read((char *) &DataOffset,sizeof(DataOffset));
			
		}
		else
		{
			throw exception("File is End when Get BlockSource");
		}
		if(DataStream.bad() )
		{
			throw exception("read error when Get BlockSource");
		}
		
		
		
		if(!DataStream.eof())
		{
			
			DataStream.read((char *) &m_UnCompressBlockLength,sizeof(m_UnCompressBlockLength));
			
			
		}
		else
		{
			throw exception("File is End when Get BlockSource");
		}
		if(DataStream.bad() )
		{
			throw exception("read error when Get BlockSource");
		}
		
		
		
		
		
		if(!DataStream.eof())
		{
			
			DataStream.read((char *) &m_BlockLength,sizeof(m_BlockLength ) - 1);
			
			
		}
		else
		{
			throw exception("File is End when Get BlockSource");
		}
		
		if(DataStream.bad() )
		{
			throw exception("read error when Get BlockSource");
		}
		

		
		if(!DataStream.eof())
		{
			
			DataStream.read((char*)&m_CompressMethod,sizeof(m_CompressMethod));
			
			
		}
		else
		{
			throw exception("File is End when Get BlockSource");
		}
		
		if(DataStream.bad() )
		{
			throw exception("read error when Get BlockSource");
		}
		
		
		
		
		
		m_BlockPoint  = DataPointFromStream(DataStream,DataOffset);
		
		/*int Pos = DataStream.tellg();
		
		  m_BlockLength = ComputeBlockSize(m_UnCompressBlockLength,m_BlockPoint.GetStreamPointToData() );
		  
			DataStream.seekg(Pos,ios::beg); 
		*/
		
		return * this;
		
		
		
	}
	
	
	
	BlockSource::BlockSource()
		:m_BlockPoint()
		,m_BlockID(0)
		,m_BlockLength(0)
	{
		
		
	}
	
	
	
	const BlockSource& BlockSource::operator =(BlockSource &Source)
	{
		if(&Source != this)
		{
			
			
			
			m_BlockID = Source.GetID();
			m_BlockLength = Source.GetLength();
			m_UnCompressBlockLength = Source.m_UnCompressBlockLength;
			m_CompressMethod        = Source.m_CompressMethod; 
			m_BlockPoint            = Source.m_BlockPoint; 
			
			
			
			
			
			
			
		}
		
		
		
		return * this;
		
	}
	
	const BlockSource& BlockSource::operator >>(iostream &DataStream)
	{
		DataStream.write((char *)&m_BlockID,sizeof(m_BlockID));
		
		
		unsigned Offset = m_BlockPoint.GetPointPosition();
		DataStream.write((char *)&(Offset),sizeof(Offset));
		
		DataStream.write((char *)&m_UnCompressBlockLength,sizeof(m_UnCompressBlockLength));
		
		DataStream.write((char *)&m_BlockLength,sizeof(m_BlockLength )- 1);

		DataStream.write((char *)&m_CompressMethod,sizeof(m_CompressMethod));

		
		if(DataStream.bad() )
		{
			throw exception("write BlockSource Error");
		}
		
		
		
		
		
		return * this;
		
		
		
	}
	
	BlockSource::BlockSource(DataPointFromStream &Point)
		:m_BlockLength(0)
		,m_BlockID(0)
		,m_BlockPoint(Point)
		,m_UnCompressBlockLength(0)
	{
		
		
		
	}
	

	BlockSource::BlockSource(const BlockSource &Source)
	{
		m_BlockPoint     = Source.m_BlockPoint ;
		m_BlockID        = Source.m_BlockID ;
		m_BlockLength    = Source.m_BlockLength ;
		m_CompressMethod = Source.m_CompressMethod;
		m_UnCompressBlockLength = Source.m_UnCompressBlockLength ;
		
		
		
		
	}
	void BlockSource::CloneBlockSourceToMe(BlockSource& Source)
	{
		iostream& WrirtePoint = m_BlockPoint.GetStreamPointToData();
		
		m_BlockLength           = Source.m_BlockLength;
		m_BlockID               = Source.m_BlockID ;
		m_UnCompressBlockLength = Source.m_UnCompressBlockLength;
		m_CompressMethod        = Source.m_CompressMethod;
		
		iostream& SourcePoint = Source.GetBlockStream();
		
		
		
		char * Data = new char [m_BlockLength];
		
		auto_ptr<char> DataBlock(Data);
		
		SourcePoint.read(Data,m_BlockLength) ; 



        if(SourcePoint.bad() )
		{
			throw exception ("read file error");
			
		}
		if(SourcePoint.eof() )
		{
			throw exception("File is End when Read File BlockData");
		}
		
		

		
		
		WrirtePoint.write(Data,m_BlockLength) ;
		
		if(WrirtePoint.bad() )
		{
			throw exception ("write file error");
		}
		
		


		/*istream_iterator<char> StartPoint (SourcePoint);
		istream_iterator<char> EndPoint =   StartPoint;
        
   		
		for( int i = 0 ; i < Source.m_BlockLength ; ++i )
		{
		
			++EndPoint;
		}
		copy(StartPoint, EndPoint, ostream_iterator<char>(WrirtePoint));
		*/
		
		
		
		
		
		
		
		
	}
	
	
	
	
	
	
	
	BlockSource::BlockSource(int ID, int DataLength,int UnCompressLength,char CompressMethod, DataPointFromStream &Point)
        :m_BlockID(ID)
		,m_UnCompressBlockLength(UnCompressLength)
		,m_BlockPoint(Point)
		,m_BlockLength(DataLength)
		,m_CompressMethod(CompressMethod)
	{
		
		
	}


	unsigned char BlockSource::GetCompressMethod()
	{
		return m_CompressMethod;
		
	}
}