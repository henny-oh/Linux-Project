#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <utime.h>
#include <sys/time.h>


#define length 100

typedef struct record{

  char name[100];
  char size[100];

}record;

typedef struct mrecord{
  char srcpath[100];
  char dstpath[100];
}mrecord;

typedef struct trecord{
  char path[100];
  char filename[100];
}trecord;

int rop=0;
int top=0;
int mop=0;
char totalsize[10];
record arr[100];
mrecord marr[100];
trecord tarr[100];

int tnum=0;
int recordnum=0;
int mnum=0;
char sigdel[length];
char sigadd[length];

void make_copy(char *dpath,char *copypath);//dst 복사본 만들기
void copy_dstfiles(char *filepath,char *copypath,char *originpath);//dst 안 파일 복사
void rmdirs(char *path);//디렉토리 삭제
int src_check(char *spath,char *dpath,char *origin);//<src>파일들 탐색
int dst_check(char *filename,char *compath,char *origin);//<src>와 <dst> 비교-동기화 결정
void write_log(char *command);//“ssu_rsync_log”에 기록
void top_log(char *command);//t 옵션시 “ssu_rsync_log”에 기록
void signal_handler(int sig);//sigint 들어왔을 때 수행 함수
void namesave(char *filename,char *compath,char *origin);//<src>안 모든 파일 이름 저장
void mfunction(char *spath,char *dpath,char *origin);//m 옵션-<dst>안 파일 탐색
void mremove(char *dpath,char *compath);//m옵션-namesave로 저장된 이름에 없으면 삭제
int taroption(char *spath,char *dpath);//t 옵션- tar 만들기
void runtime(struct timeval* start,struct timeval*end);//수행시간 구하기
void (*ssu_func)(int);//signal

