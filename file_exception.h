#include "CException.h"

enum {
	SEND_FILE_OPEN_FILE_ERR = 0x11,//for sender use
	RECV_FILE_OPEN_FILE_ERR = 0x21,//for receiver use
};

void file_exception_handler(CEXCEPTION_T e, const char *name);

void file_exception_handler_handle_send_file_open_file_err();
void file_exception_handler_handle_recv_file_open_file_err();
