// BlockSource.h: interface for the BlockSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLOCKSOURCE_H__701243D3_7556_46B9_BB6C_9573FD6E142F__INCLUDED_)
#define AFX_BLOCKSOURCE_H__701243D3_7556_46B9_BB6C_9573FD6E142F__INCLUDED_

#include "DataPointFromStream.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include"DataPointFromStream.h"
#define SliceSize 0x10000L
#include <iterator>
using namespace std;
	
namespace PackageCQ
{	
	
	class BlockSource  
	{
	public:
		unsigned char GetCompressMethod();
		BlockSource(int ID, int DataLength,int UnCompressLength,char CompressMethod, DataPointFromStream &Point);
		void CloneBlockSourceToMe(BlockSource& Source);
		BlockSource(const BlockSource& Source);
		BlockSource(DataPointFromStream & Point);
		BlockSource();
		iostream& GetBlockStream() ;
		int GetLength() const;
		int GetID() const;
		virtual ~BlockSource();
		bool operator ==(const BlockSource &Source) const;
		bool operator > (const BlockSource &Source) const;
		bool operator < (const BlockSource &Source) const;
		const BlockSource& operator <<(iostream &DataStream);
		const BlockSource& operator = (BlockSource &Source);
		
		const BlockSource& operator >>(iostream &DataStream);
		
	private :
		DataPointFromStream  m_BlockPoint;
		unsigned long  m_BlockID;
		long  m_BlockLength;
		long  m_UnCompressBlockLength;
		unsigned char  m_CompressMethod;
		
	};
	
}
#endif // !defined(AFX_BLOCKSOURCE_H__701243D3_7556_46B9_BB6C_9573FD6E142F__INCLUDED_)
