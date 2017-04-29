#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {

	initSerial();// initialize serial port
	initIdt(); // initialize idt
	initIntr(); // initialize 8259a
	initTimer(); // initialize 8253
	initSeg(); // initialize gdt, tss
	enableInterrupt();	// sti
	loadUMain(); // load user program, enter user space

	while(1);
	assert(0);
}