int main(int argc,char *argv[]){
  char src[length]={0,};//<src>
  char dst[length]={0,};//<dst>
  char spath[length];//<src> path
  char dpath[length];//<dst> path
  char path[length];//path buf
  char op[4]={0,};
  char copypath[length]={0,};//<dst> copy path
  int option=0;//옵션 여부
  struct stat statbuf;
  int ox=0;
  char command[100]={0,};//명령어 입력 string
  double diff;
  struct timeval start,end;


    gettimeofday(&start, NULL);


    if(argc<3){//인자가 3개 미만 입력 시 매뉴얼 출력
      fprintf(stderr,"usage: %s [option] <src> <dst>\n",argv[0]);

      exit(1);
    }
    else if(argc==3){//3개 입력 시 src와 dst 저장
      strcpy(src,argv[1]);
      strcpy(dst,argv[2]);

    }else if(argc==4){//4개 입력 시 src와 dst, 옵션 저장
      strcpy(op,argv[1]);
      strcpy(src,argv[2]);
      strcpy(dst,argv[3]);
      option=1;
    }
    else{//4개 이상 입력 시 매뉴얼 출력
      fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>",argv[0]);
      exit(1);
    }

  if(option==1){//옵션이 들어왔으면
    if(!strcmp(op,"-r")||!strcmp(op,"-t")||!strcmp(op,"-m")) {//r,m,t중 하나인지 확인
      if(!strcmp(op,"-r")) rop=1;//r옵션 설정
      else if(!strcmp(op,"-t")) top=1;//t옵션 설정
      else if(!strcmp(op,"-m")) mop=1;//m옵션 설정
      option=1;//올바른 옵션
    }
    else option=0;//틀린 옵션

    if(option==0) {//틀린 옵션 시 매뉴얼 출력
      fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>1\n",argv[0]);
      exit(1);
    }
  }



  strcpy(command,argv[0]);
  for(int i=1;i<argc;i++){//입력 인자들로 명령어 입력 string 만들기
    strcat(command," ");
    strcat(command,argv[i]);
  }

  realpath(".",path);
  realpath(src,spath);

  realpath(dst,dpath);

  if(access(spath,F_OK)<0){//<src>가 존재안하면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>2\n",argv[0]);
    exit(1);
  }
  if(access(spath,R_OK)<0){//<src>의 읽는 권한이 존재안하면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>3\n",argv[0]);
    exit(1);
  }
  if(access(spath,W_OK)<0){//<src>의 쓰는 권한이 존재안하면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>4\n",argv[0]);
    exit(1);
  }


  if(access(dpath,F_OK)<0){//<dst>가 존재안하면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>5\n",argv[0]);
    exit(1);
  }

  if(access(dpath,R_OK)<0){//<dst>의 읽는 권한이 존재안하면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>6\n",argv[0]);
    exit(1);
  }
  if(access(dpath,W_OK)<0){//<dst>의 쓰는 권한이 존재안하면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>7\n",argv[0]);
    exit(1);
  }

  lstat(dpath,&statbuf);
  if(!S_ISDIR(statbuf.st_mode)){//<dst>가 디렉토리가 아니면 매뉴얼 출력
    fprintf(stderr,"usage: %s [option(r/m/t)] <src> <dst>8\n",argv[0]);
    exit(1);
  }

  strcpy(copypath,dpath);
  strcat(copypath,"<copy>");//<dst>복사본 경로 copypath에 저장


  make_copy(dpath,copypath);//<dst> 복사본 만들기
  strcpy(sigdel,dpath);//sigint 입력시 삭제할 디렉토리
  strcpy(sigadd,copypath);//sigint 입력시 이름을 <dst>로 바꿀 디렉토리

  if (signal(SIGINT,signal_handler)==SIG_ERR) {//signal handler 등록
    fprintf(stderr,"cannot handle\n");
    exit(1);
  }


  ox=src_check(spath,dpath,spath);//<src> 동기화 여부 탐색


  if(mop==1){//m옵션 설정 시
     mfunction(spath,dpath,spath); //<src> 탐색
     mremove(dpath,dpath);//<src>에 없는 파일들을 <dst>에서 삭제
   }
   if(top==1){//t옵션 설정 시
     ox=taroption(spath,dpath);//tar 만들기
   }


  rmdirs(copypath);//<dst>복사본 ㅈ우기
  rmdir(copypath);
  chdir(path);

  if(top==0) write_log(command);//t 옵션이 설정 되지 않으면 “ssu_rsync_log”에 기록
  else top_log(command);//t 옵션이 설정 시 “ssu_rsync_log”에 다른 형식으로 기록

  gettimeofday(&end, NULL);
  runtime(&start,&end);//소요시간 출력

  exit(0);//프로그램 종료
}
void runtime(struct timeval* start,struct timeval*end){//소요시간 출력
  end->tv_sec-=start->tv_sec;//sec 구하기

  if(end->tv_usec<start->tv_usec){//종료된 시점의 usec이 시작 시점 usec보다 작을 경우
    end->tv_sec--;
    end->tv_usec+=1000000;

  }

  end->tv_usec-=start->tv_usec;//usec 구하기

  printf("%ld:%ld(sec:usec)\n",end->tv_sec,end->tv_usec);//시간 출력
}
int taroption(char *spath, char *dpath){//tar 만들기
  char command[length*10]="tar cvf";
  char command2[length]="tar xv";
  struct stat statbuf;
  char dst[length];
  long int size;
  char buf[length];
  char tarname[length]="t.tar";

  sprintf(dst," %s/%s",dpath,tarname);
  strcat(command,dst);

  if(tnum==0) return 0;


  for(int i=0;i<tnum;i++){//동기화 할 파일들 문자열에 붙이기
    strcat(command," ");
    strcat(command,tarr[i].filename);

  }


  chdir(spath);
  realpath("./",buf);


  system(command);//명령어 실행 “tar cvf <dst>/t.tar 동기화 파일들”


  chdir(dpath);

  if(access(tarname,F_OK)<0){//t.tar 존재 여부 확인
      fprintf(stderr,"usage: \n");
      exit(1);
    }

  if(stat(tarname,&statbuf)==-1){
    fprintf(stderr,"lstat error\n");
    exit(1);
  }

  realpath("./",buf);

  size=statbuf.st_size;//tar 크기 저장

  sprintf(totalsize,"%ld",size);

  strcat(command2,dst);


  system("tar xvf t.tar");//tar 합축 해제
  remove("t.tar");//tar 삭제


  return 1;

}
void top_log(char *command){//t 옵션 로그 찍기
  FILE *fp;
  char *filename="ssu_rsync_log";
  char timebuf[3*length]="[";
  time_t current=time(NULL);
  char input[length]={0,};
  char *ptr;

  strcat(timebuf,ctime(&current));//시간 구하기
  for(int i=0;i<length;i++)
    if(timebuf[i]=='\n') timebuf[i]=']';
  strcpy(input,command);
  ptr=strstr(input, "./");
  strcpy(input,ptr+2);//입력 명령어 string




  if((fp=fopen(filename,"a"))==NULL){//"ssu_rsync_log" 없으면 생성
      fprintf(stderr,"fopen error1\n");
      exit(1);
  }
  fclose(fp);

  if((fp=fopen(filename,"r+"))==NULL){//"ssu_rsync_log" 읽기 ,쓰기 가능으로 열기
      fprintf(stderr,"fopen error2\n");
      exit(1);
  }


  fseek(fp,0,SEEK_END);//파일 끝에다

  fwrite(&timebuf,sizeof(timebuf),1,fp);//시간 입력
  fwrite(" ",sizeof(" "),1,fp);
  fwrite(&input,strlen(input),1,fp);//명령어 입력
  fwrite("\n",sizeof("\n"),1,fp);
  if(tnum>0){
  fwrite("\ttotatlSize ",sizeof("\ttotatlSize "),1,fp);//tar 사이즈 입력
  fwrite(&totalsize,sizeof(totalsize),1,fp);
  fwrite("\n",sizeof("\n"),1,fp);

  for(int i=0;i<tnum;i++){//동기화한 파일들 이름 입력
    fwrite("\t",sizeof("\n"),1,fp);
    fwrite(&tarr[i].filename,sizeof(tarr[i].filename),1,fp);
    fwrite("\n",sizeof("\n"),1,fp);
  }
}
}
void write_log(char *command){//t 옵션 미설정 시 로그 찍기
  FILE *fp;
  char *filename="ssu_rsync_log";
  char timebuf[3*length]="[";
  time_t current=time(NULL);//시간 구하기
  char input[length]={0,};
  strcat(timebuf,ctime(&current));
  char *ptr;

  for(int i=0;i<length;i++)
    if(timebuf[i]=='\n') timebuf[i]=']';
  strcpy(input,command);
  ptr=strstr(input, "./");
  strcpy(input,ptr+2);//입력 명령어 string


  if((fp=fopen(filename,"a"))==NULL){//"ssu_rsync_log" 없으면 생성
      fprintf(stderr,"fopen error1\n");
      exit(1);
  }
  fclose(fp);

  if((fp=fopen(filename,"r+"))==NULL){//"ssu_rsync_log" 읽기 ,쓰기 가능으로 열기
      fprintf(stderr,"fopen error2\n");
      exit(1);
  }


  fseek(fp,0,SEEK_END);//파일 끝에다

  fwrite(&timebuf,sizeof(timebuf),1,fp);//시간 입력
  fwrite(" ",sizeof(" "),1,fp);
  fwrite(&input,strlen(input),1,fp);//입력 명령어 입력
  fwrite("\n",sizeof("\n"),1,fp);

  for(int i=0;i<recordnum;i++){//동기화한 파일이름과 크기 입력
    fwrite("\t",sizeof("\n"),1,fp);
    fwrite(&arr[i].name,sizeof(arr[i].name),1,fp);
    fwrite(" ",sizeof("\n"),1,fp);
    fwrite(&arr[i].size,sizeof(arr[i].size),1,fp);
    fwrite("\n",sizeof("\n"),1,fp);
  }
  fclose(fp);



}
int dst_check(char *filename,char *compath,char *origin){//동기화 여부 확인
  char dstpath[length]={0,};
  char file[length]={0,};
  char buf[length]={0,};
  char make[length]={0,};
  char comp[length]={0,};
  char pathbuf[length]={0,};
  char *ptr;
  FILE *fp1;
  FILE *fp2;
  int ox=0;
  struct stat statbuf1;
  struct stat statbuf2;
  struct utimbuf ubuf;
  char tsize[10];



  strcpy(file,filename);

  ptr=strstr(file,origin);
  strcpy(file,ptr+strlen(origin)+1);


  strcpy(dstpath,compath);
  strcat(dstpath,"/");
  strcat(dstpath,file);





  if(access(dstpath,F_OK)<0) {//filename으로 들어온 동기화할 <src>의 파일이 생성될 dstpath가 존재하지 않는다면 동기화 진행
    ox=1;

  }
  else{//filename으로 들어온 동기화할 <src>의 파일이 생성될 dstpath가 존재 시
    if(stat(filename,&statbuf1)==-1){
      fprintf(stderr,"lstat error\n");
      exit(1);
    }
    if(stat(dstpath,&statbuf2)==-1){
      fprintf(stderr,"lstat error\n");
      exit(1);
    }
	//filename과 크기가 다르거나 수정 시간이 다르면 동기화 진행
    if(statbuf1.st_size!=statbuf2.st_size) ox=1;
    if(statbuf1.st_mtime!=statbuf2.st_mtime) ox=1;

  }
  if(ox==0) return 0;//ox==0이면 동기화 안함
  else{//동기화 진행
    strcpy(make,file);
    strcpy(comp,compath);
    ptr=strtok(make,"/");

    while(ptr!=NULL){//<dst>내부 동기화할 파일이 저장될 위치 만들기

        strcat(comp,"/");
        strcat(comp,ptr);

        if(access(comp,F_OK)<0){//디렉토리가 없다면 생성
            mkdir(comp,0777);
          }
        ptr=strtok(NULL,"/");
    }

    rmdir(comp);//마지막 파일이 생성될 이름의 디렉토리 삭제

    if(top==1){//t옵션 설정시

	//tarr에 동기화할 파일의 이름과 절대경로 저장
      realpath(filename,pathbuf);
      strcpy(tarr[tnum].path,pathbuf);
      strcpy(tarr[tnum].filename,file);

      tnum++;
      return 1;//동기화 진행하지 않고 나가기
    }

    if((fp1=fopen(filename,"r+"))==NULL){//동기화할 파일 열기
        fprintf(stderr,"fopen errororigin\n");
        exit(1);
    }

    if((fp2=fopen(dstpath,"w+"))==NULL){//동기화로 만들 파일 생성
        fprintf(stderr,"fopen errorcopy\n");
        exit(1);
    }

    while((fgets(buf,length,fp1))!=0){//복사하기
      fputs(buf,fp2);
    }

    if(stat(filename,&statbuf1)==-1){
      fprintf(stderr,"lstat error\n");
      exit(1);
    }

    ubuf.modtime =  statbuf1.st_mtime;//동기화로 만든 파일 수정시간 수정



    fclose(fp1);
    fclose(fp2);

    if(utime(dstpath,&ubuf)<0){
      fprintf(stderr,"utimeerror\n");
      exit(1);
    }
//동기화한 파일의 이름과 크기 저장
    strcpy(arr[recordnum].name,file);

    sprintf(tsize,"%ld",statbuf1.st_size);
    strncpy(arr[recordnum].size,tsize,sizeof(tsize));


    recordnum++;

    return 1;
  }

}
void namesave(char *filename,char *compath,char *origin){//이름저장
  char dstpath[length]={0,};
  char file[length]={0,};

  char *ptr;

  strcpy(file,filename);
	//기준 <src>보다 깊이가 깊은 절대경로
  ptr=strstr(file,origin);
  strcpy(file,ptr+strlen(origin)+1);


  strcpy(dstpath,compath);
  strcat(dstpath,"/");
  strcat(dstpath,file);

//기준 <src>보다 깊이가 깊은 절대경로 marr에 절대경로와 파일이름 저장
  strcpy(marr[mnum].dstpath,dstpath);

  strcpy(marr[mnum].srcpath,filename);

  mnum++;



}
void mfunction(char *spath,char *dpath,char *origin){//m옵션 설정 시<src> 탐색
  char srcpath[length]={0,};
  char dstpath[length]={0,};
  char filename[length]={0,};
  char filepath[length]={0,};
  DIR *dp;
  struct dirent *dentry;
  struct stat statbuf;
  struct stat statbufs;
  int ox=1;

  strcpy(srcpath,spath);
  strcpy(dstpath,dpath);

  if(stat(srcpath,&statbuf)==-1){
    fprintf(stderr,"lstat error\n");
    exit(1);
  }

  if(!S_ISREG(statbuf.st_mode)){//<src> 내부 탐색
    if((dp=opendir(srcpath))==NULL||chdir(srcpath)==-1){
      fprintf(stderr,"cant open dir\n");
      exit(1);
    }

  while((dentry=readdir(dp))!=NULL){
    memset(filename,0,length);
    memset(filepath,0,length);
    strcpy(filename,dentry->d_name);
    realpath(filename,filepath);

    if(!strcmp(filename,".")||!strcmp(filename,"..")) continue;

    if(lstat(filename,&statbufs)==-1){
      fprintf(stderr,"lstat error1\n");
      exit(1);
    }

    if(!S_ISREG(statbufs.st_mode)){//디렉토리면
      mfunction(filepath,dpath,origin);//재귀
    }else{//파일이면
      namesave(filepath,dpath,origin);//이름 저장하는 함수로
    }

    chdir(srcpath);
  }


  }else{//<src>가 파일이면
    namesave(srcpath,dpath,origin);//이름 저장하는 함수로
  }


}

