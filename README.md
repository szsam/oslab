# Lab4 进程同步 实验报告
151220091 沈明杰

## 1 实验进度
完成了所有实验内容。成功实现实现SEM_INIT、SEM_POST、SEM_WAIT、SEM_DESTROY系统调用。

## 2 实验步骤
结构体Semapore在`kernel/kernel/syscall/semaphore.c`中定义：
``` C
typedef struct Semaphore
{
  int value;
  ListHead block;	// blocking queue
  ListHead list;
}Semaphore;
```
value为信号量的值，block是阻塞在该信号量上的进程的队列，list用于链接*信号量池*中的各Semaphore对象。

信号量池的定义如下：
``` C
#define NR_SEM 4
Semaphore sem_pool[NR_SEM];

ListHead free_sem, head_sem;
```
代码中定义了信号量结构的池 sem_pool , 还有两个链表头节点 head_sem 和 free_sem , 其中 head_sem 用于组织使用中的信号量结构, free_sem 用于组织空闲的信号量结构, init_sem_pool() 函数会对两个链表进行初始化。

PV操作的实现如下：
``` C
void P(Semaphore *s)
{
  s->value--;
  if (s->value < 0)
  {	// 阻塞current进程
    // 将current进程从就绪队列中移除
    list_del(&current->list);	
    // 将current进程加入阻塞在该信号量上的队列的尾部
    list_add_before(&s->block, &current->list);
    // 将current进程的状态置为阻塞
    current->state = BLOCKED;
  }
}
```

``` C
void V(Semaphore *s)
{
  s->value++;
  if (s->value <= 0)
  {	// 释放一个原先阻塞在该信号量上的进程
    // 获取阻塞队列的队首
    ListHead *tmp = s->block.next;
    // 从阻塞队列中移除
    list_del(tmp);
    // 加入就绪队列
    list_add_before(&ready, tmp);
    // 将其状态设为就绪态
    list_entry(tmp, PCB, list)->state = RUNNABLE;
  }
}
```

sem_init(sem_t *sem, uint32_t value) 函数首先从free_sem链表中拿出一个空闲的Semaphore对象，将其加入head_sem链表。然后，用value对该信号量赋初始值，并将该信号量的阻塞队列初始化为空队列。最后，在sem指向的用户空间中存入该Semapore结构体在sem_pool[ ]中的索引值，这样，后面的sem_post()和sem_wait()函数就可以用这一索引找到对应的Semaphore结构体。

sem_destroy(sem_t *sem)函数将相应的Semaphore结构体从head_sem链表归还到free_sem链表。

## 3 实验结果
![](oslab4-result.png)

## 4 实验环境
* QEMU版本： QEMU emulator version 2.8.0
* GCC版本： gcc (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609
* 操作系统： Ubuntu 16.04 LTS， 32-bit
* 虚拟机：VMware® Workstation 11.1.2 build-2780323

## 参考资料
[1] http://cslabcms.nju.edu.cn/ics/index.php/os:2012/lab1  
