#ifndef OBD_CLIENT_SESSION_H_
#define OBD_CLIENT_SESSION_H_
#include "InformationCore.h"
#include <string>
using namespace std;
#include "macro_define.h"
#include "obd_protocol.h"

const char CLIENT_PASSWORD[] = "JBTCARAPP";

 void client_read_cb(struct bufferevent *bev, void *ctx);
 void client_write_cb(struct bufferevent *bev, void *ctx);
 void client_event_cb(struct bufferevent *bev, short events, void *ctx);

class obd_client_session 
{
public:
	obd_client_session(void);
	~obd_client_session(void);

	bool initialize_client_info(ListenerInfo * pInfo, struct bufferevent *pEvBuffer, struct sockaddr *pClientAddr);


	void read_notify();
	void connect_notify();
	void error_notify();
	void disconnect_notify();
	void timeout_notify();

	bool disconnect();
	void pro_data(char * data_, size_t bytes_pro);
	void put_half_pkg(char * pkg_data_, const size_t pkg_len_);
	void get_all_data(char * all_data_, size_t & all_byte_, const char * new_data_, const size_t new_byte_);
	bool is_have_pkg();
	void remove_half_pkg();
	bool get_client_live() {return is_keep_live_;}

	bool send_buffer(const void* buf, size_t sz);

	void pro_message(tagRequestInfo_t *pkg,  size_t sz);

	void login_handle(tagLogin_t * pkg);
	bool login_reply_handle(LOGIN_REPLY_STATE state);
	void uplaod_file_head_request_handle(tagUploadFileHead_t *pkg);
	void upload_file_data_request_handle(tagUploadFileData_t *pkg);
	//IDLE_CONNECT_REQUEST
	void idle_connect_request_handle();
	void logout_request_handle();

	bool is_client_logined() { return is_logined_;}

	std::string save_data_file(const char * data, size_t sz);

	inline size_t generate_file_index()
	{
		static size_t iNo = 0;

		++iNo;
		return iNo;
	}
private:
	ListenerInfo			*listener_info_ptr_;
	CCriticalSectionEx		event_buffer_lock_;
	struct bufferevent		*event_buffer_ptr_;
	struct event_base		*event_base_ptr_;

	std::string				client_ip_;
	unsigned short			client_port_;

	size_t					half_len_;
	char					half_pkg_[SOCKET_RECEIVE_BUFFER];
	char					*all_buffer_data_;
	bool					is_keep_live_;

	bool					is_logined_;
	std::string				series_numuber_;
	size_t					verify_current_file_size_;
};

#endif	//OBD_CLIENT_SESSION_H_


