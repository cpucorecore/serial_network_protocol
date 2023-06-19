#include "file_exception.h"

void file_exception_handler(CEXCEPTION_T e, const char *name)
{
	switch (e)
	{
	case SEND_FILE_OPEN_FILE_ERR:
		file_exception_handler_handle_send_file_open_file_err();
		break;
	case RECV_FILE_OPEN_FILE_ERR:
		file_exception_handler_handle_recv_file_open_file_err();
		break;
	}
}

void file_exception_handler_handle_send_file_open_file_err()
{
	//frame_t *f = get_sender_frame();
	//frame_err(f, SEND_FILE_OPEN_FILE_ERR);
}

void file_exception_handler_handle_recv_file_open_file_err()
{
	//frame_t *f = get_receiver_frame();
	//frame_err(f, RECV_FILE_OPEN_FILE_ERR);
}
