#include "semaphore.h"
#include "adt/list.h"
#include "common.h"
#include "process.h"

typedef struct Semaphore
{
	int value;
	ListHead block;	// blocking queue
	ListHead list;
}Semaphore;

#define NR_SEM 4
Semaphore sem_pool[NR_SEM];

ListHead free_sem, head_sem;

void P(Semaphore *s)
{
	s->value--;
	if (s->value < 0)
	{
		list_del(&current->list);
		list_add_before(&s->block, &current->list);
		current->state = BLOCKED;
	}
}

void V(Semaphore *s)
{
	s->value++;
	if (s->value <= 0)
	{
		ListHead *tmp = s->block.next;
		list_del(tmp);
		list_add_before(&ready, tmp);
		list_entry(tmp, PCB, list)->state = RUNNABLE;
	}
}

void init_sem_pool()
{
	list_init(&head_sem);

	list_init(&free_sem);
	for (int i = 0; i < NR_SEM; i++)
	{
		list_add_before(&free_sem, &sem_pool[i].list);
	}
}

int sys_sem_init(sem_t *sem, uint32_t value)
{
	if (list_empty(&free_sem)) return -1;

	ListHead *tmp = free_sem.next;
	list_del(tmp);
	list_add_before(&head_sem, tmp);
	Semaphore *sp = list_entry(tmp, Semaphore, block);
	sp->value = value;
	list_init(&sp->block);

	sem = (sem_t *)((char *)sem + current->segBase);
	*sem = sp - sem_pool;
	return 0;
}

int sys_sem_post(sem_t *sem)
{
	sem = (sem_t *)((char *)sem + current->segBase);
	V(sem_pool + *sem);
	return 0;
}

int sys_sem_wait(sem_t *sem)
{
	sem = (sem_t *)((char *)sem + current->segBase);
	P(sem_pool + *sem);
	return 0;
}

int sys_sem_destroy(sem_t *sem)
{
	return 0;
}
