#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>

int ssu_daemon_init(void);//데몬 프로그램-모니터링 함수
int Scandir(char *directory,  int depth);//해당 경로에 들어 있는 파일 스캔 함수
void create(int new,int past);//creat, 생성을 log.txt에 기록할 정보 찾는 함수
void delete_dae(int new,int past);//delete, 삭제를 log.txt에 기록할 정보 찾는 함수
void modify(int new);//modify,변경을 log.txt에 기록할 정보 찾는 함수
void write_log(char *name);//log.txt에 write 하는 함수


void delete(char *ptr);//delete명령어 수행 함수
void *delete_thread(void *arg);//delete할 예약시간을 받았을 때, thread 만드는 함수
void recover(char *ptr);//recover 함수실행

void size(char *ptr);//해당 파일 및 디렉토리 사이즈 알려주는 함수
long int size_Scandir(char *filepath, int depth);//디렉토리 크기 탐색하는 함수
long int size_Scandir_d(char *filepath, int depth,int num);//size의 d옵션 수행하는 함수
void start(void);//명령 프롬프트 함수



extern void tree(void);//tree 명령어
extern void help(void);//매뉴얼 실행
extern void size_check(char ipath[],char fpath[]);//info 디렉토리 안 파일 크기 확인


char logpath[10000]={0,};//log.txt가 있는 절대 경로:logpath
char daemonpath[10000]={0,};//모니터링 함수가 지켜볼 지정 디렉토리 절대 경로:daemonpath
char *newname[1000000];//현재 파일 이름들
char *oldname[1000000];//과거 파일 이름들
char pastm[100][100000];//과거 파일들의 mtime
char prem[100][100000];//현재 파일들의 mtime
int number=0;//파일 개수
char action[8]={0,};//행위

char endtime_date[100]={0,};
char endtime_time[50]={0,};
int dif;
static int i2op=0;
int threadop=0;

char filename[50]={0,};
char trashname[50]={0,};
char path[100]={0,};
char staticpath[100]={0,};
char fpath[100]={0,};
char ipath[100]={0,};
char tpath[100]={0,};
char filepath[100]={0,};
char movepath[100]={0,};
char cpath[100]={0,};
char ispath[100]={0,};


char threadpath[100]={0,};
char threadname[100]={0,};

static int  indent = 0;



char compath[1000]={0,};
struct st *mcheck;

int main(void)
{
		pid_t pid;
		//pid=getpid();
		if((pid=fork())<0) //자식프로세스 생성
			{
				fprintf(stderr,"fork error\n");
				exit(1);//실패하면 나가기
		}
		else if(pid == 0)//자식 프로세스는
		  	ssu_daemon_init();//데몬 프로그램-모니터링 함수 실행
		else//부모 프로세스는
				start();//명령 프롬프트 함수 실행

		exit(0);
}
void start(void)//프롬프트 명령어를 수행하는 함수
{
		char input[50]={0,};//총 입력되어진 데이터
		char command[10]={0,};//입력 데이터 중 명령어 부분
		char *ptr;//filename,option 등을 확인하는 포인터


		realpath(".",path);//현재 디렉토리의 절대경로를 path에 저장

		while(1){

				printf("20170776>");//prompt
				scanf("%[^\n]s", input);//입력 데이터 input에 저장
				 getchar();

				ptr = strtok(input, " ");//command

				memcpy(command,ptr,sizeof(ptr));

				chdir(path);

				if(!strcmp(command,"delete")) {
						if(threadop==1) printf("no..deleting..\n");//delete시간예약이 실행되어진 경우 끝날 때까지 실행 불가
						else delete(ptr);//DELETE
				 }
				else if(!strcmp(command,"recover"))  recover(ptr);//RECOVER
				else if(!strcmp(command,"tree"))  tree();//TREE
				else if(!strcmp(command,"exit")) break;//EXIT
				else if(!strcmp(command,"help")) help();//HELP
				else if(!strcmp(command,"size")) size(ptr);//SIZE
				else help();//HELP

			//경로 및 입력 데이터 초기화
			memset(filename,0,50);
			memset(filepath,0,100);
			memset(cpath,0,100);
			memset(command,0,10);
			memset(input,0,50);
			memset(ispath,0,50);
  }

  return;
}

