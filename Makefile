target:send recv
send:checksum.c checksum.h main_send.c serial_dev.c serial_dev.h file.h file.c frame.c frame.h CException.h CException.c file_exception.h file_exception.c ui.h ui.c timer.h timer.c
	/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/bin/gcc checksum.c main_send.c serial_dev.c file.c frame.c file_exception.c CException.c ui.c Md5.c timer.c -g -Wall -o mips_update_client

recv:checksum.c checksum.h main_send.c serial_dev.c serial_dev.h file.h file.c frame.c frame.h CException.h CException.c file_exception.h file_exception.c ui.h ui.c timer.h timer.c
	/opt/crosstool/mipsel-hardfloat-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.28.10-sanitized/bin/mipsel-glibc-linux-gnu-gcc checksum.c main_recv.c serial_dev.c file.c frame.c file_exception.c CException.c ui.c Md5.c timer.c -g -Wall -o mips_update_server

clean:
	rm mips_update_client mips_update_server
