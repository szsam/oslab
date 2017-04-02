#include "boot.h"

#define SECTSIZE 512
#define KER_START_ADDR 0x100000

void bootMain(void) {
	/* 加载内核至内存，并跳转执行 */
	// Kernel occupies 200 sectors (1 to 200), located at memory address 0x100000
	int i;
	for (i = 0; i < 200; i++)
		readSect((void *)(KER_START_ADDR + SECTSIZE * i), i + 1);

	ELFHeader *elfhdr = (ELFHeader *)KER_START_ADDR;
	// jump to the entry point of kernel
	((void(*)())elfhdr->entry) ();
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}
