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

#define SYS_sem_init 50
#define SYS_sem_post 51
#define SYS_sem_wait 52
#define SYS_sem_destroy 53

int sem_init(sem_t *sem, uint32_t value)
{
	return syscall(SYS_sem_init, sem, value);
}

int sem_post(sem_t *sem)
{
	return syscall(SYS_sem_post, sem);
}

int sem_wait(sem_t *sem)
{
	return syscall(SYS_sem_wait, sem);
}

int sem_destroy(sem_t *sem)
{
	return syscall(SYS_sem_destroy, sem);
}

