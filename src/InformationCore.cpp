
#include "InformationCore.h"
#include "obd_client_session.h"

createFileSingleton(InformationCore);

InformationCore::InformationCore(void)
{

}

InformationCore::~InformationCore(void)
{
}

void InformationCore::AddListenerInfo(ListenerInfo* pInfo)
{
	__lock(m_csListenerInfo);
	m_setListenerInfo.insert(pInfo);
}

ListenerInfo * InformationCore::GetListenerInfo(WORD nPort)
{
	ListenerInfo * pInfo = NULL;
	__lock(m_csListenerInfo);
	ITR_ListenerInfo itr = m_setListenerInfo.begin();
	for (; itr != m_setListenerInfo.end(); ++ itr)
	{
		if((*itr)->sin.sin_port == htons(nPort)) 
		{
			pInfo = (*itr);
			break; 
		}	
	}
	return pInfo;
}

void InformationCore::RemoveListenerInfo(WORD nPort)
{
	__lock(m_csListenerInfo);
	ITR_ListenerInfo itr, itr2;
	for (itr = m_setListenerInfo.begin(); itr != m_setListenerInfo.end(); ++ itr)
	{
		itr2 = itr;
		if( (*itr2)->sin.sin_port == htons(nPort)) 
		{
			
			if((*itr2)->pListener != NULL)
			{
				evconnlistener_free((*itr2)->pListener);
			}
			delete (*itr2);
			m_setListenerInfo.erase(itr2);
		}	
	}
}

void InformationCore::RemoveAllListenerInfo()
{
	ListenerInfo * p = NULL;
	{
		__lock(m_csListenerInfo); 
		ITR_ListenerInfo itr;
		for (itr = m_setListenerInfo.begin(); itr != m_setListenerInfo.end(); ++ itr)
		{	p = (*itr);
			if(p != NULL && p->pListener != NULL)
			{
				evconnlistener_free(p->pListener);
				delete p;
				p = NULL;
			}
		}
		m_setListenerInfo.clear();
	}
}

void InformationCore::AddServerTimer(UINT nIDEvent, TimerInfo* pInfo)
{
	__lock(m_csServerTimer);
	m_mapServerTimer.insert(std::make_pair(nIDEvent, pInfo));
}

void InformationCore::RemoveServerTimer(UINT nIDEvent)
{
	__lock(m_csServerTimer);
	m_mapServerTimer.erase(nIDEvent);
}

TimerInfo* InformationCore::GetServerTimer(UINT nIDEvent)
{
	TimerInfo *pInfo = NULL;
	__lock(m_csServerTimer);
	ITR_Timer itr = m_mapServerTimer.find(nIDEvent);
	if (itr != m_mapServerTimer.end())
	{
		pInfo = itr->second;
	}
	return pInfo;
}


void InformationCore::AddClientTimer(UINT nIDEvent, TimerInfo* pInfo)
{
	__lock(m_csClientTimer);
	m_mapClientTimer.insert(std::make_pair(nIDEvent, pInfo));
}

void InformationCore::RemoveClientTimer(UINT nIDEvent)
{
	__lock(m_csClientTimer);
	m_mapClientTimer.erase(nIDEvent);
}

TimerInfo* InformationCore::GetClientTimer(UINT nIDEvent)
{
	TimerInfo *pInfo = NULL;
	__lock(m_csClientTimer);
	ITR_Timer itr = m_mapClientTimer.find(nIDEvent);
	if (itr != m_mapClientTimer.end())
	{
		pInfo = itr->second;
	}
	return pInfo;
}

////////////////////////////////////////////////////////////////
//
void InformationCore::AddClient(obd_client_session * pCli)
{
	__lock(m_csClient);
	m_setClient.insert(pCli);

}
void InformationCore::RemoveClient(obd_client_session * pCli)
{
	__lock(m_csClient);
	m_setClient.erase(pCli);
	delete (pCli);
	pCli = NULL;

}

void InformationCore::VerifyClientLive()
{
	__lock(m_csClient);
	ITR_Client itr, itr2;
	obd_client_session *p = NULL;
	for (itr = m_setClient.begin(); itr != m_setClient.end(); )
	{
		p = *itr;
		itr2 = itr;
		++ itr;
		if( ! (p->get_client_live())) 
		{
			p->disconnect();
			delete p;
			p = NULL;
			m_setClient.erase(itr2);
			printf("The closed socket was removed from  from SET...\n");
		}	
	}
}

/*
 *  
 */ 
int InformationCore::app_log_debug(FILE * logfp, const char * fmt, ...)
{
	
	time_t t = time(NULL);
	struct tm * gmt = localtime(&t);
	int length = 0;
	va_list ap;
	va_start(ap, fmt);
	char log_buf[MAX_LOGBUF_LEN];
	length += snprintf(log_buf , MAX_LOGBUF_LEN, "[ %4d-%02d-%02d %02d:%02d:%02d | <%s>\t<%d>\t<%s>]", gmt->tm_year+1900, gmt->tm_mon+1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec,__FILENAME__, __LINE__, __FUNCTION__);
	length += vsnprintf(&log_buf[length], MAX_LOGBUF_LEN - length, fmt, ap);
	if(NULL != logfp) {
		fprintf(logfp, "%s", log_buf );
	}
	va_end(ap);
	printf("%s", log_buf);
	return 1;
}

int InformationCore::app_log_info( const char * fmt, ...)
{

	time_t t = time(NULL);
	struct tm * gmt = localtime(&t);
	int length = 0;
	va_list ap;
	va_start(ap, fmt);
	char log_buf[MAX_LOGBUF_LEN];
	length += snprintf(log_buf , MAX_LOGBUF_LEN, "[ %4d-%02d-%02d %02d:%02d:%02d ]", gmt->tm_year+1900, gmt->tm_mon+1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	length += vsnprintf(&log_buf[length], MAX_LOGBUF_LEN - length, fmt, ap);
	va_end(ap);
	printf("%s", log_buf);

	return 1;
}

std::string InformationCore::get_datetime_string()
{
	time_t t = time(NULL);
	struct tm * gmt = localtime(&t);

	char date_buf[STRINE_LENGHT_256] = {0};
	sprintf(date_buf, "%4d_%02d_%02d_%02d_%02d_%02d", gmt->tm_year+1900, gmt->tm_mon+1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	
	return std::string(date_buf);
}



