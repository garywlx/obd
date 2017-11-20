#include "obd_client_session.h"
#include <arpa/inet.h>
#include "obd_protocol.h"
#include "autozi_data_handler.h"
/*
bev：触发回调的bufferevent
ctx: 用户为此bufferevent指定的用户数据指针
*/
void client_read_cb(struct bufferevent *bev, void *ctx)
{
	if (ctx == NULL)
	{
		//LogInfo("接收消息的客户端句柄无效！");
		return ;
	}
	/* This callback is invoked when there is data to read on bev. */
	//struct evbuffer *input = bufferevent_get_input(bev);
	//struct evbuffer *output = bufferevent_get_output(bev);

	obd_client_session *pClient = (obd_client_session*)ctx;
	ASSERT(pClient != NULL);

	pClient->read_notify();

	/* Copy all the data from the input buffer to the output buffer. */
	//evbuffer_add_buffer(output, input);
}

void client_write_cb(struct bufferevent *bev, void *ctx)
{
	//会在 buf 内容小于 low watermark 时回调
	//默认 low watermark 就是 0，相当于数据发送完成时通知外部。

	obd_client_session *pClient = (obd_client_session*)ctx;
	if (pClient != NULL)
	{
		//pClient->write_notify();
	}
}

//事件回调
void client_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	//TRACE("\nclient_event_cb events = 0x%08x \n", events);
	obd_client_session *pClient = (obd_client_session*)ctx;
	if (pClient == NULL)
	{
		struct evbuffer * pOut =  bufferevent_get_output(bev);
		size_t nSizeOut = evbuffer_get_length(pOut);
		if(nSizeOut == 0)
		{
			bufferevent_free(bev);
			bev = NULL;
		}
		return;
	}

	if (events & BEV_EVENT_EOF)//eof file reached
	{
		pClient->disconnect_notify();
		return ;
	}

	if (events & BEV_EVENT_ERROR)//An error occurred during a bufferevent operation
	{
		pClient->error_notify();
		return ;
	}
	if (events & BEV_EVENT_TIMEOUT)//user-specified timeout reached
	{
		pClient->timeout_notify();
		return ;
	}
}

obd_client_session::obd_client_session(void)
{
	listener_info_ptr_ = NULL;
	event_base_ptr_ = NULL;
	event_buffer_ptr_ = NULL;
	client_port_ = 0;
	is_keep_live_ = false;
	is_logined_ = false;
	series_numuber_.clear();

}

obd_client_session::~obd_client_session(void)
{
	is_keep_live_  = false;
}

bool obd_client_session::initialize_client_info(ListenerInfo * pInfo, struct bufferevent *pEvBuffer, struct sockaddr *pClientAddr)
{
	listener_info_ptr_ = pInfo;
	sockaddr_in* pSIn = (sockaddr_in*)pClientAddr;
	client_ip_ = inet_ntoa(pSIn->sin_addr);
	client_port_ = ntohs(pSIn->sin_port);
	{
		__lock(event_buffer_lock_);
		event_buffer_ptr_ = pEvBuffer;
	}
	return TRUE;
}

bool obd_client_session:: disconnect()
{
	if(event_buffer_ptr_ != NULL)
	{
		bufferevent_disable(event_buffer_ptr_, EV_READ|EV_WRITE);
		evutil_closesocket(bufferevent_getfd(event_buffer_ptr_));
	}
	return true;
}
/////////////////////////////////////////////////////////////
//
bool obd_client_session::send_buffer(const void* buf, size_t sz)
{
	if (event_buffer_ptr_ == NULL)
	{
		return false;
	}

	if (buf == NULL || sz <= 0 )
	{
		return false;
	}

	if(bufferevent_write(event_buffer_ptr_,buf,sz) == 0)
	{
		return true;
	}
	
	sInfoCore.app_log_info("send data failed...\n");

	return false;
}
////////////////////////////////
//
void obd_client_session::read_notify()
{
	__lock(event_buffer_lock_);
	if(event_buffer_ptr_ == NULL)
	{
		//LogInfo("ClientSession::SetParameter中 struct bufferevent 设置错误！");
		return ;
	}

	struct evbuffer *input = bufferevent_get_input(event_buffer_ptr_);
	size_t nSizeAll = evbuffer_get_length(input);
	//sInfoCore.app_log_info("接收到数据的长度 =  %d\n", nSizeAll);

	if(nSizeAll > 0)
	{
		char *pBuffer = new char[nSizeAll];
		if(pBuffer != NULL) 
		{
			size_t nRet = bufferevent_read(event_buffer_ptr_, (void *)pBuffer , nSizeAll);
			if( nRet >0)
			{
				
				pro_data(pBuffer, nRet);
			}
			//send_buffer((void *)pBuffer, nRet);
			delete [] pBuffer;
			pBuffer = NULL;
		}
	}
}

