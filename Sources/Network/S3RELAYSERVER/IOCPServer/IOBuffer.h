/********************************************************************
	created:	2003/02/14
	file base:	IOBuffer
	file ext:	h
	author:		liupeng
	
	purpose:	Header file for CIOBuffer class
*********************************************************************/
#ifndef __INCLUDE_IOBUFFER_H__
#define __INCLUDE_IOBUFFER_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include <winsock2.h>

#include "CriticalSection.h" 
#include "tstring.h"

#include "NodeList.h"
#include "OpaqueUserData.h"

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
 * CIOBuffer
 */
class CIOBuffer : public OVERLAPPED, public CNodeList::Node, public COpaqueUserData
{
public :

	class Allocator;
	friend class Allocator;

	WSABUF *GetWSABUF() const;

	size_t GetUsed() const;
	size_t GetSize() const;

	const BYTE *GetBuffer() const;

	void SetupRead();
	void SetupWrite();

	void AddData( const char * const pData, size_t dataLength );
	void AddData( const BYTE * const pData, size_t dataLength );
	void AddData( BYTE data );

	void Use( size_t dataUsed );

	CIOBuffer *SplitBuffer( size_t bytesToRemove );

	CIOBuffer *AllocateNewBuffer() const;

	void ConsumeAndRemove( size_t bytesToRemove );

	void Empty();

	void AddRef();
	void Release();

	size_t GetOperation() const;
	void SetOperation( size_t operation );

private :
	
	/*
	 * Data member in the class
	 */
	size_t m_operation;
	
	WSABUF m_wsabuf;
	
	Allocator &m_allocator;
	
	long			m_ref;
	const size_t	m_size;
	size_t			m_used;

	/*
	 * Start of the actual buffer, must remain the last
	 */
	BYTE			m_buffer[0];
	
private :
	
	static void *operator new( size_t objSize, size_t bufferSize );
	static void operator delete( void *pObj, size_t bufferSize );
	
	CIOBuffer( Allocator &m_allocator, size_t size );
	
	/*
	 * No copies do not implement
	 */
	CIOBuffer( const CIOBuffer &rhs );
	CIOBuffer &operator=( const CIOBuffer &rhs );
};

/*
 * CIOBuffer::Allocator
 */
class CIOBuffer::Allocator
{
public :
	
	friend class CIOBuffer;
	
	explicit Allocator( size_t bufferSize, size_t maxFreeBuffers );	
	virtual ~Allocator();
	
	CIOBuffer *Allocate();
	
protected :
	
	void Flush();
	
private :
	
	void Release( CIOBuffer *pBuffer );
	
	virtual void OnBufferCreated() {}
	virtual void OnBufferAllocated() {}
	virtual void OnBufferReleased() {}
	virtual void OnBufferDestroyed() {}
	
	void DestroyBuffer( CIOBuffer *pBuffer );
	
	const size_t m_bufferSize;
	
	typedef TNodeList<CIOBuffer> BufferList;
	
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

} // End of namespace OnlineGameLib
} // End of namespace Win32


#endif //__INCLUDE_IOBUFFER_H__