int ssu_daemon_init(void)
{
		pid_t pid;
		int fd,maxfd;
		int past=0;
		int new=0;

		//struct dirent **namelist;



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

		realpath("log.txt",logpath);//log.txt가 있는 절대 경로:logpath
		realpath("check",daemonpath);//모니터링 함수가 지켜볼 지정 디렉토리 절대 경로:daemonpath

		Scandir(daemonpath,0);//지정 디렉토리에 들어 있는 파일 스캔하기
		past = number;//파일 개수 초기값 설정


		for(int i=0;i<past;i++){//들어있는 파일들 모두
				oldname[i]=newname[i];//초기 이름들안에 넣기
				strcpy(pastm[i],prem[i]);//초기 시간들 안에 넣기
		}

		int i=0;




		memset(newname,0,sizeof(newname));//현재 이름들 초기화
		memset(prem,0,sizeof(prem));//현재 시간들 초기화


		number=0;//현재 파일 개수 초기화


		while(1){//무한 반복
			sleep(1);
			chdir(daemonpath);//지정 디렉토리로 작업 디렉토리 변경
			Scandir(daemonpath,0);//지정 디렉토리에 들어 있는 파일 스캔하기
			new=number;//현재 파일 개수
			if(new>past){//파일 개수가 증가하면
					 strcpy(action,"create_");//행위는 creat, 생성
					 create(new,past); //creat, 생성을 log.txt에 기록하기
			}
			else if(new<past){//파일 개수가 감소하면
					strcpy(action,"delete_");//행위는 delete, 삭제
					delete_dae(new,past);//delete, 삭제를 log.txt에 기록하기
			}else{//파일 개수의 변화가 없으면
					strcpy(action,"modify_");//행위는 modify, 변경
					modify(new);//modify, 변경를 log.txt에 기록하기
			}

			memset(action,0,sizeof(action));//행위 초기화
			number=0;//파일 개수 초기화
			memset(oldname,0,sizeof(oldname));//과거 파일 이름들 초기화
			memset(pastm,0,sizeof(pastm));//과거 파일 mtime 초기화

			for(int i=0;i<new;i++){//현재 파일 개수만큼
					oldname[i]=newname[i];//과거 파일 이름들을 현재 파일 이름들로 정보 갱신
					strcpy(pastm[i],prem[i]);;//과거 파일 mtime들을 현재 파일 mtime들로 정보 갱신
			}

			past=new;//과거 파일 개수를 현재 파일 개수로 초기화

			memset(newname,0,sizeof(newname));//현재 파일 이름들 초기화
			memset(prem,0,sizeof(prem));//현재 파일들 mtime들 초기화

		}


		return 0;//프로그램 종료
}
void modify(int new)//modify,변경을 log.txt에 기록할 정보 찾는 함수
{
		int q=0;

		for(int i=0;i<new;i++){//파일 개수만큼 비교
				if(strcmp(pastm[i],prem[i])){//파일의 mtime이 과거와 다르면
						write_log(newname[i]);//log.txt.에 modify행위 기록
						break;
						 //printf("%s\n",pastm[i]);
						 //printf("%s\n",prem[i]);
				}
		}

  }
void create(int new,int past)//creat, 생성을 log.txt에 기록할 정보 찾는 함수
{
		int q=0;

		for(int i=0;i<new;i++){//현재 파일 이름 검사
		 q=0;
				for(int j=0;j<past;j++){//과거 파일 이름 검사
						//printf("%s\n",newname[i]);
						//printf("%s\n",oldname[j]);
						if(!strcmp(newname[i],oldname[j])) {//현재 파일 이름이 과거 파일에 있으면
						q--;//q감소
						}
				}
				if(q==0) write_log(newname[i]);//현재 파일 이름을 과거 파일 이름에서 찾지 못하면 log.txt에 create 행위기록

			}

  }
void delete_dae(int new,int past)//delete, 삭제를 log.txt에 기록할 정보 찾는 함수
{
		int q=0;

		for(int i=0;i<past;i++){//과거 파일 이름 검사
				q=0;
				for(int j=0;j<new;j++){//현재 파일 이름검사

						if(!strcmp(oldname[i],newname[j])) {//과거 파일 이름이 현재 파일 이름에 있으면
									q--;//q감소
							}
				 }
				 if(q==0) write_log(oldname[i]);//과거 파일 이름을 현재 파일 이름에 없으면 log.txt에 delete행위기록

		}

    }
void write_log(char *name)//log.txt에 write 하는 함수
{
		FILE *fp;

		char buf1[10000]={0,};
		char buf2[10000]={0,};
		char * pt;
		time_t t = time(NULL);//현재시간
		struct tm tm = *localtime(&t);
		int ch='/';

		pt = strrchr( name, ch );//파일 이름

		memset(buf1,0,sizeof(buf1));//buf1 초기화
		memset(buf2,0,sizeof(buf2));//buf2 초기화

		sprintf(buf1,"[%04d-%02d-%02d %02d:%02d:%02d]",tm.tm_year+1900, tm.tm_mon+1,tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec);//현재 시간
		sprintf(buf2,"[%s%s]\n",action,pt+1);//행위
		strcat(buf1,buf2);//buf에 정보 넣기


		fp=fopen(logpath,"a");//log.txt.가 있나 확인
		fclose(fp);//닫기

		fp=fopen(logpath,"r+");//파일 열어
		fseek(fp,0,SEEK_END);//오프셋을 맨 뒤로 옮기고
		fputs(buf1,fp);//buf(시간, 행위, 파일 이름)을 log.txt에 쓰기
		fclose(fp);//파일 닫기


}

