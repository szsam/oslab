# Lab3 Process Switching

## 1 Aims
Implement process context switching mechanism, implement simple task scheduling, and implement FORK, SLEEP, and EXIT system calls.

## 2 Steps
### 2.1 Process Context Switching
Looking back at our Lab2, whenever the user program initiates a system call through a trap instruction, the CPU will jump to the interrupt handler according to the IA-32 interrupt processing mechanism, save the system state information to the stack, and then handle the interrupt. Restore the system state based on the previously saved information, and finally return from the interrupt. In fact, as long as we slightly modify the code of the Lab2 interrupt handling, we can support multitasking!
``` asm
# ...... # other entry points for different interrupts and exceptions
irq0: pushl $1000; jmp asm_do_irq
asm_do_irq:
  pushal
  pushl %esp
  call irq_handle
  # switch %esp to other program
  popal
  addl $4, %esp
  iret
```

In asm_do_irq, the code will store the general-purpose registers of program A on the stack of A. The contents of these registers together with the previously saved #irq and the hardware-preserved EFLAGS, CS, EIP form the data of the "trap frame", which records the state of A before the interrupt arrives. Notice that the "trap frame" is on the stack. At this time, asm_do_irq passes A's stack top pointer as a parameter to the C function irq_handle.  After returning, if we don't hurry to restore A's state, but first switch the ESP register to B's stack, then B's state will be restored: Restore B's general-purpose register, pop-up #irq, Restore B's EIP, CS, EFLAGS. After returning from the interrupt, we are already running program B!

Where did program A go? Don't worry, it is only temporarily suspended. Before it is suspended, it has saved the state information on its own stack. If ESP is switched, at some point in the future, to the stack of A, the code will restore the state of A according to the information saved before by A (the "trap frame" of A), and A will wake up and execute.

Each time a context switch is made, the stack location in the TSS is set to the kernel stack of the new process via the set_tss_esp0 function (defined in src/kernel/memory/kvm.c).

The base address of the user data segment and the user code segment is modified by the set_gdt_usr_seg_base function each time the context switch is performed.

### 2.2 Process Model
We have already learned about the concept of the process in the theory class. A process is a running program. Although this sentence contains only a small amount of information, the concept of the process is of vital importance in operating system: the user program runs on OS as a process. OS inherently serves processes. But the process is not born to exist: there is no concept of the process after the system starts. Thus, it needs to be created by the operating system.

To facilitate the management of processes, the operating system uses a data structure called a Process Control Block (PCB) and maintains a PCB for each process.
``` C
#define KSTACK_SIZE 4096
typedef struct PCB {
    struct TrapFrame *tf;
    uint8_t kstack[KSTACK_SIZE];
    enum {RUNNABLE, BLOCKED, DEAD} state;
    int sleepTime;
    uint32_t segBase;
} PCB;
```
Tf is a pointer to the process "trap frame"; kstack is the kernel stack of the process; state is the state of the process, RUNNABLE indicates the ready state, BLOCKED indicates the blocking state, DEAD indicates the termination state; sleepTime records the sleep event of the process; segBase is the base address of the process data segment and code segment.

### 2.3 Create First Process
To create the first process, we only need to artificially initialize a "trap frame" on the process's stack, and let the PCB's tf pointer point to it, so that in the future switching, the artificial "trap frame" can be used to properly restore the state.

Initializing a correct "trap frame" requires consideration of various issues. Specifically, how to initialize each field in the "trap frame":

* General purpose register
* irq, error_code - will not be used when restoring the state, so it can be ignored during initialization
* cs - need to be set to the correct code segment
* ds, es, fs, gs - need to be set to the correct data segment
* eflags - Pay particular attention to the IF bit in it, as it will determine if the CPU responds to an external interrupt. If the settings are incorrect and the state is restored, the context switch will not occur again!
* eip - interrupt return address. should be initialized to the user program's entry address
* ss, esp - the segment selector and stack top pointer of the user stack

### 2.4 Schedule
With the PCB, we can schedule between different processes. The kernel maintains a global pointer `current` to indicate the currently running process. The job of scheduling is to select a ready process as the target for context switching.

``` asm
asm_do_irq:
  pushal
  pushl %esp
  call irq_handle
  # %esp = current->tf
  popal
  addl $4, %esp
  iret
```
-----------------------------------------
``` C
void irq_handle(TrapFrame *tf) {
  // handle interrupts and exeptions
  // ...

  // save the trap frame pointer for the old process
  current->tf = tf;

  // choose a runnable process by updating current, that is
  // current = choose_next_process();
  schedule();
}
```
We can use the Round Robin strategy, that is, schedule process 1, process 2, ..., process n, process 1...

