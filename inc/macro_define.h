#ifndef _MACRO_DEFINE_H_
#define  _MACRO_DEFINE_H_

const size_t SOCKET_RECEIVE_BUFFER = 1024 * 64;
const size_t SOCKET_SEND_BUFFER = 1024 * 64;

const size_t MAX_LOGBUF_LEN = 1024;
const size_t BUFFER_LEN = 1024;

const size_t STRINE_LENGHT_256 = 256;
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)


#endif  //_MACRO_DEFINE_H_