int Scandir(char *directory,  int depth)//해당 경로에 들어 있는 파일 스캔 함수
{
    struct dirent **namelists;//파일 이름들 저장할 공간
    int count;//파일 이름 개수

    if (chdir(directory) < 0){
			fprintf(stderr,"chdir error\n");
			exit(1);
    }

    count = scandir(".", &namelists, NULL, alphasort);//파일 이름들을 items에 오름차순으로 저장하고 그 개수를 return


    for (int i = 0; i < count; i++){

        struct stat fstat;//파일 정보 기록할 stat 구조체

        if ( (!strcmp(namelists[i]->d_name, ".")) || (!strcmp(namelists[i]->d_name, "..")) )//파일 이름이 현재 디렉토리거나 이전 디렉토리면 pass
            continue;//건너 뛰기

        newname[number] = realpath(namelists[i]->d_name,NULL);//현재 이름에 파일 절대 경로 저장

        lstat(namelists[i]->d_name, &fstat);//파일 정보 fstat에 저장/

				strcpy(prem[number], ctime(&fstat.st_mtime));//현재 파일 mtime 저장


        number++;//개수 증가
        if ((fstat.st_mode & S_IFDIR) == S_IFDIR){//파일이 디렉토리면
				if (indent < (depth-1) || (depth == 0)){//깊이 만큼 검사하고 깊이가 0이면
						indent ++;//깊이 증가
						Scandir(namelists[i]->d_name, depth);//재귀함수 호출
				 }

				}
		 }
		// items메모리 해제
		for (int i = 0; i < count; i++)
				free(namelists[i]);
		free(namelists);

		indent --;//다 확인 후깊이 감소
		chdir("..");//이전 디렉토리로 돌아가기
}
void size(char *ptr){//해당 파일 사이즈 출력하는 함수
		long int size=0;//size
		int dop=0;//dop여부
		char *p;//파일 상대경로 알아내는 포인터
		int num;
		char cpath[100];
		realpath("check",cpath);
		chdir(cpath);
		//FILENAME
		ptr = strtok(NULL, " ");
		memcpy(filename,ptr,strlen(ptr));
		//OPTION
		ptr = strtok(NULL, " ");
		if(ptr !=0){//doption 확인
				if(!strcmp(ptr,"-d")) {
					dop=1;//dopion 확인
					ptr = strtok(NULL, " ");
					num=atoi(ptr);
				}
				else {//옵션을 잘못입력했으면
				printf("옵션을 다시 입력해 주세요\n");//에러 메시지
				return;//프롬프트로 돌아가기
      }
		 }

		realpath(filename,filepath);//입력받은 파일의 절대경로

		if(dop==1){//doption이 있으면
				size=size_Scandir_d(filepath, 0,num);// SIZE doption 실행 함수 실행

		}else{//doption이 없으면
				size=size_Scandir(filepath, 0);//디렉토리 내부 파일들의 합
				p=strstr(filepath,cpath);//filepath 문자열을 가리키는 포인터 얻기
				memset(filename,0,sizeof(filename));//filename 초기화
				sprintf(filename,".%s",p+strlen(cpath));//./filename 으로 상대경로로 바꾸기

				printf("%ld\t\t%s\n",size,filename);//size와 filename의 상대 경로 출력
		}
		return;//프롬프트로 돌아가기

}
long int size_Scandir(char *filepath, int depth){
		struct  dirent **namelists;//파일 이름을 저장하는 list
		int count;//파일 개수
		struct stat checking;//파일 정보 가져오는 stat구조체
		long int size=0;//크기

		if (chdir(filepath) < 0)//현재 작업 디렉토리를 탐색할 경로로 변경
		{
				printf("%s error\n", filepath);//실패하면 chdir 에러 메시지 출력
				return 0;//함수 나가기
		}
		count = scandir(".", &namelists, NULL, alphasort);//현재 작업 디렉토리의 파일 개수 구하기


		for (int i = 0; i < count; i++)//현재 작업 디렉토리 안 모든 파일 검사
		{
				if ( !strcmp(namelists[i]->d_name, ".")) //파일이름이 현재 디렉토리이면
							     continue;//넘어가기
				if(!strcmp(namelists[i]->d_name, "..") )//파일이름이 이전 디렉토리이면
							     continue;//넘어가기

				lstat(namelists[i]->d_name, &checking);//파일 정보 가져오기

				if ((checking.st_mode & S_IFDIR) == S_IFDIR)//파일이 디렉토리이면
				{
					if (indent < (depth-1) )//깊이만큼 안의 디렉토리 검사,(0일 경우는 상관없이 검사)
						{
								indent ++;//깊이 +1
								size+=size_Scandir(namelists[i]->d_name, depth);//해당 디렉토리 안의 파일들의 크기의 합 더하기
						}
					if (depth == 0)//깊이만큼 안의 디렉토리 검사,(0일 경우는 상관없이 검사)
						{
								indent ++;//깊이 +1
								size+=size_Scandir(namelists[i]->d_name, depth);//해당 디렉토리 안의 파일들의 크기의 합 더하기
						}
				} else //파일이면
					size+=checking.st_size;//파일 사이즈, 크기에 더하기



}
		  for(int i=0;i<number;i++)//namelists  메모리 해제
                  free(namelists[i]);
		  free(namelists);//namelists  메모리 해제

	    indent --;//깊이 -1
    	chdir("..");//이전 디렉토리로 돌아가기

		  return size;//크기 return

}
long int size_Scandir_d(char *filepath, int depth, int num){
		  struct  dirent **namelist;
		  struct stat check;
		  int count;
		  int i;
		  long int size=0;
		  char ppath[100]={0,};

		  if (chdir(filepath) < 0)
		  {

			  perror("chdir ");
			  exit(1);
		  }


		  count = scandir(".", &namelist, NULL, alphasort);//파일 개수 새기

		  //
		  for (i = 0; i < count; i++)//파일 다 검사
		  {


			  //
			  if ( (!strcmp(namelist[i]->d_name, ".")) || (!strcmp(namelist[i]->d_name, "..")) )//현재 나 이전 디렉토리는
			  {
				  continue;//pass
			  }

			  //파일 정보얻기
			  lstat(namelist[i]->d_name, &check);



			  if ((check.st_mode & S_IFDIR) == S_IFDIR)//디렉토리면
			  {

				  if (indent < (depth-1) || (depth == 0))
				  {
						 indent ++;
						 if(indent==num) return size;//입력된 깊이에 도달하면 return
						 size+=size_Scandir_d(namelist[i]->d_name, depth,num);


				  }
			  }else size+=check.st_size;
			  printf("%ld\t\t%s\n",size,namelist[i]->d_name);
			  size=0;
			}
			return size;
			indent --;
			chdir("..");
}
void recover(char *ptr){
		 char op[3]={0,};
		 int endop=0;
		 char repath[100]={0,};
		 FILE *fp;
		 char buf1[100]={0,};
		 char buff[100]={0,};
		 char dtime[100]={0,};
		 char mtime[100]={0,};
		 char cpath[100]={0,};
		 char infopath[100]={0,};
		 char checkpath[100]={0,};
		 char ifname[100]={0,};
		 char repath2[100]={0,};
		 int i=0;
		 char *p;
		 DIR* dp=NULL;
		 struct dirent* entry=NULL;
		 struct stat buf;
		 int a=0;
		 char findfile[100]={0,};
		 char *arr[100];


		 //filename
		 ptr = strtok(NULL, " ");
		 memcpy(filename,ptr,strlen(ptr));
		 //lop;
		 ptr = strtok(NULL, " ");
		 if(ptr !=0){
			 memcpy(op,ptr,strlen(ptr));
			 endop=1;
		 }

		 //info 디렉토리로 이동
		 realpath("trash/info",ipath);
		 realpath("trash/files",fpath);
		 chdir(ipath);

		 //파일을 열어 원래 있던 곳의 절대경로를 얻는다.
		 if((dp=opendir(ipath))==NULL){
		   printf("fail\n");
		   return;
		 }
		 //절대 경로에서 맨 뒤에 "/원래 이름"을 얻어 입력받은 filename과 같은 지 확인한다.
		 while((entry=readdir(dp))!=NULL){

			   if((strstr(entry->d_name,filename))!=NULL){//파일이름이 filename과 비슷해 이 파일인지 의심된다면
					 memset(dtime,0,sizeof(dtime));
					 memset(repath,0,sizeof(repath));
					 memset(mtime,0,sizeof(dtime));
					 memset(buff,0,sizeof(buff));

					 fp=fopen(entry->d_name,"r+");
					  fgets(buff,100,fp);
					  fgets(repath,100,fp);
					  fgets(dtime,100,fp);

					  dtime[strlen(dtime) - 1] = '\0';

					  fgets(mtime,100,fp);

					  memset(buff,0,sizeof(buff));

					  repath[strlen(repath) - 1] = '\0';

					  p=strrchr(repath,'/');//"/원래 이름"을 얻기
					  sprintf(buff,"/%s",filename);
						fclose(fp);
					//입력받은 파일의 원래 이름이 입력 받은 filename과 같다면 a++
					  if(!strcmp(p,buff))
							a++;


			   }
		 }
		 closedir(dp);

		 if(a==0){// 동일한 이름이 없다면
				   printf("There is no '%s' in the 'trash' directory\n",filename);//없음 메시지 출력
				   return ;//프롬프트 돌아가기
		 }else if(a==1){//1개 있다면

				  dp=opendir(ipath);

				 while((entry=readdir(dp))!=NULL){

						   if((strstr(entry->d_name,filename))!=NULL){//파일이름이 비슷해 의심가는 파일이 있다면
									 memset(dtime,0,sizeof(dtime));
									 memset(repath,0,sizeof(repath));
									 memset(mtime,0,sizeof(dtime));
									 memset(buff,0,sizeof(buff));

									 //printf("%s\n",entry->d_name);
									 fp=fopen(entry->d_name,"r+");
									  fgets(buff,100,fp);
									  fgets(repath,100,fp);
									  fclose(fp);

									  //원래 파일의 절대 경로 값을 가져와 뒤의 파일 이름을 비교한다.
									  memset(buff,0,sizeof(buff));

									  repath[strlen(repath) - 1] = '\0';

									  p=strrchr(repath,'/');
									  sprintf(buff,"/%s",filename);

									  if(!strcmp(p,buff)){//똑같은 이름을 찾았다면
												strcpy(ifname,entry->d_name);//해당 파일이름 저장하고
												break;//디렉토리 탐색 종료

										}

								 }
						 }
				 closedir(dp);

				 //files 디렉토리 속 파일이름 절대 경로 저장
				 chdir(fpath);
				 realpath(ifname,checkpath);
				 chdir(ipath);
				 realpath(ifname,infopath);
				 if(access(repath,F_OK)==0){//있다면

				 i=0;
				 while(1){//숫자_파일이름의 형태로 숫자를 1씩 늘려가기
					 i++;
					 sprintf(buf1,"/%d",i);
					 strcat(buf1,"_");
					 strcat(buf1,filename);
					 p=strrchr(repath,'/');
					 strcpy(p,buf1);

					 if(access(repath,F_OK)!=0) break;//숫자_파일이름이 원래 절대경로에 없으면 해당 숫자_파일이름을 복구할 이름으로 지정
					 //memcpy(filename,buf1,sizeof(filename));
					 memset(buf1,0,sizeof(buf1));

				 }


				 }



		 }else{//여러개가 의심된다면

				   i=0;
				   dp=opendir(ipath);

				   while((entry=readdir(dp))!=NULL){

						 if((strstr(entry->d_name,filename))!=NULL){//파일이름과 입력값이 비슷해 의심된다면
								   memset(dtime,0,sizeof(dtime));
								   memset(repath,0,sizeof(repath));
								   memset(mtime,0,sizeof(dtime));
								   memset(buff,0,sizeof(buff));

								   //해당 파일 열고 절대 경로 데이터 얻기
								   fp=fopen(entry->d_name,"r+");
									fgets(buff,100,fp);
									fgets(repath,100,fp);
									fgets(dtime,100,fp);

									dtime[strlen(dtime) - 1] = '\0';

									fgets(mtime,100,fp);

									memset(buff,0,sizeof(buff));

									repath[strlen(repath) - 1] = '\0';
									//절대 경로 뒤에 /파일이름을 얻어 비교해 보기
									p=strrchr(repath,'/');
									sprintf(buff,"/%s",filename);
									fclose(fp);
									if(!strcmp(p,buff)){//파일 이름이 같다면
											  i++;
											  printf("%d. %s : %s %s",i,filename, dtime,mtime);//순서와 이름 삭제 시간, mtime 출력
									}

						 }
				   }

				   printf("choose:");
				   scanf("%d",&a);
				   getchar();
				   //여러개의 같은 이름 파일 중 하나 입력 받기

				   closedir(dp);
				   dp=opendir(ipath);
				   i=0;
				   while((entry=readdir(dp))!=NULL){
							 memset(buff,0,sizeof(buff));
							 if((strstr(entry->d_name,filename))!=NULL){//입력 받은 파일이름과 같다고 의심되는 파일들
										//파일을 열고 절대경로 가져오기
									   fp=fopen(entry->d_name,"r+");
										fgets(buff,100,fp);
										fgets(repath,100,fp);
										fgets(buff,100,fp);
										fgets(buff,100,fp);

										fclose(fp);
										memset(buff,0,sizeof(buff));
										repath[strlen(repath) - 1] = '\0';
										p=strrchr(repath,'/');


										sprintf(buff,"/%s",filename);

										if(!strcmp(p,buff)){//입력 받은 파일 이름과 같으면
												  i++;//i++
												  if(a==i){//사용자가 입력한 숫자와 i가 같아지면


															strcpy(buff,entry->d_name);//recover할 파일 확정

															break;//디렉토리 탐색 중단
										  }
										}
								memset(buff,0,sizeof(buff));
								memset(repath,0,sizeof(repath));

							 }
				   }
				   closedir(dp);
				   //files 폴더 속 해당 이름 의 절대 경로 얻기

				   chdir(fpath);
				   realpath(buff,checkpath);

				   //info 폴더 속 해당 이름의 절대 경로 얻기
				   chdir(ipath);
				   realpath(buff,infopath);

				   //해당 파일의 원래 절대경로의 같은 이름의 파일이 있는지 확인하기
				  if(access(repath,F_OK)==0){//있다면

					i=0;
					while(1){//숫자_파일이름의 형태로 숫자를 1씩 늘려가기
					  i++;
					  sprintf(buf1,"/%d",i);
					  strcat(buf1,"_");
					  strcat(buf1,filename);
					  p=strstr(repath,filename);
					  strcpy(p,buf1);
					  if(access(repath,F_OK)!=0) break;//숫자_파일이름이 원래 절대경로에 없으면 해당 숫자_파일이름을 복구할 이름으로 지정
					  //memcpy(filename,buf1,sizeof(filename));
					  memset(buf1,0,sizeof(buf1));

					}

				  }
					p=strrchr(checkpath,'/');
					memset(buf1,0,sizeof(buf1));
					sprintf(buf1,"%c",'/');
					strcat(buf1,filename);
					strcpy(p,buf1);


					realpath(filename,infopath);

			 }




			 rename(checkpath,repath);//files 폴더에서 지정 경로로 이동
			 remove(infopath);//해당 파일의 info 정보 지우기



			 return ;//프롬프트로 돌아가기

		}