Finally, we mention the IDLE thread. When there are no other ready threads to schedule, the system should select the IDLE thread for scheduling. The IDLE thread does nothing, waiting for the next interrupt. In oslab, we don't need to manually create the IDLE thread. After the kernel performs a series of initialization, it will enable the interrupt. At this point, the execution flow changes and becomes the IDLE thread, waiting for the interrupt to arrive.

### 2.5 System Calls
#### 2.5.1 fork()
The FORK system call is used to create a child process. The kernel needs to allocate a separate memory for the child process, completely copy the address space and user state stack of the parent process into the memory of the child process, allocate independent process control blocks for the child process, and complete the setting of the process control block for the child process.

The PCB of the child process is almost the same as the PCB of the parent process, but the following fields are different:

* The tf of the child process should point to the "trap frame" located in the kernel stack of the child process.
* The eax in the child process TrapFrame is 0, because the return value of the fork function for the child process is 0.
* The segment base address of the child process must be different from the segment base address of the parent process

#### 2.5.2 sleep()
Set the state of the process to blocking state (BLOCKED) and fill the sleepTime field in the PCB. Each time a clock interrupt arrives, all sleepTimes in the blocked state are decremented by one. If a process's sleepTime is reduced to 0, it is placed back into the ready queue so that it can be scheduled again.

#### 2.5.3 exit()
Set the state of the process to the state of termination (DEAD) so that the scheduler does not schedule the process again.

## References
[1] http://cslabcms.nju.edu.cn/ics/index.php/os:2012/lab1  
[2] INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986.  
[3] Using the GNU Compiler Collection. For gcc version 5.4.0.

------------------------------------
# Lab3 进程切换 实验报告

## 1 实验进度
完成了所有实验内容。成功实现进程上下文切换机制，实现简单的任务调度，实现FORK、SLEEP、EXIT系统调用。

## 2 实验步骤
### 2.1 上下文切换
回顾我们的Lab2, 每当用户程序通过陷阱指令发起系统调用的时候, CPU将会按照IA-32中断处理机制跳转到中断处理程序, 把现场的信息保存到堆栈上之后对中断进行处理, 然后根据之前保存的信息恢复现场, 最后从中断返回. 事实上, 我们只要对Lab2中断处理的代码稍作修改, 我们就可以支持多任务了!
``` asm
# ...... # other entry points for different interrupts and exceptions
irq0: pushl $1000; jmp asm_do_irq
asm_do_irq:
  pushal
  pushl %esp
  call irq_handle
  # switch %esp to other program
  popal
  addl $4, %esp
  iret
```
在asm_do_irq中, 代码将会把A的通用寄存器保存到A的堆栈上, 这些寄存器的内容连同之前保存的#irq和硬件保存的EFLAGS, CS, EIP形成了"陷阱帧"(trap frame)的数据结构, 它记录了A在中断到来之前的状态. 注意到"陷阱帧"是在堆栈上的, 此时asm_do_irq将A的栈顶指针作为参数传给C函数irq_handle. 神奇的地方来了, 从irq_handle返回后, 如果我们先不着急恢复A的现场, 而是先将ESP寄存器切换到B的堆栈上, 接下来的恢复现场操作将会恢复成B的现场: 恢复B的通用寄存器, 弹出#irq, 恢复B的EIP, CS, EFLAGS. 从中断返回之后, 我们已经在运行程序B了!

那程序A到哪里去了呢? 别担心, 它只是被暂时挂起了而已. 在被挂起之前, 它已经把现场的信息保存在自己的堆栈上了, 如果将来的某一时刻ESP被切换到A的堆栈上, 代码将会根据A之前保存的信息(A的"陷阱帧")恢复A的现场, A将得以唤醒并执行.

在每次进行上下文切换的时候, 还要通过set_tss_esp0函数(在src/kernel/memory/kvm.c中定义)将TSS中的堆栈位置设置成新进程的内核栈.

在每次进行上下文切换的时候, 还要通过set_gdt_usr_seg_base函数，修改用户数据段和用户代码段的基地址。

