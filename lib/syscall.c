#include "lib.h"
#include "types.h"
#include <stdarg.h>


#define SYS_exit 1
#define SYS_fork 2
#define SYS_write 4
#define SYS_sleep 162

int __attribute__((__noinline__))
syscall(int id, ...) {
	int ret;
	int *args = &id;
	asm volatile("int $0x80": "=a"(ret) : "a"(args[0]), "b"(args[1]), "c"(args[2]), "d"(args[3]));
	return ret;
}


ssize_t write(int fd, const void *buf, size_t len) {
	return syscall(SYS_write, fd, buf, len);
}

int fork() {
	return syscall(SYS_fork);
}

int sleep(uint32_t time) {
	return syscall(SYS_sleep, time);
}

int exit() {
	return syscall(SYS_exit);
}
