#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "SSU_Sem.h"
#include <unistd.h>

#define NUM_THREADS 3
#define NUM_ITER 10

SSU_Sem child[NUM_THREADS];

void *justprint(void *data)
{
  int thread_id = *((int *)data);
  int count=0;
  //printf("%d in\n",thread_id);

  SSU_Sem_down(&child[thread_id]);

  for(int i=0; i < NUM_ITER; i++)
    {

      printf("This is thread %d\n", thread_id);
      if(thread_id==NUM_THREADS-1) {
        //printf("%d:------\n",i);
        SSU_Sem_up(&child[0]);
      }
      else SSU_Sem_up(&child[thread_id+1]);
      count++;
      if(count==NUM_ITER) break;
      SSU_Sem_down(&child[thread_id]);
    }
  return 0;
}

int main(int argc, char *argv[])
{

  pthread_t mythreads[NUM_THREADS];
  int mythread_id[NUM_THREADS];


  for(int i=0;i<NUM_THREADS;i++)
    SSU_Sem_init(&child[i],0);

//  printf("main in\n");



  //pthread_create(&t1, NULL, justprint, (void *)&pid);
  //pthread_create(&t2, NULL, justprint, (void *)&pid2);

  for(int i =NUM_THREADS-1; i >=0; i--)
    {
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);

    }
    sleep(1);
  //printf("time to wake up\n");

//  printf("loop1\n");

    //printf("0 start\n");
    SSU_Sem_up(&child[0]);


  for(int i =NUM_THREADS-1; i >=0; i--)
    {
      pthread_join(mythreads[i], NULL);
    }

  return 0;
}
