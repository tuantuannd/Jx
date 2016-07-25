// PackageFileHead.h: interface for the PackageFileHead class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PACKAGEFILEHEAD_H__1E5A6AAF_1702_42F3_8336_D3C12F628B4C__INCLUDED_)
#define AFX_PACKAGEFILEHEAD_H__1E5A6AAF_1702_42F3_8336_D3C12F628B4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include<string>
#include<iostream>
using namespace std;
namespace PackageCQ
{
	
#define FileID         "PACK"
#define FileIDLength   4 
#define ReservedLength 12

	struct pack_header
	{
		unsigned char signature[4];			//"PACK"
		unsigned long count;				//数据的条目数
		unsigned long index_offset;			//索引的偏移量
		unsigned long data_offset;			//数据的偏移量
		unsigned long crc32;
		unsigned char reserved[12];
		
	};
	
	class PackageFileHead  
	{
	public:
		unsigned long GetDataOffset();
		unsigned long GetIndexOffset();
		PackageFileHead(unsigned long BlockCount, unsigned long IndexOffset, const string &FileTypeString,unsigned long DataOffset,unsigned long CRC32);
		unsigned long GetBlockCount() const;
		const string& GetFileType() const;
		PackageFileHead();
		virtual ~PackageFileHead();
		const PackageFileHead& operator<< (iostream &PackageDataStream);
		PackageFileHead& operator >>(iostream &PackageDataStream);
	private:
		pack_header m_FileHeadInfo;
		string      m_FileTypeString;

		
		
		
		
	};
}

#endif // !defined(AFX_PACKAGEFILEHEAD_H__1E5A6AAF_1702_42F3_8336_D3C12F628B4C__INCLUDED_)
