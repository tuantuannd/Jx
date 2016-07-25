/********************************************************************
	created:	2003/04/08
	file base:	ICommand
	file ext:	h
	author:		liupeng
	
	purpose:	
*********************************************************************/
#ifndef __INCLUDE_ICOMMAND_H__
#define __INCLUDE_ICOMMAND_H__

#include "Macro.h"

/*
 * Interface class for execute command
 */
class ICommand
{
public:

	virtual bool Execute() = 0;
	virtual unsigned long GetFaildValue() = 0;

	virtual ~ICommand(){};
};

/*
 * template classs
 */
template <class Receive>
class CTaskCommand : public ICommand
{
public:

	typedef bool ( Receive::* Action )();
	
	explicit CTaskCommand( Receive *pReceiver, Action pfunAction, unsigned long dwFaildValue );
	virtual ~CTaskCommand();

	virtual bool Execute();
	virtual unsigned long GetFaildValue() { return m_dwFaildValue; };

protected:

	CTaskCommand();

	/*
	 * This function pointer is used to execute command
	 */
	Action	 m_pfunAction;

	/*
	 * The receiver class
	 */
	Receive	*m_pReceiver;

	/*
	 * Store a faild value for return it to user when this class was found error
	 */
	unsigned long m_dwFaildValue;
};

template <class Receive>
CTaskCommand<Receive>::CTaskCommand( Receive *pReceiver, Action pfunAction, unsigned long dwFaildValue )
			: m_pReceiver( pReceiver ), 
			m_pfunAction( pfunAction ),
			m_dwFaildValue( dwFaildValue )
{
}

template <class Receive>
CTaskCommand<Receive>::CTaskCommand()
{
	ASSERT( FALSE );
}

template <class Receive>
CTaskCommand<Receive>::~CTaskCommand()
{
}

template <class Receive>
bool CTaskCommand<Receive>::Execute()
{
	return ( m_pReceiver->*m_pfunAction )();
}

#endif // __INCLUDE_ICOMMAND_H__