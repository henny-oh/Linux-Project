#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <asm/param.h>
#include <time.h>

int aop=0;
int uop=0;
int xop=0;
int nop=0;
int auop=0;
int axop=0;
int uxop=0;
int auxop=0;

void getmyinfo();
void searchproc();
void gettime(char statinfo[][10]);

void gettty();//터미널 번호 얻기

void getmyuid();//내 uid 얻기

void searchproc_uop();
void getcpu(char statinfo[][10]);
void finduser();


void getstat(char num[10]);
void getstarttime(char statinfo[][10]);
int searchstatus(char num[10]);//status 파일 탐색
void getstatfield(char statinfo[][10]);

char filestinfo[51][10];
char mybuf[500];
char tty[10];
char username[10];
uid_t id;
char myid[5];
int ttynum;
int ptsnum;


int main(int argc, char* argv[]){

  if(argc==1){//pps 명령어 수행
     getmyinfo();//현재 프로세스의 정보가져오기

     /*for(int i=0;i<52;i++){
       printf("%i:%s\n",i,filestinfo[i]);
     }*/
     nop=1;


     printf("%-s\t%s\t%-s\t%s\n","PID","TTY","TIME","CMD");


     searchproc();//"proc"파일 탐색
   }
   else if(argc == 2){//옵션 입력 시


     if(!strcmp(argv[1],"a")){//a 옵션
       aop=1;
     }
     else if(!strcmp(argv[1],"u")){//u 옵션
       uop=1;
     }
     else if(!strcmp(argv[1],"x")){//x 옵션
       xop=1;
     }
     else if(!strcmp(argv[1],"au")){
       auop=1;
     }
     else if(!strcmp(argv[1],"ax")){
       axop=1;

     }
     else if(!strcmp(argv[1],"ux")){
       uxop=1;
     }
     else if(!strcmp(argv[1],"aux")){
       auxop=1;

     }
     else{
       printf("wrong input! try agin\n");
       exit(1);
     }

   }
   else{
     printf("wrong input! try agin\n");
     exit(1);
   }

   if(uop==1){

     getmyuid();//내 uid 얻기
     gettty();//터미널 번호 얻기
     printf("  USER\tPID\t%%CPU\t%%MEM\tVSZ\tRSS\tTTY\tSTAT\tSTART\tTIME\tCOMMAND\n");


     sprintf(myid,"%d",id);
     searchproc_uop();
   }
   else if(aop==1){//a 옵션
     gettty();//터미널 번호 얻기
     printf(" PID\tTTY STAT TIME\tCOMMAND\n");
     searchproc();//"proc"파일 탐색
   }else if(xop==1){//x 옵션

     finduser();//로그인된 user 찾기
      printf(" PID\tTTY STAT TIME\tCOMMAND\n");
      gettty();//터미널 번호 얻기
      searchproc();//"proc"파일 탐색
   }else if(auop==1){//au 옵션
     gettty();//터미널 번호 얻기
     getmyuid();//내 uid 얻기
     printf("  USER\tPID\t%%CPU\t%%MEM\tVSZ\tRSS\tTTY\tSTAT\tSTART\tTIME\tCOMMAND\n");
     searchproc_uop();//"proc"파일 탐색
   }else if(uxop==1){
     finduser();//로그인된 user 찾기
     printf("  USER\tPID\t%%CPU\t%%MEM\tVSZ\tRSS\tTTY\tSTAT\tSTART\tTIME\tCOMMAND\n");
     searchproc_uop();//"proc"파일 탐색
   }else if(axop==1){
     gettty();//터미널 번호 얻기
     printf(" PID\tTTY STAT TIME\tCOMMAND\n");
     searchproc();//"proc"파일 탐색
   }else if(auxop==1){
     gettty();//터미널 번호 얻기
     printf("  USER\tPID\t%%CPU\t%%MEM\tVSZ\tRSS\tTTY\tSTAT\tSTART\tTIME\tCOMMAND\n");
     searchproc_uop();//"proc"파일 탐색
   }



}
void getstarttime(char statinfo[][10]){//starttime 구하기
  char path[30];
  FILE *fp;
  char *ptr;
  char buf[50];
  int btime;
  int num;
  char field[20];
  int starttime;
  struct tm *tm;
  time_t time;

  strcpy(path,"/proc/stat");

  if((fp=fopen(path,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }

  if(fp!=NULL){
    while(!feof(fp)){
      fgets(buf,sizeof(buf),fp);

      ptr=strtok(buf,"\n");

      while(ptr!=NULL){
        strcpy(buf,ptr);
//btime 구하기
        sscanf(buf,"%s %d",field,&num);
        if(!strcmp(field,"btime")){

          btime=num;
        }
        //printf(": %s\n",buf);
        ptr=strtok(NULL,"\n");

        }
      }
    }
    fclose(fp);

    starttime=atoi(statinfo[21]);
//프로세스 스타트 타임 구하기
    btime+=(starttime/sysconf(_SC_CLK_TCK));
    time=btime;
    tm=localtime(&time);



    printf("%02d:%02d\t",tm->tm_hour,tm->tm_min);



}
void getstatfield(char statinfo[][10]){
  int num;
  FILE *fp;
  char buf[500];
  char path[100];
  char *ptr;
  char field[10];
//s 출력
  printf("%s",statinfo[2]);

  if(!strcmp(statinfo[0],statinfo[4])) printf("s");

  sprintf(path,"/proc/%s/status",statinfo[0]);

  if((fp=fopen(path,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }

  if(fp!=NULL){
    while(!feof(fp)){
      fgets(buf,sizeof(buf),fp);

      ptr=strtok(buf,"\n");

      while(ptr!=NULL){
        strcpy(buf,ptr);

        sscanf(buf,"%s %d",field,&num);
        if(!strcmp(field,"Mems_allowed_list")){//mems_allowed_list 가 0이 아니면
          if(num!=0)
          printf("L");

        }
        //printf(": %s\n",buf);
        ptr=strtok(NULL,"\n");

        }
      }
    }
    fclose(fp);
  num=atoi(statinfo[19]);//thread 개수가 1 이 아니면
  if(num!=1) printf("l");

  if(!strcmp(statinfo[4],statinfo[7])) printf("+");//foreground process
  num=atoi(statinfo[18]);//우선순위가 높으면
  if(num<0) printf("<");
  else if(num>0) printf("N");//우선 순위가 낮으면
  printf("  ");
}
void finduser(){//로그인된 user 찾기
  char name[10];
  struct passwd *us;

  strcpy(name,getlogin());//username
  us=getpwnam(name);

  id=us->pw_uid;//uid 구하기
  //printf("%d\n",id);
}
void gettty(){//터미널 번호 얻기
  char path[30];
  FILE *fp;
  char *ptr;
  char buf[50];
  int num;
  char device[20];
//종류 별 major 번호 찾기
  strcpy(path,"/proc/devices");

  if((fp=fopen(path,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }

  if(fp!=NULL){
    while(!feof(fp)){
      fgets(buf,sizeof(buf),fp);

      ptr=strtok(buf,"\n");

      while(ptr!=NULL){
        strcpy(buf,ptr);
        sscanf(buf,"%d %s",&num,device);
        if(!strcmp(device,"tty")){//tty의 major번호 저장
          ttynum=num;
        }
        if(!strcmp(device,"pts")){//pts의 major번호 저장
          ptsnum=num;
        }
        //printf(": %s\n",buf);
        ptr=strtok(NULL,"\n");

        }
      }
    }

}
int searchstatus(char num[10]){//status 파일 탐색
  char path[100];
  FILE *fp;
  char buf[500];
  sprintf(path,"/proc/%d/status",atoi(num));
  int uid;
  char *ptr;
  //printf("%s\n",path);

  if((fp=fopen(path,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }
//프로세스의 uid구하기
  if(fp!=NULL){

      while(!feof(fp)){

      fgets(buf,sizeof(buf),fp);

      if(strstr(buf,"Uid") )   {
        //printf("%s\n",statusbuf);
        sscanf(buf,"%*s %d %*d %*d",&uid);
      }

      ptr=strtok(buf," ");

    }

  }
  fclose(fp);
  if(id==uid) return 1;//프로세스의 uid와 내 uid가 같으면 1
  else return 0;
}
void searchproc_uop(){
  char path[50];
  DIR *dirp;
  struct dirent *dentry;
  int num;
  char dnum[50];
  FILE *fp;
  char *ptr;
  char *ptr2;
  char buf[500];
  char statusbuf[500];
  char meminfobuf[500];
  char statinfo[51][10];
  int i=0;
  int cpu;
  int min,sec,hour=0;
  int duidnum;
  char duid[5];
  int k=0;
  char vsz[10];
  int rss=0;
  int ex=0;
  int memtotal;
  double mem;
  int terminal;
  int question=0;
  int minor;
  struct passwd*tidfo;


//meminfo에서 메모리 정보를 가져온다.
  strcpy(path,"/proc/meminfo");

  if((fp=fopen(path,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }

//memtotal 가져오기
    fgets(meminfobuf,sizeof(meminfobuf),fp);
    //printf("%s\n",meminfobuf);



    sscanf(meminfobuf,"%*s %d %*s",&memtotal);
    //printf("mem :%d\n",memtotal);


  fclose(fp);
//"/proc"안의 파일을 탐색한다.
  if((dirp=opendir("/proc"))==NULL){
    printf("opendir error\n");
    exit(0);
  }

  while((dentry=readdir(dirp))){
    num=0;
    i=0;
    rss=0;
    if(strcmp(dentry->d_name,".")&&!strcmp(dentry->d_name,".."))//".",".."는 패스
      continue;

    for(int i=0;i<strlen(dentry->d_name);i++){//이름이 숫자로만 이뤄져있는지 확인
      if(isdigit(dentry->d_name[i])==0) num=1;
    }
    if(num==1) continue;//파일 이름이 숫자로만 이뤄져 있으면 계속 진행

  //  printf("%s\n",dentry->d_name);

    strcpy(dnum,dentry->d_name);

    sprintf(path,"/proc/%s/status",dnum);

    //printf("%s\n",path);
//"/proc/pid/status" 안 정보 저장
    if((fp=fopen(path,"r"))==NULL){
      fprintf(stderr,"fopen error\n");
      exit(1);
    }

    if(fp!=NULL){

        while(!feof(fp)){

        fgets(statusbuf,sizeof(statusbuf),fp);
//uid,vmsize,rssanon,rssfile,rssshmem 저장
        if(strstr(statusbuf,"Uid") )   {
          //printf(" %s",statusbuf);
          sscanf(statusbuf,"%*s %s %d %*d",duid,&duidnum);
          //printf("duid %s\n",duid);
        }
        if(strstr(statusbuf,"VmSize")) {
          //printf("%s\n",statusbuf);
          sscanf(statusbuf,"%*s %s %*s",vsz);
        }
        if(strstr(statusbuf,"RssAnon")||strstr(statusbuf,"RssFile")||strstr(statusbuf,"RssShmem")){
          //printf("%s\n",statusbuf);
          sscanf(statusbuf,"%*s %d %*s",&ex);
          rss+=ex;
        }
      //  printf("total:%d\n",rss);
        ptr=strtok(statusbuf," ");

      }

    }
      //printf("%s\n",buf);


      //printf("my %s\n",myid);

      fclose(fp);




            sprintf(path,"/proc/%s/stat",dnum);
//"/proc/pid/stat" 정보 저장
            if((fp=fopen(path,"r"))==NULL){
              fprintf(stderr,"fopen error\n");
              exit(1);
            }


            k=0;
            if(fp!=NULL){
              fgets(buf,sizeof(buf),fp);
              //printf("%s\n",buf);

              ptr=strtok(buf," ");



              while(ptr!=NULL){
                strcpy(statinfo[k],ptr);
                //printf("%d : %s\n",k,statinfo[k]);
                ptr=strtok(NULL," ");
                k++;


              }
            }
            fclose(fp);

            if(uop==1){//u 옵션
              //printf("dnum:%d",duidnum);
              //printf("myid:%s\n",myid);
              sprintf(duid,"%d",duidnum);
              if(!strcmp(duid,myid)){//uid가 같으면 출력
                terminal=atoi(statinfo[6]);//터미날 번호

                if(terminal==0) {//터미널 번호가 0 이면 ?
                  question=1;
                  strcpy(tty,"?");
                }
                else if(ttynum*256<=terminal&&terminal<ttynum*256+256){//해당 범위에 해당하면 tty
                  minor=terminal-ttynum*256;
                  sprintf(tty,"tty%d",minor);
                  //printf("tty%d ",minor);
                  question=0;
                }
                else if(ptsnum*256<=terminal&&terminal<ptsnum*256+256){//해당 범위에 해당하면 pts
                  minor=terminal-ptsnum*256;
                  sprintf(tty,"pts/%d",minor);
                  //rintf("pts/%d ",minor);
                  question=0;
                }
                else{//그 외 ?
                  question=1;
                  strcpy(tty,"?");
                }

                if(!strcmp(tty,"?")) continue;//터미널과 관련 없으면 출력 안함


                printf("%s %s ",username,statinfo[0]);//username pid

                getcpu(statinfo);//%cpu

                //mem 구하기
                mem=(double)rss/memtotal;
                mem*=100;

                printf("%.1f\t",mem);
                printf("%s\t",vsz);
                printf("%d\t",rss);
                printf("%s\t",tty);

                getstatfield(statinfo);//s 섹션 구하기

                getstarttime(statinfo);//starttime
                gettime(statinfo);//time
                getstat(dentry->d_name);//command

                printf("\n");
              }
          }

        else if(auop==1){//au옵션

          terminal=atoi(statinfo[6]);//terminal number

          for(int i=ttynum*256;i<ttynum*256+256;i++){//해당 범위에 해당하면 tty
            if(i==terminal){


              minor=terminal-ttynum*256;//minor number
              sprintf(tty,"tty%d",minor);



              tidfo=getpwuid(duidnum);//프로세스 username

              //strcpy(username,tidfo->pw_name);

              printf("%s %s ",tidfo->pw_name,statinfo[0]);//user pid

              getcpu(statinfo);//cpu


              mem=(double)rss/memtotal;
              mem*=100;

              printf("%.1f\t",mem);//%mem
              printf("%s\t",vsz);
              printf("%d\t",rss);
              printf("%s\t",tty);

              getstatfield(statinfo);//stat

              getstarttime(statinfo);//starttime
              gettime(statinfo);//time
              getstat(dentry->d_name);//command


              printf("\n");
            }
          }
            for(int i=ptsnum*256;i<ptsnum*256+256;i++){//해당 범위에 해당하면 pts
              if(i==terminal){


                minor=terminal-ptsnum*256;//minor number
                sprintf(tty,"pts/%d",minor);




                tidfo=getpwuid(duidnum);//프로세스 username

                //strcpy(username,tidfo->pw_name);

                printf("%s %s ",tidfo->pw_name,statinfo[0]);//user pid


                getcpu(statinfo);//cpu


                mem=(double)rss/memtotal;
                mem*=100;

                printf("%.1f\t",mem);//%mem
                printf("%s\t",vsz);
                printf("%d\t",rss);
                printf("%s\t",tty);

                getstatfield(statinfo);//stat

                getstarttime(statinfo);//starttime
                gettime(statinfo);//time
                getstat(dentry->d_name);//command


                printf("\n");


              }
          }
        }  else if(uxop==1){//ux옵션
            if(searchstatus(dentry->d_name)){//uid가 같으면 출력


              terminal=atoi(statinfo[6]);//terminal number

              if(terminal==0) {//터미널 번호가 0 이면 ?
                strcpy(tty,"?");
                question=1;
              }
              else if(ttynum*256<=terminal&&terminal<ttynum*256+256){//해당 범위에 해당하면 tty
                minor=terminal-ttynum*256;

                sprintf(tty,"tty%d",minor);//minor number
                question=1;
              }
              else if(ptsnum*256<=terminal&&terminal<ptsnum*256+256){//해당 범위에 해당하면 pts
                minor=terminal-ptsnum*256;

                sprintf(tty,"pts/%d",minor);//minor number
                question=1;
              }

              if(question==0) strcpy(tty,"?");//그 외 ?

              tidfo=getpwuid(duidnum);

              //strcpy(username,tidfo->pw_name);

              printf("%s %s ",tidfo->pw_name,statinfo[0]);//user pid

              getcpu(statinfo);//%dpu


              mem=(double)rss/memtotal;
              mem*=100;

              printf("%.1f\t",mem);
              printf("%s\t",vsz);
              printf("%d\t",rss);
              printf("%s\t",tty);

              getstatfield(statinfo);//stat

              getstarttime(statinfo);//starttime
              gettime(statinfo);//time
              getstat(dentry->d_name);//command


              printf("\n");
              }



            }else if(auxop==1){//aux 옵션
              terminal=atoi(statinfo[6]);//terminal number

              if(terminal==0) {//터미널 번호가 0 이면 ?
                strcpy(tty,"?");
                question=1;
              }
              else if(ttynum*256<=terminal&&terminal<ttynum*256+256){//해당 범위에 해당하면 tty
                minor=terminal-ttynum*256;//minor number

                sprintf(tty,"tty%d",minor);
                question=1;
              }
              else if(ptsnum*256<=terminal&&terminal<ptsnum*256+256){//해당 범위에 해당하면 pts
                minor=terminal-ptsnum*256;//minor number

                sprintf(tty,"pts/%d",minor);
                question=1;
              }

              if(question==0) strcpy(tty,"?");//그 외 ?

              tidfo=getpwuid(duidnum);

              //strcpy(username,tidfo->pw_name);

              printf("%s %s ",tidfo->pw_name,statinfo[0]);//user pid

              getcpu(statinfo);//%dpu


              mem=(double)rss/memtotal;
              mem*=100;

              printf("%.1f\t",mem);
              printf("%s\t",vsz);
              printf("%d\t",rss);
              printf("%s\t",tty);

              getstatfield(statinfo);//stat

              getstarttime(statinfo);//starttime
              gettime(statinfo);//time
              getstat(dentry->d_name);//command


              printf("\n");
            }

            question=0;
}
}
void getmyuid(){//현재 프로세스의 정보가져오기

  struct passwd*idfo;

  id=getuid();//userid 얻기
  idfo=getpwuid(id);//username얻기

  strcpy(username,idfo->pw_name);

}
void getcpu(char statinfo[][10]){//%cpu
  FILE *fp;
  char buf[20];

  int utime;
  int stime;
  int starttime;
  double cstime;
  double non;
  double time;
  double uptime;
  int sum;

  char field[10];
  int num;
  int btime;
  char *ptr;
  int cpu;
  //utime;


  if((fp=fopen("/proc/uptime","r"))==NULL){
    printf("fopen error\n");

  }

  fgets(buf,20,fp);
  //printf("\nbuf: %s\n",buf);
  sscanf(buf,"%s %lf",field,&non);
  fclose(fp);
//uptime
  uptime=atof(field);




    starttime=atoi(statinfo[21]);//starttime


    utime=atoi(statinfo[13]);//utime

    //stime;
    stime=atoi(statinfo[14]);//stime



    sum=utime+stime;//time
    //%cpu구하/
    uptime-=(int)(starttime/sysconf(_SC_CLK_TCK));

    if(uptime) {
      cpu=(int)(sum*1000/sysconf(_SC_CLK_TCK)/uptime);

    }
    else cpu=0;
    //rintf("cpu :%d ",cpu);
    printf("%d.%d\t",cpu/10,cpu%10);




}
void gettime(char statinfo[][10]){
  int second;
  int sec;
  int min;
  int hour;

//(stime+utime)/hz=time
  second=atoi(statinfo[13])+atoi(statinfo[14]);
  second/=sysconf(_SC_CLK_TCK);

  min=second/60;
  hour=min/60;
  sec=second%60;
  min=min%60;
  if (nop==1) printf("%02d:%02d:%02d\t",hour,min,sec);
  else printf("%d:%02d\t",min,sec);
}
void searchproc(){
  char path[50];
  DIR *dirp;
  struct dirent *dentry;
  int num;
  char dnum[50];
  FILE *fp;
  FILE *fp2;
  char *ptr;
  char *ptr2;
  char buf[500];
  char statinfo[51][10];
  int i=0;
  int terminal=0;
  int min,sec,hour=0;
  int terop=0;
  int minor=0;
  int question=0;

  if((dirp=opendir("/proc"))==NULL){
    printf("opendir error\n");
    exit(0);
  }
//"proc"파일 내 모든 파일 탐색
  while((dentry=readdir(dirp))){
    num=0;
    i=0;
    if(strcmp(dentry->d_name,".")&&!strcmp(dentry->d_name,".."))//"."나 ".." 제외
      continue;

    for(int i=0;i<strlen(dentry->d_name);i++){//이름이 숫자로 이뤄져 있으면 계속 진행
      if(isdigit(dentry->d_name[i])==0) num=1;
    }
    if(num==1) continue;//아니면 다음 파일 이름 탐색

  //  printf("%s\n",dentry->d_name);

    strcpy(dnum,dentry->d_name);

    sprintf(path,"/proc/%s/stat",dnum);

    //printf("%s\n",path);
//"proc/pid/stat" 안 정보 탐색
    if((fp=fopen(path,"r"))==NULL){
      fprintf(stderr,"fopen error\n");
      exit(1);
    }
//statinfo구조체에 띄어쓰기를 기준으로 정보 저장
    if(fp!=NULL){
      fgets(buf,sizeof(buf),fp);
      //printf("%s\n",buf);

      ptr=strtok(buf," ");

      while(ptr!=NULL){
        strcpy(statinfo[i],ptr);
        //printf("%d : %s\n",i,statinfo[i]);
        ptr=strtok(NULL," ");
        i++;
        }
      }
      if(nop==1){//"ps"명령어가 입력됐을 때,
          if(!strcmp(statinfo[5],filestinfo[5])) {//session id가 같으면 출력

            //username
            printf("%5s ",dentry->d_name);
            //tty
            for(int j=5;j<strlen(tty);j++)
              printf("%c",tty[j]);

            printf(" ");
            gettime(statinfo);//time 얻기
            //cmd
            for (int j = 1; j < strlen(statinfo[1])-1; j++) {
            printf("%c",statinfo[1][j] );
            }
            printf("\n" );

          }
    }
    if(aop==1){//a 옵션

        terminal=atoi(statinfo[6]);//터미널 번호 얻기
        for(int i=ttynum*256;i<ttynum*256+256;i++){//tty의 터미널 번호 범위에 들어가면
          if(i==terminal){

            printf("%5s ",dentry->d_name);
            minor=terminal-ttynum*256;//minor 번호
            printf("tty%d ",minor);//tty출력

            printf(" %s  ",statinfo[2]);//프로세스 상태 출력

            gettime(statinfo);//프로세스 상태 출/

            getstat(dentry->d_name);//command

            printf("\n");
          }
        }
          for(int i=ptsnum*256;i<ptsnum*256+256;i++){//pts의 터미널 번호 범위에 들어가면
            if(i==terminal){

              printf("%5s ",dentry->d_name);
              minor=terminal-ptsnum*256;//minor 번호
              printf("pts/%d ",minor);//pts 출력

              printf(" %s  ",statinfo[2]);//프로세스 상태 출력

              gettime(statinfo);//time
              getstat(dentry->d_name);//command



              printf("\n");
            }
        }

    }
    else if(xop==1){//x 옵션
      if(searchstatus(dentry->d_name)){//uid가 같으면

        printf("%5s ",dentry->d_name);//pid
        terminal=atoi(statinfo[6]);//terminal number

        if(terminal==0) {//terminal number가 0이면 ?
          printf("%s ","?");
          question=1;
        }
        else if(ttynum*256<=terminal&&terminal<ttynum*256+256){//terminal number가 범위에 해당하면 tty
          minor=terminal-ttynum*256;

          printf("tty%d ",minor);
          question=1;
        }
        else if(ptsnum*256<=terminal&&terminal<ptsnum*256+256){//terminal number가 범위에 해당하면 pts
          minor=terminal-ptsnum*256;

          printf("pts/%d ",minor);
          question=1;
        }

        if(question==0) printf("%s ","?");//그 외 ?


        printf(" %s  ",statinfo[2]);//stat

        gettime(statinfo);//time
        getstat(dentry->d_name);//command

        printf("\n");
        }



      }else if(axop==1){
        printf("%5s ",dentry->d_name);//pid
        terminal=atoi(statinfo[6]);


        if(terminal==0) {//terminal number가 0이면 ?
          printf("%s ","?");
          question=1;
        }
        else if(ttynum*256<=terminal&&terminal<ttynum*256+256){//terminal number가 범위에 해당하면 tty
          minor=terminal-ttynum*256;

          printf("tty%d ",minor);
          question=1;
        }
        else if(ptsnum*256<=terminal&&terminal<ptsnum*256+256){//terminal number가 범위에 해당하면 pts
          minor=terminal-ptsnum*256;

          printf("pts/%d ",minor);
          question=1;
        }

        if(question==0) printf("%s ","?");//그 외 ?


         getstatfield(statinfo);//stat

        gettime(statinfo);//time
        printf("  ");
        //getstat(dentry->d_name);//command
        for (int j = 1; j < strlen(statinfo[1])-1; j++) {
        printf("%c",statinfo[1][j] );
        }
        printf("\n");
        question=0;
      }




      fclose(fp);
    }

}
void getstat(char num[10]){//command
  char path[50];
  FILE *fp;
  char buf[100];
  int size;
  char *ptr;
//command 구하기
  sprintf(path,"/proc/%d/cmdline",atoi(num));

  if((fp=fopen(path,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }
  fseek(fp,0,SEEK_END);
  size=ftell(fp);
  //printf("size%ld ",ftell(fp));

  if(size==0){//파일에 내용이 없으

    fseek(fp,0,SEEK_SET);
    fgets(buf,sizeof(buf),fp);
    fclose(fp);
    //printf("buf:%s %ld %ld\n",buf,sizeof(buf),strlen(buf));
    printf("%s",buf);
  }else{//"proc/pid/stat"의 두번째 필드에서 구하기
    fclose(fp);
    sprintf(path,"/proc/%d/stat",atoi(num));

    if((fp=fopen(path,"r"))==NULL){
      fprintf(stderr,"fopen error\n");
      exit(1);
    }



          fgets(buf,sizeof(buf),fp);
          //printf("%s\n",buf);

          ptr=strtok(buf," ");

          ptr=strtok(NULL," ");

          printf("%s\n",ptr);
          fclose(fp);

            }




}
void getmyinfo(){//프로세스의 정보 얻기
  FILE *fp;
  int me=getpid();//내 pid
  char myroot[100];
  int myname;
  int mysessionid;
  char *ptr;
  int i=0;
//"proc/mypid/stat" 안 정보 저장
  sprintf(myroot,"/proc/%d/stat",me);
  //printf("myroot:%s\n",myroot);

  if((fp=fopen(myroot,"r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }
  if(fp!=NULL){
    fgets(mybuf,sizeof(mybuf),fp);
    //printf("%s\n",buf);

    ptr=strtok(mybuf," ");
//띄어쓰기를 기준으로 fileinfo에 정보 저장
    while(ptr!=NULL){
      strcpy(filestinfo[i],ptr);
      //printf("%i : %s\n",i,filestinfo[i]);
      ptr=strtok(NULL," ");
      i++;

    }

  }
  fclose(fp);
//프로세스 tty얻기
  strcpy(tty,ttyname(0));
  //printf("tty %s\n",tty);



}
