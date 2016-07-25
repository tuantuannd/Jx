/********************************************************************
	created:	2003/04/26
	file base:	Buffer
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_ONLINEGAMELIB_WIN32_BUFFER_H__
#define __INCLUDE_ONLINEGAMELIB_WIN32_BUFFER_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif

/*
 * Identifier was truncated to '255' characters 
 * in the debug information
 */
#pragma warning(disable : 4786)

#include "CriticalSection.h"

#include "NodeList.h"
#include "OpaqueUserData.h"

#include "Utils.h"
#include "Macro.h"

#include <map>

/*
 * Nonstandard extension used : zero-sized array in struct/union
 */
#pragma warning(disable: 4200)

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

/*
 * CBuffer class
 */
class CBuffer : public CNodeList::Node, public COpaqueUserData
{
public:
	
	class Allocator;

	friend class Allocator;

	size_t GetUsed() const { return m_used; };
	size_t GetSize() const { return m_size; };

	const BYTE *GetBuffer() const { return m_buffer_ptr; };

	CBuffer *GetHeadPack( size_t packLength );
	CBuffer *GetNextPack();
	bool	 HaveNextPack() { return ( m_offsetNextPack < m_used ) ? true : false; };
	
	void AddData( const char * const pData, size_t dataLength );
	void AddData( const BYTE * const pData, size_t dataLength );
	void AddData( BYTE data );

	void Use( size_t dataUsed ) { m_used += dataUsed; };

	CBuffer *SplitBuffer( size_t bytesToRemove );

	CBuffer *AllocateNewBuffer() const;

	void ConsumeAndRemove( size_t bytesToRemove );

	void Empty() { m_used = 0; };

	void AddRef() { ::InterlockedIncrement( &m_ref ); };
	void Release();

private:

	Allocator &m_allocator;

	long			m_ref;

	const size_t	m_size;
	size_t			m_used;

	size_t			m_packLength;
	size_t			m_offsetNextPack;

	/*
	 * start of the actual buffer, must remain the last
	 * data member in the class.
	 */
	BYTE *m_buffer_ptr;			// four bytes aligned
	
	//BYTE m_buffer_base_addr[0];

private:

	static void *operator new( size_t objSize, size_t bufferSize );
	static void operator delete( void *pObj, size_t bufferSize );
      
	CBuffer( Allocator &allocator, size_t size );
	~CBuffer();

	/*
	 * No copies do not implement
	 */
	CBuffer( const CBuffer &rhs );
	CBuffer &operator=( const CBuffer &rhs );
	
};

/*
 * CBuffer::Allocator class
 */
class CBuffer::Allocator
{
public:

	friend class CBuffer;

	explicit Allocator( size_t bufferSize, size_t maxFreeBuffers );

	virtual ~Allocator();

	CBuffer *Allocate();

	size_t GetBufferSize() const { return m_bufferSize; };
	
	bool ReSet( size_t bufferSize, size_t maxFreeBuffers );

protected:

	void Flush();

private:
	
	void Release( CBuffer *pBuffer );
	
	virtual void OnBufferCreated() {}
	virtual void OnBufferAllocated() {}
	virtual void OnBufferReleased() {}

	void DestroyBuffer( CBuffer *pBuffer );

	const size_t m_bufferSize;

	typedef TNodeList< CBuffer > BufferList;

	BufferList m_freeList;
	BufferList m_activeList;

	const size_t m_maxFreeBuffers;

	CCriticalSection m_criticalSection;

    /*
	 * No copies do not implement
	 */
	Allocator( const Allocator &rhs );
	Allocator &operator=( const Allocator &rhs );

};

/*
 * class CPackager
 */
class CPackager
{
public:

	CPackager( size_t bufferSize = 65536 /* 1024*64 */, size_t maxFreeBuffers = 16 );
	virtual ~CPackager();

	/*
	 * Send
	 */
	CBuffer *GetHeadPack( BYTE cID, size_t packLength = 512 );
	CBuffer *GetNextPack( BYTE cID );
	
	void AddData( BYTE cID, const char * const pData, size_t dataLength, unsigned long lnUserData = 0 );
	void AddData( BYTE cID, const BYTE * const pData, size_t dataLength, unsigned long lnUserData = 0 );

	void DelData( BYTE cID );

	/*
	 * Recv
	 */
	CBuffer *PackUp( const void *pData, size_t dataLength );

	/*
	 * Common
	 */
	bool ReSet( size_t bufferSize, size_t maxFreeBuffers ) { return m_theAllocator.ReSet( bufferSize, maxFreeBuffers ); }

	void Empty();

	static BYTE Peek( const void *pData, size_t index = 0 ) { 
		if ( !pData )
		{ 
			return 0; 
		} 
		return *( ( const BYTE * )pData + index ); 
	}
	
private:

	CCriticalSection	m_csSend;
	CCriticalSection	m_csRecv;

	CBuffer::Allocator	m_theAllocator;

	typedef std::map< BYTE, CBuffer * >	BUFFER_MAP;

	BUFFER_MAP			m_theSend;
	BUFFER_MAP			m_theRecv;

	/*
	 * +-----+-----+-----+-----+
	 * | 8 7 | 6 5 | 4 3 | 2 1 |  <==> enumPackFlag, sizeof( BYTE )
	 * +-----+-----+-----+-----+
	 *    A     B     C     D
	*/
	enum enumPackFlag
	{
		enumPackHeader	= 0xc0,	// A segment
		enumPackMiddle	= 0x30, // B segment
		enumPackTail	= 0xc,	// C segment
		enumPackErr		= 0x3	// D segment
	};

	bool _Header( BYTE cFlag )	{ return ToBool<int>( cFlag & enumPackHeader ); }
	bool _Middle( BYTE cFlag )	{ return ToBool<int>( cFlag & enumPackMiddle ); }
	bool _Tail( BYTE cFlag )	{ return ToBool<int>( cFlag & enumPackTail ); }
	bool _Error( BYTE cFlag )	{ return ToBool<int>( cFlag & enumPackErr ); }

};

inline void CBuffer::ConsumeAndRemove( size_t bytesToRemove )
{
	m_used -= bytesToRemove;

	memmove( m_buffer_ptr, m_buffer_ptr + bytesToRemove, m_used );
}


inline bool CBuffer::Allocator::ReSet( size_t bufferSize, size_t maxFreeBuffers )
{
	memcpy( const_cast< size_t * >( &m_bufferSize ), &bufferSize, sizeof( size_t ) );
	memcpy( const_cast< size_t * >( &m_maxFreeBuffers ), &maxFreeBuffers, sizeof( size_t ) );

	return true;
}

inline void CBuffer::Allocator::DestroyBuffer( CBuffer *pBuffer )
{
	SAFE_DELETE( pBuffer );
}
	
} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif //__INCLUDE_ONLINEGAMELIB_WIN32_BUFFER_H__