// DataPointFromStream.cpp: implementation of the DataPointFromStream class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataPointFromStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace PackageCQ
{
DataPointFromStream::DataPointFromStream(iostream& stream,unsigned long Position)
:m_DataArea(&stream)
,m_Pos(Position)
{


}

DataPointFromStream::~DataPointFromStream()
{

}

iostream& DataPointFromStream::GetStreamPointToData()
{
	if(m_DataArea !=NULL)
	{
	
	 m_DataArea->seekg(m_Pos,ios_base::beg);
	 return * m_DataArea;
	}
	else
	{
	     throw exception("DataPoint Is NULL when GetStreamPointData" );

	}
	

}

const DataPointFromStream& DataPointFromStream::operator =(const DataPointFromStream &Source)
{
	m_DataArea = Source.m_DataArea ;

	m_Pos      = Source.m_Pos ;


	return * this;

}

DataPointFromStream::DataPointFromStream()
:m_DataArea(NULL)
,m_Pos(0)
{

}

int DataPointFromStream::GetPointPosition() const
{
	return m_Pos;

}
}