void obd_client_session::connect_notify()
{
	sInfoCore.app_log_info("Client [%s:%d] connect finished\n", client_ip_.c_str(), client_port_);
	is_keep_live_ = true;

}

void obd_client_session::disconnect_notify()
{
	sInfoCore.app_log_info("Client [%s : %d] disconnect\n",  client_ip_.c_str(), client_port_);
	is_keep_live_ = false;
	is_logined_ = false;
}

void obd_client_session::error_notify()
{
	//int nErrorCode = EVUTIL_SOCKET_ERROR();
	sInfoCore.app_log_info("client [%s:%d] error... \n" ,client_ip_.c_str(), client_port_);
	is_keep_live_ = false;
	is_logined_ = false;

}

void obd_client_session::timeout_notify()
{
	printf("Closing socket[%s:%d] due to ping timeout.\n", client_ip_.c_str(), client_port_);
	is_keep_live_ = false;
	is_logined_ = false;
}

///////////////////////////////////////////////////////////////////////////
//
void obd_client_session::pro_data(char * data_, size_t bytes_pro)
{
	
	if(all_buffer_data_ == NULL)
	{
		all_buffer_data_ = new char[SOCKET_RECEIVE_BUFFER * 2];
	}
	memset(all_buffer_data_, '\0', SOCKET_RECEIVE_BUFFER * 2);
	size_t all_byte_ = bytes_pro;
	get_all_data(all_buffer_data_, all_byte_, data_, bytes_pro);
	tagRequestInfo_t *recv_pkg_;
	unsigned short cmd_id_;
	size_t  buff_len_;
	size_t pkg_pos_ = 0;
	//printf("packet ======= pkg_pos_ = %lu, all_byte = %lu \n", pkg_pos_, all_byte_);

	while(pkg_pos_ < all_byte_ && pkg_pos_ >= 0) {
		if((pkg_pos_ + REQUEST_HEAD_SIZE) > all_byte_) {
			goto PUT_HALT_PKG;
		}

		//printf("packet ======= pkg_pos_\n\r");
		recv_pkg_ = (tagRequestInfo_t*)(all_buffer_data_ + pkg_pos_);
		cmd_id_ = recv_pkg_->iCmdId;
		buff_len_ = recv_pkg_->iDataLen - REQUEST_HEAD_SIZE;
		if(buff_len_ > SOCKET_RECEIVE_BUFFER){
			sInfoCore.app_log_debug(NULL, "data error...\n");
			goto DATA_ERR;
		}
		if((int)CMD_SYN != (int)recv_pkg_->iCmdSyn) {
			sInfoCore.app_log_debug(NULL,"syn head error %d---%d...\n", (int)CMD_SYN , (int)recv_pkg_->iCmdSyn);
			goto DATA_ERR;
		}
		if ((pkg_pos_ + REQUEST_HEAD_SIZE + buff_len_) > all_byte_){
			sInfoCore.app_log_info("put halt package...\n");
			goto PUT_HALT_PKG;
		}
		pkg_pos_ += REQUEST_HEAD_SIZE + buff_len_;
		pro_message(recv_pkg_,  buff_len_+REQUEST_HEAD_SIZE);
	
		all_byte_ -= pkg_pos_;
	}
	goto PROC_FINISHED;
PUT_HALT_PKG:
	put_half_pkg(all_buffer_data_ + pkg_pos_, all_byte_ - pkg_pos_);
	goto PROC_FINISHED;
DATA_ERR:
	remove_half_pkg();
PROC_FINISHED:
	memset(all_buffer_data_, '\0', SOCKET_RECEIVE_BUFFER * 2);
	all_byte_ = 0;
}
///////////////////////////////////////////////////////////////////////////////////
//
void obd_client_session::put_half_pkg(char * pkg_data_, const size_t pkg_len_)
{
	if( pkg_data_ != NULL )
	{
		memcpy(half_pkg_, pkg_data_,pkg_len_);
		half_len_ = pkg_len_;
	}
}
bool obd_client_session::is_have_pkg()
{
	return (half_len_>0 && SOCKET_RECEIVE_BUFFER >half_len_)? true:false;
}
void obd_client_session::get_all_data(char * all_data_, size_t & all_byte_, const char * new_data_, const size_t new_byte_)
{
	size_t pos_ = 0;
	if(is_have_pkg()) {
		memcpy(all_data_, half_pkg_, half_len_);
		pos_ += half_len_;
		remove_half_pkg();
	}
	if( new_data_ != NULL )
		memcpy(all_data_ + pos_, new_data_, new_byte_);
}
void obd_client_session::remove_half_pkg()
{
	half_len_ = 0;
	memset(half_pkg_, 0, SOCKET_RECEIVE_BUFFER);
}


