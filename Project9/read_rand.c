#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>



#define SUFFLE_NUM	10000	

int gettimeofday(struct timeval *tv,struct timezone *tz);
void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
	int fd;
	char *fname=argv[1];
	char buf[100];
	int num_of_records;
	int count;
	struct timeval starttime, endtime;
	int  difftime;
	int offset;

	fd=open(fname,O_RDWR);
	lseek(fd,0,SEEK_SET);
	while(count=read(fd,&buf,100)>0){
		num_of_records++;
	}

	int *read_order_list;



	read_order_list=(int *)malloc(sizeof(int)*num_of_records);
	
	GenRecordSequence(read_order_list, num_of_records);

	gettimeofday(&starttime,NULL);
	
	for(int i=0;i<num_of_records;i++){
		offset=read_order_list[i];
		lseek(fd,offset*100,SEEK_SET);
		read(fd,&buf,100);

}

	gettimeofday(&endtime,NULL);

	endtime.tv_usec+=1000000*endtime.tv_sec;
	starttime.tv_usec+=1000000*starttime.tv_sec;
	difftime=endtime.tv_usec-starttime.tv_usec;

	printf("#record=%d timecost=%d us\n",num_of_records,difftime);
	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}

	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
