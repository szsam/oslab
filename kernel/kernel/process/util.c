#include "process.h"

extern PCB procTbl[2];

void initProc(uint32_t entry) {
	procTbl[0].state = RUNNABLE;

	procTbl[0].tf = (struct TrapFrame *)
			(procTbl[0].kstack + KSTACK_SIZE - 128);

	procTbl[0].tf->ds = USEL(SEG_UDATA);
	procTbl[0].tf->es = USEL(SEG_UDATA);
	procTbl[0].tf->fs = USEL(0);
	procTbl[0].tf->gs = USEL(0);

	procTbl[0].tf->cs = USEL(SEG_UCODE);
	procTbl[0].tf->eip = entry;
	procTbl[0].tf->eflags = 0x202;		// set IF
	procTbl[0].tf->ss = USEL(SEG_UDATA);
	procTbl[0].tf->esp = 0x210000;


	procTbl[1].state = DEAD;
}
