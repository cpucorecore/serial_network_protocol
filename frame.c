#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "frame.h"
#include "serial_dev.h"
#include "checksum.h"
#include "ui.h"
#include "global.h"
#include "Md5.h"

struct _frame {
   unsigned char r_raw_data[MTU];
   unsigned char s_raw_data[MTU];
   int r_len;
   int s_len;
   int r_last_seq_number;
   int s_last_seq_number;
   int r_seq_number_in_raw_data;
   int s_seq_number_in_raw_data;
   int r_op;
   int s_op;
   int r_status;
   int s_status;
   int r_err_code;
   int s_err_code;
   unsigned char r_checksum[2];
   unsigned char s_checksum[2];
};

static frame_t g_frame;

int frame_init(const char *dev)
{
   memset((void*)&g_frame, 0, sizeof(frame_t));

   g_frame.r_last_seq_number = -1;
   g_frame.s_last_seq_number = -1;
   return serial_dev_init(dev);
}

void frame_fini()
{
   serial_dev_fini();
}

int frame_check()
{
   frame_decode_frame();

   if(0 == frame_checksum_check()) {
      printf("frame_checksum_check FAIL\n");
      return CHECKSUM_ERR;
   }

   if (0 == frame_seq_number_check()) {
      printf("frame_seq_number_check FAIL\n");
      return SEQ_NUMBER_ERR;
   }

   return 0;
}

void frame_set_header()
{
   g_frame.s_raw_data[0] = 'k';
   g_frame.s_raw_data[1] = 'l';
   g_frame.s_raw_data[2] = 'j';
   g_frame.s_raw_data[3] = 'c';
}

void frame_set_seq_number(int seq_number)
{
   g_frame.s_raw_data[6] = seq_number >> 8;
   g_frame.s_raw_data[7] = seq_number & 0xff;
}

void frame_set_checksum()
{
   memcpy((void*)(g_frame.s_raw_data + g_frame.s_len - 2), (void*)g_frame.s_checksum, 2);
}

void frame_set_len(int len)
{
   g_frame.s_len = len;
   g_frame.s_raw_data[4] = len >> 8;
   g_frame.s_raw_data[5] = len & 0xff;
}

void frame_set_op(int op)
{
   g_frame.s_raw_data[8] = op;
}

void frame_set_status(int status)
{
   g_frame.s_raw_data[9] = status;
}

int frame_req()//only sender --> receiver
{
   /*|4byte  |2byte|2byte|1 |2byte   |         req frame len of sum = 11
    *|header |len  |sn   |op|checksum|
    *|k|l|j|c|00|12|00|00|01|xx|xx   |
    */

   g_frame.s_last_seq_number++;//TODO 应该确保发送出去后才++
   frame_set_header();
   frame_set_len(11);
   frame_set_seq_number(0);
   frame_set_op(REQ);
   frame_calculate_send_frame_checksum();
   frame_set_checksum();

   return serial_dev_nwrite((void*)g_frame.s_raw_data, g_frame.s_len);
}

int frame_data(unsigned char *data, int len)
{
   /*|4byte  |2byte|2byte|1 |1  |data|2byte   |
    *|header |len  |sn   |op|f  |    |checksum|
    *|k|l|j|c|xx|xx|xx|xx|3 |0/1|    |xx|xx   |
    */

   if(len > MAX_DATA_LEN) return -1;
   g_frame.s_last_seq_number++;
   frame_set_header();
   frame_set_len(11 + len);
   frame_set_seq_number(g_frame.s_last_seq_number);
   frame_set_op(DATA);
   memcpy((void*)(g_frame.s_raw_data + 9), (void*)data, len);
   frame_calculate_send_frame_checksum();
   frame_set_checksum();

   /*
    * test case for checksum err
   if (g_frame.s_last_seq_number >= 5 && g_frame.s_last_seq_number <= 40) {
      g_frame.s_raw_data[118] = 'a';
      g_frame.s_raw_data[119] = 'a';
      g_frame.s_raw_data[120] = 'a';
      g_frame.s_raw_data[121] = 'a';
      g_frame.s_raw_data[122] = 'a';
      g_frame.s_raw_data[123] = 'a';
   }
   */

   return serial_dev_nwrite((void*)g_frame.s_raw_data, g_frame.s_len);
}

int frame_ack_ok()//only receiver --> sender
{
   /*|4byte  |2   |2  |1 |1     |2byte   |    ack frame len of sum = 12
    *|header |len |sn |op|status|checksum|
    *|k|l|j|c|0|12|x|x|3 |1     |xx|xx   |
    */

   frame_set_header();
   frame_set_len(12);
   frame_set_seq_number(g_frame.r_seq_number_in_raw_data);//接受端受到什么编号就回什么编号
   frame_set_op(ACK);
   frame_set_status(1);
   frame_calculate_send_frame_checksum();
   frame_set_checksum();

   return serial_dev_nwrite((void*)g_frame.s_raw_data, g_frame.s_len);
}

int frame_ack_not_ok()//only receiver --> sender
{
   /*|4byte  |2   |2  |1 |1     |2byte   |    ack frame len of sum = 12
    *|header |len |sn |op|status|checksum|
    *|k|l|j|c|0|12|x|x|3 |0     |xx|xx   |
    */

   frame_set_header();
   frame_set_len(12);
   frame_set_seq_number(g_frame.r_last_seq_number);
   frame_set_op(ACK);
   frame_set_status(0);
   frame_calculate_send_frame_checksum();
   frame_set_checksum();

   return serial_dev_nwrite((void*)g_frame.s_raw_data, g_frame.s_len);
}

