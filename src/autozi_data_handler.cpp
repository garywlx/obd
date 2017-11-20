#include "autozi_data_handler.h"
#include "InformationCore.h"
#include "obd_client_session.h"
autozi_data_handler *s_autozi_data_handler = NULL;
autozi_data_handler::autozi_data_handler(void)
{
	autozi_client_ = NULL;
	deq_autozi_data_.clear();
	is_running_ = false;
}

autozi_data_handler::~autozi_data_handler(void)
{
}

UINT autozi_data_handler::ThreadFunc() 
{
	std::string file_name;
	while(!IsThreadCanceled() && is_running_)
	{
		__lock(m_cs);
		if(autozi_client_ == NULL)
		{
			Sleep(1000*10);
			deq_autozi_data_.clear();
		}
		else
		{
			if (!deq_autozi_data_.empty())
			{
				file_name = deq_autozi_data_.front();
				deq_autozi_data_.pop_front();
				autozi_client_->send_buffer((const void *)(file_name.c_str()), file_name.size());
			}
			else
			{
				Sleep(50);
			}
		}
		Sleep(50);
	}
	return 0; 
}

int autozi_data_handler::push_file_name(  std::string file_name)
{
	sInfoCore.app_log_info("push file_name [%s] to queue...\n", file_name.c_str());
	__lock(m_cs);

	if (NULL == autozi_client_)
		return deq_autozi_data_.size();

	if (deq_autozi_data_.size() > 20)
	{
		return deq_autozi_data_.size();
	}

	deq_autozi_data_.push_back(file_name);

	return deq_autozi_data_.size();
}

void autozi_data_handler::set_autozi_client(obd_client_session * ptr)
{
	autozi_client_ = ptr;
}

bool autozi_data_handler::start_process_data_service()
{
	is_running_ = true;
	return (0==Start("AutoziThread"));
}
bool autozi_data_handler::stop_process_data_service()
{
	is_running_ = false;
	Stop();
	return true;
}

bool autozi_data_handler::is_running_service()
{
	return (is_running_ && IsThreadCanceled());
}