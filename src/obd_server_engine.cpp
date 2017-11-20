#include "obd_server_engine.h"
#include "obd_client_session.h"
#include <arpa/inet.h>

/*
listener：evconnlistener监听器的evconnlistener
fd：FD新文件描述符
address：地址连接的源地址
socklen：socklen addr的长度
ctx：通过evconnlistener_new（）参数user_arg指针,即用户数据
*/

static void
accept_conn_cb(struct evconnlistener *listener,
			   evutil_socket_t fd, struct sockaddr *address, int socklen,
			   void *ctx)
{
	ListenerInfo *pinfo = (ListenerInfo *)ctx;
	ASSERT(pinfo != NULL);

	sockaddr_in* pSaddr = (sockaddr_in*)address;
	LogInfoV("Server listen to a new client connect, ip: %s port：%d ; connect the server at port %d", 
		inet_ntoa(pSaddr->sin_addr), ntohs(pSaddr->sin_port), ntohs(pinfo->sin.sin_port));

	/* We got a new connection! Set up a bufferevent for it. */
	//返回evconnlistener的关联event_base。
	struct event_base *base = evconnlistener_get_base(listener);
#ifdef WIN32
	evthread_use_windows_threads();//win上设置
#else
	evthread_use_pthreads();    //unix上设置
#endif
	//bufferevent 实现了为向后面添加数据和从前面移除数据而优化的字节队列
	struct bufferevent *bev = bufferevent_socket_new(base, fd, \
		BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE);
	//建立对等的客户端
	obd_client_session *pClient = new obd_client_session();
	pClient->initialize_client_info(pinfo, bev, address);
	pClient->connect_notify();
	//参数cbarg将提供给每个回调的参数
	bufferevent_setcb(bev, client_read_cb, client_write_cb, client_event_cb, pClient);
	bufferevent_enable(bev, EV_READ|EV_WRITE);
	obd_server_engine * pServer = (obd_server_engine *)pinfo->pServer;
	pServer->set_send_buffer(fd, SOCKET_SEND_BUFFER);
	pServer->set_receive_buffer(fd, SOCKET_RECEIVE_BUFFER);
	//由于onAccept可能直接拒绝，导致把pClient对象销毁
	//所以必须把timer_out超时先加入到base中，再进行Accept操作
	//调用连接
	pServer->accept_notify(pClient);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	//int err = EVUTIL_SOCKET_ERROR();

	event_base_loopexit(base, NULL);
}

initialiseSingleton(obd_server_engine);
obd_server_engine::obd_server_engine(void)
{
	event_base_ptr_ = NULL;
	is_initialized_ = false;
}

obd_server_engine::~obd_server_engine(void)
{
}

//////////////////////////////////////////////////
//
bool obd_server_engine::initialize_server_engine()
{
	if(!is_initialized_)
	{
		
		if(event_base_ptr_ == NULL)
		{
			event_base_ptr_ = event_base_new();
		}
		ASSERT(event_base_ptr_ != NULL);
		//创建通信通道，并关联event
		if (-1 == evthread_make_base_notifiable(event_base_ptr_))
		{
			event_base_free(event_base_ptr_);
			return false;
		}
		is_initialized_ = true;
	}
	return true;
}
//////////////////////////////////////////////////
//
bool obd_server_engine::uninitialize_server_engine()
{
	if(!is_initialized_)
		return false;
	if(event_base_ptr_ != NULL)
	{
		event_base_free(event_base_ptr_);
		event_base_ptr_ = NULL;
	}
	
	return true;
}
//////////////////////////////////////////////////
//
bool obd_server_engine::start_server(unsigned short port)
{
	if(port < 1025 && port > 65534)
	{
		sInfoCore.app_log_debug(NULL, "Port set error... \n");
		return false;
	}
	if(!is_initialized_)
	{
		sInfoCore.app_log_debug(NULL,"Non Initialize... \n ");
		return false;
	}
	
	size_t sin_len = 0;
	struct sockaddr *sin = NULL;

	ListenerInfo* pInfo = new ListenerInfo;
	memset(&pInfo->sin, 0, sizeof(pInfo->sin));
	pInfo->sin.sin_family = AF_INET;
	pInfo->sin.sin_addr.s_addr = htonl(0);
	pInfo->sin.sin_port = htons(port);
	sin = (struct sockaddr *)&pInfo->sin;
	sin_len = sizeof(pInfo->sin);

	//返回监听的对象
	pInfo->pListener = evconnlistener_new_bind(event_base_ptr_, (evconnlistener_cb)accept_conn_cb, pInfo,
		LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, sin, sin_len);
	if (pInfo->pListener  == NULL) 
	{
		sInfoCore.app_log_debug(NULL,"evconnlistener object to listen for incoming TCP error...\n");
		delete pInfo;
		pInfo = NULL;
		return false;
	}
	sInfoCore.app_log_info("Server listen  at port %d ...\n",  ntohs(pInfo->sin.sin_port));
	//设置当listen错误时的回调函数
	evconnlistener_set_error_cb(pInfo->pListener, accept_error_cb);
	pInfo->pServer = this;
	sInfoCore.AddListenerInfo(pInfo);
	int res = Start("ServerEngin");
	if(res !=0)
	{
		sInfoCore.app_log_debug(NULL, "Start thread faild, return code = %d  ....\n", res);
		stop_server();
		return false;
	}
	 res = event_base_dispatch(event_base_ptr_);
	if(res == 0)
	{
		sInfoCore.app_log_info("successful");
	} 
	else if(res == 1)
	{
		printf("no events were registered");
	}
	else
	{
		printf("error occurred");
	}
	return true;
}

bool obd_server_engine::stop_server()
{
	if (event_base_ptr_ != NULL)
	{
		sInfoCore.RemoveAllListenerInfo();

		timeval tm = {0};
		tm.tv_sec = 0;
		event_base_loopexit(event_base_ptr_,&tm);
		event_base_free(event_base_ptr_);
		event_base_ptr_ = NULL;
		Stop();

		return true;
	}
	return false;
}

UINT obd_server_engine::ThreadFunc()
{
	size_t loop_counter = 0;

	printf("========Start OBD Server engine Thread ========== \n");
	while (!IsThreadCanceled() && event_base_ptr_ != NULL)
	{
		if(!(++loop_counter % 400))	 // 20 seconds
			sInfoCore.VerifyClientLive();
		Sleep(50);
	}
	return 0;
}

bool obd_server_engine::set_send_buffer(SOCKET fd, int nSize)
{
	return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&nSize, sizeof(int)) < 0);
}

bool obd_server_engine::set_receive_buffer(SOCKET fd, int nSize)
{
	return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, sizeof(int)) < 0);
}

void obd_server_engine::accept_notify(obd_client_session *cli)
{
	if(NULL == cli)
		return ;
	sInfoCore.AddClient(cli);
}

