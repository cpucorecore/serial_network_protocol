#ifndef FILE_H_
#define FILE_H_

#include <stdio.h>

enum {
   OPEN_FILE_FOR_READ,
   OPEN_FILE_FOR_WRITE,
};

void file_open(const char *file_name, int mode);
void file_close();
int file_nread(void *buf, int size);
int file_nwrite(void *buf, int size);

size_t file_get_file_size(const char *file);
void file_get_file_MD5(const char *file, char *md5);
int check_file_md5(const char *file, const char *expected_md5);

#endif//FILE_H_
