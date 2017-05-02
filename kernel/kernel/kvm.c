#include "x86.h"
#include "device.h"
//#include <string.h>

static SegDesc gdt[NR_SEGMENTS];
static TSS tss;

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

void set_tss_esp0(uint32_t esp) {
	tss.esp0 = esp;
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


