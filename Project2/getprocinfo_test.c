#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "processInfo.h"

int main(void)
{
    int max;
    struct processInfo *info=malloc(sizeof(struct processInfo));

    printf(1,"PID PPID SIZE NUMBER OF CONTEXT SWITCH\n");

    max=get_max_pid();

    for(int i=1;i<=max;i++){

      if(get_proc_info(i,info)!=-1)
        printf(1,"  %d  %d  %d   %d\n",i,info->ppid,info->psize,info->numberContextSwitches);
    }
    exit();
}
