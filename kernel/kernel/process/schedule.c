#include "process.h"
#include "device.h"
#include "adt/list.h"

PCB idle, *current = &idle;


void
schedule(void) {
	/* implement process/thread schedule here */
	if (!list_empty(&ready))
	{
		if (current->state == RUNNABLE)
		{	// put current at the end of the ready queue
			list_del(&current->list);
			list_add_before(&ready, &current->list);
		}
		current = list_entry(ready.next, PCB, list);
	}
	else
		current = &idle;
	

	// for debug
#define NR_PROCESS 4
extern PCB pcbPool[NR_PROCESS];
	
	if (current == pcbPool + 0)
		putChar('0');
	else if (current == pcbPool + 1)
		putChar('1');
	else if (current == pcbPool + 2)
		putChar('2');
	else if (current == pcbPool + 3)
		putChar('3');
	else if (current == &idle)
		putChar('i');
	else
		putChar('e');

}
