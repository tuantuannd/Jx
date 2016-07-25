// LockMRSW.h: interface for the CLockMRSW & CLockMRSW2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCKMRSW_H__F65AF80D_9748_4100_B0AC_823695855E5D__INCLUDED_)
#define AFX_LOCKMRSW_H__F65AF80D_9748_4100_B0AC_823695855E5D__INCLUDED_


//lock: multi read & single write
//reading is parallel, and writing is exclusive

class CLockMRSW 
{
public:
	CLockMRSW();
	virtual ~CLockMRSW();

public:
	void EnterRead();
	void LeaveRead();

	void EnterWrite();
	void LeaveWrite();

private:
	CRITICAL_SECTION	m_csExclusive;
	CRITICAL_SECTION	m_csModi;
	HANDLE				m_eventIdle;
	LONG				m_lUsing;
};




//assistant

struct KAutoLockRead
{
	CLockMRSW& theLock;
	KAutoLockRead(CLockMRSW& locksrc) : theLock(locksrc) {theLock.EnterRead();}
	~KAutoLockRead() {theLock.LeaveRead();}
};
struct KAutoLockWrite
{
	CLockMRSW& theLock;
	KAutoLockWrite(CLockMRSW& locksrc) : theLock(locksrc) {theLock.EnterWrite();}
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




#endif // !defined(AFX_LOCKMRSW_H__F65AF80D_9748_4100_B0AC_823695855E5D__INCLUDED_)
