#include "process.h"
#include "device.h"

PCB idle, *current = &idle;
PCB procTbl[2];

void
schedule(void) {
	/* implement process/thread schedule here */
	if (procTbl[0].state == RUNNABLE && procTbl[1].state == RUNNABLE) {
		if (current == procTbl) {
			current = procTbl + 1;
		}
		else
			current = procTbl;
	}
	else if (procTbl[0].state == RUNNABLE) {
		current = procTbl;
	}
	else if (procTbl[1].state == RUNNABLE) {
		current = procTbl + 1;
	}
	else {
		current = &idle;
	}

	// output schedule info for debugging
	if (current == procTbl)
		putChar('0');	
	else if (current == procTbl + 1)
		putChar('1');
	else
		putChar('i');
}
