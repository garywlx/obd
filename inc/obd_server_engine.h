#ifndef OBD_SERVER_ENGINE_
#define OBD_SERVER_ENGINE_

#include "Singleton.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/rpc.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>
#include "macro_define.h"
#include <sys/types.h>  
#include <sys/socket.h>
#include "CThread.h"


class obd_client_session;

class obd_server_engine : public CThread
						, public Singleton<obd_server_engine>
{

public:
	obd_server_engine(void);
	~obd_server_engine(void);
	
	bool initialize_server_engine();
	bool uninitialize_server_engine();
	
	bool start_server(unsigned short port);
	bool stop_server();

	bool set_send_buffer(SOCKET fd, int sz);
	bool set_receive_buffer(SOCKET fd, int sz);

	void accept_notify(obd_client_session *cli);

protected:
	virtual UINT ThreadFunc();

private:
	struct event_base	*event_base_ptr_;
	bool		is_initialized_;
};

#define sObdServerEngine obd_server_engine::getSingleton()
//#define sObdServerEnginePtr (void *)obd_server_engine::getSingletonPtr()

#endif //OBD_SERVER_ENGINE_

