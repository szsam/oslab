#include "x86.h"
#include "device.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* TODO Reassign segment registers */
	asm volatile("movw %w0, %%ds" : : "r"(KSEL(SEG_KDATA)));
	asm volatile("movw %w0, %%es" : : "r"(KSEL(SEG_KDATA)));

	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}

	/* TODO Restore segment registers */
	asm volatile("movw %w0, %%ds" : : "r"(USEL(SEG_UDATA)));
	asm volatile("movw %w0, %%es" : : "r"(USEL(SEG_UDATA)));
}

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
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		write_video_memory(buf, len);
		return len;
	}
	else {
		return -1;
	}
}

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	switch(tf->eax) {
		case SYS_write:
			tf->eax = sys_write(tf->ebx, (void *)tf->ecx, tf->edx);
			break;
		default:	// Unhandled system call
			assert(0);
			break;
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
