/**********************************************************************
	obd_protocol.h

	by  wei 2015/8/25 
***********************************************************************/
#ifndef _OBD_PROTOCOL_H_
#define  _OBD_PROTOCOL_H_
#include "TypeDefine.h"

const size_t CMD_SYN	= '@';     
const size_t BUFFER_SIZE = 1024 * 4;

enum 
{
	kSuccess = 0,
	kUserInexistence = 1,
	kPasswordError,
	kOther,
};

typedef int LOGIN_REPLY_STATE;


#pragma pack(push, 1)

// The packet's info
struct tagRequestInfo_t
{
	tagRequestInfo_t()
	{
		iCmdSyn = CMD_SYN;	
		iCmdId = 0;
		iDataLen = 0;
		memset(buff, '\0', sizeof(buff));
	}

	char			iCmdSyn;		// The command's synchronous verify value.
	unsigned short 	iCmdId;			// Command code.
	int			iDataLen;		// The packet's  buffer size.
	char buff[BUFFER_SIZE];			// The packet's data buffer.
};

// The fixed length of packet header(except the data buffer).
const size_t REQUEST_HEAD_SIZE = sizeof(tagRequestInfo_t) - BUFFER_SIZE;


//1.2.1  login    /*登录的验证怎么做，数据源*/
struct tagLogin_t {

	tagLogin_t () {
		memset(szSN, '\0', sizeof(szSN));
		memset(szIMEI, '\0', sizeof(szIMEI));
		memset(szPassword, '\0', sizeof(szPassword));
		memset(szBootVersion, '\0', sizeof(szBootVersion));
		memset(szSDKVersion, '\0', sizeof(szSDKVersion));
		memset(szAppVersion, '\0', sizeof(szAppVersion));
	}
	char szSN[16];			// serial number
	char szIMEI[16];		// international mobile equipment identity 国际移动设备标志 
	char szPassword[16];	
	char szBootVersion[32];	// Boot版本
	char szSDKVersion[32];	// SDK
	char szAppVersion[32];
};

//1.2.2login reply
struct tagLoginReply_t
{
	tagLoginReply_t() {
		iResult = kSuccess;
		lDateTime = 0;   //是时间戳吗？？
	}

	LOGIN_REPLY_STATE iResult;			//login result: zero if successful; 
							// otherwise nonzero.  
							//1:username not  exist; 2: pwd error; 3: other

	unsigned long lDateTime;  // Return server time;

};

//1.3.1 upload file head request
struct tagUploadFileHead_t
{
	tagUploadFileHead_t ()
	{
		
	}
	int		 iFileSize;		//upload file size;
	char	szMd5[20];		//upload file verify value
};

//1.3.2  upload file reply and 1.3.4 uplaod file data reply
struct tagUploadFileReply_t
{
	tagUploadFileReply_t()
	{
		szResult = '1';
	}
	/*怎样判断文件存在*/
	char szResult;		//'0'allow  ; '1' not allow ,file exist;

};

//1.3.3 upload file data request 
struct tagUploadFileData_t 
{
	tagUploadFileData_t()
	{
		iFileSize = 0;
		memset((void *)&szBuf,0, (BUFFER_SIZE-sizeof(int)) );
	}

	int iFileSize;
	char szBuf[BUFFER_SIZE-sizeof(int)];
};

// 1.5.1 and 1.5.2    idle connect request andr reply  message body NULL;

//1.6.2  logout reply
struct tagLogoutReply_t
{
	tagLogoutReply_t()
	{
		szResult = '1';
	}
	char szResult;		//'0'success  ; '1' error;

};


#pragma pack(pop)

enum CMD_SN
{
	LOGIN_REQUEST				= 0x3005,		// Client Login request.
	LOGIN_REPLY					= 0x3006,		// Reply Login.

	UPLOAD_FILE_HEAD_REQUEST	= 0x3001,		// Upload file head.
	
	UPLOAD_FILE_HEAD_REPLY		= 0x3002,		//	Upload file head reply.

	UPLOAD_FILE_DATA_REQUEST	= 0x3003,		// Upload file data.

	UPLOAD_FILE_DATA_REPLY		= 0x3004,		//	Upload file data reply.

	IDLE_CONNECT_REQUEST		= 0x3009,	IDLE_CONNECT_REPLY		    = 0x3010,	LOGOUT_REQUEST				= 0x3007,	LOGOUT_REPLY				= 0x3008,	AUTOZI_CLIENT_REQUEST		= 0x9000,	AUTOZI_CLIENT_REPLY			= 0x9001,
};


#endif  //_OBD_PROTOCOL_H_

