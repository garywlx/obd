#ifndef _INFOMATION_CORE_H_
#define _INFOMATION_CORE_H_
#include "TypeDefine.h"
#include "Singleton.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/rpc.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>
#include <set>
#include <map>
#include <string>
#include "macro_define.h"
using namespace std;

class obd_client_session;
struct tagListenerInfo_t {
	tagListenerInfo_t (){
		pServer = NULL;
		pListener = NULL;
		ZeroM (sin);
	}
	void *					pServer;
	struct evconnlistener*	pListener;
	struct sockaddr_in		sin;
};
typedef tagListenerInfo_t ListenerInfo;

typedef std::set <ListenerInfo *> SET_ListenerInfo;
typedef SET_ListenerInfo::iterator ITR_ListenerInfo;

struct tagTimerInfo_t
{
	tagTimerInfo_t()
	{
		pEvent = NULL;
		pServer = NULL;
		pClient = NULL;
		nIDEvent = 0;
	}

	struct event	*pEvent;
	void			*pServer;
	void			*pClient;
	UINT			nIDEvent;
};
typedef struct tagTimerInfo_t TimerInfo;

typedef std::map<UINT, TimerInfo*> MAP_Timer;
typedef MAP_Timer::iterator ITR_Timer;


typedef std::set<obd_client_session *> SET_Client;
typedef SET_Client::iterator ITR_Client;

class InformationCore : public Singleton<InformationCore>
{
public:
	InformationCore(void);
	~InformationCore(void);

	void AddListenerInfo(ListenerInfo *pInfo);
	void RemoveListenerInfo(WORD nPort);
	void RemoveAllListenerInfo();
	ListenerInfo * GetListenerInfo(WORD nPort);

	void AddServerTimer(UINT nIDEvent, TimerInfo* pInfo);
	void RemoveServerTimer(UINT nIDEvent);
	TimerInfo* GetServerTimer(UINT nIDEvent);

	void AddClientTimer(UINT nIDEvent, TimerInfo* pInfo);
	void RemoveClientTimer(UINT nIDEvent);
	TimerInfo* GetClientTimer(UINT nIDEvent);

	void AddClient(obd_client_session * pCli);
	void RemoveClient(obd_client_session * pCli);
	void VerifyClientLive();

	int app_log_debug(FILE * logfp, const char * fmt, ...);
	int app_log_info( const char * fmt, ...);

	std::string get_datetime_string();

	
private:
	SET_ListenerInfo m_setListenerInfo;
	CCriticalSectionEx m_csListenerInfo;
	MAP_Timer m_mapServerTimer;
	CCriticalSectionEx m_csServerTimer;

	MAP_Timer m_mapClientTimer;
	CCriticalSectionEx m_csClientTimer;

	SET_Client m_setClient;
	CCriticalSectionEx m_csClient;
};

#define sInfoCore InformationCore::getSingleton()

//#if defined(_WIN32) || defined(_WIN64)
//#define log(fmt, ...)  sInfoCore.app_log("<%s>\t<%d>\t<%s>,"fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
//#else
//#define log(fmt, args...)  sInfoCore.app_log("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
//#endif

#endif		//_INFOMATION_CORE_H_
