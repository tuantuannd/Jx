#include "stdafx.h"
#include "NodeList.h"
#include "Macro.h"

/*
 * namespace OnlineGameLib
 */

namespace OnlineGameLib {

/*
 * CNodeList
 */
CNodeList::CNodeList()
		: m_pHead( 0 ),
		m_numNodes( 0 ) 
{
}

void CNodeList::PushNode( Node *pNode )
{
	ASSERT( pNode );

	pNode->AddToList( this );

	pNode->Next( m_pHead );

	m_pHead = pNode;

	++ m_numNodes;
}

CNodeList::Node *CNodeList::PopNode()
{
	Node *pNode = m_pHead;

	if ( pNode )
	{
		RemoveNode( pNode );
	}

	return pNode;
}

void CNodeList::RemoveNode( Node *pNode )
{
	ASSERT( pNode );

	if ( pNode == m_pHead )
	{
		m_pHead = pNode->Next();
	}

	pNode->Unlink();

	-- m_numNodes;
}

/*
 * CNodeList::Node
 */

CNodeList::Node::Node() 
		: m_pNext( 0 ),
		m_pPrev( 0 ),
		m_pList( 0 ) 
{
}

CNodeList::Node::~Node() 
{
	try
	{
		RemoveFromList();   
	}
	catch( ... )
	{
		TRACE( "CNodeList::Node::~Node() exception!" );
	}

	m_pNext = 0;
	m_pPrev = 0;
	m_pList = 0;
}

void CNodeList::Node::AddToList( CNodeList *pList )
{
	m_pList = pList;
}

void CNodeList::Node::RemoveFromList()
{
	if ( m_pList )
	{
		m_pList->RemoveNode( this );
	}
}

void CNodeList::Node::Unlink()
{
	if ( m_pPrev )
	{
		m_pPrev->m_pNext = m_pNext;
	}

	if ( m_pNext )
	{
		m_pNext->m_pPrev = m_pPrev;
	}
   
	m_pNext = 0;
	m_pPrev = 0;

	m_pList = 0;
}

} // End of namespace OnlineGameLib