void mremove(char *dpath,char *compath){//m옵션-저장되지 않은 파일들 삭제
  char srcpath[length]={0,};
  char dstpath[length]={0,};
  char filename[length]={0,};
  char filepath[length]={0,};
  DIR *dp;
  struct dirent *dentry;
  struct stat statbuf;
  struct stat statbufs;
  int ox=0;
  char cpath[length]={0,};
  char savepath[length]={0,};
  char *ptr;

  strcpy(cpath,compath);
  ptr=strstr(dpath,cpath);
  strcpy(savepath,ptr+strlen(dpath)+1);

  strcpy(dstpath,dpath);

  if(stat(dstpath,&statbuf)==-1){
    fprintf(stderr,"lstat error\n");
    exit(1);
  }

  if(!S_ISREG(statbuf.st_mode)){//<dst>탐색
    if((dp=opendir(dstpath))==NULL||chdir(dstpath)==-1){
      fprintf(stderr,"cant open dir\n");
      exit(1);
    }

  while((dentry=readdir(dp))!=NULL){
    memset(filename,0,length);
    memset(filepath,0,length);
    strcpy(filename,dentry->d_name);
    realpath(filename,filepath);


    if(!strcmp(filename,".")||!strcmp(filename,"..")) continue;

    if(lstat(filename,&statbufs)==-1){
      fprintf(stderr,"lstat error1\n");
      exit(1);
    }

    if(!S_ISREG(statbufs.st_mode)){//디렉토리면
      mremove(filepath,cpath);//재귀
    }else{//파일이면
      ox=0;
      for(int i=0;i<mnum;i++){
	//저장된 파일이름에 저장된 파일이면 ox=1
        if(!strcmp(filepath,marr[i].dstpath)) ox=1;

      }
      if(ox==0) {//저장되지 않은 파일이면
        strcpy(cpath,compath);
        ptr=strstr(filepath,cpath);
        strcpy(savepath,ptr+strlen(dpath)+1);

        strcpy(arr[recordnum].name,savepath);//arr에 이름저장
        strcpy(arr[recordnum].size,"delete");//size에 delete 저장
        recordnum++;

        remove(filepath);//삭제
      }
    }


    chdir(dstpath);
  }


  }else{//파일이면

    for(int i=0;i<mnum;i++){
	//저장된 파일이름에 저장된 파일이면 ox=1
      if(!strcmp(dstpath,marr[i].dstpath)) ox=1;
    }
    if(ox==0) {//저장되지 않은 파일이면
      strcpy(arr[recordnum].name,savepath);//arr에 이름저장
      strcpy(arr[recordnum].size,"delete");//size에 delete 저장
      recordnum++;

      remove(dstpath);//삭제
    }
  }
}
int src_check(char *spath,char *dpath,char *origin){//동기화할 파일 찾기
  char srcpath[length]={0,};
  char dstpath[length]={0,};
  char filename[length]={0,};
  char filepath[length]={0,};
  DIR *dp;
  struct dirent *dentry;
  struct stat statbuf;
  struct stat statbufs;
  int ox=1;

  strcpy(srcpath,spath);
  strcpy(dstpath,dpath);


  if(stat(srcpath,&statbuf)==-1){
    fprintf(stderr,"lstat error\n");
    exit(1);
  }

  if(!S_ISREG(statbuf.st_mode)){//디렉토리면
    if((dp=opendir(srcpath))==NULL||chdir(srcpath)==-1){
      fprintf(stderr,"cant open dir\n");
      exit(1);
    }

  while((dentry=readdir(dp))!=NULL){//디렉토리 탐색
    memset(filename,0,length);
    memset(filepath,0,length);
    strcpy(filename,dentry->d_name);
    realpath(filename,filepath);


    if(!strcmp(filename,".")||!strcmp(filename,"..")) continue;


    if(lstat(filename,&statbufs)==-1){
      fprintf(stderr,"lstat error1\n");
      exit(1);
    }

    if(!S_ISREG(statbufs.st_mode)){//디렉토리면
      if(rop==1){//r옵션 설정 시

        if(src_check(filepath,dstpath,origin)==0) ox= 1;//재귀

      }
    }else{//파일이면

      if(dst_check(filepath,dstpath,origin)==1) ox= 1;//동기화 여부 판단
    }
    chdir(srcpath);
  }


  }else{//파일이면
    if((ox=dst_check(filepath,dstpath,origin))==0) {//동기화 여부 판단

      ox= 0;
    }else ox=1;
  }

  if(ox==0) return 0;//동기화 한 파일이 없으면 0
  else return 1;//있으면 1 리턴
}
void make_copy(char *dpath,char *copypath){//<dst>복사본 만들기

  char originpath[length]={0,};
  char copy[length]={0,};

  strcpy(originpath,dpath);
  strcpy(copy,copypath);

  mkdir(copy,0777);//복사할 디렉토리 만들기
  copy_dstfiles(originpath,copy,originpath);// 내부 파일 복사하기

}
void rmdirs(char *path){//디렉토리 삭제
  DIR *dp;
  struct dirent *dentry;
  struct stat statbuf;
  char filename[length];
  char filepath[length];

  if((dp=opendir(path))==NULL||chdir(path)==-1){//디렉토리 탐색
    fprintf(stderr,"cant open dir\n");
    exit(1);
  }

  while((dentry=readdir(dp))!=NULL){
    memset(filename,0,length);
    strcpy(filename,dentry->d_name);
    realpath(filename,filepath);

    if(!strcmp(filename,".")||!strcmp(filename,"..")) continue;

    if(stat(filename,&statbuf)==-1){
      fprintf(stderr,"lstat error\n");
      exit(1);
    }

    if(!S_ISREG(statbuf.st_mode)){//디렉토리면

      rmdirs(filepath);//재귀-내부 파일 삭제
      rmdir(filepath);//해당 디렉토리 지우기
    }
    else {//파일이면

      remove(filepath);//파일 삭제
    }


}
}
//<dst> 파일들 복사
void copy_dstfiles(char *filepath,char *copypath,char *originpath){
  char file[length];
  char copy[length];
  char filename[length];
  char fpath[length];
  char copybuf[length];
  struct stat statbuf;
  struct utimbuf ubuf;
  char buf[length];
  struct dirent *dentry;
  DIR *dirp;
  FILE *fp1;
  FILE *fp2;

  strcpy(file,filepath);
  strcpy(copy,copypath);
  strcpy(copybuf,copypath);

  if(stat(file,&statbuf)==-1){
    fprintf(stderr,"lstat error\n");
    exit(1);
  }

  if(!S_ISREG(statbuf.st_mode)){//디렉토리면 내부 탐색

    if((dirp=opendir(file))==NULL||chdir(file)==-1){
      fprintf(stderr,"cant open dir\n");
      exit(1);
    }

    while((dentry=readdir(dirp))!=NULL){
      memset(filename,0,length);
      strcpy(filename,dentry->d_name);
      realpath(filename,fpath);

      if(!strcmp(filename,".")||!strcmp(filename,"..")) continue;

      if(stat(filename,&statbuf)==-1){
        fprintf(stderr,"lstat error\n");
        exit(1);
      }
      strcpy(copy,copybuf);
      strcat(copy,"/");
      strcat(copy,filename);

      if(!S_ISREG(statbuf.st_mode)){//디렉토리면


        mkdir(copy,0777);//<dst>안에 동기화할 파일 디렉토리 생성
        copy_dstfiles(fpath,copy,originpath);//재귀
      }
      else {//파일이면

        if((fp1=fopen(fpath,"r+"))==NULL){//동기화할 파일 열기
          fprintf(stderr,"fopen errororigin\n");
          exit(1);
        }

        if((fp2=fopen(copy,"w+"))==NULL){//<dst>에 동기화 진행할 파일만들기
          fprintf(stderr,"fopen errorcopy\n");
          exit(1);
        }

        while((fgets(buf,length,fp1))!=0){//파일 복사
          fputs(buf,fp2);
        }
        ubuf.actime = statbuf.st_atime;//접근 시간 설정
        ubuf.modtime =  statbuf.st_mtime;//수정 시간 설정
        fclose(fp1);
        fclose(fp2);
        if(utime(copy,&ubuf)<0){
          fprintf(stderr,"utimeerror\n");
          exit(1);
        }
      }

    }
  }else{//파일이면
    if((fp1=fopen(filepath,"r+"))==NULL){//동기화할 파일 열기
      fprintf(stderr,"fopen errororigin\n");
      exit(1);
    }

    if((fp2=fopen(copy,"w+"))==NULL){//<dst>에 동기화 진행할 파일만들기
      fprintf(stderr,"fopen errorcopy\n");
      exit(1);
    }
    while((fgets(buf,length,fp1))!=0){//파일 복사
      fputs(buf,fp2);
    }
    ubuf.actime = statbuf.st_atime;//접근 시간 설정
    ubuf.modtime =  statbuf.st_mtime;//수정 시간 설정
    fclose(fp1);
    fclose(fp2);
    if(utime(copy,&ubuf)<0){
      fprintf(stderr,"utimeerror\n");
      exit(1);
    }
  }
}
void signal_handler(int sig){//sigint 설정 시 실행 함수
  if(sig==SIGINT){//sigint가 들어왔을 때
    if(access(sigdel,F_OK)<0){//<dst>의 파일이 존재한지 않는다면
        signal(SIGINT,ssu_func);//돌아가기
      }
    else{//존재한다면
    rmdirs(sigdel);//원본<dst>안 파일들 지우기
    rmdir(sigdel);//원본 <dst> 디렉토리 지우기
    rename(sigadd,sigdel);//<dst><copy>를 <dst>로 바꾸기
    printf("sigint!\n");

    exit(EXIT_SUCCESS);//종료
    }
  }
}
