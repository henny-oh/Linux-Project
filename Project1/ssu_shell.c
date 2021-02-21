#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
void nonterminal(char **tokens);//단일 명령
void terminal(int pcount,char **tokens);//터미널 명령
char **tokenize(char *line)//입력 토큰 나누
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];
//띄어쓰기, 개행, 탭을 기준으로 토큰 나누기
    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0;
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
//저장된 토큰 리턴
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];
	char  **tokens;
	int i;


	int pcount;
	FILE* fp;

	if(argc == 2) {//배치식 모드로 입력받으면
		fp = fopen(argv[1],"r");
		if(fp < 0) {//argv[1]로 입력받은 파일이 존재하지 않을 때 에러처리
			printf("File doesn't exists.");
			return -1;
		}
	}
	while(1) {
    /* BEGIN: TAKING INPUT */
		pcount=0;
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
      fflush(stdin);
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
    if(strcmp(line,"\n")==0) {//아무것도 입력되지 않았을 때
      printf("input again\n");

      continue;
    }
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

       //do whatever you want with the commands, here we just print them

		for(i=0;tokens[i]!=NULL;i++){//터미널 기호가 입력에 존재할 때, 터미널 개수 확인
			if(strcmp(tokens[i],"|")==0) pcount++;
		}

    if(pcount==0) nonterminal(tokens);//터미널 기호가 없을 때
    else terminal(pcount,tokens);//터미널 기호가 존재할 때


	// Freeing the allocated memory
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
void terminal(int pcount,char **tokens){//터미널 기호가 존재하는 입력의 경우
  char path[100];
  int pid[pcount+1];
  char **pipetokens=malloc((pcount+1)*sizeof(char*));
  char route[MAX_TOKEN_SIZE+5];
  int j;
  int pipearr[pcount+2];
  int tokencount=0;
  int length=0;
  int pipefd[2];
  int temp=0;
  int status;
  pipearr[0]=-1;
  j=1;

  for(int i=0;tokens[i]!=NULL;i++){//토큰 개수와 터미널 기호 위치 확인
      if(strcmp(tokens[i],"|")==0) {
        pipearr[j]=i;
        j++;
      }
      tokencount++;//토큰 개수
  }

  pipearr[j]=tokencount;//입력의 끝을 표시
  //pipearr에 표시된 인덱스로 토큰을 나눈다.
  j=0;



  for(int i=0;i<pcount+1;i++){//pipearr에 저장된 인덱스를 기준으로 명령어들을 나눈다.
    length=0;
    for(int i=0;i<pipearr[j+1]-pipearr[j];i++)//명령어 저장 공간 할당.
        pipetokens[i] = malloc(MAX_TOKEN_SIZE*sizeof(char));
        //pipetokens에 나눈 명령어를 차례로 저장한다.
    for(int input=pipearr[j]+1;input<pipearr[j+1];input++){

      strcpy(pipetokens[length],tokens[input]);

      length++;
    }


    pipetokens[length]=NULL;//마지막 토큰 표시


    if(pipe(pipefd)<0){//파이프 생성
        return;
    }

    pid[i]=fork();//자식 프로세스 생성
    if(pid[i]<0){//fork 에러 발생시

            fprintf(stderr,"error\n");
            exit(1);
    }
    else if(pid[i]==0){//자식 프로세스의 경우

          /*  for(int q=0;pipetokens[q]!=NULL;q++){
              printf("%s %d\n",pipetokens[q],q);

            }*/
            //child

              dup2(temp,0);//input에 그 전 프로세스의 output 넣기
              if(i!=pcount) dup2(pipefd[1],1);//마지막 프로세스가 아니면 output을 다음 프로세스로 보내기




              close(pipefd[0]);//사용하지 않는 터미널 닫기
              //close(pipefd[1]);
              if(!strcmp(pipetokens[0],"pps")){//명령어 pps가 입력되면
                realpath(pipetokens[0],path);//pps 실행 파일의 절대 경로 생성
                //printf("%s\n",path);

                if(execvp(path,pipetokens)==-1){//exec함수 실행하여 명령 수행
                  printf("SSUShell : Incorrect command\n");
                  exit(1);
                }
              }
              else if(!strcmp(pipetokens[0],"ttop")){//명령어 ttop가 입력되면
                realpath(pipetokens[0],path);//ttop 실행 파일의 절대 경로 생성
                //printf("%s\n",path);

                if(execvp(path,pipetokens)==-1){//exec함수 실행하여 명령 수행
                  printf("SSUShell : Incorrect command\n");
                  exit(1);
                }
              } //그 외 명령어는 그대로 수행
              if (execvp(pipetokens[0],pipetokens)==-1){//exec함수 실행하여 명령 수행
              printf("SSUShell : Incorrect command\n");
              exit(1);
            }

            exit(1);
            //printf("done\n");



    }
    else{//부모 프로세스의 경우
          wait(NULL);//자식 프로세스 종료 시 까지 기다리기

            if(!WIFEXITED(status)){//정상 종료확인
              //printf("not normal\n");
            }
            close(pipefd[1]);//사용하지 않는 파이프 닫기
            temp=pipefd[0];//output을 temp에 저장하기

          //close(pipefd[0]);
    }
//메모리 free
    for(int q=0;q<pipearr[j+1]-pipearr[j]-1;q++){
      free(pipetokens[q]);
      //printf("%d\n",q);
    }
    j++;
    //printf("found\n");
  }
    free(pipetokens);
}



void nonterminal(char **tokens){//단일 명령어 수행
  int pid;
  int status;
  char path[100];

	pid=fork();
  if(pid<0){

          fprintf(stderr,"error\n");
  }
  else if(pid==0){
    //printf("%s\n",tokens[0]);
    if(!strcmp(tokens[0],"pps")){//명령어 pps가 입력되면
      realpath(tokens[0],path);//pps 실행 파일의 절대 경로 생성
      //printf("%s\n",path);

      if(execvp(path,tokens)==-1){//exec함수 실행하여 명령 수행
        printf("SSUShell : Incorrect command\n");
        exit(1);
      }
    }
    else if(!strcmp(tokens[0],"ttop")){//명령어 ttop가 입력되면
      realpath(tokens[0],path);//ttop 실행 파일의 절대 경로 생성
      //printf("%s\n",path);

      if(execvp(path,tokens)==-1){//exec함수 실행하여 명령 수행
        printf("SSUShell : Incorrect command\n");
        exit(1);
      }
    }
          if(execvp(tokens[0],tokens)==-1){//그 외 명령어는 그대로 수행
            printf("SSUShell : Incorrect command\n");
            exit(1);
          }
  }
  else{
    wait(NULL);//자식 프로세스 종료 시 까지 기다리기
    if(WIFEXITED(status)){//정상 종료확인
      //printf("normal terminated\n");
    }

  }

}
