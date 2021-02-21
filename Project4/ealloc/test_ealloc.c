#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "ealloc.h"

typedef struct{
	int *add;//memory address
	int section;
	int status;//1-using,0-not using
}mst;

mst mmr[4][512];

int sz;
int num;

int m;


void *map[4];


void printvsz(char *hint) {
  char buffer[256];
  sprintf(buffer, "echo -n %s && echo -n VSZ: && cat /proc/%d/stat | cut -d\" \" -f23", hint, getpid());
  system(buffer);
  //getchar();
}
void init_alloc(void){
  num=0;
	m=0;
	sz=0;

	for(int i=0;i<4;i++){
		for(int j=0;j<512;j++){
			mmr[i][j].add=0;
			mmr[i][j].section=num;
			mmr[i][j].status=0;
		}
	}

}
char *alloc(int a){
  int *mad;
  int address;
	int size=0;
	int search=0;
	int start;
	int *sol=0;
	char *ans;

	sz+=a;
	//printf("%d\n",sz)
	if(sz%4096==0)
		m=sz/4096-1;
	else
		m=sz/4096;

	//printf("%d %d\n",sz,m);
  if(m<4){

    map[m]=mmap(0,a,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);

    if(map[m]==MAP_FAILED)
  		fprintf(stderr,"mmap failure");

  //  printf("map:%d-%p\n",m,map[m]);
    mad=((int *)map[m]);
		ans=(char *)mad;
    for(int i=0;i<512;i++){
  		mmr[m][i].add=mad;
  		mad++;
  		mad++;
			//printf("map:%p\n",mmr[m][i].add);
			mmr[m][i].status=1;
			mmr[m][i].section=num;

  	}
		//m++;
		num++;
		//printf("alloc done\n");
		//printf("map:%d-%p\n",m,mad);

		//printf("return %p\n",ans);
		return ans;
  }	else{

		 for(int i=0;i<4;i++){
				for(int j=0;j<512;j++){
					if(mmr[i][j].status==1){
						search=0;
						continue;
					}
					if(search==1&&mmr[i][j].status==0){
						size+=8;
					}
					if(mmr[i][j].status==0&&search==0){
						size=0;
						start=j;
						search=1;
						size+=8;
					}
					if(size==a){
					//printf("found %d\n",i);
						sol=mmr[i][start].add;
						for(int q=start;q<=j;q++){
							mmr[i][q].status=1;
							mmr[i][q].section=num;
							//printf("%d\n",size);
						//printf("input section %p %d %d\n",sol,i,q);
						}
						//printf("input section %p \n",sol);
						break;
					}

				}

			}
			num++;
			ans=(char *)sol;
			return ans;
	}
}
void dealloc(char *str){
	int *dadd;
	dadd=((int *)str);
	int ans;

	for(int i=0;i<4;i++){
		for(int j=0;j<512;j++){
			if(mmr[i][j].add==dadd) {
				ans=mmr[i][j].section;
				//printf("%p %p\n",str,mmr[i][j].add);
			//	printf("section %d\n",ans);
			}

			if(mmr[i][j].section==ans) {

				mmr[i][j].status=0;
				mmr[i][j].section=0;
			}
		}
	}
	//printf("delete section %d\n",ans);

}
void cleanup(void){


	for(int i=0;i<4;i++){
		for(int j=0;j<512;j++){

				mmr[i][j].add=0;
				mmr[i][j].status=0;
				mmr[i][j].section=0;
		}
	}
}
int main()
{

  printf("\nInitializing memory manager\n\n");
  init_alloc();

  //Start tests

  printf("Test1: checking heap expansion; allocate 4 X 4KB chunks\n");
  printvsz("start test 1:");

  char *a[4];
  for(int i=0; i < 4; i++) {
    a[i] = alloc(4096);
		//printf("address:%p\n",a[i]);
    //write to chunk
    for(int j=0; j < 4096; j++)
      *(a[i]+j) = 'a';
		//printf("write done\n");
    printvsz("should increase by 4KB:");
  }

  //read all content and verify;
  int mismatch=0;
  for(int i=0; i < 4; i++) {
    //read each chunk
    for(int j=0; j < 4096; j++)
      {
	char x = *(a[i]+j);
	if(x != 'a')
	  mismatch = 1;
      }
  }

  if(mismatch) {
    printf("ERROR: Chunk contents did not match\n");
    exit(1);
  }

  for(int i=0; i < 4; i++) {
    dealloc(a[i]);
  }

  printvsz("should not change:");
  printf("Test1: complete\n\n");

  printf("Test2: Check splitting of existing free chunks: allocate 64 X 256B chunks\n");
  printvsz("start test 2:");

  //we know the heap has 4 X 4KB free chunks
  //now ask for 64 X 256B chunks
  //no new memory should be used

  char *b[64];
  for(int i=0; i<64; i++) {
    b[i] = alloc(256);

    for(int j=0; j< 256; j++)
        *(b[i]+j) = 'b';
  }
  printvsz("should not change:");

  //read each chunk
  mismatch = 0;
  for(int i=0; i < 64; i++) {

    for(int j=0; j < 256; j++)
      {
	char x = *(b[i]+j);
	if(x != 'b')
	  mismatch = 1;
      }
  }

  if(mismatch) {
    printf("ERROR: Chunk contents did not match\n");
    exit(1);
  }

  for(int i=0; i < 64; i++) {
    dealloc(b[i]);
  }

  printvsz("should not change:");
  printf("Test2: complete\n\n");

  printf("Test3: checking merging of existing free chunks; allocate 4 X 4KB chunks\n");
  printvsz("start test 3:");

  char *c[4];
  for(int i=0; i < 4; i++) {
    c[i] = alloc(4096);
		//printf("alloc\n");
    //write to chunk
    for(int j=0; j < 4096; j++)
      *(c[i]+j) = 'c';

    printvsz("should not change:");
  }

  //read all content and verify;
  mismatch=0;
  for(int i=0; i < 4; i++) {
    //read each chunk
    for(int j=0; j < 4096; j++)
      {
	char x = *(c[i]+j);
	if(x != 'c')
	  mismatch = 1;
      }
  }

  if(mismatch) {
    printf("ERROR: Chunk contents did not match\n");
    exit(1);
  }

  for(int i=0; i < 4; i++) {
    dealloc(c[i]);
  }

  printvsz("should not change:");
  printf("Test3: complete\n\n");


  cleanup();
  printf("All tests complete\n");
}
