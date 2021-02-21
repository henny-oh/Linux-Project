#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


int main(void)
{

    printf(0,"Total Number of Active Process: %d\n",get_num_proc());
    exit();
}
