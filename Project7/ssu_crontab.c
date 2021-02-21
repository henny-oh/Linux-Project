#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>

#define length 100

//각 섹션별 최대 최소 값들
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

FILE *fp;



void add(char *input);//명령어저장
void remo(int num);//명령어 삭제
int check_time(char *input);//실행주기 섹션 나누기
int grammar_check(int min,int max,char *input);//섹션별 부분 나누기
int token_check(int min, int max,char *input);//부분별 유형 나누기
int bar_check(int min, int max,char *input);//“-”가 들어간 부분 문법 검사
int slash_check(int min, int max,char *input);//“/”가 들어간 부분 문법 검사
void write_log(char *command,char*input);//log에 기록
void runtime(struct timeval* start,struct timeval*end);//실행 시간 계산


int main(void){
  char *line=(void *)malloc(length);
  char *filename="ssu_crontab_file";
  char *command;
  char *left=(void *)malloc(length);
  int size;
  int i=0;
  int renum;
  int ox;
  char save[100];
  char put[length]={0,};
  struct timeval start,end;


  gettimeofday(&start, NULL);//프로그램 실행 시작
  memset(line,0,length);
  memset(left,0,length);
  if((fp=fopen(filename,"a"))==NULL){//"ssu_crontab_file"가 없으면 생성
      fprintf(stderr,"fopen error\n");
      exit(1);
  }
  fclose(fp);



  while(1){

    if((fp=fopen(filename,"r+"))==NULL){//"ssu_crontab_file" 열기
        fprintf(stderr,"fopen error\n");
        exit(1);
    }



    fseek(fp,0,SEEK_END);
    size=ftell(fp);



    fseek(fp,0,SEEK_SET);
    if(size>0){//"ssu_crontab_file"에 내용이 있으면
      size=size/length;
      i=0;
      for(int j=0;j<size;j++){//한 줄씩 읽기

        fread(line,length,1,fp);
        printf("%d.%s\n",j,line);
        memset(line,0,length);
      }

    }


    memset(line,0,length);
    memset(put,0,length);
    printf("\n20170776>");//프롬프트 출력
    scanf("%[^\n]",line);//입력 받기


    getchar();

    if(strlen(line)==0) continue;//엔터 입력 시 다시 프롬프트 출력

    if((command=strtok(line," "))==0) {//입력에서 명령어 부분 얻기
      printf("wrong input\n");
      continue;
    }

    if(strcmp("add",command)==0){//명령어가 add면
      left=strtok(NULL,"\0");//명령어 뒤 실행주기와 실행하고자 하는 명령어 얻기

      strcpy(save,left);
      ox=check_time(left);//실행주기 섹션 별 나눠 문법 체크
      if(ox==0) {//문법이 틀리면
        printf("wrong time input\n");
        continue;//다시 프롬프트 출력
      }
      else {//문법이 맞으면

        add(save);//실행주기와 실행하고자 하는 명령어 저장

      }
    }
    else if(strcmp("remove",command)==0){//명령어가 “remove”면
      left=strtok(NULL,"\0");//명령어 뒤 실행주기와 실행하고자 하는 명령어 얻기

      strcpy(save,left);
      renum=atoi(left);// 숫자 얻기
      if(renum>size)//파일에 저장된 문자보다 크면 오류 출력
        printf("wrong input\n");
      else{
        remo(renum);//해당 줄 제거
        if((fp=fopen(filename,"r+"))==NULL){//"ssu_crontab_file"열기
            fprintf(stderr,"fopen error\n");
            exit(1);
        }
        fseek(fp,0,SEEK_END);
        size=ftell(fp);//파일 크기 구하기

        fseek(fp,0,SEEK_SET);
        if(size>0){//내용이 있으면
          size=size/length;

          for(int j=0;j<size;j++){//라인별 출력

            fread(line,length,1,fp);

            memset(line,0,length);
          }

        }
      }

    }
    else if(strcmp("exit",command)==0){//명령어가 “exit”이면
        break;//탈출
    }

    else{//그 외 입력 에러 처리
      printf("wrong input\n");
    }

    fclose(fp);

  }
  gettimeofday(&end, NULL);//실행 종료
  runtime(&start,&end);//실행 시간 계산
  fclose(fp);
  exit(0);//프로그램 종료
}
void runtime(struct timeval* start,struct timeval*end){
  end->tv_sec-=start->tv_sec;//sec구하기

  if(end->tv_usec<start->tv_usec){//실행 종료 usec이 실행 시작 보다 작으면
    end->tv_sec--;//sec에서 1 빼고
    end->tv_usec+=1000000;//usec에 더한다.

  }

  end->tv_usec-=start->tv_usec;//usec 구하기
  printf("Runtime:%ld:%06ld(sec:usec)\n",end->tv_sec,end->tv_usec);//시간 출력
}
void write_log(char *command,char*input){//"ssu_crontab_log"에 기록
  FILE *fp;
  char *filename="ssu_crontab_log";
  time_t current=time(NULL);
  char timebuf[27]={0,};
  char commandbuf[length]={0,};

  int fd;
  struct flock lock;

  strcpy(commandbuf,input);

  if((fp=fopen(filename,"a"))==NULL){//"ssu_crontab_log" 없으면 생성
      fprintf(stderr,"fopen error\n");
      exit(1);
  }
  fclose(fp);




  if((fp=fopen(filename,"r+"))==NULL){//"ssu_crontab_log" 열기
      fprintf(stderr,"fopen error\n");
      exit(1);
  }

  strcpy(timebuf,ctime(&current));//명령어입력시간 받기

  for(int i=0;i<strlen(timebuf);i++)//계행 제거
    if(timebuf[i]=='\n') timebuf[i]='\0';

    if((fd = fileno(fp)) == -1) {//fd얻기
      fprintf(stderr, "Error:fileno\n");
      exit(1);
    }


    lock.l_type=F_WRLCK;
    lock.l_whence=0;
    lock.l_start=0;
    lock.l_len=0;
    if(fcntl(fd,F_SETLKW,&lock));//"ssu_crontab_log" write lock
//실행시간, 행위, 명령어 넣기
  fseek(fp,0,SEEK_END);
  fwrite("[",1,1,fp);
  fwrite(timebuf,strlen(timebuf),1,fp);
  fwrite("] ",2,1,fp);
  fwrite(command,strlen(command),1,fp);
  fwrite(" ",1,1,fp);
  fwrite(commandbuf,strlen(commandbuf),1,fp);
  fwrite("\n",1,1,fp);

  lock.l_type=F_UNLCK;
  fcntl(fd,F_SETLK,&lock);//writelock 해제
  fclose(fp);


}
void add(char *input){//명령어 “ssu_crontab_file”에 추가
  time_t current=time(NULL);
  char commandbuf[length-1]={0,};
  int pos;
  char buf[100];
  int fd;
  struct flock lock;

  lock.l_type=F_RDLCK;
  lock.l_whence=0;
  lock.l_start=0;
  lock.l_len=0;

  if((fd = fileno(fp)) == -1) {//“ssu_crontab_file”의 fd얻기
    fprintf(stderr, "Error:fileno\n");
    exit(1);
  }
  if(fcntl(fd,F_SETLKW,&lock));//“ssu_crontab_file”에 write lock
  strncpy(commandbuf,input,strlen(input));

  for(int i=0;i<length;i++)//계행 제거
    if(commandbuf[i]=='\n') commandbuf[i]='\0';

  fseek(fp,0,SEEK_END);
  pos=ftell(fp);

  fwrite(commandbuf,length,1,fp);//파일 끝에다 명령어 쓰기

  fseek(fp,pos,SEEK_END);
  fread(buf,length,1, fp);

  lock.l_type=F_UNLCK;
  fcntl(fd,F_SETLK,&lock);//“ssu_crontab_file” write lock 해제

  write_log("add",input);//“add”행위로 명령어 log파일에 기록

}
int check_time(char *input){//실행 주기 섹션 별 나누기
  char *ptr;
  char min[100];
  char hour[100];
  char day[100];
  char month[100];
  char week[100];
  int ox=1;
//실행주기를 분,시간,일,달,요일로 나눈다.
  if((ptr = strtok(input, " "))==0) return 0;

  strcpy(min,ptr);


  if((ptr= strtok(NULL, " "))==0) return 0;
  memcpy(hour,ptr,sizeof(ptr));


  if((ptr= strtok(NULL, " "))==0) return 0;
  memcpy(day,ptr,sizeof(ptr));
  if((ptr= strtok(NULL, " "))==0) return 0;
  memcpy(month,ptr,sizeof(ptr));
  if((ptr= strtok(NULL, " "))==0) return 0;
  memcpy(week,ptr,sizeof(ptr));
//섹션별로 올바르게 입력된 실행주기인지 확인한다. 하나라도 틀리면 0을 리턴
  ox=grammar_check(min_min,min_max,min);
  if(ox==0) return 0;

  ox=grammar_check(hour_min,hour_max,hour);
  if(ox==0) return 0;

  ox=grammar_check(day_min,day_max,day);
  if(ox==0) return 0;

  ox=grammar_check(month_min,month_max,month);
  if(ox==0) return 0;

  ox=grammar_check(week_min,week_max,week);
  if(ox==0) return 0;

  return 1;//모두 올바르면 1을 리턴
}
int grammar_check(int min, int max,char *input){//입력된 섹션을 “,”로 나누기
  char *ptr;
  char *cmp;
  int ox;
  int result=1;
  char arr[100][100]={0,};
  int count=0;

  ptr=strchr(input,',');
  if(ptr==NULL){//“,”가 없다면 입력된 부분의 문법을 검사한다.
    ox=token_check(min,max,input);
    result=1;
    if(ox==0) return 0;
  }else{//“,”가 있다면
    ptr=strtok(input,",");

    while(ptr!=NULL){//“,”을 기준으로 전부 나눈다.

      strcpy(arr[count],ptr);
      ptr=strtok(NULL,",");
      count++;

     }

     for(int i=0;i<count;i++){//나눠진 부분을 모두 문법 검사를 한다.
       ox=token_check(min,max,arr[i]);
       if(ox==0) return 0;//하나라도 틀리면 0을 리턴한다.
     }
  }

  return 1;//모두 올바르면 1을 리턴한다.
}
int token_check(int min, int max,char *input){//문법을 확인
  char *ptr;
  char *cmp;
  int num;

  int ox;

  if((ptr=strchr(input,'/'))!=NULL){//“/”가 있다면
    ox=slash_check(min,max,input);//“/”검사하는 함수 호출

  }
  else if((ptr=strchr(input,'-'))!=NULL){//“-”가 있다면 “-”검사하는 함수 호출
    ox=bar_check(min,max,input);

  }
  else if(!strcmp(input,"*")) ox=1;//“*”가 있다면 모든 값이므로 항상 옳다
  else if(atoi(input)==min) return 1;//입력된 숫자가 0이면 섹션 최솟값이면 옳다
  else if((num=atoi(input))!=0){//숫자가 입력되었다면
    if(min<=num&&num<=max){//그 숫자가 최솟값과 최댓값 사이 값인 지 확인

    return 1;//사이 값이면 옳다
    }
    else return 0;//아니면 틀리다
  }
  else return 0;//그 외의 값이 입력되면 틀리다

  if(ox==0) return 0;//하나라도 틀리면 0
  else return 1;//다 맞으면 1을 리턴
}
int bar_check(int min, int max,char *input){//“-” 부분 검사
  char *cmp;
  int num1;
  int num2;
  int ox=0;

  cmp=strtok(input,"-");//“-” 앞부분 검사

  if((num1=atoi(cmp))==0) {//“0”이 입력되면
     if(atoi(input)==min) ox= 1;//그 값이 최솟값이랑 같다면 옳다
     else return 0;//아니면 틀리다
  }
  else ox=1;//그 외의 값은 일단 옳다고 판단

  cmp=strtok(NULL," ");
	//그 뒤의 값
  if(strcmp(cmp,"*")==0) ox= 1;//*가 입력되면 항상 옳다
  else{//그 외의 값은
    if((num2=atoi(cmp))==0) return 0;//그 뒤의 값이 0이면 틀리다
    else {//0이 아니라면
	//앞 뒤 숫자가 모두 최댓값과 최솟값 사이어야 한다
      if(min<=num1&&num1<=max&&min<=num2&&num2<=max) {
        if(num1>=num2) {//앞의 값이 뒤의 값보다 크면 틀리다

          return 0;
        }
        else {//아니면 맞다

          return 1;
        }
    }else return 0;
  }
}
}
int slash_check(int min, int max,char *input){//“/”부분 검사
  char *ptr;
  char *cmp;
  int num2;
  int ox;
  char buf[length]={0,};

  cmp=strtok(input,"/");//“/”앞 확인
  ptr=strtok(NULL," ");

  if((strchr(input,'-'))!=NULL){//“-”로 되어있으면
    ox=bar_check(min,max,cmp);//“-”검사 함수 호출
    if(ox==0) return 0;//틀리면 0 리턴

  }else if(!strcmp(cmp,"*")) {//“*”이 입력되면 pass
    ;
  }
  else return 0;//그 외의 값은 틀리다.


  sprintf(buf,"%s",ptr);

  if((num2=atoi(buf))==0) {//그 뒤의 값이 0이면 틀리다

    return 0;
  }
  else{//0이 아닌 값이라면
    if(min<=num2&&num2<=max) {//그 숫자가 섹션 범위안의 숫자라면 옳다

    }
    else return 0;//아니면 틀리다

  }

  return 1;

}
void remo(int num){//명령어 제거 함수
  char arr[1000][length];
  char buf[length];
  int end;
  char put[length]={0,};
  int fd;
  struct flock lock;

  fseek(fp,0,SEEK_END);
  end=ftell(fp);
//“ssu_crontab_file” read lock걸기
  lock.l_type=F_RDLCK;
  lock.l_whence=0;
  lock.l_start=0;
  lock.l_len=0;

  if((fd = fileno(fp)) == -1) {
    fprintf(stderr, "Error:fileno\n");
    exit(1);
  }

  if(fcntl(fd,F_SETLKW,&lock));
  fseek(fp,0,SEEK_SET);
  for(int i=0;i<end/100;i++){//파일 안 모든 명령어를 한줄씩 배열에 저장
    fread(buf,length,1,fp);
    strcpy(arr[i],buf);

  }
  fclose(fp);
  fp=fopen("ssu_crontab_file","w");//새로운 “ssu_crontab_file”파일 만들기
  for(int i=0;i<end/100;i++){//저장된 배열 하나씩 새로운 파일에 저장
    if(i==num){//제거할 줄의 명령어는
      strcpy(put,arr[i]);//log에 넣을 문자열에 넣기

      continue;//계속 진행
    }
    fwrite(arr[i],length,1,fp);//그외의 값은 파일에 저장
  }
//read lock 해제
  lock.l_type=F_UNLCK;
  fcntl(fd,F_SETLK,&lock);

  fclose(fp);
  write_log("remove",put);//“remove”행위로 제거 배열 log 에 저장


}
