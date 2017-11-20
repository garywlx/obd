#ifndef _THREAD_H
#define _THREAD_H

#include <unistd.h>
#include "jthread.h"
#include "TypeDefine.h"

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

#ifndef WIN32
#define EXCEPTION_EXECUTE_HANDLER 1
#endif
class CThread : public JThread
{
public:
	CThread()
	{
		m_bCanceled = false;
		m_szThreadName[0] = 0;
	}

#define MS_VC_EXCEPTION 0x406D1388


	void SetThreadName( LPCTSTR threadName,DWORD dwThreadID)
	{
		Sleep(10);
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;

// 		__try
// 		{
// 			RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
// 		}
// 		__except(EXCEPTION_EXECUTE_HANDLER)
// 		{
// 		}
	}

	virtual bool IsThreadCanceled() const
	{
		return m_bCanceled;
	}
	virtual void CancelThread()
	{
		m_bCanceled = true;
	}
	virtual void *Thread()
	{
		ThreadStarted();

		if (strlen(m_szThreadName) > 0 )
		{
			SetThreadName(m_szThreadName,getpid());
		}

		return (void*)ThreadFunc();
	}
	int Start(LPCTSTR lpName = NULL)
	{
		printf("CThread Start");
		if (lpName != NULL)
		{
			strcpy(m_szThreadName,lpName);
		}
		return JThread::Start();
	}
	void Stop(int nWaitSeconds = 10)
	{
		m_bCanceled = true;

		for ( int i = 0 ; i < nWaitSeconds * 1000/100; ++i)
		{
			if (IsRunning())
			{
				Sleep(100);
			}
			else
			{
				break;
			}
		}
		Kill();
	}


	virtual UINT ThreadFunc() = 0;

protected:
	bool  m_bCanceled;
	TCHAR m_szThreadName[128];
};

unsigned int _beginthread (void (* _StartAddress) (void *),unsigned int _StackSize, void * _ArgList);


#endif