### 2.2 进程模型
我们已经在理论课上听说过进程的概念了, 进程就是正在运行的程序, 虽然这句话包含的信息量不多, 但进程确实在操作系统中占有举足轻重的地位: 用户程序以进程的身份运行在操作系统之上, 因此操作系统天生就是为进程提供服务的. 但进程并不是天生就有的, 系统启动之后并没有进程的概念, 它需要由操作系统来创建.

为了方便对进程进行管理, 操作系统使用一种叫进程控制块(PCB)的数据结构, 并为每一个进程维护一个PCB.
``` C
#define KSTACK_SIZE 4096
typedef struct PCB {
    struct TrapFrame *tf;
    uint8_t kstack[KSTACK_SIZE];
    enum {RUNNABLE, BLOCKED, DEAD} state;
    int sleepTime;
    uint32_t segBase;
} PCB;
```
tf是指向该进程"陷阱帧"的指针；kstack是该进程的核心栈；state是进程的状态，RUNNABLE表示就绪态，BLOCKED表示阻塞态，DEAD表示终止态；sleepTime记录进程的休眠事件；segBase是该进程数据段和代码段的基地址。

### 2.3 创建第一个进程
为创建第一个进程，我们只需要在进程的堆栈上人为初始化一个"陷阱帧", 并让PCB的tf指针指向它, 使得将来切换的时候可以根据这个人工"陷阱帧"来正确地恢复现场即可.

初始化一个正确的"陷阱帧"还需要考虑各种问题, 具体来说, 就是如何初始化"陷阱帧"中的每一个域:

* 通用寄存器
* irq, error_code - 恢复现场的时候不会用到, 因此初始化的时候可以忽略
* cs - 需要设置成正确的代码段
* ds, es, fs, gs - 需要设置成正确的数据段
* eflags - 尤其需要注意其中的IF位, 因为它将决定CPU是否响应外部中断. 如果设置错误, 恢复了现场之后, 上下文切换将无法再次发生!
* eip - 中断返回地址, 应该初始化为用户程序的入口地址
* ss, esp - 用户栈的段选择符和栈顶指针

### 2.4 简单的调度
有了PCB之后, 我们就可以在不同的线程之间进行调度了. 内核通过维护一个全局指针current来指示当前运行的线程, 调度的工作就是选择一个就绪的线程作为上下文切换的目标.
``` asm
asm_do_irq:
  pushal
  pushl %esp
  call irq_handle
  # %esp = current->tf
  popal
  addl $4, %esp
  iret
```
-----------------------------------------
``` C
void irq_handle(TrapFrame *tf) {
  // handle interrupts and exeptions
  // ...

  // save the trap frame pointer for the old process
  current->tf = tf;

  // choose a runnable process by updating current, that is
  // current = choose_next_process();
  schedule();
}
```
我们采用轮转调度(Round Robin)的策略即可, 即依次调度进程1, 进程2, ..., 进程n, 进程1...

最后要提一下IDLE线程. 当没有其它就绪线程可以进行调度的时候, 系统应该选择IDLE线程进行调度. IDLE线程什么也不做, 等待下一次中断的到来. 在oslab中不需要手动创建IDLE线程.  内核执行一系列的初始化工作后将会打开中断, 此时执行流摇身一变, 成为了IDLE线程, 等待中断的到来.

### 2.5 系统调用
#### 2.5.1 fork()
FORK系统调用用于创建子进程，内核需要为子进程分配一块独立的内存，将父进程的地址空间、用户态堆栈完全拷贝至子进程的内存中，并为子进程分配独立的进程控制块，完成对子进程的进程控制块的设置。

子进程的PCB和父进程的PCB几乎一样，但以下域不同：

* 子进程的tf应指向位于子进程内核栈的"陷阱帧"
* 子进程TrapFrame中的eax是0，因为fork函数对于子进程的返回值是0
* 子进程的段基地址与父进程的段基地址必须不同

#### 2.5.2 sleep()
将进程的状态设置为阻塞态(BLOCKED), 并填充PCB中的sleepTime域。每次时钟中断到来时，将所有处于阻塞态进程的sleepTime减一，如果某个进程的sleepTime减为0，则将其重新放入就绪队列，使它能够再次被调度。

#### 2.5.3 exit()
将进程的状态设置为终止态(DEAD)即可，这样调度程序不会再调度该进程。

## 3 实验结果
![](oslab3-result.png)

## 参考资料
[1] http://cslabcms.nju.edu.cn/ics/index.php/os:2012/lab1  
[2] INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986.  
[3] Using the GNU Compiler Collection. For gcc version 5.4.0.
