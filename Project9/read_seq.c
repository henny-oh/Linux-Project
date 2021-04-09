#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

int gettimeofday(struct timeval *tv,struct timezone *tz);



int main(int argc,char **argv){

	int fd;
	char *fname=argv[1];
	int count=0;
	int r_num=0;
	char buf[100];
	fd=open(fname,O_RDONLY);

	struct timeval starttime, endtime;
	int  difftime;

	lseek(fd,0,SEEK_SET);

	while((count=read(fd,&buf,100))>0){
		r_num++;
	}
	lseek(fd,0,SEEK_SET);

	gettimeofday(&starttime,NULL);
	while((count=read(fd,&buf,100))>0){

		
	}
	
	gettimeofday(&endtime,NULL);

	endtime.tv_usec+=1000000*endtime.tv_sec;
	starttime.tv_usec+=1000000*starttime.tv_sec;
	difftime=endtime.tv_usec-starttime.tv_usec;
	printf("#record=%d timecost=%d us\n",r_num,difftime);


	return 0;
}
