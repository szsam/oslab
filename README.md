# Lab2 系统调用 实验报告

## 1 实验进度
完成所有实验内容。成功实现系统调用write和库函数printf。

## 2 实验步骤
### 2.1 bootloader
在start.S中，设置GDT，初始化段寄存器，并使处理器进入保护模式。在boot.c中，加载内核(kernel.elf)至内存，然后跳转到kernel的入口地址处执行。与lab1不同的是，在lab2中，内核是一个ELF文件。加载一个ELF文件的主要步骤是: 读取ELF头和程序头表，把每个需加载段从文件复制到内存。Kernel占200个扇区，即第1号扇区至第200号扇区。

值得一提的是，bootloader程序的大小不能超过一个扇区（512字节），这要求代码必须尽可能的精简。在编译bootloader时，我们可以使用`-Os`优化选项，使用这一优化选项能使编译器生成的机器代码的体积尽可能小。GCC手册有如下说明：
> -Os Optimize for size. '-Os' enables all '-O2' optimizations that do not typically increase code size. It also performs further optimizations designed to reduce code size.

### 2.2 初始化 GDT 表项和 TSS 段
在kernel中，重新设置GDT。新的GDT包含5个段：

* 内核态代码段
* 内核态数据段
* 用户态代码段
* 用户态数据段
* TSS 段

核心段的特权等级为0, 而用户段的特权等级为3.  重新设置GDT后，需要重新加载各段寄存器：( 位于kernel/kernel/kvm.c : initSeg() )·	
``` C
asm volatile("movw %w0, %%ds" : : "r"(KSEL(SEG_KDATA)));
asm volatile("movw %w0, %%es" : : "r"(KSEL(SEG_KDATA)));
asm volatile("movw %w0, %%ss" : : "r"(KSEL(SEG_KDATA)));
asm volatile("ljmp %w0, $farjmp \n\t"
       "farjmp: " : : "i"(KSEL(SEG_KCODE)));
```

TSS 段用于任务切换, 我们只需要关注其中的 ss0 和 esp0 两个值:
``` C
tss.ss0 = KSEL(SEG_KDATA);
tss.esp0 = 128 << 20;
asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));
```
我们把内核栈的基地址设为128MB.

### 2.3 加载用户程序
在框架代码中，内核被填充到了200个扇区的大小, 也就是说, 用户程序将存放在磁盘的第201个扇区的位置, 因此, 我们需要将从第201个扇区开始的用户程序加载到内存的相应位置, 这部分内容与bootloader加载内核的代码非常类似。用户程序约占15个扇区。

### 2.4 跳转到用户空间
我们不能直接跳转用户程序运行, 这里的跳转需要有特权级的转换. 并且需要为用户程序设置段寄存器。
#### 2.4.1 为用户设置段寄存器
 kernel/kernel/kvm.c : enterUserSpace()
``` C
asm volatile("movw %w0, %%ds" : : "r"(USEL(SEG_UDATA)));
asm volatile("movw %w0, %%es" : : "r"(USEL(SEG_UDATA)));
```
#### 2.4.2 用`iret` 指令实现特权级转化
为了能够实现特权级转换的跳转, 我们使用 `iret` 指令. 由于操作系统首先是运行在内核态的, 所以我们需要手动为 `iret` 做准备:

* SS 入栈
* ESP 入栈
* EFLAGS 入栈
* CS 入栈
* EIP 入栈
* IRET

具体的实现如下：
kernel/kernel/kvm.c : enterUserSpace()
``` C
/* push %ss */
asm volatile("pushl %0" : : "i"(USEL(SEG_UDATA)));
/* push %esp */
asm volatile("pushl %0" : : "i"(127 << 20));
/* push eflags */
asm volatile("pushfl");
/* push %cs */
asm volatile("pushl %0" : : "i"(USEL(SEG_UCODE)));
/* push %eip */
asm volatile("pushl %0" : : "r"(entry));

asm volatile("iret");
```
我们将用户栈的基地址设为127MB.

现在，我们给出qemu物理内存地址空间分布情况
![](address-space.png)

图1 物理内存地址空间分布图

### 2.5 中断处理
在kernel/kernel/irqHandle.c中的irqHandle()函数中，需将各段寄存器设置为内核态的段选择符；中断处理完成后，需将各段寄存器恢复为用户态的段选择符。

### 2.6  系统调用
用户进程通过`int $0x80`指令进行系统调用后，用户进程的所有现场信息都会保存在TrapFrame中了, 内核很容易获取它们. 

本次实验只需实现一个系统调用，即write系统调用。 模仿linux系统，write系统调用的原型为`ssize_t write(int fd, const void *buf, size_t len)`, 系统调用号为4。在本次实验中，只处理fd=STDOUT_FILENO的情况。（STDOUT_FILENO是一个宏，表示标准输出文件的文件描述符，其值为1）

内核通过直接写显存的方式，完成字符串的打印；内核还需维护当前输出位置这一信息。

### 2.7 库函数printf()
我们实现的printf()需支持`%d`, `%x`, `%s`,`%c`四种格式符。为了实现printf，我们需要使用可变参数（variable arguments）机制。实现的大致思路是：建立一个输出缓冲区，遍历格式化字符串中的每个字符，如果这一字符不是'%'，则直接将其复制到缓冲区；如果它是'%', 那么它的下一个字符是格式符，根据格式符的类型，从变长参数列表中取出参数，对其格式化，将结果存入缓冲区。遍历结束后，发起系统调用write，将缓冲区中的字符输出到屏幕。

## 3 实验结果
### 3.1 printf()输出结果
![](printf.png)

图2  printf()输出结果

### 3.2 从内核态切换到用户态
执行iret指令前，%esp指向内核栈的栈顶，%eip指向内核代码，%cs引用内核代码段（RPL=0），%ss引用内核数据段（RPL=0）。

![](kernel.png)

图3a 执行iret指令前，各寄存器的值

执行iret指令后，%esp指向用户栈的栈顶，%eip指向用户代码，%cs引用用户代码段（RPL=3），%ss引用用户数据段（RPL=3）。

![](user.png)

图3b 执行iret指令后，各寄存器的值

### 3.3 用户通过`int  $0x80`指令陷入内核态
执行`int  $0x80`指令前，%esp指向用户栈的栈顶，%eip指向用户代码，%cs引用用户代码段（RPL=3），%ss引用用户数据段（RPL=3）。系统调用的各参数及系统调用号存放在各个通用寄存器中。

![](before-int80.png)

图4a 执行`int $0x80`指令前，各寄存器的值

执行`int $0x80`指令后，%esp指向内核栈的栈顶，%eip指向内核代码，%cs引用内核代码段（RPL=0），%ss引用内核数据段（RPL=0）。

![](after-int80.png)

图4b 执行`int $0x80`指令后，各寄存器的值

## 参考资料
[1] 南京大学 计算机科学与技术系 计算机系统基础 课程实验 2016 <https://nju-ics.gitbooks.io/ics2016-programming-assignment/content/>   
[2] Stephen Prata. *C Primer Plus*. Sixth Edition. Addison-Wesley.  Chapter 16 The C Processor and the C library, variable arguments, pp 765-768.   
[3] *How to write my own printf() in C?* <http://stackoverflow.com/questions/1735236/how-to-write-my-own-printf-in-c>  
[4]INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986.  
[5] Using the GNU Compiler Collection. For gcc version 5.4.0.
