#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


#define length 100


#define min_max 59
#define min_min 0
#define hour_max 23
#define hour_min 0
#define day_max 59
#define day_min 1
#define month_max 12
#define month_min 1
#define week_max 6
#define week_min 0
#define minposition 1
#define hourposition 2
#define dayposition 3
#define monthposition 4
#define weekposition 5


void *start(void *arg);//매분마다 “ssu_crontab_file”에 있는 명령어 검사
void *divide(void *arg);//입력된 명령어의 실행여부에 따라 행동
int check(char *command);//실행주기 섹션 별로 나누기
int token_check(int min, int max,char *input,int position);//섹션을 “,”기준으로 나누기
int ssu_crond(void);//데몬프로그램
int timer(int min, int max,char *input,int position);//부분 유형 나누기
int comparetime(int num,int position);//현재 시간과 비교해 실행여부 판단
int succeed(char *command);//실행주기가 맞다고 판단되면 실행하고 log찍는 함수

char *filename="ssu_crontab_file";
char savetime[27]={0,};
time_t current;

int main(void){
  pid_t pid;
		//pid=getpid();
		if((pid=fork())<0) //자식프로세스 생성
			{
				fprintf(stderr,"fork error\n");
				exit(1);//실패하면 나가기
		}
		else if(pid == 0)//자식 프로세스는
		    ssu_crond();//데몬 프로그램-모니터링 함수 실행
		else//부모 프로세스는


		exit(0);

}
int ssu_crond(void)
{
		pid_t pid;
		pthread_t tid;
		int fd,maxfd;
		int status;


		if((pid=fork())<0){//자식프로세스 생성
				fprintf(stderr,"fork error\n");//실패하면 fork 에러 메시지 출력
				exit(1);
			}
		else if(pid !=0)//부모 프로세스는
				exit(0);//종료


		setsid();//새로운 세션 생성

		signal(SIGTTIN,SIG_IGN);//제어 터미널에서 읽으려고 하는 백그라운드 프로세스 무시
		signal(SIGTTOU,SIG_IGN);//제어 터미널에서 쓰려고 하는 백 그라운드 프로세스 무시
		signal(SIGTSTP,SIG_IGN);//잡거나 무시할 수 없는 종단 신호무시
		maxfd=getdtablesize();//파일 디스크립터의 총수

		for(fd=0;fd<maxfd;fd++)//오픈 되어 있는 모든 파일 디스크립터 닫기
				close(fd);

		umask(0);//파일 모드 생성 마스크 해제


		fd=open("/dev/null",O_RDWR);//표준 입출력과 표준에러를 "/dev/null"로 재지정

		dup(0);
		dup(0);



		//명령어 실행 여부 판단하는 함수 thread로 생성
		if(pthread_create(&tid,NULL,start,NULL)!=0){
			fprintf(stderr,"pthread_create error\n");
			exit(1);
		}


		pthread_join(tid,(void *)&status);
		//exit(0);
}

