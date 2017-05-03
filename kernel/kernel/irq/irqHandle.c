#include "x86.h"
#include "device.h"
#include "process.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void schedule();

void minus_sleep_time();

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* TODO Reassign segment registers */
//	asm volatile("movw %w0, %%ds" : : "r"(KSEL(SEG_KDATA)));
//	asm volatile("movw %w0, %%es" : : "r"(KSEL(SEG_KDATA)));

	// handle interrupts and exeptions
	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x20:
			//putChar('t');
			minus_sleep_time();
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}

//	/* TODO Restore segment registers */
//	asm volatile("movw %w0, %%ds" : : "r"(USEL(SEG_UDATA)));
//	asm volatile("movw %w0, %%es" : : "r"(USEL(SEG_UDATA)));

	// save the trap frame pointer for the old process
	current->tf = tf;

	// choose a runnable process by updating current, that is
	// current = choose_next_process();
	schedule();

	// set kernel stack for the new process
	set_tss_esp0((uint32_t)(current->tf + 1));
	// modify gdt for user's processes
	if (current != &idle) set_gdt_usr_seg_base(current->segBase);
}


void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}


void minus_sleep_time() {
	if (procTbl[0].state == BLOCKED) {
		--procTbl[0].sleepTime;
		if (procTbl[0].sleepTime == 0)
			procTbl[0].state = RUNNABLE;
	}
	if (procTbl[1].state == BLOCKED) {
		--procTbl[1].sleepTime;
		if (procTbl[1].sleepTime == 0)
			procTbl[1].state = RUNNABLE;
	}
}
