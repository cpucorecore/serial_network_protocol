#include "serial_dev.h"
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

static int fd;

int serial_dev_init(const char *dev)
{
   struct termios opt;

   fd = open(dev, O_RDWR | O_NOCTTY);
   if (fd < 0) {
      printf("open dev(%s) err\n", dev);
      //TODO throw
      return -1;
   }
   printf("open dev(%s) ok\n", dev);

   tcgetattr(fd, &opt);
   cfmakeraw(&opt);
   cfsetispeed(&opt, B115200);
   cfsetospeed(&opt, B115200);
   tcsetattr(fd, TCSANOW, &opt);

   printf("serial_dev_init ok\n");
   return fd;
}

void serial_dev_fini()
{
   close(fd);
}

int serial_dev_nread(void *buf, int size)
{
   int ret = 0;
   int cnt = 0;

   memset(buf, 0, size);
   while (cnt < size) {
      ret = read(fd, (void*)(buf + cnt), size - cnt);
      if(0 == ret) {
         return cnt;
      }
      cnt += ret;
   }
   return cnt;
}

int serial_dev_nwrite(void *buf, int size)
{
   int ret = 0;
   int cnt = 0;

   while (cnt < size) {
      ret = write(fd, buf + cnt, size - cnt);
      if(-1 == ret) {
         //TODO throw
      }
      cnt += ret;
   }
   return cnt;
}

int get_fd()
{
   return fd;
}
