#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static int  indent = 0;//tree depth
char cpath[100];//check(지정 디렉토리) 절대 경로


char path[100];
char filename[100];

char cdir[256];

void tree_Scandir(char *treepath, int depth);//파일 지정디렉토리(check)안의 파일들을 재귀적으로 호출, 출력하는 함수

void size_check(char ipath[],char fpath[]){
		  int num;
		  FILE *fp;
		  char buff[100]={0,};
		  char repath[100]={0,};
		  char dtime[100]={0,};
		  char depath[100]={0,};

		  struct  dirent **namelist;

		  char filename[100]={0,};
		  int j=0;
		  int size=0;


		  int fyear=9999;
		  int fmonth=99;
		  int fday=99;
		  int fh=99;
		  int fm=99;
		  int fs=99;

		  int dyear=9999;
		  int dmonth=99;
		  int dday=99;
		  int dh=99;
		  int dm=99;
		  int ds=99;

		  char deleten[100]={0,};
		  char dpath[100]={0,};
		  char dipath[100]={0,};
      char *result;
		  time_t tmdl;
		  time_t tmcom;
		  struct tm delete;
		  struct tm compare;

		  struct stat check;

		//초기 시간 설정
		  delete.tm_year=9999;
		  delete.tm_mon=0;
		  delete.tm_mday=0;
		  delete.tm_hour=0;
		  delete.tm_min=0;
		  delete.tm_sec=0;
		  tmdl=mktime(&delete);



		  chdir(ipath);

		  num=scandir(ipath, &namelist, NULL, alphasort);//파일이름들 가져오기


		  for(int i=0;i<num;i++){//파일 모두 검사
					if ( (!strcmp(namelist[i]->d_name, ".")) || (!strcmp(namelist[i]->d_name, "..")) )//파일이름이 현재 디렉토리이거나 이전 디렉토리이면
						continue;// 넘어가기

					stat(namelist[i]->d_name, &check);//파일 정보 가져오기

					size+=check.st_size;//파일 크기 합하기

					//해당 이름 폴더 열어 delete time 가져오기
					if((fp=fopen(namelist[i]->d_name, "r+"))<0) printf("fail\n");

					fgets(buff, 100, fp);
					fgets(repath, 100, fp);
					fgets(dtime, 100, fp);
					fgets(buff,100, fp);

					fclose(fp);


					//문자열 숫자만 남기기
					while(dtime[j]!='\0'){
							if('0'<=dtime[j]&&dtime[j]<='9'){
									j++;
									continue;
							}
							dtime[j]=' ';
							j++;
						}
						j=0;

					//year
					result=strtok(dtime," ");
					fyear=atoi(result);
					//month
					result=strtok(NULL," ");
					fmonth=atoi(result);
					//day
					result=strtok(NULL," ");
					fday=atoi(result);
					//hour
					result=strtok(NULL," ");
					fh=atoi(result);
					//minute
					result=strtok(NULL," ");
					fm=atoi(result);
					//sec
					result=strtok(NULL," ");
					fs=atoi(result);

					//위 정보들로 시간 구조체 만들기
					compare.tm_year=fyear;
					compare.tm_mon=fmonth;
					compare.tm_mday=fday;
					compare.tm_hour=fh;
					compare.tm_min=fm;
					compare.tm_sec=fs;
					tmcom=mktime(&compare);

					//저장되 있던 시간보다 더 빠르다면 갱신하고 그 파일이름 저장
					if((difftime(tmcom,tmdl))<0){

							  tmdl=mktime(&compare);
							  memset(deleten,0,sizeof(deleten));
							  strcpy(deleten,namelist[i]->d_name);
					}
				  }
				// 삭제할 파일들의 절대경로 찾기
				  strcpy(dpath,fpath);
				  strcat(dpath,"/");
				  strcat(dpath,deleten);
				  strcpy(dipath,ipath);
				  strcat(dipath,"/");
				  strcat(dipath,deleten);

				   for(int i=0;i<num;i++)//namelists  메모리 해제
							free(namelist[i]);
					free(namelist);//namelists  메모리 해제


				if(size<2000) {//size가 2000보다 작으면 함수종료
						return;
				}else{//2000보다 크다면
						//오래된 파일을 info와 files 폴더에서 삭제
						remove(dpath);
						remove(dipath);
						size_check(ipath,fpath);//한번더 size_check;

			  }
}


