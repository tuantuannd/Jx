#include "stdafx.h"
#include "IOBuffer.h"

#include "Exception.h"
#include "Utils.h"

/*
 * namespace OnlineGameLib::Win32
 */

namespace OnlineGameLib {
namespace Win32 {

CIOBuffer::CIOBuffer( Allocator &allocator, size_t size )
	:  m_operation( 0 )
      , m_sequenceNumber( 0 )
      , m_allocator( allocator )
      , m_ref( 1 )
      , m_size( size )
      , m_used( 0 )
{
	//( ( BYTE * )( ( DWORD )( pMemory + 3 ) & ( ~3 ) ) )
	m_buffer_ptr = new BYTE[ size ];

	memset( this, 0, sizeof( OVERLAPPED ) );

	Empty();
}

CIOBuffer::~CIOBuffer()
{
	SAFE_DELETE_ARRAY( m_buffer_ptr );
}

void CIOBuffer::Empty()
{
	m_wsabuf.buf = reinterpret_cast< char * >( m_buffer_ptr );
	m_wsabuf.len = m_size;

	m_used = 0;
}

void *CIOBuffer::operator new( size_t objectSize, size_t /* bufferSize */ )
{
	/*
	 * ASSERT( sizeof( DWORD ) == 4 );
	 *
	 * For four bytes aligned base on win32 system
	 */

	void *pMem = new char[ objectSize ]; // + bufferSize + 4 ];

	return pMem;
}

void CIOBuffer::operator delete( void *pObject, size_t /* bufferSize*/ )
{
	SAFE_DELETE_ARRAY( pObject );
}

CIOBuffer *CIOBuffer::SplitBuffer( size_t bytesToRemove )
{
	CIOBuffer *pNewBuffer = m_allocator.Allocate();

	pNewBuffer->AddData( m_buffer_ptr, bytesToRemove );

	m_used -= bytesToRemove;

	memmove( m_buffer_ptr, m_buffer_ptr + bytesToRemove, m_used );

	return pNewBuffer;
}

VOID CIOBuffer::RemoveBuffer( size_t bytesToRemove )
{
	if ( m_used < bytesToRemove )
		return;

	m_used -= bytesToRemove;

	memmove( m_buffer_ptr, m_buffer_ptr + bytesToRemove, m_used );
}

CIOBuffer *CIOBuffer::AllocateNewBuffer() const
{
	return m_allocator.Allocate();
}

void CIOBuffer::AddData( const char * const pData, size_t dataLength )
{
	if (dataLength > m_size - m_used)
	{
		DEBUG_ONLY( Message( "CIOBuffer::AddData - Not enough space in buffer!" ) );
		
		throw CException( _T("CIOBuffer::AddData"), _T("Not enough space in buffer") );
	}

	memcpy( m_buffer_ptr + m_used, pData, dataLength );

	m_used += dataLength;
}

void CIOBuffer::AddData( const BYTE * const pData, size_t dataLength )
{
	AddData( reinterpret_cast< const char * >( pData ), dataLength );
}

void CIOBuffer::AddData( BYTE data )
{
	AddData( &data, 1 );
}

void CIOBuffer::Release()
{
	if (m_ref == 0)
	{
		/*
		 * Error! double release
		 */
		throw CException( _T("CIOBuffer::Release()"), _T("m_ref is already zero") );
	}

	if ( 0 == ::InterlockedDecrement( &m_ref ) )
	{
		m_sequenceNumber = 0;
		m_operation = 0;
		m_used = 0;

		m_allocator.Release( this );
	}
}

/*
 * CIOBuffer::Allocator
 */

CIOBuffer::Allocator::Allocator( size_t bufferSize, size_t maxFreeBuffers )
   :  m_bufferSize( bufferSize ),
      m_maxFreeBuffers( maxFreeBuffers )
{

}

CIOBuffer::Allocator::~Allocator()
{
	try
	{
		Flush();
	}
	catch(...)
	{
		TRACE( "CIOBuffer::Allocator::~Allocator exception!" );
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
			throw CException( _T("CIOBuffer::Allocator::Allocate()"), _T("Out of memory") );
		}
		
		OnBufferCreated();
	}

	m_activeList.PushNode( pBuffer );
	
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
	
	OnBufferReleased();
	
	/*
	 * unlink from the in use list
	 */	
	pBuffer->RemoveFromList();
	
