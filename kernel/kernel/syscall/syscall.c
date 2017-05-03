#include "x86.h"
#include "device.h"
#include "process.h"
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
	return 0;
}

static int sys_exit() {
	current->state = DEAD;
	return 0;
}

extern PCB procTbl[2];
void *memcpy(void *dest, const void *src, size_t n);

static int do_fork() {
	// copy address space
	memcpy((void *)0x300000, (void *)0x200000, 0x100000);

	procTbl[1].state = procTbl[0].state;

	procTbl[1].tf = (struct TrapFrame *)
			(procTbl[1].kstack + KSTACK_SIZE - 128);

	procTbl[1].tf->ds = procTbl[0].tf->ds;
	procTbl[1].tf->es = procTbl[0].tf->es;
	procTbl[1].tf->fs = procTbl[0].tf->fs;
	procTbl[1].tf->gs = procTbl[0].tf->gs;

	procTbl[1].tf->cs = procTbl[0].tf->cs;
	procTbl[1].tf->eip = procTbl[0].tf->eip;
	procTbl[1].tf->eflags = procTbl[0].tf->eflags;
	procTbl[1].tf->ss = procTbl[0].tf->ss;
	procTbl[1].tf->esp = procTbl[0].tf->esp;

	procTbl[1].tf->eax = 0;
	procTbl[1].tf->ebx = procTbl[0].tf->ebx;
	procTbl[1].tf->ecx = procTbl[0].tf->ecx;
	procTbl[1].tf->edx = procTbl[0].tf->edx;
	procTbl[1].tf->esi = procTbl[0].tf->esi;
	procTbl[1].tf->edi = procTbl[0].tf->edi;
	procTbl[1].tf->esp = procTbl[0].tf->esp;
	procTbl[1].tf->ebp = procTbl[0].tf->ebp;

	procTbl[1].segBase = 0x100000;
	return 1;
}

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
		default:	// Unhandled system call
			assert(0);
			break;
	}
}
