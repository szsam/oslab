#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"
#include "x86/memory.h"

#define KSTACK_SIZE 4096
typedef struct PCB {
		struct TrapFrame *tf;
		uint8_t kstack[KSTACK_SIZE];
		enum {RUNNABLE, BLOCKED, DEAD} state;
} PCB;

extern PCB *current;

void initProc();
#endif