	if ( m_maxFreeBuffers == 0 || m_freeList.Count() < m_maxFreeBuffers )
	{
		pBuffer->Empty();           
		
		/*
		 * add to the free list
		 */
		
		m_freeList.PushNode( pBuffer );
	}
	else
	{
		DestroyBuffer( pBuffer );
	}
}

void CIOBuffer::Allocator::DestroyBuffer( CIOBuffer *pBuffer )
{
	SAFE_DELETE( pBuffer );
}

void CIOBuffer::Allocator::Flush()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	while ( !m_activeList.Empty() )
	{
		OnBufferReleased();
		
		DestroyBuffer( m_activeList.PopNode() );
	}
	
	while ( !m_freeList.Empty() )
	{
		DestroyBuffer( m_freeList.PopNode() );
	}
}

/*
 * CIOBuffer::InOrderBufferList
 */

CIOBuffer::InOrderBufferList::InOrderBufferList(
				CCriticalSection &criticalSection)
			:   m_next(0),
				m_criticalSection( criticalSection )
{
}

void CIOBuffer::InOrderBufferList::AddBuffer( CIOBuffer *pBuffer )
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	std::pair< BufferSequence::iterator, bool > result = 
		m_list.insert( BufferSequence::value_type( pBuffer->GetSequenceNumber(), pBuffer ) );
	
	if ( !result.second )
	{
		DEBUG_ONLY( Output( _T("UNEXPECTED! element already in map!") ) );
	}
}

CIOBuffer *CIOBuffer::InOrderBufferList::ProcessAndGetNext()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	m_next ++;
	
	CIOBuffer *pNext = 0;
	
	BufferSequence::iterator it;
	
	it = m_list.begin();
	
	if ( it != m_list.end() )
	{
		if ( it->first == m_next )
		{
			pNext = it->second;
			
			m_list.erase( it );
		}
		else
		{
			DEBUG_ONLY( Output( ToString( this ) +
				_T(" Got buffer : ") + 
				ToString( it->first ) +
				_T("Want buffer : ") +
				ToString( m_next ) ) );
		}
	}
	
	return pNext;
}

CIOBuffer *CIOBuffer::InOrderBufferList::GetNext()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	CIOBuffer *pNext = 0;
	
	BufferSequence::iterator it;
	
	it = m_list.begin();
	
	if ( it != m_list.end() )
	{
		if ( it->first == m_next )
		{
			pNext = it->second;
			
			m_list.erase(it);
		}
		else
		{
			DEBUG_ONLY( Output( ToString( this ) + 
				_T(" Got buffer  : ") + 
				ToString( it->first ) + 
				_T("Want buffer : ") + 
				ToString( m_next ) ) );
		}
	}
	
	return pNext;
}

CIOBuffer *CIOBuffer::InOrderBufferList::GetNext( CIOBuffer *pBuffer )
{
	CCriticalSection::Owner lock( m_criticalSection );
	
	if ( m_next == pBuffer->GetSequenceNumber() )
	{
		return pBuffer;
	}
	
	std::pair< BufferSequence::iterator, bool > result = 
		m_list.insert( BufferSequence::value_type( pBuffer->GetSequenceNumber(), pBuffer ) );
	
	if ( !result.second )
	{
		DEBUG_ONLY( Output( _T("UNEXPECTED! element already in map!") ) );
	}
	
	CIOBuffer *pNext = 0;
	
	BufferSequence::iterator it;
	
	it = m_list.begin();
	
	if (it != m_list.end()) 
	{
		if (it->first == m_next)
		{
			pNext = it->second;
			
			m_list.erase(it);
		}
		else
		{
			DEBUG_ONLY( Output( ToString( this ) +
				_T(" Got buffer  : ") + 
				ToString( it->first ) + 
				_T("Want buffer : ") + 
				ToString( m_next ) ) );
		}
	}
	
	return pNext;
}

void CIOBuffer::InOrderBufferList::ProcessBuffer()
{
	CCriticalSection::Owner lock( m_criticalSection );

	DEBUG_ONLY( Output( ToString( this ) +
		_T(" Processed : ") + 
		ToString( m_next ) ) );

	m_next ++;
}

void CIOBuffer::InOrderBufferList::Reset()
{
	m_next = 0;

	if ( !m_list.empty() )
	{
		DEBUG_ONLY( Output( _T("List not empty when reset !") ) );
	}
}

} // End of namespace Win32
} // End of namespace OnlineGameLib 