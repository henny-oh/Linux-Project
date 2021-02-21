#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "processInfo.h"

int
sys_get_prio(void){

	return get_prio(); //get_prio 함수 호출
}
int
sys_set_prio(void){
	int n;
	int r;

	argint(0,&n);	//사용자모드에서 넘겨준 0번째 인자를 n에 저장
	r=set_prio(n);	//n을 인자로 set_prio 호출
	return r;	//set_prio 값을 리턴
}

int
sys_get_proc_info(void){
	int i;
	struct processInfo *info;
	int r;

	argint(0,&i);	//0번째 인자를 i에 저장
	argptr(1,(void *)&info,sizeof(*info));	//1번째 인자를 processInfo 구조체에 저장

	r=get_proc_info(i,info);//인자들을 get_proc_info 값으로 넘겨줌


	return 	r; //함수 리턴값을 사용자 모드로 리턴
}
int
sys_get_max_pid(void){
	int max;
	max=get_max_pid(); //get_max_pid 호출
	return max;//함수값 리턴
}
int
sys_get_num_proc(void){
	int count;
	count=get_num_proc(); //get_num_proc 호출

	return count;//함수값 리턴
}
int
sys_helloname(void){
	char *input;
	argstr(0,&input);	//0번째 인자를 input
	cprintf("hello %s\n",input);//input 값 커널모드에서 출력
	return 0;
}
int
sys_hello(void){
	cprintf("helloxv6\n");//커널모드에서 출력
	return 0;
}
int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
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
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
