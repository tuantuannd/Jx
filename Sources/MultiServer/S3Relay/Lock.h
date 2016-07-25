// Lock.h: interface for the CLockXXXX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCK_H__F65AF80D_9748_4100_B0AC_823695855E5D__INCLUDED_)
#define AFX_LOCK_H__F65AF80D_9748_4100_B0AC_823695855E5D__INCLUDED_


class ALock
{
protected:
	ALock() {}
	virtual ~ALock() {}

public:
	virtual void EnterRead() = 0;
	virtual void LeaveRead() = 0;
	virtual void EnterWrite() = 0;
	virtual void LeaveWrite() = 0;
};



//lock: single read & single write (critical section)
//reading and writing is exclusive

class CLockSRSW : public ALock
{
public:
	CLockSRSW();
	~CLockSRSW();

public:
	virtual void EnterRead();
	virtual void LeaveRead();
	virtual void EnterWrite();
	virtual void LeaveWrite();

private:
	CRITICAL_SECTION	m_csExclusive;
};


//lock: multi read & single write
//reading is parallel, and writing is exclusive

class CLockMRSW : public ALock
{
public:
	CLockMRSW();
	~CLockMRSW();

public:
	virtual void EnterRead();
	virtual void LeaveRead();
	virtual void EnterWrite();
	virtual void LeaveWrite();

private:
	CRITICAL_SECTION	m_csExclusive;
	CRITICAL_SECTION	m_csModi;
	HANDLE				m_eventIdle;
	LONG				m_lUsing;
};




//assistant

struct KAutoLockRead
{
	ALock& theLock;
	KAutoLockRead(ALock& locksrc) : theLock(locksrc) {theLock.EnterRead();}
	~KAutoLockRead() {theLock.LeaveRead();}
};
struct KAutoLockWrite
{
	ALock& theLock;
	KAutoLockWrite(ALock& locksrc) : theLock(locksrc) {theLock.EnterWrite();}
	~KAutoLockWrite() {theLock.LeaveWrite();}
};



//for more easy

#define AUTOLOCKREAD(lock)	\
	KAutoLockRead __local_autolockread_(lock);
#define AUTOLOCKREAD_(name, lock)	\
	KAutoLockRead name(lock);

#define AUTOLOCKWRITE(lock)	\
	KAutoLockWrite __local_autolockwrite_(lock);
#define AUTOLOCKWRITE_(name, lock)	\
	KAutoLockWrite name(lock);




#endif // !defined(AFX_LOCK_H__F65AF80D_9748_4100_B0AC_823695855E5D__INCLUDED_)
