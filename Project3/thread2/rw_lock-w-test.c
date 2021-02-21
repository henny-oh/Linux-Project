#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //	Write the code for initializing your read-write lock.

  rw->count=0;
  rw->line=0;
  rw->w_wait=0;
}

void r_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
  int num=rw->count++;
  //printf("%d\n",num);
  while(1){
    //  printf("%d : %d %d\n",num,rw->line,rw->w_wait);
    if(rw->w_wait==0){
      break;
    }
    if(num==rw->line&&rw->w_wait==0){
      break;
    }
      usleep(1000);
  }
}

void r_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
  rw->line++;
}

void w_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
  int num=rw->count++;

  rw->w_wait++;

  while(1){
  //  printf("%d : %d %d\n",num,rw->line,rw->w_wait);

    if(num==rw->line&&rw->w_wait>=1){
      //printf("in\n");
      break;
    }
    usleep(1000);
  }
}

void w_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
  rw->line++;
  rw->w_wait--;
}
