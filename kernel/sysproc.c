#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}


uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);

  // lab4-2
  // 测试是用sleep()系统调用测试的，所以在这里也打印
  backtrace();

  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// lab4-3添加对系统调用的实际函数实现
uint64
sys_sigalarm(void)
{
    // 将寄存器中的值取出，保存在当前进程的interval和handler中。
    int interval;
    uint64 handler; // 函数指针就是个地址
    struct proc *p;

    // 取出时间间隔，要求时间间隔非负,时间间隔为0是可以的，表示取消调用
    // 取出0号寄存器和1号寄存器的地址
    if(argint(0, &interval) < 0 || argaddr(1, &handler) < 0 || interval < 0)
    {
        return -1;
    }

    // 取出当前进程结构体，进行赋值
    p = myproc();
    p->interval = interval;
    p->handler = handler;
    p->passedticks = 0; // 重置时钟

    return 0;
}
// lab4-3 test1/test2
// 在trap.c的usertrap中保存了函数调用之前的寄存器，现在要返回用户空间的时候进行复原
uint64
sys_sigreturn(void)
{
    struct proc *p = myproc();
    // 判断下地址，防止在没有进行复制副本之前调用这个系统调用
    if(p->trapframecopy != p->trapframe + 512)
    {
        return -1;
    }

    // 进行还原
    memmove(p->trapframe, p->trapframecopy, sizeof(struct trapframe));
    p->passedticks = 0;
    p->trapframecopy = 0;

    return 0;
}