void obd_client_session::pro_message(tagRequestInfo_t *pkg,  size_t sz)
{
	//printf("=============pro_message data===================\n");
	unsigned short cmd_id = pkg->iCmdId;
	size_t data_size = pkg->iDataLen - REQUEST_HEAD_SIZE;
	
	switch(cmd_id)
	{
	case LOGIN_REQUEST:
		sInfoCore.app_log_info("Login request----------------------------\n");
		if(data_size >0 && data_size == sizeof(tagLogin_t))
		{
			tagLogin_t * login_ptr = new tagLogin_t;
			memcpy((void*)login_ptr, pkg->buff, data_size);
			login_handle(login_ptr);
			delete login_ptr;
			login_ptr = NULL;
		}
		break;
	case UPLOAD_FILE_HEAD_REQUEST:
		sInfoCore.app_log_info("Upload file head request----------------------------\n");
		if(data_size >0 && data_size == sizeof(tagUploadFileHead_t))
		{
			tagUploadFileHead_t * file_head_ptr = new tagUploadFileHead_t;
			memcpy((void*)file_head_ptr, (void*)pkg->buff, data_size);
			uplaod_file_head_request_handle(file_head_ptr);
			delete file_head_ptr;
			file_head_ptr = NULL;
		}
		else
		{
			sInfoCore.app_log_debug(NULL, "Upload file head request data size error[ data_size = %d, struct size = %d ]...\n", data_size , sizeof(tagUploadFileHead_t));
		}
		break;
	case UPLOAD_FILE_DATA_REQUEST:
		sInfoCore.app_log_info("Upload file data request----------------------------\n");
		if(data_size == (verify_current_file_size_ + sizeof(int)))
		{
			tagUploadFileData_t * file_data_ptr = new tagUploadFileData_t;
			memcpy((void*)file_data_ptr, pkg->buff, data_size);
			upload_file_data_request_handle(file_data_ptr);
			delete file_data_ptr;
			file_data_ptr = NULL;
		}
		else
		{
			sInfoCore.app_log_debug(NULL, "Upload file data request data size error[ data_size = %d, verify size = %d ]...\n", data_size , verify_current_file_size_);
		}

		break;
	case IDLE_CONNECT_REQUEST:
		idle_connect_request_handle();
		break;
	case LOGOUT_REQUEST:
		logout_request_handle();
		break;


	case AUTOZI_CLIENT_REQUEST:
		sAutoziClientHandler.set_autozi_client(this);
		break;
	default:
		break;	
	}
}

bool obd_client_session::login_reply_handle(LOGIN_REPLY_STATE state)
{
	tagRequestInfo_t req;
	req.iCmdSyn = CMD_SYN;
	req.iCmdId = LOGIN_REPLY;
	req.iDataLen = sizeof(tagLoginReply_t)+REQUEST_HEAD_SIZE;
	tagLoginReply_t reply;
	ZeroM(reply);
	reply.lDateTime = (unsigned long)time(NULL);
	reply.iResult = state;
	memcpy((void *)&req.buff, (const void *)&reply, sizeof(tagLoginReply_t) );

	bool res = send_buffer((void *)&req, req.iDataLen );
	if(!res)
	{
		sInfoCore.app_log_debug(NULL, "Reply login failed...\n");
	}
	is_keep_live_ = false;
	
	return res;
}

void obd_client_session::login_handle(tagLogin_t * pkg)
{
	sInfoCore.app_log_info("recv struct tagLogin_t size = %lu  sn = %s \n", sizeof(tagLogin_t) , pkg->szSN);
	if(0==strcmp(pkg->szPassword, CLIENT_PASSWORD))
	{
		series_numuber_ = pkg->szSN;
		is_logined_ = true;
		if(!login_reply_handle(kSuccess))
		{
			return ;
		}
		sInfoCore.app_log_info("Client[%s:%d] login successful...\n", client_ip_.c_str(), client_port_);
	}
	else
	{
		sInfoCore.app_log_debug(NULL, "[%s:%d] login password error...\n", client_ip_.c_str(), client_port_);
		if(!login_reply_handle(kPasswordError))
			return ;
	}
}

