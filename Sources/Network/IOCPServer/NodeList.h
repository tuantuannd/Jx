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

	/*
	 * Node
	 */
	class Node
	{
	public:
		
		Node *Next() const;		
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

	Node *Head() const;
	size_t Count() const;
	bool Empty() const;

private :
	
	friend void Node::RemoveFromList();	
	void RemoveNode( Node *pNode );
	
	Node *m_pHead;	
	size_t m_numNodes;
};

/*
 * TNodeList {template class}
 */
template <class T>
class TNodeList : public CNodeList
{
public:
	
	void PushNode( T *pNode );
	T *PopNode();
	
	T *Head() const;	
	static T *Next( const T *pNode );
};

template <class T>
void TNodeList<T>::PushNode( T *pNode )
{
	CNodeList::PushNode( pNode );
}

template <class T>
T *TNodeList<T>::PopNode()
{
	return static_cast<T *>( CNodeList::PopNode() );
}

template <class T>
T *TNodeList<T>::Head() const
{
	return static_cast<T *>( CNodeList::Head() );
}

template <class T>
T *TNodeList<T>::Next( const T *pNode )
{
	return static_cast<T *>( pNode->Next() );
}

} // End of namespace OnlineGameLib	

#endif