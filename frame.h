#ifndef FRAME_H_
#define FRAME_H_

typedef struct _frame frame_t;
enum {REQ = 1, DATA, ACK, FINISH, ERR};
enum {
   NO_ERR,
   CHECKSUM_ERR,
   SEQ_NUMBER_ERR,
   RETRY_TOO_MUCH_ERR,
   MD5_ERR,
   CTRL_C_SIG_ERR,
   UNKNOWN_PKT_TYPE,
   OUT_OF_MAX_RETRY_TIMES,
   TIME_OUT
};

int frame_init(const char *dev);
void frame_fini();
int frame_check();
void frame_set_header();
void frame_set_seq_number(int seq_number);
void frame_set_checksum();
void frame_set_len(int len);
void frame_set_op(int op);
void frame_set_status(int status);
int frame_req();//only sender --> receiver
int frame_data(unsigned char *data, int len);
int frame_ack_ok();//only receiver --> sender
int frame_ack_not_ok();//only receiver --> sender
int frame_finish(char *md5);//only sender --> receiver
int frame_err(int err);
int frame_get_op_from_raw_data();
int frame_get_err_from_raw_data();
int frame_get_status_from_raw_data();
int frame_seq_number_check();
int frame_get_seq_number_from_raw_data();
int frame_checksum_check();
void frame_calculate_receive_frame_checksum();
void frame_calculate_send_frame_checksum();
int frame_get_frame();
int frame_get_remainder_frame();
int frame_get_frame_len();
void frame_get_frame_header();
void frame_decode_frame();
int frame_get_data_len();
void* frame_get_data_pos();
int u_frame_get_op();
const char* u_frame_get_md5_from_finish_pkt(char *md5);

#endif//FRAME_H_
