#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"
#include "x86/memory.h"
#include "adt/list.h"

#define KSTACK_SIZE 4096
typedef struct PCB {
		struct TrapFrame *tf;
		uint8_t kstack[KSTACK_SIZE];
		enum {RUNNABLE, BLOCKED, DEAD} state;
		int sleepTime;
		uint32_t segBase;

		ListHead list;
} PCB;

extern PCB *current;
// extern PCB procTbl[2];
extern PCB idle;
extern ListHead ready, block, free;

void initProc();
#endif
