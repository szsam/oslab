#include "x86/memory.h"

.code32

.global irqEmpty
irqEmpty:
	pushl $0 // push dummy error code
	pushl $-1 // push interrupt vector into kernel
	jmp asmDoIrq

.global irqGProtectFault
irqGProtectFault:
	pushl $0xd
	jmp asmDoIrq

.global irqSyscall
irqSyscall:
	pushl $0 // push dummy error code
	pushl $0x80 // push interrupt vector into kernel stack
	jmp asmDoIrq

.global irqTimer
irqTimer:
	pushl $0 // push dummy error code
	pushl $0x20 // push interrupt vector into kernel stack
	jmp asmDoIrq

# the label "current" here is the memory address of variable current
.extern current
.extern irqHandle


.global asmDoIrq
asmDoIrq:
	cli
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs
	pushal

	movw $KSEL(SEG_KDATA), %ax
	movw %ax, %ds
	movw %ax, %es
	ljmp $KSEL(SEG_KCODE), $farjmp

farjmp:
	pushl %esp
	call irqHandle
	
# YOU NEED TO SWITCH STACK TO current->tf
# SO YOU NEED TO ADD TWO LINES OF INTERRUPT CODE
# HINT:
#     1. USE movl INSTRUCTION
#     2. USE (address) CAN REFERENCE MEMORY LOCATION OF address
#     3. YOU MAY FLUSH ANY GENRAL PURPOSE REGISTER AS YOU WISH
#     4. REGISTERS ARE REFERENCED BY "%", SUCH AS %esp

################### your work ###################
	movl (current), %eax
	movl (%eax), %esp

#################################################
	# addl $4, %esp # when you finish this task, this instruction should be removed

	popal
	popl %gs
	popl %fs
	popl %es
	popl %ds
	addl $8, %esp

	iret
