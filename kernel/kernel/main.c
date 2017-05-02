#include "common.h"
#include "x86.h"
#include "device.h"
#include "process.h"

void kEntry(void) {

	initSerial();// initialize serial port
	initIdt(); // initialize idt
	initIntr(); // initialize 8259a
	initTimer(); // initialize 8253
	initSeg(); // initialize gdt, tss
	uint32_t entry = loadUMain(); // load user program
	initProc(entry);	// initialize processes
	enableInterrupt();	// sti

	/* This context now becomes the idle process. */
	while(1) {
		waitForInterrupt();
	}

	assert(0);
}
