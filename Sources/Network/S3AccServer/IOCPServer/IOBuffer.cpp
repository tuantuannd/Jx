#include "IOBuffer.h"

#include "Exception.h"
#include "Utils.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CIOBuffer::CIOBuffer( Allocator &allocator, size_t size )
   : m_operation(0),
     m_allocator(allocator),
     m_ref(1),
     m_size(size), 
     m_used(0)
{
	memset( this, 0, sizeof( OVERLAPPED ) );
	
	Empty();
}

WSABUF *CIOBuffer::GetWSABUF() const
{
	return const_cast<WSABUF *>( &m_wsabuf );
}

size_t CIOBuffer::GetUsed() const
{
	return m_used;
}

size_t CIOBuffer::GetSize() const
{
	return m_size;
}

const BYTE *CIOBuffer::GetBuffer() const
{
   return &m_buffer[0];
}

void CIOBuffer::Empty()
{
	m_wsabuf.buf = reinterpret_cast<char *>( m_buffer );
	m_wsabuf.len = m_size;

	m_used = 0;
}

void *CIOBuffer::operator new( size_t objectSize, size_t bufferSize )
{
	void *pMem = new char[objectSize + bufferSize];

	return pMem;
}

void CIOBuffer::operator delete( void *pObject, size_t /* bufferSize*/ )
{
	delete [] pObject;
}  

void CIOBuffer::Use( size_t dataUsed )
{
	m_used += dataUsed;
}

CIOBuffer *CIOBuffer::SplitBuffer( size_t bytesToRemove )
{
	CIOBuffer *pNewBuffer = m_allocator.Allocate();
	
	pNewBuffer->AddData( m_buffer, bytesToRemove );
	
	m_used -= bytesToRemove;
	
	memmove( m_buffer, m_buffer + bytesToRemove, m_used );
	
	return pNewBuffer;
}

CIOBuffer *CIOBuffer::AllocateNewBuffer() const
{
	return m_allocator.Allocate();
}

void CIOBuffer::ConsumeAndRemove( size_t bytesToRemove )
{
	m_used -= bytesToRemove;
	
	memmove( m_buffer, m_buffer + bytesToRemove, m_used );
}

void CIOBuffer::SetupRead()
{
	if ( 0 == m_used )
	{
		m_wsabuf.buf = reinterpret_cast<char *>( m_buffer );
		m_wsabuf.len = m_size;
	}
	else
	{
		m_wsabuf.buf = reinterpret_cast<char *>( m_buffer ) + m_used;
		m_wsabuf.len = m_size - m_used; 
	}
}

void CIOBuffer::SetupWrite()
{
	m_wsabuf.buf = reinterpret_cast<char *>( m_buffer );
	m_wsabuf.len = m_used;
	
	m_used = 0;
}

void CIOBuffer::AddData( const char * const pData, size_t dataLength )
{
	if ( dataLength > m_size - m_used )
	{
		throw CException( _T("CIOBuffer::AddData"), _T("Not enough space in buffer") );
	}
	
	memcpy( m_buffer + m_used, pData, dataLength );
	
	m_used += dataLength;
}

void CIOBuffer::AddData( const BYTE * const pData, size_t dataLength )
{
	AddData( reinterpret_cast<const char*>( pData ), dataLength );
}

void CIOBuffer::AddData( BYTE data )
{
	AddData( &data, 1 );
}

void CIOBuffer::AddRef()
{
	::InterlockedIncrement( &m_ref );
}

void CIOBuffer::Release()
{
	if ( 0 == ::InterlockedDecrement( &m_ref ) )
	{
		m_allocator.Release( this );
	}
}

size_t CIOBuffer::GetOperation() const
{
	return m_operation;
}
      
void CIOBuffer::SetOperation( size_t operation )
{
	m_operation = operation;
}

/*
 * CIOBuffer::Allocator
 */

CIOBuffer::Allocator::Allocator( size_t bufferSize, size_t maxFreeBuffers )
   : m_bufferSize(bufferSize), m_maxFreeBuffers(maxFreeBuffers)
{
   /*
    * TODO : share this code with the socket pool
	*/
}

CIOBuffer::Allocator::~Allocator()
{
	try
	{
		Flush();
	}
	catch(...)
	{
	}
}

CIOBuffer *CIOBuffer::Allocator::Allocate()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	CIOBuffer *pBuffer = 0;
	
	if ( !m_freeList.Empty() )
	{
		pBuffer = m_freeList.PopNode();
		
		pBuffer->AddRef();
	}
	else
	{
		pBuffer = new( m_bufferSize )CIOBuffer( *this, m_bufferSize );
		
		if ( !pBuffer )
		{
			throw CException( _T("CIOBuffer::Allocator::Allocate()"),_T("Out of memory") );
		}
		
		/*
		 * Call to unqualified virtual function
		 */
		OnBufferCreated();
	}
	
	m_activeList.PushNode( pBuffer );
	
	/*
	 * call to unqualified virtual function
	 */
	OnBufferAllocated();
	
	return pBuffer;
}

void CIOBuffer::Allocator::Release( CIOBuffer *pBuffer )
{
	if ( !pBuffer )
	{
		throw CException( _T("CIOBuffer::Allocator::Release()"), _T("pBuffer is null") );
	}
	
	CCriticalSection::Owner lock( m_criticalSection );
	
	/*
	 * Call to unqualified virtual function
	 */
	OnBufferReleased();
	
	/*
	 * unlink from the in use list
	 */
	
	pBuffer->RemoveFromList();
	
	if ( m_maxFreeBuffers == 0 || m_freeList.Count() < m_maxFreeBuffers )
	{
		pBuffer->Empty();           
		
		/*
		 * Add to the free list
		 */
		
		m_freeList.PushNode( pBuffer );
	}
	else
	{
		DestroyBuffer( pBuffer );
	}
}

void CIOBuffer::Allocator::Flush()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	while ( !m_activeList.Empty() )
	{
		/*
		 * Call to unqualified virtual function
		 */
		OnBufferReleased();
		
		DestroyBuffer( m_activeList.PopNode() );
	}
	
	while ( !m_freeList.Empty() )
	{
		DestroyBuffer( m_freeList.PopNode() );
	}
}

void CIOBuffer::Allocator::DestroyBuffer( CIOBuffer *pBuffer )
{
	delete pBuffer;
	
	/*
	 * Call to unqualified virtual function
	 */
	OnBufferDestroyed();
}

} // End of namespace OnlineGameLib
} // End of namespace Win32