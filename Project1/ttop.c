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

int mfree;
int mtotal;
int buffer;
int cached;
int sfree;
int stotal;

void getcpu(char statinfo[][10]);
void getmeminfo();
void getproc();

int main(int argc,char* argv[]){
  while(1){
    getmeminfo();//meminfo 정보 가져오기
    printf("KIB Mem : %d total, %d free, %d used,%d buff/cache\n",
          mtotal,mfree,mtotal-mfree-buffer-cached,buffer+cached);
    printf("KIB Swap : %d total, %d free,  used, buff/cache\n",
          stotal,sfree);

    printf(" PID\t USER\tPR\tNI\tVIRT\tRES\tSHR\tS\t%%CPU\t%%MEM\tTIME+COMMAND\n");
    getproc();//proc 정보 가져오기
    sleep(3);//3초 후 실행
  }
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

  //printf("stime %d\n",stime);
  //starttime;
  starttime=atoi(statinfo[22]);
  //printf("%f\n",utime);
  //stime
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
    stime=atoi(statinfo[14]);



    sum=utime+stime;
//%cpu구하/
    uptime-=(int)(starttime/sysconf(_SC_CLK_TCK));

    if(uptime) {
      cpu=(int)(sum*1000/sysconf(_SC_CLK_TCK)/uptime);

    }
    else cpu=0;
    //rintf("cpu :%d ",cpu);
    printf("%d.%d\t",cpu/10,cpu%10);




}
void getproc(){//proc 정보 가져오기
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
  int minor=0;
  char field[10];
  struct passwd*tidfo;
  int duidnum;
  char em[10];
  char duid[10];
  char vsz[10];
  int rss=0;
  int ex=0;
  int memtotal;
  double mem;
  int question=0;
  int second;
  int shr;


  if((dirp=opendir("/proc"))==NULL){
    printf("opendir error\n");
    exit(0);
  }

  while((dentry=readdir(dirp))){
    num=0;
    i=0;
    if(strcmp(dentry->d_name,".")&&!strcmp(dentry->d_name,".."))
      continue;

    for(int i=0;i<strlen(dentry->d_name);i++){
      if(isdigit(dentry->d_name[i])==0) num=1;
    }
    if(num==1) continue;

  //  printf("%s\n",dentry->d_name);
    shr=0;
    strcpy(dnum,dentry->d_name);

    sprintf(path,"/proc/%s/stat",dnum);

    //printf("%s\n",path);

    if((fp=fopen(path,"r"))==NULL){
      fprintf(stderr,"fopen error\n");
      exit(1);
    }
    rss=0;
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

      fclose(fp);

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


            if(strstr(buf,"Uid")){
              sscanf(buf,"%s %d %s %*d",field,&num,em);
              duidnum=num;
              strcpy(duid,em);
            }
            if(strstr(buf,"VmSize")) {
              //printf("%s\n",statusbuf);
              sscanf(buf,"%*s %s %*s",vsz);
            }
            if(strstr(buf,"RssAnon")||strstr(buf,"RssFile")||strstr(buf,"RssShmem")){
              //printf("%s\n",statusbuf);
              sscanf(buf,"%*s %d %*s",&ex);
              rss+=ex;
            }if(strstr(buf,"RssFile")||strstr(buf,"RssShmem")) {
              //printf("%s\n",statusbuf);
              sscanf(buf,"%*s %d%*s",&ex);
              shr+=ex;
            }
            //printf(": %s\n",buf);
            ptr=strtok(NULL,"\n");

            }
          }
        }
        fclose(fp);

        printf("%s\t",statinfo[0]);
        duidnum=atoi(duid);
        //printf("duid:%d ",duidnum);
        tidfo=getpwuid(duidnum);

        //strcpy(username,tidfo->pw_name);

        printf("%-s\t",tidfo->pw_name);//user

        printf("%-4s\t",statinfo[17]);//priority
        printf("%-4s\t",statinfo[18]);//nice
        //printf("%10s\t",statinfo[22]);//tVIRT
        printf("%-s\t",statinfo[22]);
        printf("%-d\t",rss);//res
        printf("%-d\t",shr);
        printf("%s\t",statinfo[2]);//s
        getcpu(statinfo);//%cpu

        mem=(double)rss/mtotal;
        mem*=100;

        printf("%-.1f\t",mem);//%mem

        second=atoi(statinfo[13])+atoi(statinfo[14]);
        second/=sysconf(_SC_CLK_TCK);
        //printf("second:%d\n",second);

        min=second/60;
        hour=min/60;
        sec=second%60;
        min=min%60;
        printf("%02d:%02d:%02d ",hour,min,sec);//time


        for (int j = 1; j < strlen(statinfo[1])-1; j++) {//command
        printf("%c",statinfo[1][j] );
        }

        printf("\n");



}
}
void getmeminfo(){//meminfo 저장
  FILE *fp;
  char buf[500];
  char *ptr;

  if((fp=fopen("/proc/meminfo","r"))==NULL){
    fprintf(stderr,"fopen error\n");
    exit(1);
  }

  if(fp!=NULL){

      while(!feof(fp)){

        fgets(buf,sizeof(buf),fp);

        if(strstr(buf,"MemTotal") )   {
        //printf("%s\n",statusbuf);
          sscanf(buf,"%*s %d",&mtotal);
        }
        else if(strstr(buf,"MemFree") )   {
        //printf("%s\n",statusbuf);
          sscanf(buf,"%*s %d",&mfree);
        }
        else if(strstr(buf,"Buffer") )   {

          sscanf(buf,"%*s %d",&buffer);
        }
        else if(strstr(buf,"SwapCached") )   {
        }
        else if(strstr(buf,"Cached") )   {

          sscanf(buf,"%*s %d",&cached);
        }
        else if(strstr(buf,"SwapTotal") )   {
        //printf("%s\n",statusbuf);
          sscanf(buf,"%*s %d",&stotal);
        }
        else if(strstr(buf,"SwapFree") )   {
        //printf("%s\n",statusbuf);
          sscanf(buf,"%*s %d",&sfree);
        }



    }

  }
}
