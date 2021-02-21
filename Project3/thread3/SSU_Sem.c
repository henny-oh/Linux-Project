#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "SSU_Sem.h"
#include "unistd.h"

void SSU_Sem_init(SSU_Sem *s, int value) {
  s->count=value;
  s->wait=0;
  s->end=0;

}

void SSU_Sem_down(SSU_Sem *s) {
  int num=--s->count;
  int list=s->wait++;

  while(1){
    //usleep(100);
   //printf("%d %d %d %d\n",num,s->count,list,s->wait);
    if(num<=s->count&&list==s->wait){
      break;
    }
  }

}

void SSU_Sem_up(SSU_Sem *s) {
  //printf("up\n");
  s->count++;
  s->wait--;
  s->end++;
}