void *start(void *arg){
	FILE *fp;
	pthread_t tid[100];
	int i=0;
	char buf[100][length]={0,};
 	 struct stat statbuf;
  	char timebuf[27]={0,};


	while(1){
    		current=time(NULL);//현재시간 구하기
   		 strcpy(timebuf,ctime(&current));//buf에 저장
		//초에 해당하는 시간이 00이면
    		if(timebuf[17]!='0') continue;
    		if(timebuf[18]!='0') continue;
    		strcpy(savetime,ctime(&current));//기록할 시간 문자열에 저장
	  	if(access(filename,F_OK)<0){//“ssu_crontab_file“이 존재하지 않으면

				continue;//다시 돌아가기
		}
		else {//존재하면
	//그 파일에 명령어가 들어있지 않다면 다시 돌아가기
      lstat(filename,&statbuf);
      if(statbuf.st_size==0) continue;

      fp=fopen(filename,"r");//”ssu_crontab_file“열기
      i=0;
			while(fread(buf[i],length,1,fp)){//한줄씩 일기



				for(int j=0;j<length;j++){//한줄시 arr에 저장
					if(buf[i][j]=='\0') buf[i][j]='|';
				}

//저장된 모든 명령어를 인자로 devide함수 호출해서 실행여부 판단
				if(pthread_create(&tid[i],NULL,divide,(void *)&buf[i])!=0){
					fprintf(stderr,"pthread_create error\n");
					exit(1);
				}

				i++;

			}
			fclose(fp);
      sleep(1);//1초 쉬기

		}

	}
}
void *divide(void *arg){//입력된 문자열로 수행여부 판단하기
	char thread_buf[length];
	char *ptr;
	char command[length]={0,};
	char command1[length]={0,};
	int ox=0;
	strncpy( thread_buf, (char*)arg, length );

	//끝을 표시
	ptr=strtok(thread_buf,"|");
	strncpy(command,ptr,strlen(ptr));

	strcpy(command1,command);

	ox=check(command);//실행주기가 현재 시간과 일치하는 지 확인
	if(ox==1) succeed(command1);//일치한다면 실행하고 log에 기록

	pthread_exit(NULL);//thread 종료
	return NULL;
}
int succeed(char *command){
//실행주기가 현재 시간과 일치해 실행하고 log에 기록하는 함수
	FILE *fp;
  char *filename1="ssu_crontab_log";

  char timebuf[27]={0,};
  char commandbuf[length]={0,};
	char *ptr;
  int fd;
  struct flock lock;
//
  lock.l_type=F_WRLCK;
  lock.l_whence=0;
  lock.l_start=0;
  lock.l_len=0;

  strcpy(timebuf,savetime);

  if((fp=fopen(filename1,"a"))==NULL){
      fprintf(stderr,"fopen error\n");
      exit(1);
  }
  fclose(fp);
//"ssu_crontab_log" write lock
  if((fp=fopen(filename1,"r+"))==NULL){
      fprintf(stderr,"fopen error\n");
      exit(1);
  }
  if((fd = fileno(fp)) == -1) {
    fprintf(stderr, "Error:fileno\n");
    exit(1);
  }

  if(fcntl(fd,F_SETLKW,&lock));


  for(int i=0;i<27;i++){//계행 없애기
    if(timebuf[i]=='\n') timebuf[i]='\0';
  }


//”ssu_crontab_log“에 시간, run,명령어 쓰기
  fseek(fp,0,SEEK_END);
  fwrite("[",1,1,fp);
  fwrite(timebuf,sizeof(timebuf),1,fp);
  fwrite("] ",2,1,fp);
  fwrite("run",strlen("run"),1,fp);
  fwrite(" ",1,1,fp);
  fwrite(command,strlen(command),1,fp);
  fwrite("\n",1,1,fp);
//write lock 해제
  lock.l_type=F_UNLCK;
  fcntl(fd,F_SETLK,&lock);

  fclose(fp);

	//실행주기를 뺀 실행할 명령어만 얻기
	ptr = strtok(command, " ");
	ptr= strtok(NULL, " ");
	ptr= strtok(NULL, " ");
	ptr= strtok(NULL, " ");
	ptr= strtok(NULL, " ");
	ptr= strtok(NULL, "\0");
	strcpy(commandbuf,ptr);

	//명령어 실행
	system(commandbuf);
}
int check(char *command){//실행주기 섹션 별로 나누기
	char *ptr;
  char min[100];
  char hour[100];
  char day[100];
  char month[100];
  char week[100];
  int ox=1;

//섹션 별로 분,시간,일, 월, 요일 나누기
  ptr = strtok(command, " ");
  strcpy(min,ptr);


  ptr= strtok(NULL, " ");
  memcpy(hour,ptr,sizeof(ptr));


  ptr= strtok(NULL, " ");
  memcpy(day,ptr,sizeof(ptr));
  ptr= strtok(NULL, " ");
  memcpy(month,ptr,sizeof(ptr));
  ptr= strtok(NULL, " ");
  memcpy(week,ptr,sizeof(ptr));
//섹션별로 현재시간과 일치하나 검사
	ox=token_check(min_min,min_max,min,minposition);
  if(ox==0) return 0;
	ox=token_check(hour_min,hour_max,hour,hourposition);
  if(ox==0) return 0;
	ox=token_check(day_min,day_max,day,dayposition);
  if(ox==0) return 0;
	ox=token_check(month_min,month_max,month,monthposition);
  if(ox==0) return 0;
	ox=token_check(week_min,week_max,week,weekposition);
  if(ox==0) return 0;

//한섹션이라도 틀리면 실행하지 않음-0, 모두 일치하면 실행-1 리턴
	return 1;
}
int token_check(int min, int max,char *input,int position){
//섹션을 ”,“기준으로 부분나누기
	char *ptr;
	int ox;
	char command[length]={0,};
	int count=0;

	ptr=strchr(input,',');
  if(ptr==NULL){//”,“가 없다면 입력인자 바로 검사
    ox=timer(min,max,input,position);

  }else{//”,“가 있다면
		ptr=strtok(input,",");

    while(ptr!=NULL){//”,“로 나눠 모든 부분 검사하기

      strcpy(command,ptr);
			ox=timer(min,max,command,position);
			if(ox==1) return 1;
      ptr=strtok(NULL,",");
      count++;

     }

	}//하나라도 일치하는 게 있다면 실행-1, 없다면 –0을 리턴
	if(ox==1) return 1;
	else return 0;
}

