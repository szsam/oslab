#include "x86.h"
#include "device.h"
#include "process.h"
#include "semaphore.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define VIDEO_MEMORY_ADDR 0xb8000

static void write_video_memory(const char *buf, size_t len) {
	static int row = 0;
	static int col = 0;

	for (size_t i = 0; i < len; i++) {
		if (buf[i] == '\n') { 
			col = 0; row ++; 
		}
		else {
			int pos = (80*row+col)*2;
			*(uint16_t *)(VIDEO_MEMORY_ADDR + pos) = (0xc << 8) | buf[i];
			++col;
			if (col == 80) { col = 0; row ++; }
		}

	}

}

static ssize_t sys_write(int fd, const void *buf, size_t len) {
	buf = (char *)buf + current->segBase;
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		write_video_memory(buf, len);
		return len;
	}
	else {
		return -1;
	}
}

#define SYS_sleep 162
static int sys_sleep(uint32_t time) {
	current->sleepTime = time;
	current->state = BLOCKED;
	// remove current from the ready queue
	list_del(&current->list);
	// add current to block queue
	list_add_before(&block, &current->list);
	return 0;
}

static int sys_exit() {
	current->state = DEAD;
	list_del(&current->list);
	list_add_before(&free, &current->list);
	return 0;
}

void *memcpy(void *dest, const void *src, size_t n);

static int do_fork() {
	putChar('f');
	// copy address space
	memcpy((void *)0x300000, (void *)0x200000, 0x100000);

	ListHead *child = free.next;
	list_del(child);
	list_add_before(&ready, child);
	PCB *child_pcb = list_entry(child, PCB, list);

	child_pcb->state = current->state;

	child_pcb->tf = (struct TrapFrame *)
			(child_pcb->kstack + KSTACK_SIZE - 128);

	child_pcb->tf->ds = current->tf->ds;
	child_pcb->tf->es = current->tf->es;
	child_pcb->tf->fs = current->tf->fs;
	child_pcb->tf->gs = current->tf->gs;

	child_pcb->tf->cs = current->tf->cs;
	child_pcb->tf->eip = current->tf->eip;
	child_pcb->tf->eflags = current->tf->eflags;
	child_pcb->tf->ss = current->tf->ss;
	child_pcb->tf->esp = current->tf->esp;

	child_pcb->tf->eax = 0;
	child_pcb->tf->ebx = current->tf->ebx;
	child_pcb->tf->ecx = current->tf->ecx;
	child_pcb->tf->edx = current->tf->edx;
	child_pcb->tf->esi = current->tf->esi;
	child_pcb->tf->edi = current->tf->edi;
	child_pcb->tf->esp = current->tf->esp;
	child_pcb->tf->ebp = current->tf->ebp;

	child_pcb->segBase = 0x100000;
	return 1;
}

#define SYS_sem_init 50
#define SYS_sem_post 51
#define SYS_sem_wait 52
#define SYS_sem_destroy 53

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	switch(tf->eax) {
		case SYS_write:
			tf->eax = sys_write(tf->ebx, (void *)tf->ecx, tf->edx);
			break;
		case SYS_sleep:
			tf->eax = sys_sleep(tf->ebx);
			break;
		case SYS_exit:
			tf->eax = sys_exit();
			break;
		case SYS_fork:
			tf->eax = do_fork();
			break;
		case SYS_sem_init:
			tf->eax = sys_sem_init((sem_t *)tf->ebx, tf->ecx);
			break;
		case SYS_sem_post:
			tf->eax = sys_sem_post((sem_t *)tf->ebx);
			break;
		case SYS_sem_wait:
			tf->eax = sys_sem_wait((sem_t *)tf->ebx);
			break;
		case SYS_sem_destroy:
			tf->eax = sys_sem_destroy((sem_t *)tf->ebx);
			break;
		default:	// Unhandled system call
			assert(0);
			break;
	}
}
