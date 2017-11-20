//
//  TypeDefine.h
//  VeiPhoneVideo
//
//  Created by wei laixi on 13-6-21.
//  Copyright (c) 2013年 garywlx. All rights reserved.
//

#ifndef VeiPhoneVideo_TypeDefine_h
#define VeiPhoneVideo_TypeDefine_h


#ifndef WIN32

extern int g_nSaveLog;
extern char g_pLogPath[255];

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "jthread.h"
#include "jmutexautolock.h"

# define  LOGI(...)  (void)0
# define  LOGE(...)  (void)0

//#define interface struct

#ifndef ASSERT_RET

#ifndef ASSERT
#define ASSERT assert
#endif


#define ASSERT_RET(x) { ASSERT((x));if(!(x)) return ; }
#define ASSERT_RETF(x) { ASSERT((x));if(!(x)) return false; }
#define ASSERT_RETT(x) { ASSERT((x));if(!(x)) return true; }
#define ASSERT_RET0(x) { ASSERT((x));if(!(x)) return 0; }
#define ASSERT_RET1(x) { ASSERT((x));if(!(x)) return 0; }
#define ASSERT_RETN1(x) { ASSERT((x));if(!(x)) return -1; }
#define ASSERT_RETCSTR(x) { ASSERT((x));if(!(x)) return CString(); }
#define ASSERT_RETVAL(x,v) { ASSERT((x));if(!(x)) return v; }

#define ASSERT_RETFHR(x) { ASSERT(SUCCEEDED(x));if(!(SUCCEEDED(x))) return FALSE; }
#define ASSERT_RETHR(x) { ASSERT(SUCCEEDED(x));if(!(SUCCEEDED(x))) return; }

#define VERIFY ASSERT
#define VERIFY_RET(x) { VERIFY((x));if(!(x)) return ; }
#define VERIFY_RETF(x) { VERIFY((x));if(!(x)) return FALSE; }
#define VERIFY_RETT(x) { VERIFY((x));if(!(x)) return TRUE; }
#define VERIFY_RET0(x) { VERIFY((x));if(!(x)) return 0; }
#define VERIFY_RET1(x) { VERIFY((x));if(!(x)) return 0; }
#define VERIFY_RETN1(x) { VERIFY((x));if(!(x)) return -1; }
#define VERIFY_RETCSTR(x) { VERIFY((x));if(!(x)) return CString(); }
#define VERIFY_RETVAL(x,v) { VERIFY((x));if(!(x)) return v; }

#define VERIFY_RETFHR(x) { VERIFY(SUCCEEDED(x));if(!(SUCCEEDED(x))) return FALSE; }
#define VERIFY_RETHR(x)  { VERIFY(SUCCEEDED(x));if(!(SUCCEEDED(x))) return; }

#endif

#ifndef TYPE_DEFINE
#define TYPE_DEFINE


typedef unsigned long  ULONG_PTR;
typedef const char* LPCSTR;
typedef const char* LPCTSTR;
//typedef unsigned char byte;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int  UINT;
typedef unsigned long ULONG;
typedef unsigned short      USHORT;
typedef unsigned int        UINT32;
typedef long LONG;
typedef long HRESULT;
typedef  long long __int64;
typedef char TCHAR;
typedef char CHAR;
typedef void* PVOID;
typedef void* LPVOID;
typedef int				SOCKET;
typedef unsigned short  u_short;

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1


typedef unsigned char byte;


#ifndef INVALID_SOCKET
#define INVALID_SOCKET	0
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define CALLBACK
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))


typedef struct {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	byte           Data4[ 8 ];
} GUID;


#define S_OK                                   ((HRESULT)0L)
#define S_FALSE                                ((HRESULT)1L)
#define SUCCEEDED(hr)   (((HRESULT)(hr)) >= 0)
#define FAILED(hr)      (((HRESULT)(hr)) < 0)

const int MAX_LOG4_SIZE = 4096;

void LogInfo(LPCTSTR message,unsigned int w  = 0,unsigned int l = 0);
void LogWarn(const char* message,unsigned int w = 0,unsigned int l = 0);

inline void AddLogError(const char* lpFmt,unsigned int l /* = 0 */)
{
	
}
/*
void LOGI(char* pBuffer)
{
    NSString* str = [NSString stringWithCString:pBuffer encoding:NSUTF8StringEncoding];
    NSLog(@"Info %s", str);
}
void LOGE(char* pBuffer)
{
    NSString* str = [NSString stringWithCString:pBuffer encoding:NSUTF8StringEncoding];
    NSLog(@"Error %s", str);
}
*/
inline void AddLogV(const char* lpFmt,...)
{
	char buf[MAX_LOG4_SIZE]={0};
	va_list argptr;
	va_start(argptr, lpFmt);
	::vsnprintf(buf,MAX_LOG4_SIZE - 1,lpFmt, argptr);
	va_end(argptr);
    
	strcat(buf, "\n");
	LOGI(buf);
	//LOGI("Open path..., g_nSaveLog = %d ", g_nSaveLog);
//	if (g_nSaveLog)
//	{
//		FILE *fd;
//		//LOGI("Open path, %s ", g_pLogPath);
//		fd = fopen(g_pLogPath, "a+");
//		if (fd == NULL)
//		{
//			LOGI("Open File %s Error", g_pLogPath);
//			return;
//		}
//		fwrite(buf, strlen(buf), 1, fd);
//		fclose(fd);
//	}
}

