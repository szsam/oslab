#ifndef __lib_h__
#define __lib_h__

#include "types.h"

int printf(const char *format,...);
ssize_t write(int fd, const void *buf, size_t len);
int fork();
int sleep(uint32_t);
int exit();

int sem_init(sem_t *sem, uint32_t value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_destroy(sem_t *sem);

#endif
