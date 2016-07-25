// DataPointFromStream.h: interface for the DataPointFromStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAPOINTFROMSTREAM_H__39AB387C_D68E_4549_B0A9_8F9A59891740__INCLUDED_)
#define AFX_DATAPOINTFROMSTREAM_H__39AB387C_D68E_4549_B0A9_8F9A59891740__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include<iostream>
#include<memory>
using namespace  std;
namespace PackageCQ
{	
	
	class DataPointFromStream  
	{
	public:
		int GetPointPosition() const;
		DataPointFromStream();
		iostream& GetStreamPointToData();
		
		DataPointFromStream(iostream& stream,unsigned long Position);
		virtual ~DataPointFromStream();
		const DataPointFromStream& operator =(const DataPointFromStream &Source);
	private:
		iostream *   m_DataArea;
		unsigned long  m_Pos;
	};
}
#endif // !defined(AFX_DATAPOINTFROMSTREAM_H__39AB387C_D68E_4549_B0A9_8F9A59891740__INCLUDED_)
