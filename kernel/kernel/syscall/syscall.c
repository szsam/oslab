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
		default:	// Unhandled system call
			assert(0);
			break;
	}
}
