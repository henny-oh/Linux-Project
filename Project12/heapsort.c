#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음
void heapify(char **heaparray,int node,int recordnum,int max);
int recordnum;
int pagenum;
int depage=-1;
int derecord=-1;
// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{

	fseek(fp,pagenum*PAGE_SIZE,SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);

}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp,pagenum*PAGE_SIZE,SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며,
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{

	int node;
	char *pagebuf=(void *)malloc(PAGE_SIZE);
	char *recordbuf=(void *)malloc(RECORD_SIZE);
	int count=1;



	memset(recordbuf,0xff,sizeof(recordbuf));



	for(int i=1;i<=pagenum;i++){
		readPage(inputfp, pagebuf, i);
		strncpy(recordbuf,pagebuf,RECORD_SIZE);
		strncpy(heaparray[count],recordbuf,RECORD_SIZE);
	//	heaparray[count][RECORD_SIZE]='\0';


	//	printf("%d %s\n",count, heaparray[count]);

		if(recordnum==count) break;
		count++;

		strncpy(recordbuf,pagebuf+RECORD_SIZE,RECORD_SIZE);
		strncpy(heaparray[count],recordbuf,RECORD_SIZE);
		//sprintf(recordbuf,"%s\0",recordbuf);
		//heaparray[count]=recordbuf;
		heaparray[count][RECORD_SIZE]='\0';
	//	printf("%d %s\n",count-1, heaparray[count-1]);
	//	printf("%d %s\n",count, heaparray[count]);

		count++;
	}
	//printf("arry\n");

	node=count/2;
	for(int i=node;i>0;i--){
		heapify(heaparray,i,recordnum,node);
	}
	for(int i=1;i<=recordnum;i++){
	//	printf("%d %s\n",i,heaparray[i]);
	}
//	printf("******************\n");
}
void heapify(char **heaparray,int node,int recordnum,int max){

	char buf[RECORD_SIZE];
	char *ptr;
	char par[RECORD_SIZE];
	char b1[RECORD_SIZE];
	char b2[RECORD_SIZE];
	int a;
	int sum=1;

//	printf("recordn %d max %d\n",recordnum,max);
	if(node<1||node>recordnum||node>max) return;
//	printf("node %d\n",node);
	strncpy(buf,heaparray[node],RECORD_SIZE);
//	printf("%s\n",buf);
	ptr=strtok(buf,"#");
	strcpy(par,ptr);
//	printf("%s\n",par);

	strncpy(buf,heaparray[node*2],RECORD_SIZE);
	//printf("%s\n",buf);
	ptr=strtok(buf,"#");
	strcpy(b1,ptr);
	//printf("%s\n",b1);


	//printf("p %s\n",par);
	//printf("b %s\n",b1);
	if(( a=strcmp(par,b1))>0) {
		//printf("exchange\n");
		strncpy(buf,heaparray[node*2],RECORD_SIZE);
		strncpy(heaparray[node*2],heaparray[node],RECORD_SIZE);
		//printf("%s\n",heaparray[node*2]);
		strncpy(heaparray[node],buf,RECORD_SIZE);
		//printf("%s\n",heaparray[node]);
		//printf("%d\n",node);
		strncpy(buf,heaparray[node],RECORD_SIZE);
		//printf("%s\n",buf);
		ptr=strtok(buf,"#");
		strcpy(par,ptr);
//	printf("%s\n",par);

		strncpy(buf,heaparray[node*2],RECORD_SIZE);
		//printf("%s\n",buf);
		ptr=strtok(buf,"#");
		strcpy(b1,ptr);
	//	printf("%s\n",b1);
	//	printf("before %d\n",node);
		heapify(heaparray,node*2,recordnum,max);
	}

	//printf("a=%d\n",a);
	strncpy(buf,heaparray[node*2+1],RECORD_SIZE);
	ptr=strtok(buf,"#");
	strcpy(b2,ptr);
	//printf("%s b2 %s\n",par,b2);



	if(( a=strcmp(par,b2))>0) {
		//printf("exchange\n");
		strncpy(buf,heaparray[node*2+1],RECORD_SIZE);
		strncpy(heaparray[node*2+1],heaparray[node],RECORD_SIZE);
	//	printf("%s\n",heaparray[node*2]);
		strncpy(heaparray[node],buf,RECORD_SIZE);
		//printf("%s\n",heaparray[node]);
		//printf("node %d\n",node);
		strncpy(buf,heaparray[node],RECORD_SIZE);
	//	printf("before %d\n",node);
		heapify(heaparray,node*2+1,recordnum,max);
	}
}
//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{
	char *pagebuf=(void *)malloc(PAGE_SIZE);
	char *page=(void *)malloc(PAGE_SIZE);
	char *recordbuf=(void *)malloc(RECORD_SIZE);
	char emptypage[PAGE_SIZE];
	int count=1;
	int pagecount=1;
	int node;

	memset(pagebuf,0xff,sizeof(pagebuf));
	memset(page,0xff,sizeof(pagebuf));
	memset(emptypage,0xff,sizeof(emptypage));
	memset(recordbuf,0xff,sizeof(recordbuf));

	memcpy(pagebuf,&pagenum,sizeof(pagenum));
	memcpy(pagebuf+4,&recordnum,sizeof(recordnum));
	memcpy(pagebuf+8,&depage,sizeof(depage));
	memcpy(pagebuf+12,&derecord,sizeof(derecord));
	memcpy(pagebuf+16,&emptypage,PAGE_SIZE-16);

	writePage(outputfp, pagebuf, 0);

	for(int i=1;i<=9;i++){
		//printf("%d %s\n",i,heaparray[i]);
	}

	memset(pagebuf,0xff,sizeof(pagebuf));
	//printf("%ld\n",strlen(pagebuf));

	for(int i=recordnum;i>0;i--){
		//printf("%d\n",i);
		//printf("count %d\n",count);
		if(count>2){
			//printf("write page %s\n",pagebuf);
			writePage(outputfp, pagebuf, pagecount);
			memset(pagebuf,0xff,strlen(pagebuf));
			memset(page,0xff,strlen(pagebuf));

			count=1;
			pagecount++;
		}
		if(count==1){
			strncpy(page,heaparray[1],RECORD_SIZE);
			strcat(page,emptypage);
			strncpy(pagebuf,page,PAGE_SIZE);
			}
		else{
			strncpy(page+RECORD_SIZE,heaparray[1],RECORD_SIZE);
			strncpy(pagebuf,page,PAGE_SIZE);

		}


		//printf("write !%s\n",heaparray[1]);
		//printf("write !%s\n",pagebuf);
		count++;
		strncpy(heaparray[1],heaparray[i],RECORD_SIZE);
		strncpy(heaparray[i],emptypage,RECORD_SIZE);
	//printf("%s\n",heaparray[1]);
		//printf("new\n");
		for(int j=(i-1)/2;j>0;j--){
			heapify(heaparray,j,(i-1),(i-1)/2);
		}
		//printf("after sorting\n");
		for(int j=1;j<=recordnum;j++){
		//	printf("%d %s\n",j,heaparray[j]);
		}
}
 if(count==2){
	//printf("write page %s\n",pagebuf);
	 writePage(outputfp, pagebuf, pagecount);
	 memset(pagebuf,0xff,strlen(pagebuf));
	 memset(page,0xff,strlen(pagebuf));

	 count=1;
	 pagecount++;
 }
	for(int i=1;i<=9;i++){
		//printf("%d %s\n",i,heaparray[i]);
	}
}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터

	char op=*argv[1];
	char *inputname=argv[2];
	char *outputname=argv[3];
	char *pagebuf=(void *)malloc(PAGE_SIZE);


	char **heaparray;

	memset(pagebuf,0xff,sizeof(pagebuf));

	if(op=='s'){


		inputfp=fopen(inputname,"rb+");
		readPage(inputfp, pagebuf, 0);
		memcpy(&pagenum,pagebuf,sizeof(int));
		memcpy(&recordnum,pagebuf+sizeof(int),sizeof(int));
		memcpy(&depage,pagebuf+8,sizeof(depage));
		memcpy(&derecord,pagebuf+12,sizeof(derecord));



		heaparray = (char**)malloc(sizeof(char *)  * recordnum);

		for (int i=0; i<=recordnum; i++) {
					heaparray[i] = (char*) malloc(sizeof(char *) * RECORD_SIZE);

	}

		buildHeap(inputfp,heaparray);

		outputfp=fopen(outputname,"wb+");

		makeSortedFile(outputfp,heaparray);
		free(heaparray);
	}

	return 1;
}
