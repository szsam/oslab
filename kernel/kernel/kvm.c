#include "x86.h"
#include "device.h"
#include <elf.h>
//#include <string.h>
#include <stddef.h>

SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SECTSIZE 512
#define NR_SECT 15 // User's program occupies 13 sectors (201 to 213)
#define START_SECT 201

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
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


void initSeg() {
	// Initialize GDT
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt));

	/*
	 * TODO 初始化TSS
	 */
	tss.ss0 = KSEL(SEG_KDATA);
	tss.esp0 = 128 << 20;
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/* TODO 设置正确的段寄存器*/
	asm volatile("movw %w0, %%ds" : : "r"(KSEL(SEG_KDATA)));
	asm volatile("movw %w0, %%es" : : "r"(KSEL(SEG_KDATA)));
	asm volatile("movw %w0, %%ss" : : "r"(KSEL(SEG_KDATA)));
	asm volatile("ljmp %w0, $farjmp \n\t"
				 "farjmp: " : : "i"(KSEL(SEG_KCODE)));

	lLdt(0);
	
}

//void enterUserSpace(uint32_t entry) {
//	/*
//	 * Before enter user space 
//	 * you should set the right segment registers here
//	 * and use 'iret' to jump to ring3
//	 */
//	asm volatile("movw %w0, %%ds" : : "r"(USEL(SEG_UDATA)));
//	asm volatile("movw %w0, %%es" : : "r"(USEL(SEG_UDATA)));
//	
//	/* push %ss */
//	asm volatile("pushl %0" : : "i"(USEL(SEG_UDATA)));
//	/* push %esp */
//	asm volatile("pushl %0" : : "i"(127 << 20));
//	/* push eflags */
//	asm volatile("pushfl");
//	/* push %cs */
//	asm volatile("pushl %0" : : "i"(USEL(SEG_UCODE)));
//	/* push %eip */
//	asm volatile("pushl %0" : : "r"(entry));
//
//	asm volatile("iret");
//}

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

uint32_t loadUMain(void) {

	/*加载用户程序至内存*/
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph = NULL;

	uint8_t buf[NR_SECT * SECTSIZE];

	int ix;
	for (ix = 0; ix < NR_SECT; ix++)
		readSect(buf + ix * SECTSIZE, ix + START_SECT);

	elf = (void*)buf;

	/* TODO: fix the magic number with the correct one */
	const uint32_t elf_magic = 0x464c457f;
	uint32_t *p_magic = (void *)buf;
	assert(*p_magic == elf_magic);

	/* Load each program segment */
	ph = (Elf32_Phdr *)(buf + elf->e_phoff);
	for(ix = 0; ix < elf->e_phnum; ++ix, ++ph) {
		/* Scan the program header table, load each segment into memory */
		if(ph->p_type == PT_LOAD) {

			/* TODO: read the content of the segment from the ELF file 
			 * to the memory region [VirtAddr, VirtAddr + FileSiz)
			 */
			memcpy((void *)ph->p_vaddr, buf + ph->p_offset, ph->p_filesz);
			 
			/* TODO: zero the memory region 
			 * [VirtAddr + FileSiz, VirtAddr + MemSiz)
			 */
			if (ph->p_memsz > ph->p_filesz)
				memset((void *)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz-ph->p_filesz);

		}
	}

	return elf->e_entry;
}

inline void *memcpy(void *dest, const void *src, size_t n)
{
	char *cdest = (char *)dest;
	const char *csrc = (const char *)src;
	while (n-- > 0)
		*cdest++ = *csrc++;
	return dest;
}

inline void *memset(void *s, int c, size_t n)
{
	char *ps = (char *)s;
	while (n-- > 0)
		*ps = (char)c;
	return s;
}

