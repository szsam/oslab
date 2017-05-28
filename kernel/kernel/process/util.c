#include "process.h"

#define NR_PROCESS 4
PCB pcbPool[NR_PROCESS];

ListHead ready, block, free;

void initPCBpool() {
	list_init(&free);
	for (int i = 1; i < NR_PROCESS; i++)
	{
		list_add_before(&free, &pcbPool[i].list);
	}
}

void initProc(uint32_t entry) {
	idle.state = BLOCKED;

	// initialize free PCB pool
	initPCBpool();

	// the first ready process
	list_init(&ready);
	list_add_after(&ready, &pcbPool[0].list);

	// block queue
	list_init(&block);

	pcbPool[0].state = RUNNABLE;

	pcbPool[0].tf = (struct TrapFrame *)
			(pcbPool[0].kstack + KSTACK_SIZE - 128);

	pcbPool[0].tf->ds = USEL(SEG_UDATA);
	pcbPool[0].tf->es = USEL(SEG_UDATA);
	pcbPool[0].tf->fs = USEL(0);
	pcbPool[0].tf->gs = USEL(0);

	pcbPool[0].tf->cs = USEL(SEG_UCODE);
	pcbPool[0].tf->eip = entry;
	pcbPool[0].tf->eflags = 0x202;		// set IF
	pcbPool[0].tf->ss = USEL(SEG_UDATA);
	pcbPool[0].tf->esp = 0x210000;

	pcbPool[0].segBase = 0;

}
