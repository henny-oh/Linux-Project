#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "person.h"

typedef struct _header
{
	int pagenum;
	int recordnum;
	int depage;
	int derecord;
}Header;

Header h1;
Person p1;
int count=PAGE_SIZE/RECORD_SIZE;

//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉 페이지 단위로 읽거나 써야 합니다.

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
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 그런 후 이 레코드를 저장할 페이지를 readPage()를 통해 프로그램 상에
// 읽어 온 후 pagebuf에 recordbuf에 저장되어 있는 레코드를 저장한다. 그 다음 writePage() 호출하여 pagebuf를 해당 페이지 번호에
// 저장한다. pack() 함수에서 readPage()와 writePage()를 호출하는 것이 아니라 pack()을 호출하는 측에서 pack() 함수 호출 후
// readPage()와 writePage()를 차례로 호출하여 레코드 쓰기를 완성한다는 의미이다.
//
void pack(char *recordbuf, const Person *p)
{
		strcat(recordbuf,p->sn);
		strcat(recordbuf,"#");
		strcat(recordbuf,p->name);
		strcat(recordbuf,"#");
		strcat(recordbuf,p->age);
		strcat(recordbuf,"#");
		strcat(recordbuf,p->addr);
		strcat(recordbuf,"#");
		strcat(recordbuf,p->phone);
		strcat(recordbuf,"#");
		strcat(recordbuf,p->email);
		strcat(recordbuf,"#");

}

