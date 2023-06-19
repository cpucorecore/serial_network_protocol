#include "frame.h"
#include "file.h"
#include "ui.h"
#include "Md5.h"
#include "timer.h"
#include "global.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void ctrl_c_handler(int s)
{
   printf("receive signal");
   file_close();
   frame_err(CTRL_C_SIG_ERR);
   frame_fini();
   exit(0);
}

void usage()
{
   printf("\n serial_send - serial file sender.\n");
   printf(" Usage: serial_send [-h] [-f file_to_send] [-d serial_port]\n");
   printf("\t -h  print this help and exit\n");
   printf("\t -d <serial_port> select a serial port to send to\n");
   printf("\t -f <file_to_send> select a file to send\n");
}

void alarm_call_back_for_sender(int sig)
{
   frame_err(TIME_OUT);
   file_close();
   frame_fini();
   exit(TIME_OUT);
}

int silence = 0;
#define UI_SHOW_INFO() silence? 1:ui_show()

int main(int argc, char *argv[])
{
   int opt;
   char *file_name = "/opt/keil/mips_update/serial_update.tar";
   char *serial_device = "/dev/ttyS3";
   float max_retry_percentage = 0.1;
   int max_retry_pkt = 0;

   while ((opt = getopt(argc, argv, "hf:d:p:s")) != -1) {
      switch (opt) {
      case 'h':
         usage();
         return 0;
      case 'f':
         file_name = optarg;
         break;
      case 'd':
         serial_device = optarg;
         printf("serial_device = %s\n", serial_device);
         break;
      case 'p':
         max_retry_percentage = (float)atof(optarg);
         printf("max_retry_percentage = %f\n", max_retry_percentage);
         break;
      case 's':
         silence = 1;
         break;
      default:
         fprintf(stderr, "Illegal argument: \"%c\"\n", opt);
         return 1;
      }
   }

   signal(SIGINT, ctrl_c_handler);
   frame_init(serial_device);
   timer_init(alarm_call_back_for_sender);
   ui_init(file_get_file_size(file_name));
   max_retry_pkt = max_retry_percentage * ui_get_pkt_num();
   file_open(file_name, OPEN_FILE_FOR_READ);

   frame_req();
   timer_reset_and_start(5);

   int ret;
   int op;
   int status;
   unsigned char data[MAX_DATA_LEN];
   int data_len;
   int errcode = NO_ERR;
   do
   {
      frame_get_frame();
      ret = frame_check();
      if (CHECKSUM_ERR == ret) {
         frame_ack_not_ok();
         timer_reset_and_start(5);
      }
      else if (SEQ_NUMBER_ERR == ret) {
         errcode = SEQ_NUMBER_ERR;
         goto finish_process;
      }
      else if (NO_ERR == ret) {
         op = u_frame_get_op();
         switch (op) {
         case ACK:
            status = frame_get_status_from_raw_data();
            if (1 == status) {
               data_len = file_nread((void*)data, MAX_DATA_LEN);
               if (0 == data_len) {
                  file_close();
                  char md5[MD5_LEN + 1] = { 0 };
                  file_get_file_MD5(file_name, md5);
                  printf("send md5 = %s\n", md5);
                  frame_finish(md5);
                  timer_reset_and_start(360);//设6分钟, 接收端升级较慢
                  break;
               }
               frame_data(data, data_len);
               timer_reset_and_start(5);
               ui_update_model_sended_size(data_len);
               UI_SHOW_INFO();
            }
            else if (0 == status) {
               if (ui_get_retry_pkt_cnt() > max_retry_pkt) {
                  frame_err(OUT_OF_MAX_RETRY_TIMES);
                  file_close();
                  frame_fini();
                  exit(OUT_OF_MAX_RETRY_TIMES);
               }
               ui_update_model_err_pkt_add_one();
               frame_data(data, data_len);
               timer_reset_and_start(5);
            }
            break;
         case ERR:
            errcode = frame_get_err_from_raw_data();
            if (0 == errcode) {
               printf("upgrade successful\n");
            }
            printf("receive err pkt, errcode = %d\n", errcode);
            goto finish_process;
         default:
            errcode = UNKNOWN_PKT_TYPE;
            goto finish_process;
         }
      }
   }while(1);

finish_process:
   frame_err(errcode);
   file_close();
   frame_fini();
   exit(errcode);
}
