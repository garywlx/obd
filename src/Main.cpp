#include <stdio.h>
#include "InformationCore.h"
#include "obd_server_engine.h"
#include "autozi_data_handler.h"

int main(int argc, char *argv[])
{
	sInfoCore.app_log_info("=====================OBD_SERVER===============================\n");
	new obd_server_engine();
	bool ret = sObdServerEngine.initialize_server_engine();
	if(! ret) {
		sInfoCore.app_log_debug(NULL, "Initialize server engine failed...\n");
		return -1;
	}
	sInfoCore.app_log_info("Initialize server engine Finished...\n");
	
	s_autozi_data_handler = new autozi_data_handler();
	if(s_autozi_data_handler == NULL)
	{
		sInfoCore.app_log_debug(NULL, "Initialize autozi service error... \n");
		return -1;
	}
	
	sInfoCore.app_log_info("Initialize autozi process data service Finished...\n");
	if(!sAutoziClientHandler.start_process_data_service())
	{
		sInfoCore.app_log_debug(NULL, "Start autozi service error... \n");
		return -1;

	}
	sInfoCore.app_log_info("Start autozi process data service Successfully...\n");

	sInfoCore.app_log_info("Start server engine ...\n");
	ret = sObdServerEngine.start_server(8090);
	if(! ret) {
		sInfoCore.app_log_debug(NULL, "Start server engine failed...\n");
		return -1;
	}
	sInfoCore.app_log_info("Start server engine Finished...\n");

	return 0;
}