//
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다. 이 함수가 언제 호출되는지는
// 위에서 설명한 pack()의 시나리오를 참조하면 된다.
//
void unpack(const char *recordbuf, Person *p)
{		char *temp=(void *)malloc(RECORD_SIZE);
		memset(temp,0,RECORD_SIZE);
		char *ptr;

		strncpy(temp,recordbuf,RECORD_SIZE);

		ptr=strtok(temp, "#");
		strcpy(p->sn,temp);

		//printf("temp:%s\n",ptr);
		//printf("unpack:%s\n",p->sn);
    ptr = strtok(NULL, "#");

		strcpy(p->name,ptr);
		//printf("unpack name:%s\n",p->name);
		ptr = strtok(NULL, "#");
		strcpy(p->age,ptr);
		//memcpy(p->age,temp,sizeof(temp));
		//printf("unpack:%s\n",p->age);
		ptr = strtok(NULL, "#");
		strcpy(p->addr,ptr);
		//memcpy(p->addr,temp,sizeof(temp));
		ptr = strtok(NULL, "#");
		strcpy(p->phone,ptr);
		//memcpy(p->phone,temp,sizeof(temp));
		ptr = strtok(NULL, "#");
		strcpy(p->email,ptr);
		//memcpy(p->email,temp,sizeof(temp));
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값을 구조체에 저장한 후 아래의 insert() 함수를 호출한다.
//
void insert(FILE *fp, const Person *p)
{	char *pagebuf=(void *)malloc(PAGE_SIZE);
	char *recordbuf=(void *)malloc(RECORD_SIZE);
	char emptypage[PAGE_SIZE];

	memset(emptypage,0xff,sizeof(emptypage));
	//make recordbuf to write data
	pack(recordbuf, p);

	//find the location where data should be written

	readPage(fp, pagebuf, 0);
	memcpy(&h1.pagenum,pagebuf,sizeof(int));
	memcpy(&h1.recordnum,pagebuf+4,sizeof(int));
	memcpy(&h1.depage,pagebuf+8,sizeof(int));
	memcpy(&h1.derecord,pagebuf+12,sizeof(int));


	if(h1.depage==-1&&h1.derecord==-1){
		if(h1.recordnum%count==0){
			fseek(fp,0,SEEK_END);
			fwrite(emptypage,sizeof(emptypage),1,fp);
			readPage(fp,pagebuf, h1.pagenum);
			strncpy(pagebuf,recordbuf,strlen(recordbuf));
			writePage(fp,pagebuf, h1.pagenum);
			//printf("%s\n",pagebuf);
			h1.pagenum++;
			h1.recordnum++;
			readPage(fp, pagebuf, 0);
			memcpy(pagebuf,&h1.pagenum,sizeof(h1.pagenum));
			memcpy(pagebuf+4,&h1.recordnum,sizeof(h1.recordnum));
			memcpy(pagebuf+8,&h1.depage,sizeof(h1.depage));
			memcpy(pagebuf+12,&h1.derecord,sizeof(h1.derecord));
			memcpy(pagebuf+16,&emptypage,PAGE_SIZE-16);
			writePage(fp,pagebuf, 0);


		}else{
			readPage(fp,pagebuf, h1.pagenum-1);
			strncpy(pagebuf+RECORD_SIZE,recordbuf,strlen(recordbuf));
			writePage(fp,pagebuf, h1.pagenum-1);
			h1.recordnum++;

			readPage(fp, pagebuf, 0);
			memcpy(pagebuf,&h1.pagenum,sizeof(h1.pagenum));
			memcpy(pagebuf+4,&h1.recordnum,sizeof(h1.recordnum));
			memcpy(pagebuf+8,&h1.depage,sizeof(h1.depage));
			memcpy(pagebuf+12,&h1.derecord,sizeof(h1.derecord));
			memcpy(pagebuf+16,&emptypage,PAGE_SIZE-16);
			writePage(fp,pagebuf, 0);

		}
	}else{

		readPage(fp,pagebuf, h1.depage);
		int pn=h1.depage;
		int rn=h1.derecord;
		memcpy(&h1.depage,pagebuf+RECORD_SIZE*rn+1,sizeof(int));
		memcpy(&h1.derecord,pagebuf+RECORD_SIZE*rn+5,sizeof(int));
		strncpy(pagebuf+RECORD_SIZE*rn,recordbuf,strlen(recordbuf));
		writePage(fp,pagebuf, pn);

		readPage(fp, pagebuf, 0);
		memcpy(pagebuf,&h1.pagenum,sizeof(h1.pagenum));
		memcpy(pagebuf+4,&h1.recordnum,sizeof(h1.recordnum));
		memcpy(pagebuf+8,&h1.depage,sizeof(h1.depage));
		memcpy(pagebuf+12,&h1.derecord,sizeof(h1.derecord));
		memcpy(pagebuf+16,&emptypage,PAGE_SIZE-16);
		writePage(fp,pagebuf, 0);
	}
	fclose(fp);
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *sn)
{
	char *recordbuf=(void *)malloc(RECORD_SIZE);
	char *pagebuf=(void *)malloc(PAGE_SIZE);
	char emptypage[PAGE_SIZE];
	char *new=(void *)malloc(sizeof(int));
	char *st=(void *)malloc(sizeof(int));
	char *arr=(void *)malloc(sizeof(int)*2+sizeof(char));

	memset(emptypage,0xff,sizeof(emptypage));
	memset(recordbuf,0xff,sizeof(recordbuf));
	memset(pagebuf,0xff,sizeof(pagebuf));
	memset(new,0xff,sizeof(new));
	memset(st,0xff,sizeof(st));
	memset(arr,0xff,sizeof(arr));

	readPage(fp, pagebuf, 0);
	memcpy(&h1.pagenum,pagebuf,sizeof(int));
	memcpy(&h1.recordnum,pagebuf+4,sizeof(int));
	memcpy(&h1.depage,pagebuf+8,sizeof(int));
	memcpy(&h1.derecord,pagebuf+12,sizeof(int));



	//check all the record*/


	int pn=0,rn=0;
	int num=0;

	memcpy(new,&h1.depage,sizeof(int));
	//printf("%s\n",new);
	memcpy(st,&h1.derecord,sizeof(int));
	//printf("%s\n",st);
	memcpy(arr,"*",sizeof(char));
	memcpy(arr+sizeof(char),new,sizeof(int));
	memcpy(arr+sizeof(char)+sizeof(int),st,sizeof(int));
	//printf("%s\n",arr);

	memcpy(&pn,arr+sizeof(char),sizeof(int));
	//printf("%d\n",pn);
	memcpy(&pn,arr+sizeof(char)+sizeof(int),sizeof(int));
	//printf("%d\n",pn);


	//find record location
	for(int i=1;i<h1.pagenum;i++){
		readPage(fp,pagebuf,i);

		for(int j=0;j<count;j++){
				memset(recordbuf,0xff,sizeof(recordbuf));
				memcpy(recordbuf,pagebuf+j*RECORD_SIZE,RECORD_SIZE);

				unpack(recordbuf,&p1);


				if(strcmp(p1.sn,sn)==0){




					memcpy(pagebuf+j*RECORD_SIZE,"*",sizeof(char));

					memcpy(pagebuf+j*RECORD_SIZE+sizeof(char),new,sizeof(int));

					memcpy(&pn,pagebuf+j*RECORD_SIZE+sizeof(char),sizeof(int));


					memcpy(pagebuf+j*RECORD_SIZE+sizeof(char)+sizeof(int),st,sizeof(int));

					memcpy(&pn,pagebuf+j*RECORD_SIZE+sizeof(char)+sizeof(int),sizeof(int));

					writePage(fp,pagebuf,i);


					//chage header
					memset(pagebuf,0xff,sizeof(pagebuf));
					readPage(fp, pagebuf, 0);

					h1.depage=i;
					h1.derecord=j;
					memcpy(pagebuf,&h1.pagenum,sizeof(h1.pagenum));
					memcpy(pagebuf+4,&h1.recordnum,sizeof(h1.recordnum));
					memcpy(pagebuf+8,&h1.depage,sizeof(h1.depage));
					memcpy(pagebuf+12,&h1.derecord,sizeof(h1.derecord));
					memcpy(pagebuf+16,&emptypage,PAGE_SIZE-16);
					writePage(fp,pagebuf, 0);

					break;
				}

		}
		if(strcmp(p1.sn,sn)==0){
			break;
		}
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	char op=*argv[1];
	char *fname=argv[2];
	int size;
	int header[4]={0,};
	int arr[4]={0,};
	char *key=argv[3];

	char emptypage[PAGE_SIZE];
	char empthyrecord[RECORD_SIZE];
	char *recordbuf=(void *)malloc(RECORD_SIZE);
	char *pagebuf=(void *)malloc(PAGE_SIZE);


	memset(emptypage,0xff,sizeof(emptypage));
	memset(recordbuf,0xff,sizeof(recordbuf));
	memset(pagebuf,0xff,sizeof(pagebuf));


	if(op=='i'){
		char *name=argv[4];
		char *age=argv[5];
		char *address=argv[6];
		char *phone=argv[7];
		char *email=argv[8];




		//header check
		fp=fopen(fname,"ab");
		fseek(fp, 0, SEEK_END);
    if((size = ftell(fp))<=0){
			fclose(fp);
			fp=fopen(fname,"w");
			fseek(fp,0,SEEK_SET);

			//fwrite(emptypage,sizeof(emptypage),1,fp);

			//fseek(fp,0,SEEK_SET);

			h1.pagenum=1;
			h1.recordnum=0;
			h1.depage=-1;
			h1.derecord=-1;
			//readPage(fp, pagebuf, 0);

			memcpy(pagebuf,emptypage,PAGE_SIZE);
			memcpy(pagebuf,&h1.pagenum,sizeof(h1.pagenum));
			memcpy(pagebuf+4,&h1.recordnum,sizeof(h1.recordnum));
			memcpy(pagebuf+8,&h1.depage,sizeof(h1.depage));
			memcpy(pagebuf+12,&h1.derecord,sizeof(h1.derecord));
			memcpy(pagebuf+16,&emptypage,PAGE_SIZE-16);
			writePage(fp,pagebuf, 0);


		}
		fclose(fp);

		//make struct.person

		strcpy(p1.sn, key);
		strcpy(p1.name,name);
		strcpy(p1.age,age);
		strcpy(p1.addr,address);
		strcpy(p1.phone,phone);
		strcpy(p1.email,email);


		fp=fopen(fname,"rb+");
		insert(fp, &p1);



		}
		else if(op=='d'){

			//bring header information
			fp=fopen(fname,"rb+");
			delete(fp,key);


		}






	//free(recordbuf);

	return 0;
}
