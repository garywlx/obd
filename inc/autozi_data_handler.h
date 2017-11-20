#ifndef AUTOZI_DATA_HANDlER_H_
#define AUTOZI_DATA_HANDlER_H_

#include <string>
#include <queue>

using namespace std;
#include "CThread.h"


typedef std::deque<std::string> DEQ_AUTOZI_DATA;

class obd_client_session;

class autozi_data_handler : public CThread
{
public:
	autozi_data_handler(void);
	~autozi_data_handler(void);

	int push_file_name(  std::string file_name);
	void set_autozi_client(obd_client_session * ptr);
	bool start_process_data_service();
	bool stop_process_data_service();
	bool is_running_service();
protected:
	virtual UINT ThreadFunc() ;

private:
	CCriticalSectionEx m_cs;
	DEQ_AUTOZI_DATA deq_autozi_data_;
	obd_client_session  *autozi_client_;
	bool				is_running_;
};

extern autozi_data_handler *s_autozi_data_handler;
#define sAutoziClientHandler (*s_autozi_data_handler)
#endif

