#include "frame.h"
#include "file.h"
#include "global.h"
#include "Md5.h"
#include "timer.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void ctrl_c_handler(int s)
{
   printf("\n-----------------------s = %d\n\n", s);
   printf("ctrl c");
   file_close();
   frame_err(CTRL_C_SIG_ERR);
   frame_fini();
   exit(0);
}

void ignore(int s)
{
   printf("receive sb signal %d, ignore it\n", s);
}

void usage()
{
   printf("\n recv - serial file receiver.\n");
   printf(" Usage: recv [-h] [-f file_to_save] [-d serial_port]\n");
   printf("\t -h  print this help and exit\n");
   printf("\t -d <serial_port> select a serial port to send to\n");
   printf("\t -f <file_to_save> select a file to save\n");
}

void alarm_call_back_for_receiver(int sig)
{
   frame_err(TIME_OUT);
   file_close();
   frame_fini();
   exit(TIME_OUT);
}

int main(int argc, char *argv[])
{
   int opt;
   char *file_name = "/mnt/binfs/update/serial_update.tar";
   char *serial_device = "/dev/ttyUSB2";
   char *sh = "/bin/sh /mnt/binfs/update/mips_update.sh";

   while ((opt = getopt(argc, argv, "hf:d:")) != -1) {
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
      default:
         fprintf(stderr, "Illegal argument: \"%c\"\n", opt);
         return 1;
      }
   }

   frame_init(serial_device);
   timer_init(alarm_call_back_for_receiver);
   file_open(file_name, OPEN_FILE_FOR_WRITE);
   signal(SIGINT, ctrl_c_handler);
   signal(SIGTSTP, ignore);

   timer_reset_and_start(10);

   int ret;
   int errcode = NO_ERR;
   while (1) {
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
         switch (u_frame_get_op()) {
         case REQ:
            frame_ack_ok();
            timer_reset_and_start(5);
            break;
         case DATA:
            file_nwrite(frame_get_data_pos(), frame_get_data_len());
            frame_ack_ok();
            timer_reset_and_start(5);
            break;
         case FINISH:
            file_close();
            char md5[MD5_LEN + 1] = { 0 };
            if (!check_file_md5(file_name, u_frame_get_md5_from_finish_pkt(md5)))
               errcode = MD5_ERR;
            else
               timer_reset_and_start(360);
               errcode = system(sh);
            printf("result of running shell = %d\n", errcode);
            goto finish_process;
         case ERR:
            errcode = frame_get_err_from_raw_data();
            printf("receive err pkt, errcode = %d\n", errcode);
            goto finish_process;
         default:
            errcode = UNKNOWN_PKT_TYPE;
            goto finish_process;
         }
      }
   }

finish_process:
   frame_err(errcode);
   frame_fini();
   file_close();
   exit(errcode);
}
