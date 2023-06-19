#include "file.h"
#include "file_exception.h"
#include "Md5.h"
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int file_fd;

void file_open(const char *file_name, int mode)
{
	CEXCEPTION_T e = -1;

   Try {
	   if(OPEN_FILE_FOR_READ == mode)
	      file_fd = open(file_name, O_RDONLY);
	   else if (OPEN_FILE_FOR_WRITE == mode)
         file_fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC);
	   if (file_fd < 0) {
	      printf("open file(%s) fail\n", file_name);
	      Throw(1);//TODO how to known sender or receiver?
	      exit(-1);
	      //TODO throw
	   }
   }Catch(e)
	{
		file_exception_handler(e, file_name);
	}
}

void file_close()
{
   if(-1 == file_fd) return;
   close(file_fd);
   file_fd = -1;
}

int file_nread(void *buf, int size)
{
   static int n=0;
   n++;
   int ret = 0;
   int cnt = 0;

   memset(buf, 0, size);

   while (cnt < size) {
      ret = read(file_fd, buf + cnt, size - cnt);
      if (-1 == ret) {
         //TODO throw
      }
      if (0 == ret) {//file end
         return cnt;
      }
      cnt += ret;
   }

   return cnt;
}

int file_nwrite(void *buf, int size)
{
   int ret = 0;
   int cnt = 0;

   while (cnt < size) {
      ret = write(file_fd, buf + cnt, size - cnt);
      if (-1 == ret) {
         //TODO throw
      }
      else if (0 == ret) {
      }
      cnt += ret;
   }
   return cnt;
}

size_t file_get_file_size(const char *file)
{
   size_t size = 0;

   FILE *fp = fopen(file, "r");
   fseek(fp, 0L, SEEK_END);
   size = ftell(fp);
   fclose(fp);

   return size;
}

void file_get_file_MD5(const char *file_name, char *md5)
{
   MDFile(file_name, md5);
}

int check_file_md5(const char *file, const char *expected_md5)
{
   char md5[33] = { 0 };
   file_get_file_MD5(file, md5);
   return (!strcmp(md5, expected_md5));
}
