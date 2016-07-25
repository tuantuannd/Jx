/********************************************************************
	created:	2003/02/14
	file base:	OpaqueUserData
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_OPAQUEUSERDATA_H__
#define __INCLUDE_OPAQUEUSERDATA_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

/*
 * Define Win64 interfaces if not already defined
 */

/*
 * InterlockedExchangePointer
 */
#ifndef InterlockedExchangePointer
	#define InterlockedExchangePointer(Target, Value) \
		(PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))
#endif 

/*
 * namespace OnlineGameLib
 */

namespace OnlineGameLib {

class COpaqueUserData 
{
public:
	
	/*
	 * Attempted to take the address of a non-lvalue
	 */
	void *GetUserPtr() const
	{
		return InterlockedExchangePointer( &(const_cast<void *>(m_pUserData)), m_pUserData );
	}
	
	/*
	 * Ignoring return value of function 
	 * Expected void type, assignment, increment or decrement
	 */
	void SetUserPtr(void *pData)
	{
		InterlockedExchangePointer( &m_pUserData, pData );
	}
	
	unsigned long GetUserData() const
	{
		return reinterpret_cast<unsigned long>( GetUserPtr() );
	}
	
	void SetUserData( unsigned long data )
	{
		SetUserPtr( reinterpret_cast<void *>( data ) );
	}
	
protected:
	
	COpaqueUserData() : m_pUserData(0)
	{
	}
	
	~COpaqueUserData()
	{
		m_pUserData = 0;
	}
	
private:
	
	void *m_pUserData;
	
	/*
	 * No copies do not implement
	 */
	COpaqueUserData( const COpaqueUserData &rhs );
	COpaqueUserData &operator=( const COpaqueUserData &rhs );
};

} // End of namespace OnlineGameLib

#endif //__INCLUDE_OPAQUEUSERDATA_H__