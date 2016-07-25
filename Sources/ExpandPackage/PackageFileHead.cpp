// PackageFileHead.cpp: implementation of the PackageFileHead class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PackageFileHead.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace PackageCQ
{
	PackageFileHead::PackageFileHead()
		:m_FileTypeString()
	{
		
		memset((char *)&m_FileHeadInfo,0,sizeof(m_FileHeadInfo));
		
	}
	
	PackageFileHead::~PackageFileHead()
	{
		
	}
	
	const PackageFileHead& PackageFileHead::operator<< (iostream &PackageDataStream)
	{
		
		if(!PackageDataStream.eof())
		{
			PackageDataStream.read((char *)(&m_FileHeadInfo),sizeof(m_FileHeadInfo));
		}
		else
		{
			throw exception("File is end when Read File Head ");
		}
		if(PackageDataStream.bad() )
		{
			throw exception("read error when Read File Head");
		}
		
		char FileType[FileIDLength + 1];
		memcpy(FileType,m_FileHeadInfo.signature,FileIDLength);
		FileType[FileIDLength  ] = 0;
		
		m_FileTypeString = FileType;
		
		
		return * this;
		
	}
	
	const string& PackageFileHead::GetFileType() const
	{
		return m_FileTypeString;
		
	}
	
	unsigned long  PackageFileHead::GetBlockCount() const
	{
		return m_FileHeadInfo.count ;
		
	}
	
	PackageFileHead& PackageFileHead::operator >>(iostream &PackageDataStream)
	{
		PackageDataStream.write((char *)&m_FileHeadInfo,sizeof(m_FileHeadInfo));
		
		
		if(PackageDataStream.bad())
		{
			throw("write File Head Error");
		}
		
		
		
		
		return  * this;
		
		
		
		
		
	}
	
	PackageFileHead::PackageFileHead(unsigned long BlockCount, unsigned long IndexOffset, const string &FileTypeString,unsigned long DataOffset,unsigned long CRC32)
	{
		
		
		m_FileHeadInfo.count        = BlockCount;
		
		m_FileTypeString            = FileTypeString;
		
		
		memcpy(m_FileHeadInfo.signature,m_FileTypeString.c_str(),FileIDLength);
		
		
		m_FileHeadInfo.index_offset = IndexOffset;
		
		m_FileHeadInfo.data_offset  = DataOffset;
		
		m_FileHeadInfo.crc32        = CRC32;

		memset(m_FileHeadInfo.reserved,0,ReservedLength); 
		
		
		
		
		
		
	}
	
	
	unsigned long PackageFileHead::GetIndexOffset()
	{
		return m_FileHeadInfo.index_offset ;
		
		
	}
	
	unsigned long PackageFileHead::GetDataOffset()
	{
		return m_FileHeadInfo.data_offset;
		
	}
}