#pragma once
#include <Windows.h>

using namespace std;

class CThreadLock
{
private:
	CRITICAL_SECTION m_csLock;
public:
	inline CThreadLock()
	{
		InitializeCriticalSection(&m_csLock);
	}
	inline ~CThreadLock()
	{
		DeleteCriticalSection(&m_csLock);
	}
	inline void Lock()
	{
		EnterCriticalSection(&m_csLock);
	}
	inline void UnLock()
	{
		LeaveCriticalSection(&m_csLock);
	}
};

class CThreadLockHandle
{
private:
	CThreadLock& m_csLock;
public:
	CThreadLockHandle(CThreadLock& X) : m_csLock(X)
	{
		m_csLock.Lock();
	}
	~CThreadLockHandle()
	{
		m_csLock.UnLock();
	}
};