int timer(int min, int max,char *input,int position){
//부분을 유형별로 나누기
	int num;
	int ox=0;
	int com1;
	int com2;
	int st;
	char stand[2]={0,};
	char *ptr;
	char range[6]={0,};



	if((ptr=strchr(input,'/'))){//”/“가 있다면
		ptr = strtok(input, "/");
		strcpy(range,ptr);//앞부분 얻기
		ptr = strtok(NULL, " ");
		strcpy(stand,ptr);

		st=atoi(stand);//뒷 부분 얻기

		if(!strcmp(range,"*")){//앞 부분이 ”*“라면

			for(int i=min;i<=max;i+=st){
		//0부터 뒷부분 차이나는 숫자마다 시간 검사
				if(comparetime(i,position)) ox=1;
			}


		}else{//그 외의 값이 입력되면
			ptr = strtok(range, "-");//”-“ 앞 뒤 나누기
			com1=atoi(ptr);//앞부분
			com2=atoi(ptr+strlen(ptr)+1);//뒤 부분

		//앞부분부터 뒷부분 안의 “/”의 뒷부분만큼 커지는 숫자로 시간 검사
			for(int i=com1;i<=com2;i+=st){
				if(comparetime(i,position)) ox=1;
			}
		}
		//하나라도 맞는 것이 있다면 1, 아니면 0을 리턴
		if(ox==1) return 1;
		else return 0;

  }else if((ptr=strchr(input,'-'))){//“-”가 있다면

    ptr = strtok(input, "-");//”-“ 앞 뒤 나누기
		com1=atoi(ptr);
		com2=atoi(ptr+strlen(ptr)+1);


		for(int i=com1;i<=com2;i++){//앞부분부터 뒷부분 안의 모든 숫자 검사
			if(comparetime(i,position)) ox=1;
		}
	//하나라도 맞는 것이 있다면 1, 아니면 0을 리턴
		if(ox==1) return 1;
		else return 0;

  }else if(!strcmp(input,"*")){//”*“가 입력되면 항상 옳음

		return 1;
	}
	if(num=atoi(input)){//숫자 입력시 숫자를 인자로 검사


		ox=comparetime(num,position);

	}
	//하나라도 맞으면 1,아니면 0을 리턴
	if(ox==1) return 1;
	else return 0;
}
int comparetime(int num,int position){//현재 시간과 인자 검사

  char timebuf[25];
	char *ptr;
  char min[100];
  char hour[100];
  char day[100];
  char month[100];
  char week[100];
	int temp;
	int ox;


	strcpy(timebuf,ctime(&current));
	//현재 시간을 섹션 별로 나누기

  ptr = strtok(timebuf, " ");
  memcpy(week,ptr,sizeof(ptr));

	ptr= strtok(NULL, " ");
  memcpy(month,ptr,sizeof(ptr));

	ptr= strtok(NULL, " ");
	memcpy(day,ptr,sizeof(ptr));

	ptr= strtok(NULL, ":");
  memcpy(hour,ptr,sizeof(ptr));

	ptr= strtok(NULL, ":");
	strcpy(min,ptr);

	//분,시간,일, 달,요일 순으로 1부터 5까지 뜻함
	//검사하는 섹션 별로 확인

	if(position==1){//현재 시간과 분이 같으면 실행
		if(num==(atoi(min))) return 1;
		else return 0;
	}else if(position==2){//현재 시간과 시간이 같으면 실행
		if(num==(atoi(hour))) return 1;
		else return 0;
	}else if(position==3){//현재 시간과 날짜가 같으면 실행
		if(num==(atoi(day))) return 1;
		else return 0;
	}else if(position==4){//현재 시간과 달이 같으면 실행
		if(!strcmp(month,"Jan")) temp=1;
		else if(!strcmp(month,"Feb")) temp=2;
		else if(!strcmp(month,"Mar")) temp=3;
		else if(!strcmp(month,"Apr")) temp=4;
		else if(!strcmp(month,"May")) temp=5;
		else if(!strcmp(month,"Jun")) temp=6;
		else if(!strcmp(month,"Jul")) temp=7;
		else if(!strcmp(month,"Aug")) temp=8;
		else if(!strcmp(month,"Sep")) temp=9;
		else if(!strcmp(month,"Oct")) temp=10;
		else if(!strcmp(month,"Nov")) temp=11;
		else if(!strcmp(month,"Dec")) temp=12;
		else printf("fail\n");
		printf("temp:%d\n",temp);

		if(num==temp) return 1;
		else return 0;

	}else if(position==5){//현재 시간과 요일이 같으면 실행
		if(!strcmp(month,"Sun")) temp=0;
		else if(!strcmp(month,"Mon")) temp=1;
		else if(!strcmp(month,"Tue")) temp=2;
		else if(!strcmp(month,"Wed")) temp=3;
		else if(!strcmp(month,"Thu")) temp=4;
		else if(!strcmp(month,"Fri")) temp=5;
		else if(!strcmp(month,"Sat")) temp=6;
		else printf("fail\n");

		if(num==temp) return 1;
		else return 0;

	}else {//그외의 값은 실패
		printf("fail\n");
		return 0;
	}
	return 0;
}