void help(void){//매뉴얼 출력 함수
  printf(" --------------------------MANUAL--------------------------S\n");
  printf("1.DELETE [FILENAME][END_TIME][OPTION]\n");//DELETE 명령어 입력 양식
  printf("[DELETE OPTION]-'-i'-파일을 trash 폴더로 옮기지 않고 바로 삭제\n");//i option
  printf("[DELETE OPTION]-'-r'-지정된 시간에 삭제 여부 재확인\n");//r option
  printf("\n");
  printf("2.SIZE [FILENAME][OPTION]\n");//SIZE 명령어 입력 양식
  printf("[SIZE OPTION]-'-d [NUMBER]'-NUMBER 깊이 까지 파일 크기 출력\n");//d option
  printf("\n");
  printf("2.RECOVER [FILENAME][OPTION]\n");//RECOVER 명령어 입력 양식
  printf("[RECOVER OPTION]-'-l'-파일 삭제 시간 오래된 것 순서대로 출력 후 복구\n");//l option
  printf("\n");
  printf("4.TREE\n");//TREE 명령어 설명
  printf("-트리 구조로 check 디렉토리 안 구조 보기\n");
  printf("\n");
  printf("5.EXIT\n");//EXIT 명령어 설명
  printf("-프롬프트 프로그램 종료\n");
  printf("\n");
  printf("6.HELP\n");//HELP 명령어 설명
  printf("-MANUAL 보기\n");
  printf("명령어는 소문자로 입력하세요\n");


}
void tree(void)//지정디렉토리(check)안의 ree 구조 보여주는 tree 명령어
{
    realpath("check",cpath);//지정디렉토리(check) 절대경로 /
    printf("%s___","check");//연결선
    tree_Scandir(cpath, 0);//파일 지정디렉토리(check)안의 파일들을 재귀적으로 호출하여 출력
    printf("\n");//계행
    indent=0;//깊이 0으로 초기화
    return ;//프롬프트로 돌아가기
}


void tree_Scandir(char *treepath, int depth)//파일 지정디렉토리(check)안의 파일들을 재귀적으로 호출, 출력하는 함수
{
			struct dirent **namelists;//파일 이름을 저장하는 list
			int count;
			struct stat checking;//파일 정보 가져오는 stat구조체


			if (chdir(treepath) < 0)//현재 작업 디렉토리를 탐색할 경로로 변경
			{
				printf("%s error\n", treepath);//실패하면 chdir 에러 메시지 출력
				return;//함수 나가기
			}
			count = scandir(".", &namelists, NULL, alphasort);//현재 작업 디렉토리의 파일 개수 구하기


			 for (int i = 0; i < count; i++)//현재 작업 디렉토리 안 모든 파일 검사
			{
					if ( !strcmp(namelists[i]->d_name, ".")) //파일이름이 현재 디렉토리이면
							     continue;//넘어가기
					if(!strcmp(namelists[i]->d_name, "..") )//파일이름이 이전 디렉토리이면
							     continue;//넘어가기

					printf("\n");//계행
					for(int j=0;j<indent+1;j++)
					       printf("\t");//깊이만큼 탭해서 파일 이름 간 거리 띄우기

					printf("|_____");//연결선

					printf("%s",namelists[i]->d_name);//파일 이름 출력
					if(i==(count-1))
                printf("\n");//디렉토리 안 마지막 파일이면 계행

					lstat(namelists[i]->d_name, &checking);//파일 정보 가져오기

					if ((checking.st_mode & S_IFDIR) == S_IFDIR)//파일이 디렉토리이면
					{
							if (indent < (depth-1) )//깊이만큼 안의 디렉토리 검사
							{
									indent ++;//깊이 +1
									tree_Scandir(namelists[i]->d_name, depth);//해당 디렉토리 안의 파일들을 재귀적으로 호출, 출력
							}
							if (depth == 0)//0일 경우는 상관없이 검사
									{
									indent ++;//깊이 +1
									tree_Scandir(namelists[i]->d_name, depth);//해당 디렉토리 안의 파일들을 재귀적으로 호출, 출력
							}
					}
			 }

          for(int i=0;i<count;i++)//namelists  메모리 해제
                  free(namelists[i]);
          free(namelists);//namelists  메모리 해제

			    indent --;//깊이 -1
    			chdir("..");//이전 디렉토리로 돌아가기
}