int frame_finish(char *md5)
{
   /*|4byte  |2   |2  |1 |32   |2byte   |         finish frame len of sum = 54
    *|header |len |sn |op|md5  |checksum|
    *|k|l|j|c|0|43|0|0|4 |x... |xx  |xx |
    */

   int md5_pos;

   frame_set_header();
   frame_set_len(43);
   frame_set_seq_number(0);//发完了就是发完了，不需要检查包编号，为了统一处理保留字段并设置为0
   frame_set_op(FINISH);

   for (md5_pos = 0; md5_pos < 32; md5_pos++)
      g_frame.s_raw_data[9 + md5_pos] = md5[md5_pos];

   frame_calculate_send_frame_checksum();
   frame_set_checksum();

   return serial_dev_nwrite((void*)g_frame.s_raw_data, g_frame.s_len);
}

int frame_err(int err)
{
   /*|4byte  |2   |2  |1 |1  |2byte|      err frame len of sum = 12
    *|header |len |sn |op|err|checksum
    *|k|l|j|c|0|12|0|0|5 |e  |xx|xx|
    */
   frame_set_header();
   frame_set_len(12);
   frame_set_seq_number(0);
   frame_set_op(ERR);
   g_frame.s_raw_data[9] = err;
   frame_calculate_send_frame_checksum();
   frame_set_checksum();

   return serial_dev_nwrite((void*)g_frame.s_raw_data, g_frame.s_len);
}

int frame_get_op_from_raw_data()
{
   g_frame.r_op = g_frame.r_raw_data[8];
   return g_frame.r_op;
}

int frame_get_err_from_raw_data()
{
   g_frame.r_err_code = g_frame.r_raw_data[9];
   return g_frame.r_err_code;
}

int frame_get_status_from_raw_data()
{
   g_frame.r_status = g_frame.r_raw_data[9];
   return g_frame.r_status;
}

int frame_seq_number_check()
{
   int ok = 0;

   frame_get_seq_number_from_raw_data();
   if((g_frame.r_op == REQ || g_frame.r_op == DATA) && g_frame.r_seq_number_in_raw_data == (g_frame.r_last_seq_number + 1)) {//接收端使用
      g_frame.r_last_seq_number++;//TODO 这里做好像不合适, 但却是需要检查包的checksum，序列号合法后才能确认受到了这个包
      ok = 1;
   }
   else if(ACK == g_frame.r_op && g_frame.r_seq_number_in_raw_data == g_frame.s_last_seq_number)//发送端使用
   {
      ok = 1;
   }else {
      ok = 1;
   }

   return ok;
}

int frame_get_seq_number_from_raw_data()
{
   g_frame.r_seq_number_in_raw_data = g_frame.r_raw_data[6] << 8 | g_frame.r_raw_data[7];
   return g_frame.r_seq_number_in_raw_data;
}

int frame_checksum_check()
{
   int ok = 0;

   frame_calculate_receive_frame_checksum();
   if(!memcmp((void*)g_frame.r_checksum, (void*)(g_frame.r_raw_data + g_frame.r_len - 2), 2)) {
      ok = 1;
   } else {
      printf("Check sum not ok\n");
   }

   return ok;
}

void frame_calculate_receive_frame_checksum()
{
   getCRC(g_frame.r_raw_data, g_frame.r_len - 2, g_frame.r_checksum);
}

void frame_calculate_send_frame_checksum()
{
   getCRC(g_frame.s_raw_data, g_frame.s_len - 2, g_frame.s_checksum);
}

int frame_get_frame()
{
   frame_get_frame_header();
   frame_get_frame_len();
   frame_get_remainder_frame();
   return 0;
}

int frame_get_remainder_frame()
{
   return serial_dev_nread((void*)g_frame.r_raw_data + 6, g_frame.r_len - 6);
}

int frame_get_frame_len()
{
   int len;

   serial_dev_nread((void*)(g_frame.r_raw_data + 4), 2);
   len = g_frame.r_raw_data[4] << 8 | g_frame.r_raw_data[5];
   if(len > MTU) {
      //throw;
   }

   g_frame.r_len = len;
   return len;
}

void frame_get_frame_header()
{
   unsigned char c = 0;
come_on:
   while('k' != c) {
      serial_dev_nread((void*)&c, 1);
   }
   serial_dev_nread((void*)&c, 1);
   if('l' != c)
      goto come_on;
   serial_dev_nread((void*)&c, 1);
   if('j' != c)
      goto come_on;
   serial_dev_nread((void*)&c, 1);
   if('c' != c)
      goto come_on;

   g_frame.r_raw_data[0] = 'k';
   g_frame.r_raw_data[1] = 'l';
   g_frame.r_raw_data[2] = 'j';
   g_frame.r_raw_data[3] = 'c';
}

void frame_decode_frame()
{
   frame_get_op_from_raw_data();
   if (ERR == g_frame.r_op) {
      frame_get_err_from_raw_data();
   }
   if (ACK == g_frame.r_op) {
      frame_get_status_from_raw_data();
   }
   frame_get_seq_number_from_raw_data();
   memcpy((void*)g_frame.r_checksum, (void*)(g_frame.r_raw_data + g_frame.r_len - 2), 2);
}

int frame_get_data_len()
{
   return g_frame.r_len - 11;
}

void* frame_get_data_pos()
{
   return (void*)(g_frame.r_raw_data + 9);
}

int u_frame_get_op()
{
   return g_frame.r_op;
}

const char* u_frame_get_md5_from_finish_pkt(char *md5)
{
   int md5_pos;
   for (md5_pos = 0; md5_pos < MD5_LEN; md5_pos++)
      md5[md5_pos] = g_frame.r_raw_data[9 + md5_pos];

   printf("get md5 from pkt = %s\n", md5);
   return md5;
}
