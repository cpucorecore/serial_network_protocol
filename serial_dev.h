#ifndef SERIAL_DEV_H_
#define SERIAL_DEV_H_

int serial_dev_init(const char *dev);//没有返回值，省去判断，只要函数没有抛异常表示一定成功
void serial_dev_fini();
int serial_dev_nread(void *buf, int size);//返回值表示实际读取的个数
int serial_dev_nwrite(void *buf, int size);
int get_fd();

#endif//SERIAL_DEV_H_
