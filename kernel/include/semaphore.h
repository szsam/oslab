#ifndef __SEM_H__
#define __SEM_H__
#include <stdint.h>

typedef int sem_t;

int sys_sem_init(sem_t *sem, uint32_t value);
int sys_sem_post(sem_t *sem);
int sys_sem_wait(sem_t *sem);
int sys_sem_destroy(sem_t *sem);

void init_sem_pool();
#endif