void delete(char *ptr)//delete명령어 수행 함수
{		//op여부
		int iop=0;
		int rop=0;
		int endop=0;

		char chop[100]={0,};
		int ch=0;

		int i=0;
		char *p;
		//path,이름, mtime얻기 위한 변수
		struct dirent *dentry;
		struct stat statbuf;
		struct stat modify;
		char file[100];
		char buff[100]={0,};
		char checkpath[100]={0,};
		char newname[100]={0,};

		FILE *fp;
		DIR *dirp;
		pthread_t tid;

		//시간만들기
		time_t start,end;
		struct tm* timeinfo;
		time_t t;
		struct tm *cur;
		int year;
		int month;
		int day;
		int hour;
		int min;
		int sec;
		struct tm *mtime;

		//char c;


		realpath("trash",tpath);//trash 폴더 절대경로
		realpath("check",cpath);//해당 디렉토리 절대 경로

		//FILENAME
		ptr = strtok(NULL, " ");
        memcpy(filename,ptr,strlen(ptr));

		//ENDTIME
        ptr = strtok(NULL, " ");//ENDTIME이 입력되어졌다면
		if(ptr !=0){
				memcpy(endtime_date,ptr,strlen(ptr));//ENDTIME 날짜 추출
				endop=1;//endop 실행

				ptr = strtok(NULL, " ");//ENDTIME 시간 추출
				if(ptr !=0){

				memcpy(endtime_time,ptr,strlen(ptr));

                //OPTION CHECK
                ptr = strtok(NULL, " ");
                if(ptr !=0){
						memcpy(chop,ptr,strlen(ptr));

						if((strcmp(ptr,"-r"))==0) rop=1;//rop실행
						else if((strcmp(ptr,"-i"))==0) i2op=1;//ENDTIME과 i옵션을 함께 입력한 경우
						else {//잘못된 옵션입력
								printf("wrong option\n");
								return;
                      }
                  }
            }

        }

		if((strcmp(endtime_date,"-i")==0)) iop=1;//i option이 endtime 없이 입력되었을 경우

		chdir(cpath);//작업 수행 디렉토리를 해당 디렉토리로 변경


		//파일의 절대경로가 아닌 이름만 추출
		realpath(filename,filepath);
		p = strrchr(filepath, '/');//뒤에서 부터 "/" 검사

		strcpy(trashname,p+1);//trash폴더에 들어갈 이름

		if(access(filepath, F_OK)<0){//해당 파일이 해당 디렉토리에 없으면
			    printf("there is no '%s'\n",filepath);//없음 메시지 출력
				return;//프롬프트로 돌아가기
        }

      if(iop==1){//i옵션이 시간예약 없이 입력되었다면
				remove(filepath);//trash에 옮기지 말고 그냥 지우기
				return;//프롬프트로 돌아가기
        }



      chdir(tpath);//trash폴더로 이동


      if((ch=access(tpath, F_OK))<0){//trash폴더확인, 없으면 생성
				mkdir(tpath,S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH);
          }
      chdir(tpath);

      realpath("info",ipath);

      if((ch=access(ipath, F_OK))<0){//trash폴더 안 info 디렉토리 확인, 없으면 생성
				mkdir(ipath,S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH);
          }


      realpath("files",fpath);

      if((ch=access(fpath, F_OK))<0){//trash폴더 안 files 디렉토리 확인, 없으면 생성
				mkdir(fpath,S_IFDIR | S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH);
          }




	  //calculate time
	  if(endop==1){//ENDTIME이 입력되어졌으면

				//"-"기준으로 날짜 숫자로 얻기
				ptr = strtok( endtime_date, "-");
				year=atoi(ptr);
				ptr = strtok( NULL, "-");
				month=atoi(ptr);
				ptr = strtok( NULL, "-");
				day=atoi(ptr);

				//"-"기준으로 시간 숫자로 얻기
				ptr = strtok( endtime_time, ":");
				//printf( "%s\n", ptr);
				hour=atoi(ptr);
				ptr = strtok( NULL, ":");
				min=atoi(ptr);
				ptr = strtok( NULL, ":");
				sec=atoi(ptr);


				//입력받은 시간으로 시간구조체 만들기
				time(&end);
				timeinfo=localtime(&end);
				timeinfo->tm_year=year-1900;
				timeinfo->tm_mon=month-1;
				timeinfo->tm_mday=day;
				timeinfo->tm_hour=hour;
				timeinfo->tm_min=min;
				timeinfo->tm_sec=sec;

				strcpy(threadname,trashname);//trash폴더에 저장될 이름 저장
				strcpy(threadpath,filepath);//trash폴더에 저장될 경로 저장


				time(&start);
				end=mktime(timeinfo);
				dif=difftime(end,start);

				threadop=1;//delete thread 생성 신호

				pthread_create(&tid,NULL,delete_thread,(void *)&dif);//thread 생성

		 }else{//시간 입력이 안 되어 있으면


				chdir(ipath);//./trash/info 로 이동
				realpath(trashname,staticpath);//info에 저장될  파일의 절대 경로 저장

				if(access(staticpath,F_OK)==0){//info에 이미 존재하면

						  i=0;

						  //CHANGE filename
						  while(1){
									//파일이름 뒤에 "(숫자)"를 붙여서 없을 때까지 숫자를 증가시켜 파일이름 정하기
									i++;
									strncpy(newname,trashname,sizeof(trashname));

									p=strrchr(staticpath,'/');
									sprintf(buff,"/%s(%d)",newname,i);
									strcpy(p,buff);

									if(access(staticpath,F_OK)!=0) break;//없으면 파일 이름 확정

									memset(buff,0,sizeof(newname));

						  }

						memcpy(newname,buff,sizeof(buff));//새로운 이름 지정

						//info디렉토리의 정보입력할 파일 만들기
						fp=fopen(staticpath,"w");
						fseek(fp,0,SEEK_SET);

						fputs("[trash info]\n",fp);
						//원래 경로 입력
						fputs(filepath,fp);
						fputs("\n",fp);

						//현재 시간 알아내 delete 시간 입력
						time(&t);
						cur = localtime(&t);
						//printf("I=%d\n",i);
						year=cur->tm_year+1900;
						month=cur->tm_mon+1;
						day=cur->tm_mday;
						hour=cur->tm_hour;
						min=cur->tm_min;
						sec=cur->tm_sec;

						fprintf(fp,"D : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);

						//해당 디렉토리 안 옮겨질 파일의 정보를 가져와 mtime 기록
						lstat(filepath,&modify);
						mtime=localtime(&modify.st_mtime);
						year=mtime->tm_year+1900;
						month=mtime->tm_mon+1;
						day=mtime->tm_mday;
						hour=mtime->tm_hour;
						min=mtime->tm_min;
						sec=mtime->tm_sec;
						fprintf(fp,"M : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);
						fclose(fp);


						//trash/files로 이동하게될 절대 경로
						memset(staticpath,0,sizeof(staticpath));
						strcpy(staticpath,fpath);
						strcat(staticpath,newname);

						//해당 디렉토리에서 ./trash/files/newname으로 이동
						rename(filepath,staticpath);




              }else{//해당 파일이 없다면

						//현재 시간 저장하기
						time(&t);
						cur = localtime(&t);

						year=cur->tm_year+1900;
						month=cur->tm_mon+1;
						day=cur->tm_mday;
						hour=cur->tm_hour;
						min=cur->tm_min;
						sec=cur->tm_sec;

						chdir(ipath);//./trash/info 디렉토리로 이동
						realpath(trashname,filepath);//./trash/info/filename 절대 경로

						//./trash/info/filename 생성
						//printf("filepath%s\n",filepath);
						fp=fopen(filepath,"w");
						fseek(fp,0,SEEK_SET);
						fputs("[trash info]\n",fp);

						//원래 경로 저장
						chdir(cpath);
						realpath(trashname,filepath);


						//현재 delete 시간 저장
						fputs(filepath,fp);
						fputs("\n",fp);
						fprintf(fp,"D : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);

						//해당 디렉토리 안의 delete 할 파일 정보 가져와서 mtime 저장
						stat(filepath,&modify);
						mtime=localtime(&modify.st_mtime);
						year=mtime->tm_year+1900;
						month=mtime->tm_mon+1;
						day=mtime->tm_mday;
						hour=mtime->tm_hour;
						min=mtime->tm_min;
						sec=mtime->tm_sec;
						fprintf(fp,"M : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);
						fclose(fp);

						//해당 디렉토리 안의 파일->./trash/files로 이동
						chdir(fpath);
						realpath(trashname,checkpath);
						rename(filepath,checkpath);


				  }



				size_check(ipath,fpath);//./trash/info 파일 안의 사이즈를 확인하다.

				return;//프롬프트로 이동
}
}
void *delete_thread(void *arg){//delete할 예약시간을 받았을 때, thread 만드는 함수
		  int delaytime;
		  FILE *fp;
		  time_t t;
		  struct tm *cur;
		  int year;
		  int month;
		  int day;
		  int hour;
		  int min;
		  int sec;
		  struct stat statbuf;
		  struct stat modify;
		  struct tm* timeinfo;
		  struct tm *mtime;
		  char newname[100]={0,};
		  char buf1[100]={0,};
		  char buff[100]={0,};

		  char *p;
		  char num[100]={0,};


		  //입력받은 인자를 delay time에 저장
		  delaytime=*((int *)arg);
		  //delay시간만큼 멈추기
		  sleep(delaytime);

		  //iop옵션이 함께 들어왔다념
		  if(i2op==1){
					remove(threadpath);//바로 파일 삭제
					i2op=0;//i2op 초기화
					threadop=0;//thread 옵션 초기화
					pthread_exit(NULL);//thread 종료
					return NULL;//끝
		  }

		  //info 파일에 해당 이름의 파일이 있는 지 확인
		  memset(staticpath,0,sizeof(staticpath));
		  strcpy(staticpath,path);
		  strcat(staticpath,"/trash/info/");
		  strcat(staticpath,threadname);

		  int i=0;
		  chdir(ipath);
		  realpath(threadname,staticpath);


		  if(access(staticpath,F_OK)==0){//이미 존재 한다면

				i=0;

				while(1){//파일이름에 이름(숫자)를 붙여 파일 있나 확인
						  i++;
						  strncpy(newname,threadname,sizeof(threadname));


						  p=strrchr(staticpath,'/');
						  sprintf(buff,"/%s(%d)",newname,i);
						  strcpy(p,buff);

						  if(access(staticpath,F_OK)!=0) break;// 없으면 이름(숫자)가 바뀔 이름

						  memset(buff,0,sizeof(newname));

				}
					//정해진 이름으로 info디렉토리에 파일 생성
				  memcpy(newname,buff,sizeof(buff));
				  fp=fopen(staticpath,"w+");
				  fseek(fp,0,SEEK_SET);
				  fputs("[trash info]\n",fp);

				  //파일에 원래 경로 입력
				  fputs(threadpath,fp);
				  fputs("\n",fp);

				  //파일에 현재 시간 입력
				  time(&t);
				  cur = localtime(&t);

				  year=cur->tm_year+1900;
				  month=cur->tm_mon+1;
				  day=cur->tm_mday;
				  hour=cur->tm_hour;
				  min=cur->tm_min;
				  sec=cur->tm_sec;

				  fprintf(fp,"D : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);

				  //원래 파일의 mtime 입력
				  lstat(threadpath,&modify);
				  mtime=localtime(&modify.st_mtime);
				  year=mtime->tm_year+1900;
				  month=mtime->tm_mon+1;
				  day=mtime->tm_mday;
				  hour=mtime->tm_hour;
				  min=mtime->tm_min;
				  sec=mtime->tm_sec;
				  fprintf(fp,"M : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);
				  fclose(fp);



				  memset(staticpath,0,sizeof(staticpath));
				  strcpy(staticpath,path);

				  strcat(staticpath,"/trash/files");
				  strcat(staticpath,newname);

				  //해당 파일을 trash 폴더로 옮기기
				  rename(threadpath,staticpath);



				}else{//같은 이름의 파일이 존재 하지 않다면
					  time(&t);
					  cur = localtime(&t);

					  year=cur->tm_year+1900;
					  month=cur->tm_mon+1;
					  day=cur->tm_mday;
					  hour=cur->tm_hour;
					  min=cur->tm_min;
					  sec=cur->tm_sec;

					  //현재 이름으로 info에 파일 만들기
					  fp=fopen(staticpath,"w");
					  fseek(fp,0,SEEK_SET);
					  fputs("[trash info]\n",fp);

					  //원래 파일의 절대 경로 저장
					  fputs(threadpath,fp);
					  fputs("\n",fp);

					  //현재 delete 시간 저장
					  fprintf(fp,"D : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);

					  //원래 파일의 mtime 저장
					  stat(threadpath,&modify);
					  mtime=localtime(&modify.st_mtime);
					  year=mtime->tm_year+1900;
					  month=mtime->tm_mon+1;
					  day=mtime->tm_mday;
					  hour=mtime->tm_hour;
					  min=mtime->tm_min;
					  sec=mtime->tm_sec;
					  fprintf(fp,"M : %04d-%02d-%02d %02d:%02d:%02d\n",year,month,day,hour,min,sec);
					  fclose(fp);


					  memset(staticpath,0,sizeof(staticpath));
					  strcpy(staticpath,path);
					  strcat(staticpath,"/trash/files/");
					  strcat(staticpath,threadname);

					  //해당 파일을 trash 폴더로 이동
					  rename(threadpath,staticpath);
				}
				size_check(ipath,fpath);//info 폴더 사이즈 2kb check

				//경로 초기화
				memset(threadpath,0,sizeof(threadpath));
				memset(threadname,0,sizeof(threadname));
				memset(staticpath,0,sizeof(staticpath));
				threadop=0;//thread옵션 해제
				pthread_exit(NULL);//thread 종료
				return NULL;//끝
}
