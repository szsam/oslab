#include "process.h"

PCB idle, *current = &idle;
PCB procTbl[2];

void
schedule(void) {
	/* implement process/thread schedule here */
	current = procTbl;
}
