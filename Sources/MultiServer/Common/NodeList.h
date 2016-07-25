/********************************************************************
	created:	2003/02/14
	file base:	NodeList
	file ext:	h
	author:		liupeng
	
	purpose:	Data struct and define
*********************************************************************/
#ifndef __INCLUDE_NODELIST_H__
#define __INCLUDE_NODELIST_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include <wtypes.h>

/*
 * namespace OnlineGameLib
 */

namespace OnlineGameLib {

/*
 * CNodeList
 */
class CNodeList
{
public:

	class Node
    {
	public:

		Node *Next() const { return m_pNext; };

		void Next( Node *pNext );

		void AddToList( CNodeList *pList );

		void RemoveFromList();

	protected:

		Node();
		~Node();

	private:

		friend class CNodeList;

		void Unlink();

		Node *m_pNext;
		Node *m_pPrev;

		CNodeList *m_pList;
	};

	CNodeList();

	void PushNode( Node *pNode );

	Node *PopNode();

	Node *Head() const { return m_pHead; };

	size_t Count() const { return m_numNodes; };

	bool Empty() const { return ( 0 == m_numNodes ); };

private:

	friend void Node::RemoveFromList();

	void RemoveNode( Node *pNode );

	Node *m_pHead; 

	size_t m_numNodes;
};

inline void CNodeList::Node::Next( Node *pNext )
{
	m_pNext = pNext;

	if ( pNext )
	{
		pNext->m_pPrev = this;
	}
}

/*
 * TNodeList
 */

template <class T> class TNodeList : public CNodeList
{
public:
         
	T *PopNode();
   
	T *Head() const;

	static T *Next( const T *pNode );
};

template <class T>
T *TNodeList<T>::PopNode()
{
	return static_cast< T* >( CNodeList::PopNode() );
}

template <class T>
T *TNodeList<T>::Head() const
{
	return static_cast< T* >( CNodeList::Head() );
}

template <class T>
T *TNodeList<T>::Next( const T *pNode )
{
	return static_cast< T* >( pNode->Next() );
}

} // End of namespace OnlineGameLib	

#endif // __INCLUDE_NODELIST_H__