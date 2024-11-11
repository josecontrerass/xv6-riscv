#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h" //se agrega

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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

extern int set_priority(int pid, int priority);
extern int set_boost(int pid, int boost);


uint64 sys_setpriority(void) {
    int pid, priority;
    argint(0, &pid);
    argint(1, &priority);
    return (uint64)set_priority(pid, priority);
}

uint64 sys_setboost(void) {
    int pid, boost;
    argint(0, &pid);
    argint(1, &boost);
    return (uint64)set_boost(pid, boost);
}

//Declarar las nuevas funciones como externas
extern int mprotect(void *addr, int len);
extern int munprotect(void *addr, int len);




// Adiciones para t3
uint64 sys_mprotect(void) {
    uint64 addr;
    int len;

    argaddr(0, &addr);  // Obtener el argumento sin esperar retorno
    argint(1, &len);    // Obtener el segundo argumento sin esperar retorno

    return mprotect((void*)addr, len);
}

uint64 sys_munprotect(void) {
    uint64 addr;
    int len;

    argaddr(0, &addr);  // Obtener el argumento sin esperar retorno
    argint(1, &len);    // Obtener el segundo argumento sin esperar retorno

    return munprotect((void*)addr, len);
}