void obd_client_session::uplaod_file_head_request_handle(tagUploadFileHead_t *pkg)
{
	sInfoCore.app_log_info("recv struct tagUploadFileHead_t size = %lu  filesize = %d \n", sizeof(tagUploadFileHead_t) , pkg->iFileSize);
	if(!is_client_logined())
	{	
		if(!login_reply_handle(kOther))
			return ;
		sInfoCore.app_log_debug(NULL,"Client not login...\n");
		return ;
	}
	if(pkg->iFileSize >0)
	{
		verify_current_file_size_ = pkg->iFileSize;
	}

	tagRequestInfo_t req;
	req.iCmdSyn = CMD_SYN;
	req.iCmdId = UPLOAD_FILE_HEAD_REPLY;
	req.iDataLen = sizeof(tagUploadFileReply_t)+REQUEST_HEAD_SIZE;
	tagUploadFileReply_t reply;
	ZeroM(reply);
	if (verify_current_file_size_ >0)
	{
		reply.szResult = '0';  //allow upload
	}
	else
	{
		verify_current_file_size_ = 0;
		reply.szResult = '1';
	}
	memcpy((void *)&req.buff, (const void *)&reply, sizeof(tagUploadFileReply_t) );

	bool res = send_buffer((void *)&req, req.iDataLen );
	if(!res)
	{
		sInfoCore.app_log_debug(NULL,"Reply upload file head failed...\n");
	}
}

void obd_client_session::upload_file_data_request_handle(tagUploadFileData_t *pkg)
{
	sInfoCore.app_log_info("recv struct tagUploadFileData_t size = %d  filesize = %d \n", sizeof(tagUploadFileData_t) , pkg->iFileSize);
	tagUploadFileReply_t reply;
	ZeroM(reply);
	if(verify_current_file_size_ >0 && (size_t)pkg->iFileSize == verify_current_file_size_)
	{
		char data_buffer[BUFFER_LEN] = {0};
		memcpy((void *)data_buffer, (const void *)pkg->szBuf, verify_current_file_size_);

		std::string file_name = save_data_file(data_buffer, verify_current_file_size_);
		if(s_autozi_data_handler != NULL)
		{
			sAutoziClientHandler.push_file_name(file_name);
		}
		verify_current_file_size_ = 0;
		reply.szResult = '0';  //success upload
	}
	else
	{
		reply.szResult = '1';

	}
	tagRequestInfo_t req;
	req.iCmdSyn = CMD_SYN;
	req.iCmdId = UPLOAD_FILE_HEAD_REPLY;
	req.iDataLen = sizeof(tagUploadFileReply_t)+REQUEST_HEAD_SIZE;
	
	memcpy((void *)&req.buff, (const void *)&reply, sizeof(tagUploadFileReply_t) );

	bool res = send_buffer((void *)&req, req.iDataLen );
	if(!res)
	{
		sInfoCore.app_log_debug(NULL, "Reply upload file data failed...\n");
	}
}

void obd_client_session::idle_connect_request_handle()
{
	printf("request idle connect \n");
	tagRequestInfo_t req;
	req.iCmdSyn = CMD_SYN;
	req.iCmdId = IDLE_CONNECT_REPLY;
	req.iDataLen = REQUEST_HEAD_SIZE;

	bool res = send_buffer((void *)&req, req.iDataLen );
	if(!res)
	{
		sInfoCore.app_log_debug(NULL, "Reply idle connect failed...\n");
	}
}

void obd_client_session::logout_request_handle()
{
	if(!is_client_logined())
	{	
		if(!login_reply_handle(kOther))
			return ;
		printf("Client not login...\n");
		return ;
	}

	tagRequestInfo_t req;
	req.iCmdSyn = CMD_SYN;
	req.iCmdId = LOGOUT_REPLY;
	req.iDataLen = sizeof(tagLogoutReply_t)+REQUEST_HEAD_SIZE;
	tagLogoutReply_t reply;
	ZeroM(reply);
	reply.szResult = '0';
	memcpy((void *)&req.buff, (const void *)&reply, sizeof(tagLogoutReply_t) );

	bool res = send_buffer((void *)&req, req.iDataLen );
	if(!res)
	{
		printf("Reply logout failed...\n");
	}
	is_logined_ = false;
}

std::string obd_client_session::save_data_file(const char * data, size_t sz)
{
	char file_name[STRINE_LENGHT_256] = {0};
	sprintf(file_name, "obd_data/%s_%lu.%s", sInfoCore.get_datetime_string().c_str(), generate_file_index(), "obd");
	
	sInfoCore.app_log_info( "Test upload data name = %s..........\n", file_name);

	FILE * fp = NULL;
	fp = fopen(file_name, "w+");
	fwrite(data, 1, sz, fp);
	fclose(fp);
	return std::string(file_name);
}



