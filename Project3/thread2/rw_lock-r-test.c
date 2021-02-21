#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //	Write the code for initializing your read-write lock.
  rw-> r_count=0;
  rw-> w_count=0;
  rw-> r_end=0;
  rw-> w_end=0;
  rw-> w_lck=0;
}

void r_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
  rw->r_count++;
}

void r_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
  rw->r_end++;
}

void w_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
  rw->w_count++;
  while(1){
    if(rw->r_count==rw->r_end&&rw->w_lck==0&&rw->r_count!=0){
      rw->w_lck=1;
      break;
    }
    usleep(1000);
  }

}

void w_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
  rw->w_end++;
  rw->w_lck=0;
}