inline void LogError(const char* message,unsigned int w /*= 0*/,unsigned int l /*= 0*/)
{
    
}

inline void LogErrorV(const char* lpFmt,...)
{
	char buf[MAX_LOG4_SIZE]={0};
	va_list argptr;
	va_start(argptr, lpFmt);
	::vsnprintf(buf,MAX_LOG4_SIZE - 1,lpFmt, argptr);
	va_end(argptr);
	LOGE(buf);
}

inline void LogWarn(const char* message,unsigned int w /*= 0*/,unsigned int l  /*= 0*/)
{
    
}

inline void LogWarnV(const char* lpFmt,...)
{
	char buf[MAX_LOG4_SIZE]={0};
	va_list argptr;
	va_start(argptr, lpFmt);
	::vsnprintf(buf,MAX_LOG4_SIZE - 1,lpFmt, argptr);
	va_end(argptr);
	LOGI(buf);
}

inline void LogInfo(LPCTSTR message,unsigned int w  /*= 0*/,unsigned int l /*= 0*/)
{
    
}
inline void LogInfoV(LPCTSTR lpFmt,...)
{
	char buf[MAX_LOG4_SIZE]={0};
	va_list argptr;
	va_start(argptr, lpFmt);
	::vsnprintf(buf,MAX_LOG4_SIZE - 1,lpFmt, argptr);
	va_end(argptr);
	LOGI(buf);
}

inline void LogDebugV(LPCTSTR lpFmt,...)
{
	char buf[MAX_LOG4_SIZE]={0};
	va_list argptr;
	va_start(argptr, lpFmt);
	::vsnprintf(buf,MAX_LOG4_SIZE - 1,lpFmt, argptr);
	va_end(argptr);
	LOGI(buf);
}

inline ULONG GetTickCount(void)
{
	ULONG currentTime;
	struct timeval current;
	gettimeofday(&current, NULL);
    
	currentTime = current.tv_sec * 1000 + current.tv_usec/1000;
    
	return currentTime;
}
#define GetCurrentTime()	GetTickCount()

inline void Sleep(unsigned int millseconds )
{
	// 1 毫秒（milisecond） = 1000 微秒 （microsecond）.
	// Windows 的 Sleep 使用毫秒（miliseconds）
	// Linux 的 usleep 使用微秒（microsecond）
	// 由于原来的代码是在 Windows 中使用的，所以参数要有一个毫秒到微秒的转换。
	usleep( millseconds * 1000 );
}

inline static void ZeroMemory(PVOID Destination,int Length)
{
	memset((void*)Destination, 0, Length);
	return;
};

#ifndef ZeroM
#define ZeroM(x) ZeroMemory(&x,sizeof(x))
#define ZeroA(x) { ZeroMemory(x,sizeof(x));ASSERT(x[0] == x[1]); }
#endif

class JMutexInit :public JMutex
{
public:
	JMutexInit()
	{
		ZeroM(attr);
		InitEx();
	}
	~JMutexInit()
	{
		pthread_mutexattr_destroy(&attr);
		ZeroM(attr);
	}
    
protected:
	int InitEx()
	{
		if (initialized)
			return ERR_JMUTEX_ALREADYINIT;
        
		int nRet= pthread_mutexattr_init(&attr);
		assert(nRet == 0);
		if (nRet != 0)
		{
			//printf("\nErr:pthread_mutexattr_init err=%d",nRet);
		}
		else
		{
			int nRet = pthread_mutexattr_settype(&attr,  PTHREAD_MUTEX_RECURSIVE);
			if(nRet != 0)
			{
				printf("\nErr:pthread_mutexattr_settype err=%d",nRet);
			}
			else
			{
				//printf("\npthread_mutexattr_settype ok");
			}
		}
        
		pthread_mutex_init(&mutex,&attr);
		initialized = true;
		return 0;
	}
    
	pthread_mutexattr_t attr;
};


#endif

#ifdef __cplusplus
#ifndef __lock
#define __lock(x)  JMutexAutoLock lock(x);
typedef  JMutexInit CCriticalSectionEx;
#endif
#endif //__cplusplus


#endif

#endif



